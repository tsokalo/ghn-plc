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

//using namespace ncr;

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
  NS_LOG_UNCOND("Creating coded G.hn LLC flow");
}
GhnPlcLlcCodedFlow::~GhnPlcLlcCodedFlow ()
{

}
void
GhnPlcLlcCodedFlow::Configure (ncr::NodeType type, ncr::UanAddress dst, ncr::SimParameters sp)
{
  m_sp = sp;
  m_nodeType = type;
  m_id = m_dllMac->GetDllManagement ()->GetAddress ().GetAsInt ();
  //
  // NC uses LPDU without CRC as the payload
  //
  auto coding_header_size = ncr::coder_overhead::get (m_sp.genSize);
  GhnPlcLpduHeader header;
  NS_LOG_UNCOND("LLC headers sizes: " << header.GetSerializedSize () << "\t" << GHN_CRC_LENGTH << "\t" << coding_header_size);
  m_rxSegmenter = segmenter_ptr (
          new GhnPlcSegmenter (m_blockSize - header.GetSerializedSize () - GHN_CRC_LENGTH - coding_header_size));
  m_txSegmenter = segmenter_ptr (
          new GhnPlcSegmenter (m_blockSize - header.GetSerializedSize () - GHN_CRC_LENGTH - coding_header_size));
  m_sp.symbolSize = m_blockSize - GHN_CRC_LENGTH - coding_header_size;
  m_sp.numGen = (m_nodeType == ncr::SOURCE_NODE_TYPE) ? 2 * m_sp.numGen : m_sp.numGen;

  m_brr = routing_rules_ptr (new ncr::NcRoutingRules (m_id, m_nodeType, dst, m_sp));
  m_brr->SetLogCallback (m_addLog);

  m_oQueue = ordering_queue_ptr (new ordering_queue (m_brr->GetAckBacklogSize () * m_sp.genSize));
  //
  // initialize the feedback
  //
  m_feedback = m_brr->GetNetDiscoveryInfo ();

  if (m_nodeType == ncr::SOURCE_NODE_TYPE)
    {
      m_encQueue = encoder_queue_ptr (new ncr::encoder_queue (m_sp.numGen, m_sp.genSize, m_sp.symbolSize));
      m_encQueue->set_notify_callback (std::bind (&GhnPlcLlcCodedFlow::NotifyRcvUp, this, std::placeholders::_1));
      m_getRank = std::bind (&ncr::encoder_queue::rank, m_encQueue, std::placeholders::_1);
      m_brr->SetGetRankCallback (m_getRank);
      m_brr->SetCoderHelpInfoCallback (
              std::bind (&ncr::encoder_queue::get_help_info, m_encQueue, std::placeholders::_1, std::placeholders::_2,
                      std::placeholders::_3));

      SIM_LOG (COMM_NODE_LOG, "Node " << m_id << " type " << m_nodeType);
    }
  else
    {
      m_decQueue = decoder_queue_ptr (new ncr::decoder_queue (m_sp.numGen, m_sp.genSize, m_sp.symbolSize));
      m_getRank = std::bind (&ncr::decoder_queue::rank, m_decQueue, std::placeholders::_1);
      m_brr->SetGetRankCallback (m_getRank);
      m_brr->SetGetCodingMatrixCallback (std::bind (&ncr::decoder_queue::get_coding_matrix, m_decQueue, std::placeholders::_1));
      m_brr->SetGetCoderInfoCallback (std::bind (&ncr::decoder_queue::get_coder_info, m_decQueue, std::placeholders::_1));
      m_brr->SetCoderHelpInfoCallback (
              std::bind (&ncr::decoder_queue::get_help_info, m_decQueue, std::placeholders::_1, std::placeholders::_2,
                      std::placeholders::_3));

      SIM_LOG (COMM_NODE_LOG, "Node " << m_id << " type " << m_nodeType);
    }
}
void
GhnPlcLlcCodedFlow::SetNextHopVertex (UanAddress addr)
{
  m_brr->AddToCoalition (addr.GetAsInt ());
}
void
GhnPlcLlcCodedFlow::NotifyRcvUp (ncr::GenId genId)
{
  m_brr->UpdateRcvd (genId, m_id);
}
SendTuple
GhnPlcLlcCodedFlow::SendDown ()
{
  NS_LOG_FUNCTION(this << m_connId);

  if (m_connId.dst == UanAddress::GetBroadcast ()) return GhnPlcLlcFlow::SendDown ();

  GhnBuffer toTransmit;

  auto dll = m_dllMac->GetDllManagement ();
  auto src = dll->GetAddress ().GetAsInt ();
  auto dst = m_connId.dst.GetAsInt ();
  auto phy = dll->GetPhyManagement ()->GetPhyPcs ()->GetObject<GhnPlcPhyPcs> ();
  auto rt = dll->GetRoutingTable ();
  auto bt = dll->GetBitLoadingTable ();
  auto nh = (m_sp.mutualPhyLlcCoding) ? m_brr->GetSinkVertex () : rt->GetNextHopAddress (src, dst).GetAsInt ();

  if (m_nodeType != ncr::DESTINATION_NODE_TYPE)
    {
      NS_LOG_UNCOND("Node " << (uint16_t)src << ", Connection " << m_connId << " SEND!");
      if (nh == -1) nh = rt->GetNextHopAddress (src, dst).GetAsInt ();
      NS_ASSERT(src != nh);
      m_brr->SetSendingRate (bt->GetNumEffBits (src, nh));

      if (Simulator::Now () >= 2 * GHN_WARMUP_PERIOD)
        {
          auto dataAmount = bt->GetDataAmount (Seconds (GHN_CYCLE_MAX), src, nh);
          uint32_t pushedPkts = 0, maxPkts = floor ((double) dataAmount / (double) m_blockSize);
          assert(maxPkts > 0);

          NS_LOG_UNCOND(
                  "Node " << m_id << ", Flow " << m_connId << ": " << "dataAmount: " << dataAmount << ", maxPkts: " << maxPkts);

          if (m_nodeType == ncr::SOURCE_NODE_TYPE)
            {
              //
              // get maximum number of NC symbols that BRR allows transmitting in one train
              //
              auto maxBuf = m_brr->GetMaxAmountTxData ();
              //
              // limit this number by the amount of packets that MAC allows transmitting in one train
              //
              maxBuf = (maxPkts > maxBuf) ? maxBuf : maxPkts;

              assert(!m_genCallback.IsNull ());
              //
              // get the number of NC symbols that already can be sent by BRR
              //
              auto busy_space = m_brr->GetAmountTxData ();
              maxBuf = (busy_space < maxBuf) ? maxBuf - busy_space : 0;
              //
              // ask BRR if the number of generation in BRR buffer should be increased to certain minimum level
              //
              if (m_brr->NeedGen ())
                {
                  auto v = m_brr->GetNumGreedyGen () * m_sp.genSize;
                  maxBuf = (maxBuf < v) ? v : maxBuf;
                }

              NS_LOG_UNCOND(
                      "Node " << m_id << ", Flow " << m_connId << ": " << "maxBuf: " << maxBuf << ", busy_space: " << busy_space << ", maxPkts: " << maxBuf);

              //
              // generate data on application layer
              //
              m_genCallback (maxBuf * m_sp.symbolSize);
              //
              // segment, code the newly generated data and pass it to BRR
              //
              PrepareForSend ();
            }

          assert(m_nodeType != ncr::DESTINATION_NODE_TYPE);
          ncr::TxPlan plan = m_brr->GetTxPlan ();
          ncr::TxPlan planI;

          if (plan.empty ())
            {
              SIM_LOG (COMM_NODE_LOG, "Flow " << m_connId << ": " << "Sending the message with feedback only");
            }

          auto p_it = plan.begin_orig_order ();
          while (p_it != plan.end ())
            {
              ncr::GenId genId = p_it->first;
              auto n = p_it->second.num_all;
              uint16_t i = 0;
              for (; i < n;)
                {
                  auto contents =
                          (m_nodeType == ncr::SOURCE_NODE_TYPE) ? m_encQueue->get_coded (genId) : m_decQueue->get_coded (genId);
                  auto pkt = Create<Packet> ((uint8_t const*) contents.data (), contents.size ());
                  toTransmit.push_back (pkt);
                  pushedPkts++;
                  i++;
                  if (pushedPkts == maxPkts) break;
                }
              m_brr->UpdateSent (genId, i, true);
              planI[genId].num_all = i;
              planI[genId].all_prev_acked = p_it->second.all_prev_acked;
              if (pushedPkts == maxPkts) break;
              p_it = plan.next_orig_order (p_it);
            }

          if (m_nodeType == ncr::SOURCE_NODE_TYPE)
            {
              if (pushedPkts != maxPkts)
                {
                  SIM_LOG (COMM_NODE_LOG, "Node " << m_id << " pushed pkts: " << pushedPkts << ", max pkts: " << maxPkts);
                }
            }
          NS_LOG_UNCOND("Node " << m_id << ", Flow " << m_connId << ": " << " Tx Plan: " << planI);

          auto hBuf = ConvertBrrHeaderToPkt (planI);
          toTransmit.insert (toTransmit.begin (), hBuf.begin (), hBuf.end ());
        }
      else
        {
          NS_LOG_UNCOND("Flow " << m_connId << ": " << " Warm-up phase is not over yet");
          auto hBuf = ConvertBrrHeaderToPkt (ncr::TxPlan ());
          toTransmit.insert (toTransmit.begin (), hBuf.begin (), hBuf.end ());
        }
    }
  else
    {
      SIM_LOG (1, "Flow " << m_connId << ": " << "I am the destination. I can send the feedback only");
      nh = rt->GetNextHopAddress (src, m_connId.src).GetAsInt ();
      m_brr->SetSendingRate (bt->GetNumEffBits (src, m_connId.src.GetAsInt ()));

      auto hBuf = ConvertBrrHeaderToPkt (ncr::TxPlan ());
      toTransmit.insert (toTransmit.begin (), hBuf.begin (), hBuf.end ());
    }

  NS_LOG_UNCOND("Flow " << m_connId << ": " << "Segments number to be transmitted: " << toTransmit.size());

  //
  // add CRC
  //
  GhnBuffer::iterator it = toTransmit.begin ();
  while (it != toTransmit.end ())
    {
      (*it)->AddPaddingAtEnd (GHN_CRC_LENGTH);
      it++;
    }

  m_drCalc.Update (toTransmit.size () * m_blockSize * 8);

  SIM_LOG (COMM_NODE_LOG, "Node " << m_id << " Tx buffer size: " << toTransmit.size ());

  return SendTuple (toTransmit, m_connId, UanAddress (nh));
}
void
GhnPlcLlcCodedFlow::PrepareForSend ()
{
  if (!m_frameBuffer.empty ())
    {
      assert(m_nodeType == ncr::SOURCE_NODE_TYPE);

      //
      // create non-indexed segments
      //
      SegGhnBuffer addNonIndexed = m_txSegmenter->SegmentData (m_frameBuffer);
      m_nonindexedSegs.insert (m_nonindexedSegs.end (), addNonIndexed.begin (), addNonIndexed.end ());
      m_frameBuffer.clear ();

      auto s = m_nonindexedSegs.size () * m_nonindexedSegs.begin ()->pkt->GetSize ();
      NS_LOG_UNCOND("Segmented buffer contains " << s << " bytes");

      //
      // index segments
      //
      std::deque<Ssn> newSentSegs;
      while (!m_nonindexedSegs.empty ())
        {
          GhnPlcLpduHeader header;
          header.SetLfbo ((*m_nonindexedSegs.begin ()).posLlcFrame);
          header.SetMqf ((*m_nonindexedSegs.begin ()).validSeg);
          header.SetSsn (m_ssn.val ());
          m_ssn++;
          NS_LOG_DEBUG("Flow " << m_connId << ": " << "Indexing the segment by SSN: " << header.GetSsn());
          newSentSegs.push_back (header.GetSsn ());
          (*m_nonindexedSegs.begin ()).pkt->AddHeader (header);
          m_encQueue->enque (ConvertPacketToVec ((*m_nonindexedSegs.begin ()).pkt));
          m_nonindexedSegs.pop_front ();
        }
    }
}

GhnBuffer
GhnPlcLlcCodedFlow::ConvertBrrHeaderToPkt (ncr::TxPlan plan)
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

  assert(buffer.size () == n_bs);

  return buffer;
}
ncr::BrrHeader
GhnPlcLlcCodedFlow::ConvertPktToBrrHeader (GhnBuffer &buffer, std::deque<SegmentState> &state)
{
  //
  // convert buffer to packet
  //
  assert(!buffer.empty ());
  auto pkt = *buffer.begin ();
  auto bs = m_blockSize - GHN_CRC_LENGTH;
  assert(pkt->GetSize () == bs);

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
  ncr::BrrHeader h;
  h.Deserialize (std::string ((const char*) v, pkt->GetSize ()));
  delete[] v;
  return h;
}

GroupEncAckInfo
GhnPlcLlcCodedFlow::Receive (GhnBuffer buffer, ConnId connId)
{
  NS_LOG_FUNCTION(this << connId);

  NS_LOG_UNCOND(
          "Node " << (uint16_t)m_dllMac->GetDllManagement()->GetAddress().GetAsInt() << ", Connection " << connId << " RECEIVE!");

  NS_ASSERT(buffer.size () > 0);
  NS_ASSERT_MSG(m_connId == connId, m_connId << " " << connId);

  //
  // remove and check CRC
  //
  std::deque<SegmentState> state = CheckCrc (buffer, connId);
  assert(state.size () == buffer.size ());

  //
  // assume that the header segments are coded in a special manner so that the probability of their loss is negligible
  //
  if (0)
    {
      //
      // if CRC for the header segment fails, no data can be processed
      //
      if (*(state.begin ()) != DONE_SEGMENT_STATE)
        {
          NS_LOG_UNCOND(
                  "Node " << (uint16_t)m_dllMac->GetDllManagement()->GetAddress().GetAsInt() << ", Connection " << connId << " Header recpetion fails!");
          return GroupEncAckInfo ();
        }
    }

  //
  // process header
  //
  auto header = ConvertPktToBrrHeader (buffer, state);

  m_brr->RcvHeaderInfo (header.h);

  //
  // process data packets
  //
  SIM_LOG (COMM_NODE_LOG, "Node " << m_id << " Rx buffer size: " << buffer.size () << ", Tx plan: " << header.h.txPlan);
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

  //
  // process feedback (and produce own feedback message considering just received packets)
  //
  ProcessFeedback (header.f);

  //
  // find if I am on the main path
  //
  auto dll = m_dllMac->GetDllManagement ();
  auto src = header.h.addr;
  auto dst = m_connId.dst.GetAsInt ();
  auto rt = dll->GetRoutingTable ();
  auto nh = rt->GetNextHopAddress (src, dst).GetAsInt ();

  GroupEncAckInfo info;
  if (m_id == nh)
    {
      NS_LOG_UNCOND("Connection: " << m_connId << " " << m_id << ", Attaching ACK");
      auto hBuf = ConvertBrrHeaderToPkt (ncr::TxPlan ());
      //
      // add CRC
      //
      auto it = hBuf.begin ();
      while (it != hBuf.end ())
        {
          (*it)->AddPaddingAtEnd (GHN_CRC_LENGTH);
          it++;
        }
      info.brrFeedback.insert (info.brrFeedback.begin (), hBuf.begin (), hBuf.end ());
    }
  return info;
}
void
GhnPlcLlcCodedFlow::ReceiveAck (GroupEncAckInfo info, ConnId connId)
{
  SIM_LOG_FUNC (COMM_NODE_LOG);

  NS_LOG_UNCOND("Connection: " << m_connId << " " << m_id << ", Processing ACK");

  auto buffer = info.brrFeedback;
  assert(!buffer.empty ());

  std::deque<SegmentState> state = CheckCrc (buffer, connId);
  //
  // process header
  //
  auto header = ConvertPktToBrrHeader (buffer, state);

  m_brr->RcvHeaderInfo (header.h);
  ProcessFeedback (header.f);
}

bool
GhnPlcLlcCodedFlow::IsQueueEmpty ()
{
  SIM_LOG_FUNC (COMM_NODE_LOG);

  if (m_feedback.updated) return false;

  if (Simulator::Now () < 2 * GHN_WARMUP_PERIOD) return true;

  if (!m_frameBuffer.empty ()) return false;

  double dr = m_drCalc.Get ();
  double rel_dr = dr * PLC_Phy::GetSymbolDuration ().GetSeconds ();
  NS_LOG_UNCOND(
          "Connection: " << m_connId << " " << m_id << ", data rate: " << dr << " bps, relative data rate: " << rel_dr << " bits");

//    if (!m_brr->MaySendData (rel_dr)) return true;

  auto tx_amount = m_brr->GetAmountTxData ();
  NS_LOG_UNCOND("Connection: " << m_connId << " " << m_id << ", TX amount: " << tx_amount << " NC symbols");

  return (tx_amount == 0);
}

void
GhnPlcLlcCodedFlow::SetLogCallback (add_log_func addLog)
{
  m_addLog = addLog;
}
void
GhnPlcLlcCodedFlow::ProcessDecoded (GhnBuffer buffer, ConnId connId)
{
  //
  // drop decoded packets if I am not the destination
  //
  if (connId.dst != m_dllMac->GetDllManagement ()->GetAddress ()) return;

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

  assert(ssns.size () == state.size ());
  std::deque<Ssn> ackedSegs;
  for (uint32_t i = 0; i < ssns.size (); i++)
    {
      if (state.at (i) == DONE_SEGMENT_STATE) ackedSegs.push_back (ssns.at (i));
    }

  for (auto a : ackedSegs)
    {
      m_llcUncondedLogTrace (connId.dst.GetAsInt (), connId.src.GetAsInt (), a);
    }
  //
  // mark the received segments
  //
  m_oQueue->insert (ackedSegs);
  //
  // get SSNs of all continuously correctly received segments, which are still present in the confirmed window
  //
  ssns = m_oQueue->get ();

  if (ssns.empty ()) return;
  //  std::cout << "SSNs: ";
  //  for (auto ssn : ssns)
  //    {
  //      std::cout << ssn << " ";
  //    }
  //  std::cout << std::endl;

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

  //    GroupEncAckInfo info;
  //    m_rxArq->GetAck (info);
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
GhnPlcLlcCodedFlow::ProcessRcvdPacket (std::vector<uint8_t> vec, bool crc, ncr::UanAddress addr, ncr::TxPlan::iterator item,
        ConnId connId)
{
  ncr::GenId genId = item->first;

  if (!crc)
    {
      SIM_LOG (COMM_NODE_LOG, "Node " << m_id << " loosing segment");
      m_brr->UpdateLoss (genId, addr);
      return;
    }

  if (m_nodeType == ncr::SOURCE_NODE_TYPE) return;

  SIM_LOG (COMM_NODE_LOG, "Node " << m_id << " receives packet from generation " << genId);

  auto rank_b = m_decQueue->rank (genId);
  m_decQueue->enque (vec, genId);
  auto rank_a = m_decQueue->rank (genId);
  if (rank_b < rank_a)
    {
      auto pkts = m_decQueue->get_uncoded ();
      m_brr->UpdateRcvd (genId, addr, pkts);
      auto addr = m_dllMac->GetDllManagement ()->GetAddress ();
      if (addr == connId.dst)
        {
          if (!pkts.empty ()) ProcessDecoded (ConvertVecsToBuffer (pkts), connId);
        }
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
      SIM_LOG (COMM_NODE_LOG, "Node " << m_id << " sends the network discovery message with maximum TTL");

      m_feedback = m_brr->GetNetDiscoveryInfo ();

    }
  else
    {
      SIM_LOG (COMM_NODE_LOG, "Node " << m_id << " refuses to send the network discovery message");

      //
      // --->
      //
      if (m_brr->MaySendRetransRequest (m_decQueue->get_ranks (), addr, genId, item->second.all_prev_acked))
        {
          SIM_LOG (COMM_NODE_LOG, "Node " << m_id << " sends the retransmission request");

          m_feedback = m_brr->GetRetransRequestInfo ();

        }
      else
        {
          SIM_LOG (COMM_NODE_LOG, "Node " << m_id << " refuses to send the retransmission request");

          //
          // --->
          //
          if (m_brr->MaySendFeedback ())
            {
              SIM_LOG (COMM_NODE_LOG, "Node " << m_id << " sends the feedback");

              m_feedback = m_brr->GetFeedbackInfo ();

            }
          else
            {
              SIM_LOG (COMM_NODE_LOG, "Node " << m_id << " refuses to send the feedback");
            }
        }
    }
}

void
GhnPlcLlcCodedFlow::ProcessFeedback (ncr::FeedbackInfo f)
{
  SIM_LOG (1, "Node " << m_id << " receive feedback symbol");
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
      SIM_LOG (1, "Node " << m_id << " TTL has expired");
    }

}

void
GhnPlcLlcCodedFlow::ProcessNetDiscovery (ncr::FeedbackInfo f)
{
  SIM_LOG (1, "Node " << m_id << " receive the network discovery message with TTL " << f.ttl);

  if (m_brr->MaySendNetDiscovery (f.ttl))
    {
      SIM_LOG (1, "Node " << m_id << " sends the network discovery message with TTL " << f.ttl - 1);
      m_feedback = m_brr->GetNetDiscoveryInfo (f.ttl - 1);
    }
  else
    {
      SIM_LOG (1, "Node " << m_id << " refuses to send the network discovery message");
    }
}

void
GhnPlcLlcCodedFlow::ProcessRetransRequest (ncr::FeedbackInfo f)
{
  SIM_LOG (COMM_NODE_LOG, "Node " << m_id << " processing retransission request");

  if (m_brr->ProcessRetransRequest (f))
    {
      if (m_nodeType != ncr::SOURCE_NODE_TYPE)
        {
          SIM_LOG (COMM_NODE_LOG, "Node " << m_id << " forward retransmission request. TTL " << f.ttl);
          m_feedback = m_brr->GetRetransRequestInfo (f.ttl - 1);
        }
      else
        {
          SIM_LOG (COMM_NODE_LOG, "Node " << m_id << ". The source does not forward retransmission requests");
        }
    }
  else
    {
      SIM_LOG (COMM_NODE_LOG, "Node " << m_id << ". Retransmission request is either not set or I should not forward it");
    }

}

}
}
