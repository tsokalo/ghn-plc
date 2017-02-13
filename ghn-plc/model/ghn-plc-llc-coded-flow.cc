/*
 * GhnPlcLlcCodedFlow.cc
 *
 *  Created on: Jul 26, 2016
 *      Author: tsokalo
 */
#include "ns3/log.h"
#include "ns3/ghn-plc-lpdu-header.h"

#include "ghn-plc-llc-coded-flow.h"
#include "header-value.h"

NS_LOG_COMPONENT_DEFINE("GhnPlcLlcCodedFlow");

namespace ns3
{
namespace ghn
{

using namespace ncr;

typedef uint8_t local_msg_t;
typedef uint16_t local_size_t;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NS_OBJECT_ENSURE_REGISTERED(GhnPlcLlcCodedFlow);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TypeId
GhnPlcLlcCodedFlow::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::GhnPlcLlcCodedFlow").SetParent<GhnPlcLlcFlow> ().AddConstructor<GhnPlcLlcCodedFlow> ();
  return tid;
}

GhnPlcLlcCodedFlow::GhnPlcLlcCodedFlow ()
{
  m_rxSegmenter = NULL;
  m_txSegmenter = NULL;
}
GhnPlcLlcCodedFlow::~GhnPlcLlcCodedFlow ()
{
  DELETE_PTR(m_rxSegmenter);
  DELETE_PTR(m_txSegmenter);
}
void
GhnPlcLlcCodedFlow::Configure (NodeType type, ncr::UanAddress dst, SimParameters sp, GenCallback cb)
{
  m_genCallback = cb;
  m_sp = sp;
  m_nodeType = type;
  m_id = m_dllMac->GetDllManagement ()->GetAddress ().GetAsInt ();
  //
  // NC uses LPDU without CRC as the payload
  //
  auto coding_header_size = coder_overhead::get (m_sp.genSize);
  GhnPlcLpduHeader header;
  m_rxSegmenter = new GhnPlcSegmenter (m_blockSize - header.GetSerializedSize () - GHN_CRC_LENGTH - coding_header_size);
  m_txSegmenter = new GhnPlcSegmenter (m_blockSize - header.GetSerializedSize () - GHN_CRC_LENGTH - coding_header_size);
  m_sp.symbolSize = m_blockSize - GHN_CRC_LENGTH - coding_header_size;
  m_sp.numGen = (m_nodeType == SOURCE_NODE_TYPE) ? 2 * m_sp.numGen : m_sp.numGen;

  m_brr = routing_rules_ptr (new NcRoutingRules (m_id, m_nodeType, dst, m_sp));
  m_brr->SetLogCallback (m_addLog);
  //
  // initialize the feedback
  //
  m_feedback = m_brr->GetFeedbackInfo ();

  if (m_nodeType == SOURCE_NODE_TYPE)
    {
      m_encQueue = encoder_queue_ptr (new encoder_queue (m_sp.numGen, m_sp.genSize, m_sp.symbolSize));
      m_encQueue->set_notify_callback (std::bind (&GhnPlcLlcCodedFlow::NotifyRcvUp, this, std::placeholders::_1));
      m_getRank = std::bind (&encoder_queue::rank, m_encQueue, std::placeholders::_1);
      m_brr->SetGetRankCallback (m_getRank);
      m_brr->SetCoderHelpInfoCallback (
              std::bind (&encoder_queue::get_help_info, m_encQueue, std::placeholders::_1, std::placeholders::_2,
                      std::placeholders::_3));

      SIM_LOG(COMM_NODE_LOG, "Node " << m_id << " type " << m_nodeType);
    }
  else
    {
      m_decQueue = decoder_queue_ptr (new decoder_queue (m_sp.numGen, m_sp.genSize, m_sp.symbolSize));
      m_getRank = std::bind (&decoder_queue::rank, m_decQueue, std::placeholders::_1);
      m_brr->SetGetRankCallback (m_getRank);
      m_brr->SetGetCodingMatrixCallback (std::bind (&decoder_queue::get_coding_matrix, m_decQueue, std::placeholders::_1));
      m_brr->SetGetCoderInfoCallback (std::bind (&decoder_queue::get_coder_info, m_decQueue, std::placeholders::_1));
      m_brr->SetCoderHelpInfoCallback (
              std::bind (&decoder_queue::get_help_info, m_decQueue, std::placeholders::_1, std::placeholders::_2,
                      std::placeholders::_3));

      SIM_LOG(COMM_NODE_LOG, "Node " << m_id << " type " << m_nodeType);
    }
}

void
GhnPlcLlcCodedFlow::NotifyRcvUp (GenId genId)
{
  m_brr->UpdateRcvd (genId, m_id);
}

SendTuple
GhnPlcLlcCodedFlow::SendDown ()
{
  NS_LOG_FUNCTION(this << m_connId);

  if (m_connId.dst == UanAddress::GetBroadcast ()) return GhnPlcLlcFlow::SendDown ();

  auto dll = m_dllMac->GetDllManagement ();
  auto src = dll->GetAddress ().GetAsInt ();
  GhnBuffer toTransmit;
  ConnId connId = m_connId;

  if (m_nodeType != DESTINATION_NODE_TYPE)
    {
      auto dst = connId.dst.GetAsInt ();
      auto phy = dll->GetPhyManagement ()->GetPhyPcs ()->GetObject<GhnPlcPhyPcs> ();
      auto rt = dll->GetRoutingTable ();
      auto bt = dll->GetBitLoadingTable ();
      auto nh = (m_sp.mutualPhyLlcCoding) ? m_brr->GetSinkVertex () : rt->GetNextHopAddress (src, dst).GetAsInt ();
      if (nh == -1) nh = rt->GetNextHopAddress (src, dst).GetAsInt ();
      m_brr->SetSendingRate (bt->GetNumEffBits (src, nh));
      auto dataAmount = bt->GetDataAmount (Seconds (GHN_CYCLE_MAX), src, nh);
      uint32_t pushedPkts = 0, maxPkts = floor ((double) dataAmount / (double) m_blockSize);
      SIM_LOG(COMM_NODE_LOG,
              "Node " << m_id << ", Flow " << connId << ": " << "dataAmount: " << dataAmount << ", m_blockSize: " << m_blockSize);
      assert(maxPkts > 0);

      PrepareForSend (dataAmount);

      if (m_nodeType == SOURCE_NODE_TYPE)
        {
          auto maxBuf = m_brr->GetMaxAmountTxData ();
          assert(maxBuf >= maxPkts);
          assert(!m_genCallback.IsNull ());
          //
          // generate minimum required for buffering by ANChOR
          //
          if (m_brr->NeedGen ())
            {
              m_genCallback (m_brr->GenNumGreedyGen () * m_sp.symbolSize);
              PrepareForSend (dataAmount);
            }
          //
          // generate additional packets if the room permits
          //
          auto fspace = m_brr->GetAmountTxData ();
          while (fspace < maxPkts + m_sp.genSize)
            {
              SIM_LOG(COMM_NODE_LOG,
                      "Flow " << connId << ": generate more data, busy space: " << fspace << ", max pkts: " << maxPkts << ", frame buffer size: " << m_frameBuffer.size());
              SIM_LOG(COMM_NODE_LOG,
                      "Flow " << connId << ": amount (bytes) " << (maxPkts + m_sp.genSize - fspace) * m_sp.symbolSize);
              m_genCallback ((maxPkts + m_sp.genSize - fspace) * m_sp.symbolSize);
              PrepareForSend (dataAmount);
              auto fspace_prev = fspace;
              fspace = m_brr->GetAmountTxData ();
            }
          SIM_LOG(1,
                  "Flow " << connId << ": busy space: " << fspace << ", max pkts: " << maxPkts << ", frame buffer size: " << m_frameBuffer.size());
        }

      assert(m_nodeType != DESTINATION_NODE_TYPE);
      TxPlan plan = m_brr->GetTxPlan ();
      TxPlan planI;

      if (plan.empty ())
        {
          SIM_LOG(COMM_NODE_LOG, "Flow " << connId << ": " << "Sending the message with feedback only");
        }

      auto p_it = plan.begin_orig_order ();
      while (p_it != plan.end ())
        {
          GenId genId = p_it->first;
          auto n = p_it->second.num_all;
          uint16_t i = 0;
          for (; i < n;)
            {
              auto contents = (m_nodeType == SOURCE_NODE_TYPE) ? m_encQueue->get_coded (genId) : m_decQueue->get_coded (genId);
              //          header_value<local_msg_type>::append(contents, (local_msg_type)DATA_MSG_TYPE);
              auto pkt = Create<Packet> ((uint8_t const*) contents.data (), contents.size ());
              toTransmit.push_back (pkt);
              pushedPkts++;
              i++;
              if (pushedPkts == maxPkts) break;
            }
          m_brr->UpdateSent (genId, i);
          planI[genId].num_all = i;
          planI[genId].all_prev_acked = p_it->second.all_prev_acked;
          if (pushedPkts == maxPkts) break;
          p_it = plan.next_orig_order (p_it);
        }

      if (m_nodeType == SOURCE_NODE_TYPE)
        {
          if (pushedPkts != maxPkts)
            {
              SIM_LOG(COMM_NODE_LOG, "Node " << m_id << " pushed pkts: " << pushedPkts << ", max pkts: " << maxPkts);
            }
        }

      auto hBuf = ConvertBrrHeaderToPkt (planI);
      toTransmit.insert (toTransmit.begin (), hBuf.begin (), hBuf.end ());
    }
  else
    {
      SIM_LOG(COMM_NODE_LOG, "Flow " << connId << ": " << "I am the destination. I can send the feedback only");
      auto hBuf = ConvertBrrHeaderToPkt (TxPlan ());
      toTransmit.insert (toTransmit.begin (), hBuf.begin (), hBuf.end ());
    }

  NS_LOG_DEBUG("Flow " << connId << ": " << "Segments number to be transmitted: " << toTransmit.size());

  //
  // add CRC
  //
  GhnBuffer::iterator it = toTransmit.begin ();
  while (it != toTransmit.end ())
    {
      (*it)->AddPaddingAtEnd (GHN_CRC_LENGTH);
      it++;
    }

  SIM_LOG(COMM_NODE_LOG, "Node " << m_id << " Tx buffer size: " << toTransmit.size());

  return SendTuple (toTransmit, connId);
}

void
GhnPlcLlcCodedFlow::PrepareForSend (uint64_t dataAmount)
{
  if (!m_frameBuffer.empty ())
    {
      assert(m_nodeType == SOURCE_NODE_TYPE);

      auto freeBufSize = m_brr->GetFreeBufferSpace ();

      NS_LOG_DEBUG(
              "Flow " << m_connId << ": " << "BEFORE INDEXING: Free buffer size: " << freeBufSize << ", number of non-indexed segments: " << m_nonindexedSegs.size() << ", not-segmented data: " << m_frameBuffer.size() << ", dataAmount: " << dataAmount);

      //
      // create non-indexed segments
      //
      uint64_t dataLimit = (dataAmount > (uint64_t) freeBufSize * m_blockSize) ? freeBufSize * m_blockSize : dataAmount;
      if ((m_nonindexedSegs.size ()) * m_blockSize < dataLimit && (!m_frameBuffer.empty ()))
        {
          SegGhnBuffer addNonIndexed = m_txSegmenter->SegmentData (m_frameBuffer);
          m_nonindexedSegs.insert (m_nonindexedSegs.end (), addNonIndexed.begin (), addNonIndexed.end ());
          m_frameBuffer.clear ();
        }

      //
      // index segments
      //
      std::deque<Ssn> newSentSegs;
      while (freeBufSize != 0 && (!m_nonindexedSegs.empty ()))
        {
          GhnPlcLpduHeader header;
          header.SetLfbo ((*m_nonindexedSegs.begin ()).posLlcFrame);
          header.SetMqf ((*m_nonindexedSegs.begin ()).validSeg);
          //
          // TODO set SSN locally
          //
          header.SetSsn (m_ssn.val ());
          m_ssn++;
          NS_LOG_DEBUG("Flow " << m_connId << ": " << "Indexing the segment by SSN: " << header.GetSsn());
          newSentSegs.push_back (header.GetSsn ());
          (*m_nonindexedSegs.begin ()).pkt->AddHeader (header);
          m_encQueue->enque (ConvertPacketToVec ((*m_nonindexedSegs.begin ()).pkt));
          m_nonindexedSegs.pop_front ();
          freeBufSize--;
        }

      NS_LOG_DEBUG(
              "Flow " << m_connId << ": " << "AFTER INDEXING: Free buffer size: " << freeBufSize << ", number of non-indexed segments: " << m_nonindexedSegs.size() << ", number of indexed segments: " << m_indexedSegs.size() << ", not-segmented data: " << m_frameBuffer.size());
    }
}

GhnBuffer
GhnPlcLlcCodedFlow::ConvertBrrHeaderToPkt (TxPlan plan)
{
  auto str = m_brr->GetHeader (plan, m_feedback).Serialize ();
  m_feedback.Reset ();
  auto bs = m_blockSize - GHN_CRC_LENGTH;
  auto pkt_size = str.size () + 1;
  uint8_t n_bs = ceil ((double) pkt_size / (double) bs);

  auto pkt = Create<Packet> ((const uint8_t*) (&n_bs), 1);
  pkt->AddAtEnd (Create<Packet> ((const uint8_t*) str.c_str (), str.size ()));

  auto padding_length = n_bs * bs - pkt->GetSize ();
  NS_LOG_UNCOND("Adding " << (uint16_t)n_bs << " BRR header blocks");


  if (padding_length != 0) pkt->AddPaddingAtEnd (padding_length);

  GhnBuffer buffer;

  while (pkt->GetSize () != 0)
    {
      NS_ASSERT(pkt->GetSize () >= bs);
      buffer.push_back ((pkt->GetSize () == bs) ? pkt : pkt->CreateFragment (0, bs));
      if (pkt->GetSize () == bs) break;
      pkt->RemoveAtStart (bs);
    }
  return buffer;
}
BrrHeader
GhnPlcLlcCodedFlow::ConvertPktToBrrHeader (GhnBuffer &buffer, std::deque<SegmentState> &state)
{
  //
  // convert buffer to packet
  //
  assert(!buffer.empty ());
  auto pkt = *buffer.begin ();

  uint8_t n_bs = 0;
  pkt->CopyData (&n_bs, 1);
  assert(n_bs != 0);
  pkt->RemoveAtStart (1);

  for (uint16_t i = 1; i < n_bs; i++)
    pkt->AddAtEnd (*(buffer.begin () + i));

  buffer.erase (buffer.begin (), buffer.begin () + n_bs);

  //
  // determine the data consistency: TODO
  //
  state.erase (state.begin (), state.begin () + n_bs);

  //
  // convert packet to header
  //
  uint8_t * v = new uint8_t[pkt->GetSize () + 1];
  pkt->CopyData (v, pkt->GetSize ());
  BrrHeader h;
  h.Deserialize (std::string ((const char*) v, pkt->GetSize ()));
  delete[] v;
  return h;
}
GroupEncAckInfo
GhnPlcLlcCodedFlow::Receive (GhnBuffer buffer, ConnId connId)
{
  NS_LOG_FUNCTION(this << connId);

  NS_ASSERT(buffer.size () > 0);
  NS_ASSERT_MSG(m_connId == connId, m_connId << " " << connId);

  //
  // remove and check CRC
  //
  std::deque<SegmentState> state = CheckCrc (buffer);
  assert(state.size () == buffer.size ());

  //
  // if CRC for the header segment fails, no data can be processed
  //
  if (*(state.begin ()) != DONE_SEGMENT_STATE) return GroupEncAckInfo ();

  //
  // process header
  //
  auto header = ConvertPktToBrrHeader (buffer, state);

  m_brr->RcvHeaderInfo (header.h);
  ProcessFeedback (header.f);

  //
  // process data packets
  //
  SIM_LOG(COMM_NODE_LOG, "Node " << m_id << " Rx buffer size: " << buffer.size() << ", Tx plan: " << header.h.txPlan);
  auto tx_it = header.h.txPlan.begin_orig_order ();
  auto it = state.begin ();

  for (auto pkt : buffer)
    {
      if (tx_it->second.num_all == 0)
        {
          tx_it = header.h.txPlan.next_orig_order (tx_it);
          assert(tx_it != header.h.txPlan.end ());
        }
      assert(tx_it->second.num_all != 0);
      tx_it->second.num_all--;

      auto vec = ConvertPacketToVec (pkt);
      ProcessRcvdPacket (vec, *it == DONE_SEGMENT_STATE, header.h.addr, tx_it, connId);
      it++;

    }

  return GroupEncAckInfo ();
}

void
GhnPlcLlcCodedFlow::ReceiveAck (GroupEncAckInfo info, ConnId connId)
{
  SIM_LOG_FUNC(COMM_NODE_LOG);
  assert(0);
}
bool
GhnPlcLlcCodedFlow::IsQueueEmpty ()
{
  SIM_LOG_FUNC(COMM_NODE_LOG);

  if (!m_frameBuffer.empty ()) return false;

  if (m_feedback.updated) return false;

  if (!m_brr->MaySendData ()) return true;

  return (m_brr->GetAmountTxData () == 0);
}
void
GhnPlcLlcCodedFlow::SetLogCallback (add_log_func addLog)
{
  m_addLog = addLog;
}

void
GhnPlcLlcCodedFlow::ProcessDecoded (GhnBuffer buffer, ConnId connId)
{
  std::deque<SegmentState> state (buffer.size (), DONE_SEGMENT_STATE);
  //
  // remove LPDU header
  //
  SegGhnBuffer segmentBuffer;
  std::deque<Ssn> ssns = RemoveLpduHeaders (buffer, state, segmentBuffer);

  //
  // add/update segments to/in the receive buffer
  //
  UpdateRcvdSegments (segmentBuffer);

//  //
//  // mark the received segments by ARQ
//  //
//  m_rxArq->MarkRcvSegs (ssns, state);
//  //
//  // get SSNs of all continuously correctly received segments, which are still present in the confirmed window
//  //
//  ssns = m_rxArq->GetAckSsns ();

  //
  // De-segment the segments with such ssns
  //
  if (!ssns.empty ())
    {
      segmentBuffer.clear ();
      NS_LOG_DEBUG(
              "Flow " << m_connId << ": " << "Number of continuously acknowledged segments: " << ssns.size() << ", first: " << (*ssns.begin ()) << ", last: " << ssns.at(ssns.size() - 1));
      //
      // add segments which were acknowledged earlier but were de-segmented neither partially nor completely
      //
      AddNotDesegmented (ssns, segmentBuffer);

      //
      // add segments which are acknowledged now
      //
      AddJustAcknowledged (ssns, segmentBuffer);

      //
      // create LLC frames from the received data
      //
      buffer = ConvertSegsToLlcFrames (segmentBuffer);

      if (!buffer.empty ())
        {
          //
          // get last segment(s) from the end of the buffer, which contain only piece(s) of the last LLC frame,
          // and push them back to the receive buffer
          //
          SaveNotFullyDesegmented (segmentBuffer);

          //
          // remove LLC frame header and send the packets to the top
          //
          ProcessDeseqmented (buffer, connId);
        }
    }
  else
    {
      NS_LOG_DEBUG("Flow " << m_connId << ": " << "Number of continuously acknowledged segments: " << ssns.size());
    }

//  GroupEncAckInfo info;
//  m_rxArq->GetAck (info);
  if (m_connId.dst == UanAddress::GetBroadcast ())
    {
      //
      // we expect no acknowledgment for broadcast packets
      // therefore we reset the Tx ARQ like if we would receive the complete ACK
      //
      NS_LOG_DEBUG("Flow " << m_connId << ": " << "Reseting Rx ARQ");
      m_rxArq->Reset ();
      m_nonindexedSegs.clear ();
      m_indexedSegs.clear ();
    }

}

void
GhnPlcLlcCodedFlow::ProcessRcvdPacket (std::vector<uint8_t> vec, bool crc, ncr::UanAddress addr, TxPlan::iterator item,
        ConnId connId)
{
  GenId genId = item->first;

  if (!crc)
    {
      SIM_LOG(COMM_NODE_LOG, "Node " << m_id << " loosing segment");
      m_brr->UpdateLoss (genId, addr);
      return;
    }

  if (m_nodeType == SOURCE_NODE_TYPE) return;

  SIM_LOG(COMM_NODE_LOG, "Node " << m_id << " receives packet from generation " << genId);

  auto rank_b = m_decQueue->rank (genId);
  m_decQueue->enque (vec, genId);
  auto rank_a = m_decQueue->rank (genId);
  if (rank_b < rank_a)
    {
      auto pkts = m_decQueue->get_uncoded ();
      m_brr->UpdateRcvd (genId, addr, pkts);
      ProcessDecoded (ConvertVecsToBuffer (pkts), connId);
    }
  else
    {
      m_brr->UpdateRcvd (genId, addr, true);
    }
  //
  // --->
  //
  if (m_brr->MaySendNetDiscovery ())
    {
      SIM_LOG(COMM_NODE_LOG, "Node " << m_id << " sends the network discovery message with maximum TTL");

      m_feedback = m_brr->GetNetDiscoveryInfo ();

    }
  else
    {
      SIM_LOG(COMM_NODE_LOG, "Node " << m_id << " refuses to send the network discovery message");

      //
      // --->
      //
      if (m_brr->MaySendRetransRequest (m_decQueue->get_ranks (), addr, genId, item->second.all_prev_acked))
        {
          SIM_LOG(COMM_NODE_LOG, "Node " << m_id << " sends the retransmission request");

          m_feedback = m_brr->GetRetransRequestInfo ();

        }
      else
        {
          SIM_LOG(COMM_NODE_LOG, "Node " << m_id << " refuses to send the retransmission request");

          //
          // --->
          //
          if (m_brr->MaySendFeedback ())
            {
              SIM_LOG(COMM_NODE_LOG, "Node " << m_id << " sends the feedback");

              m_feedback = m_brr->GetFeedbackInfo ();

            }
          else
            {
              SIM_LOG(COMM_NODE_LOG, "Node " << m_id << " refuses to send the feedback");
            }
        }
    }
}

void
GhnPlcLlcCodedFlow::ProcessFeedback (FeedbackInfo f)
{
  SIM_LOG(COMM_NODE_LOG || TEMP_LOG, "Node " << m_id << " receive feedback symbol");
  m_brr->RcvFeedbackInfo (f);

  if (f.ttl != 0)
    {
      if (f.netDiscovery)
        {
          ProcessNetDiscovery (f);
        }
      else
        {
          if (m_brr->HasRetransRequest (f)) ProcessRetransRequest (f);
        }
    }
  else
    {
      SIM_LOG(COMM_NODE_LOG, "Node " << m_id << " TTL has expired");
    }

}

void
GhnPlcLlcCodedFlow::ProcessNetDiscovery (FeedbackInfo f)
{
  SIM_LOG(COMM_NODE_LOG, "Node " << m_id << " receive the network discovery message with TTL " << f.ttl);

  if (m_brr->MaySendNetDiscovery (f.ttl))
    {
      SIM_LOG(COMM_NODE_LOG, "Node " << m_id << " sends the network discovery message with TTL " << f.ttl - 1);
      m_feedback = m_brr->GetNetDiscoveryInfo (f.ttl - 1);
    }
  else
    {
      SIM_LOG(COMM_NODE_LOG, "Node " << m_id << " refuses to send the network discovery message");
    }
}
void
GhnPlcLlcCodedFlow::ProcessRetransRequest (FeedbackInfo f)
{
  SIM_LOG(COMM_NODE_LOG, "Node " << m_id << " processing retransission request");

  if (m_brr->ProcessRetransRequest (f))
    {
      if (m_nodeType != SOURCE_NODE_TYPE)
        {
          SIM_LOG(COMM_NODE_LOG, "Node " << m_id << " forward retransmission request. TTL " << f.ttl);
          m_feedback = m_brr->GetRetransRequestInfo (f.ttl - 1);
        }
      else
        {
          SIM_LOG(COMM_NODE_LOG, "Node " << m_id << ". The source does not forward retransmission requests");
        }
    }
  else
    {
      SIM_LOG(COMM_NODE_LOG, "Node " << m_id << ". Retransmission request is either not set or I should not forward it");
    }

}

}
}
