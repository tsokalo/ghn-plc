/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 TUD
 *
 *  Created on: 25.08.2015
 *      Author: Stanislav Mudriievskyi <stanislav.mudriievskyi@tu-dresden.de>
 */
#include <iostream>
#include "ns3/log.h"
#include "ghn-plc-llc-flow.h"
#include "ghn-plc-llc-frame-header.h"
#include "ghn-plc-lpdu-header.h"
#include "ghn-plc-utilities.h"
#include "ghn-plc-utilities.h"
#include "ghn-plc-header.h"
#include "bit-set.h"
NS_LOG_COMPONENT_DEFINE("GhnPlcLlcFlow");

namespace ns3
{
namespace ghn
{
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NS_OBJECT_ENSURE_REGISTERED(GhnPlcLlcFlow);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TypeId
GhnPlcLlcFlow::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::GhnPlcLlcFlow").SetParent<Object> ().AddConstructor<GhnPlcLlcFlow> ()

  .AddTraceSource ("LlcRcvLog", "Received data by LLC counting only those packets destined to this node",
          MakeTraceSourceAccessor (&GhnPlcLlcFlow::m_llcRcvLogTrace), "ns3::LlcRcvLog::TracedCallback")

  .AddTraceSource ("LlcRelayLog", "Received data by LLC, which is relayed", MakeTraceSourceAccessor (
          &GhnPlcLlcFlow::m_llcRelayedLogTrace), "ns3::LlcRelayLog::TracedCallback")

  .AddTraceSource ("LlcTtlDroppedLog", "Received data by LLC but dropped because TTL expires", MakeTraceSourceAccessor (
          &GhnPlcLlcFlow::m_llcTtlDroppedLogTrace), "ns3::LlcTtlDroppedLog::TracedCallback")

  .AddTraceSource ("LlcUncondedLog", "Uncoded symbols", MakeTraceSourceAccessor (&GhnPlcLlcFlow::m_llcUncondedLogTrace),
          "ns3::LlcUncondedLog::TracedCallback");
  return tid;
}

GhnPlcLlcFlow::GhnPlcLlcFlow ()
{
  NS_LOG_FUNCTION(this);

  m_blockSize = GHN_BLKSZ_540;
  m_winConfSize = GHN_ACK_MAX_WINDOW_SIZE_DATA(m_blockSize);
  Time blockLifeTime = Seconds (1.0);
  m_llcFrameSeqNum = 0;
  m_rxBcSeqNum = 0;

  m_txArq = tx_ack_ptr (new GhnPlcTxAckInfo (LSS_N_MAX, m_blockSize, m_winConfSize, blockLifeTime));
  m_rxArq = rx_ack_ptr (new GhnPlcRxAckInfo (LSS_N_MAX, m_blockSize, m_winConfSize, NO_ACK_COMPRESS, blockLifeTime));

  GhnPlcLpduHeader header;
  m_rxSegmenter = segmenter_ptr (new GhnPlcSegmenter (m_blockSize - header.GetSerializedSize () - GHN_CRC_LENGTH));
  m_txSegmenter = segmenter_ptr (new GhnPlcSegmenter (m_blockSize - header.GetSerializedSize () - GHN_CRC_LENGTH));

  NS_LOG_UNCOND("Creating original G.hn LLC flow");
}
void
GhnPlcLlcFlow::SetConnId (ConnId connId)
{
  m_connId = connId;
  NS_LOG_UNCOND(
          "Node " << (uint16_t) m_dllMac->GetDllManagement()->GetAddress().GetAsInt() << ": Setting LLC flow ID " << connId);
}

GhnPlcLlcFlow::~GhnPlcLlcFlow ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
GhnPlcLlcFlow::SetDllMac (Ptr<GhnPlcDllMacCsma> dllMac)
{
  NS_LOG_FUNCTION(this << m_connId);
  m_dllMac = dllMac;
}

Ptr<GhnPlcDllMacCsma>
GhnPlcLlcFlow::GetDllMac (void)
{
  return m_dllMac;
}

void
GhnPlcLlcFlow::SetDllApc (Ptr<GhnPlcDllApc> dllApc)
{
  NS_LOG_FUNCTION(this << m_connId);
  m_dllApc = dllApc;
}

Ptr<GhnPlcDllApc>
GhnPlcLlcFlow::GetDllApc (void)
{
  return m_dllApc;
}
void
GhnPlcLlcFlow::SetDllLlc (Ptr<GhnPlcDllLlc> dllLlc)
{
  NS_LOG_FUNCTION(this << m_connId);
  m_dllLlc = dllLlc;
}

Ptr<GhnPlcDllLlc>
GhnPlcLlcFlow::GetDllLlc (void)
{
  return m_dllLlc;
}

bool
GhnPlcLlcFlow::SendFrom (Ptr<Packet> packet, ConnId connId, int16_t ttl)
{
  NS_LOG_FUNCTION(this << connId << packet->GetSize());

  auto dll = m_dllMac->GetDllManagement ();

  packet = ConvertApduToLlcFrame (packet, connId, ttl);

  return Enqueue (packet, connId);
}
bool
GhnPlcLlcFlow::Enqueue (Ptr<Packet> packet, ConnId connId)
{
  NS_LOG_FUNCTION(this << connId << packet->GetSize());
  NS_ASSERT_MSG(m_connId == connId, m_connId << " " << connId);

  if (m_frameBuffer.size () >= MAX_LLC_QUEUE_LENGTH)
    {
      m_dllMac->TriggerSend ();
      return false;
    }
  m_frameBuffer.push_back (packet);
  m_dllMac->TriggerSend ();
  return true;
}

GroupEncAckInfo
GhnPlcLlcFlow::Receive (GhnBuffer buffer, ConnId connId)
{
  NS_LOG_FUNCTION(this << connId);

  NS_ASSERT(buffer.size () > 0);
  NS_ASSERT_MSG(m_connId == connId, m_connId << " " << connId);

  GroupEncAckInfo info;
  //
  // remove and check CRC
  //
  std::deque<SegmentState> state = CheckCrc (buffer, connId);

  //
  // remove LPDU header
  //
  SegGhnBuffer segmentBuffer;
  std::deque<Ssn> ssns = RemoveLpduHeaders (buffer, state, segmentBuffer);

  //
  // add/update segments to/in the receive buffer
  //
  UpdateRcvdSegments (segmentBuffer);

  //
  // mark the received segments by ARQ
  //
  m_rxArq->MarkRcvSegs (ssns, state);
  //
  // get SSNs of all continuously correctly received segments, which are still present in the confirmed window
  //
  ssns = m_rxArq->GetAckSsns ();

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

  m_rxArq->GetAck (info);
  NS_LOG_UNCOND("Flow " << m_connId << ": " << "Sent ACK: " << info);

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

  return info;
}
std::deque<SegmentState>
GhnPlcLlcFlow::CheckCrc (GhnBuffer &buffer, ConnId connId)
{
  NS_LOG_FUNCTION(this);
  GhnPlcLpduHeader header;
  std::deque<SegmentState> state;
  auto phym = m_dllMac->GetDllManagement ()->GetPhyManagement ();

  ncr::bitset crc;
  for (auto &packet : buffer)
    {
      RemoveCrc (packet);
      packet->PeekHeader (header);
      //
      // assume that the management messages are coded good enough to have no bit errors after PHY decoding
      //
      bool blockSuccess = (connId.flowId == MANAGMENT_CONN_ID) ? 1 : phym->IsBlockSuccess ();
      crc.add (blockSuccess);
      if (blockSuccess)
        {
          NS_LOG_DEBUG("Flow " << m_connId << ": " << "CRC OK. The segment will be sent to the decoder");
          state.push_back (DONE_SEGMENT_STATE);
        }
      else
        {
          NS_LOG_DEBUG("Flow " << m_connId << ": " << "CRC error. The segment is ignored: " << blockSuccess);
          state.push_back (WAIT_RETRANSMISSION_SEG_STATE);
        }
    };; auto per = 1 - crc.get_ratio ();
  auto ber = phym->GetActualBer ();
  auto path = "/home/tsokalo/workspace/ns-allinone-3.25/ns-3.25/data.txt";
  if (buffer.size () > 20) ThrowToFile (std::to_string (per) + "\t" + std::to_string (ber), path);
  NS_LOG_UNCOND("CRC status (PER: " << per << ", BER: " << ber << "): " << crc.to_full_string());
  return state;
}
std::deque<Ssn>
GhnPlcLlcFlow::RemoveLpduHeaders (GhnBuffer &buffer, std::deque<SegmentState> &state, SegGhnBuffer &segmentBuffer)
{
  NS_LOG_FUNCTION(this);
  std::deque<Ssn> ssns;
  auto it = buffer.begin ();
  auto ix = state.begin ();
  GhnPlcLpduHeader header;
  while (it != buffer.end ())
    {
      GhnSeg seg;
      (*it)->RemoveHeader (header);
      ssns.push_back (header.GetSsn ());
      seg.pkt = (*it)->Copy ();
      seg.posLlcFrame = header.GetLfbo ();
      seg.validSeg = ((*ix) == DONE_SEGMENT_STATE) ? ((header.GetVsf () == INVALID_VSF_VALUE) ? false : true) : false;
      seg.ssn = header.GetSsn ();
      segmentBuffer.push_back (seg);
      it++;
      ix++;
    }
  buffer.clear ();

  return ssns;
}
void
GhnPlcLlcFlow::UpdateRcvdSegments (SegGhnBuffer segmentBuffer)
{
  NS_LOG_FUNCTION(this);
  if (!m_segmentBuffer.empty ())
    {
      Ssn firstInRx = (*m_segmentBuffer.begin ()).ssn;
      Ssn lastInRx = (*((m_segmentBuffer.end ())--)).ssn;
      NS_LOG_DEBUG(
              "Flow " << m_connId << ": " << "First SSN in m_segmentBuffer: " << firstInRx << ", last SSN in m_segmentBuffer: " << lastInRx << ", m_segmentBuffer size: " << m_segmentBuffer.size());
      auto it = segmentBuffer.begin ();
      while (it != segmentBuffer.end ())
        {
          NS_LOG_DEBUG("Flow " << m_connId << ": " << "Looking for SSN " << (*it).ssn);
          bool found = false;
          for (Ssn i = 0; i < m_segmentBuffer.size (); i++)
            {
              if (m_segmentBuffer.at (i).ssn == (*it).ssn)
                {
                  m_segmentBuffer.erase (m_segmentBuffer.begin () + i, m_segmentBuffer.begin () + i + 1);
                  m_segmentBuffer.insert (m_segmentBuffer.begin () + i, (*it));
                  NS_LOG_DEBUG(
                          "Flow " << m_connId << ": " << "Update segment validity. SSN: " << (*it).ssn << ". Validity: " << (*it).validSeg);
                  found = true;
                }
            }
          if (!found)
            {
              NS_LOG_DEBUG(
                      "Flow " << m_connId << ": " << "Segment with SSN " << (*it).ssn << " is not in range. Add it at the end of the receive buffer");
              m_segmentBuffer.push_back (*it);
            }
          it++;
        }
    }
  else
    {
      //
      // add all segments to the receive buffer at its end
      //
      m_segmentBuffer.insert (m_segmentBuffer.end (), segmentBuffer.begin (), segmentBuffer.end ());
    }

  for (uint32_t i = 0; i < m_segmentBuffer.size (); i++)
    {
      NS_LOG_DEBUG(
              "Flow " << m_connId << ": " << "m_segmentBuffer[" << i << "].ssn : " << m_segmentBuffer.at(i).ssn << ", validity: " << m_segmentBuffer.at(i).validSeg);
    }
  NS_LOG_DEBUG("Flow " << m_connId << ": " << "Size of the receive buffer: " << m_segmentBuffer.size());
}
void
GhnPlcLlcFlow::AddNotDesegmented (std::deque<Ssn> &ssns, SegGhnBuffer &segmentBuffer)
{
  NS_LOG_FUNCTION(this);
  while (!m_segmentBuffer.empty ())
    {
      if ((*m_segmentBuffer.begin ()).ssn != (*ssns.begin ()))
        {
          NS_LOG_DEBUG(
                  "Flow " << m_connId << ": " << "Adding segment with SSN " << (*m_segmentBuffer.begin ()).ssn << " in receive buffer which is before the first ACK SSN: " << (*ssns.begin ()));
          segmentBuffer.push_back (*m_segmentBuffer.begin ());
          m_segmentBuffer.pop_front ();
        }
      else
        {
          break;
        }
    }
  //
  // the first part of the first segment (if any) in this case was already used for de-segmentation
  // therefore we ignore it; we do it indirectly adding a not valid segment to the buffer
  //
  if (!segmentBuffer.empty () && m_containPartDesegm)
    {
      GhnSeg seg;
      seg.pkt = Create<Packet> (m_blockSize);
      seg.posLlcFrame = 0;
      seg.validSeg = false;
      seg.ssn = 0;
      segmentBuffer.push_front (seg);
      NS_LOG_DEBUG("Flow " << m_connId << ": " << "Add dummy segment");
    }
  m_containPartDesegm = false;
}
void
GhnPlcLlcFlow::AddJustAcknowledged (std::deque<Ssn> &ssns, SegGhnBuffer &segmentBuffer)
{
  NS_LOG_FUNCTION(this);
  while (!ssns.empty () && !m_segmentBuffer.empty ())
    {
      //          NS_ASSERT_MSG((*ssns.begin()) == (*m_segmentBuffer.begin()).ssn, "(*ssns.begin()): " << (*ssns.begin()) << ", (*m_segmentBuffer.begin()).ssn: " << (*m_segmentBuffer.begin()).ssn);
      while ((*ssns.begin ()) != (*m_segmentBuffer.begin ()).ssn && (!ssns.empty ()))
        {
          ssns.pop_front ();
        }
      if (ssns.empty ()) break;
      NS_LOG_DEBUG(
              "Flow " << m_connId << ": " << "Found ssn " << (*m_segmentBuffer.begin()).ssn << " in receive buffer which coincides with the ACK segment: " << (*ssns.begin ()));
      segmentBuffer.push_back ((*m_segmentBuffer.begin ()));
      ssns.pop_front ();
      m_segmentBuffer.pop_front ();
    }
}
GhnBuffer
GhnPlcLlcFlow::ConvertSegsToLlcFrames (SegGhnBuffer &segmentBuffer)
{
  NS_LOG_FUNCTION(this);
  GhnBuffer buffer = m_rxSegmenter->DesegmentData (segmentBuffer);
  NS_LOG_DEBUG("Flow " << m_connId << ": " << "Size of desegmented buffer: " << buffer.size());

  if (buffer.empty ())
    {
      if (!(*segmentBuffer.begin ()).validSeg) segmentBuffer.pop_front ();
      while (!segmentBuffer.empty ())
        {
          NS_ASSERT((*segmentBuffer.begin ()).validSeg);

          m_segmentBuffer.push_front ((*(segmentBuffer.end () - 1)));
          NS_LOG_DEBUG(
                  "Flow " << m_connId << ": " << "Pushing back the segment with SSN " << (*m_segmentBuffer.begin()).ssn << " in receive buffer");
          segmentBuffer.pop_back ();
        }
    }

  return buffer;
}
void
GhnPlcLlcFlow::SaveNotFullyDesegmented (SegGhnBuffer &segmentBuffer)
{
  NS_LOG_FUNCTION(this);
  segmentBuffer = m_rxSegmenter->GetNotDesegmented (segmentBuffer);
  if (!segmentBuffer.empty ())
    {
      if ((*segmentBuffer.begin ()).posLlcFrame != 0) m_containPartDesegm = true;
    }
  m_segmentBuffer.insert (m_segmentBuffer.begin (), segmentBuffer.begin (), segmentBuffer.end ());
  for (uint32_t i = 0; i < segmentBuffer.size (); i++)
    {
      NS_LOG_DEBUG(
              "Flow " << m_connId << ": " << "Added at start m_segmentBuffer[" << i << "].ssn : " << m_segmentBuffer.at(i).ssn << ", validity: " << m_segmentBuffer.at(i).validSeg);
    }
}
void
GhnPlcLlcFlow::ProcessDeseqmented (GhnBuffer buffer, ConnId connId)
{
  NS_LOG_FUNCTION(this);
  GhnPlcLlcFrameHeader header;
  auto dll = m_dllMac->GetDllManagement ();
  if (m_aggr.empty ()) CreateLogger ();

  for (auto &packet : buffer)
    {
      NS_LOG_FUNCTION(this << connId << packet->GetSize());
      packet->RemoveHeader (header);
      m_llcRcvLogTrace (connId.dst.GetAsInt (), header.GetOrigNodeId (), connId.src.GetAsInt (), header.GetTsmp ());

      // if not destined to us
      //
      if (connId.dst != dll->GetAddress ())
        {
          NS_LOG_DEBUG("Packet is not destined to us");
          //          if(connId.dst == dll->GetBroadcast() && !IsNewBcLlcFrame(header.GetTsmp()))return;
          if (connId.dst == dll->GetBroadcast ())
            {
              NS_LOG_DEBUG("It's a broadcast packet. Forward it up!");
              m_rxBcSeqNum = header.GetTsmp () + 1;
              m_dllApc->Receive (packet->Copy (), connId.src, connId.dst);
            }
          //
          // filter packets with the expired TTL
          //
          if (header.GetTtl () <= 1)
            {
              //              std::cout << "DROP TTL:\t" << header.GetOrigNodeId () << "\t" << (uint32_t) connId.src.GetAsInt () << "\t"
              //              << (uint32_t) dll->GetAddress ().GetAsInt () << "\tTTL: " << header.GetTtl () << "\tPktSize: "
              //              << packet->GetSize () << std::endl;
              m_llcTtlDroppedLogTrace (connId.dst.GetAsInt (), connId.src.GetAsInt (), dll->GetAddress ().GetAsInt (),
                      header.GetTsmp ());
              NS_LOG_DEBUG("TTL has expired for the broadcast packet. Drop the broadcast packet!");
            }
          else
            {
              m_llcRelayedLogTrace (connId.dst.GetAsInt (), header.GetOrigNodeId (), dll->GetAddress ().GetAsInt (),
                      header.GetTsmp ());
              NS_LOG_DEBUG("Relay the packet. TTL: " << header.GetTtl() - 1);
              m_dllLlc->SendFrom (packet, dll->GetAddress (), connId.dst, header.GetTtl () - 1);
            }
        }
      //
      // if destined to us
      //

      else
        {
          NS_LOG_DEBUG("Packet is destined to us. Forward it up!");
          m_dllApc->Receive (packet, connId.src, connId.dst);
        }
    };;
}
bool
GhnPlcLlcFlow::IsNewBcLlcFrame (NcSeqNum current)
{
  NS_LOG_FUNCTION(this << current << m_rxBcSeqNum);
  NcSeqNum old = m_rxBcSeqNum;
  NcSeqNum maxDistance = (1 << (sizeof(NcSeqNum) - 1));
  return ((current > old && (current - old) < maxDistance) || (current < old && (old - current) < maxDistance) || current
          == old);
}
void
GhnPlcLlcFlow::RemoveCrc (Ptr<Packet> packet)
{
  packet->RemoveAtEnd (GHN_CRC_LENGTH);
}

void
GhnPlcLlcFlow::ReceiveAck (GroupEncAckInfo info, ConnId connId)
{
  NS_LOG_FUNCTION(this << connId);
  NS_LOG_UNCOND("Flow " << m_connId << ": " << "Received ACK: " << info);

  m_txArq->MarkAckSegs (info, NO_ACK_COMPRESS);

  //
  // get SSNs of all continuously correctly received segments, which are still present in the confirmed window
  //
  std::deque<Ssn> ssns = m_txArq->GetAckSsns ();
  NS_LOG_UNCOND("Flow " << m_connId << ": " << "Number of continuously acknowledged segments: " << ssns.size());
  //
  // form received buffer
  //
  Ssn bufSize = m_indexedSegs.size (), ssnSize = ssns.size ();
  while (!ssns.empty ())
    {
      for (Ssn i = 0; i < m_indexedSegs.size ();)
        {
          if (m_indexedSegs.empty ()) break;
          GhnPlcLpduHeader header;
          m_indexedSegs.at (i)->PeekHeader (header);

          if ((*ssns.begin ()) == header.GetSsn ())
            {
              NS_LOG_DEBUG(
                      "Flow " << m_connId << ": " << "Found ssn " << header.GetSsn() << " in transmitter buffer which coincides with the ACK segment: " << (*ssns.begin ()));
              m_indexedSegs.erase (m_indexedSegs.begin () + i, m_indexedSegs.begin () + i + 1);
              break;
            }
          else
            {
              i++;
            }
        }
      ssns.pop_front ();
    }
  NS_ASSERT(bufSize == ssnSize + m_indexedSegs.size () || bufSize < ssnSize);
}
SendTuple
GhnPlcLlcFlow::SendDown ()
{
  NS_LOG_FUNCTION(this << m_connId);

auto  dll = m_dllMac->GetDllManagement ();
  auto src = dll->GetAddress ().GetAsInt ();

  NS_ASSERT_MSG(m_connId.src.GetAsInt () == src, m_connId << " " << dll->GetAddress ());

  auto dst = m_connId.dst.GetAsInt ();
  auto phy = dll->GetPhyManagement ()->GetPhyPcs ()->GetObject<GhnPlcPhyPcs> ();
  auto rt = dll->GetRoutingTable ();
  auto bt = dll->GetBitLoadingTable ();
  auto nh = rt->GetNextHopAddress (src, dst).GetAsInt ();
  NS_ASSERT(src != nh);
  auto dataAmount = bt->GetDataAmount (Seconds (GHN_CYCLE_MAX), src, nh);

  VirtSsn freeBufSize = m_txArq->GetFreeTxBufferSize ();

  NS_LOG_UNCOND(
          "Flow " << m_connId << ": " << "BEFORE INDEXING: Free buffer size: " << freeBufSize << ", number of non-indexed segments: " << m_nonindexedSegs.size() << ", number of indexed segments: " << m_indexedSegs.size() << ", not-segmented data: " << m_frameBuffer.size() << ", dataAmount: " << dataAmount);

  uint64_t dataLimit = (dataAmount > (uint64_t) freeBufSize * m_blockSize) ? freeBufSize * m_blockSize : dataAmount;
  NS_LOG_UNCOND("Flow " << m_connId << ": Using data limit " << dataLimit);
  //
  // generate some data
  //
  if (!m_genCallback.IsNull ())
    {
      auto s = (m_nonindexedSegs.size () + m_indexedSegs.size ()) * m_txSegmenter->GetSegmentSize ();
      auto f = (dataLimit < s) ? 0 : dataLimit - s;
      m_genCallback (f);
    }

  //
  // create non-indexed segments
  //
  if ((m_nonindexedSegs.size () * m_blockSize < dataLimit) && (!m_frameBuffer.empty ()))
    {
      SegGhnBuffer addNonIndexed = m_txSegmenter->SegmentData (m_frameBuffer);
      m_nonindexedSegs.insert (m_nonindexedSegs.end (), addNonIndexed.begin (), addNonIndexed.end ());
      m_frameBuffer.clear ();
    }

  //
  // generate some more data
  //
  if (!m_genCallback.IsNull ())
    {
      m_genCallback (10 * m_txSegmenter->GetSegmentSize ());
    }

  //
  // index segments
  //
  std::deque<Ssn> newSentSegs;
  while (!(freeBufSize == 0 || m_nonindexedSegs.empty ()))
    {
      GhnPlcLpduHeader header;
      header.SetLfbo ((*m_nonindexedSegs.begin ()).posLlcFrame);
      header.SetMqf ((*m_nonindexedSegs.begin ()).validSeg);
      header.SetSsn (m_txArq->GetNextSsn ());
      NS_LOG_DEBUG("Flow " << m_connId << ": " << "Indexing the segment by SSN: " << header.GetSsn());
      newSentSegs.push_back (header.GetSsn ());
      (*m_nonindexedSegs.begin ()).pkt->AddHeader (header);
      m_indexedSegs.push_back ((*m_nonindexedSegs.begin ()).pkt);
      m_nonindexedSegs.pop_front ();
      freeBufSize--;
      if (m_nonindexedSegs.empty ()) break;
    }
  m_txArq->MarkSentSegs (newSentSegs);

  NS_LOG_DEBUG(
          "Flow " << m_connId << ": " << "AFTER INDEXING: Free buffer size: " << freeBufSize << ", number of non-indexed segments: " << m_nonindexedSegs.size() << ", number of indexed segments: " << m_indexedSegs.size() << ", not-segmented data: " << m_frameBuffer.size());

  GhnBuffer toTransmit;
  std::deque<Ssn> ssns = m_txArq->GetMarkedForSend ();
  NS_ASSERT(newSentSegs.size () <= ssns.size ());
  uint64_t pushedData = 0;
  NS_LOG_UNCOND(
          "Flow " << m_connId << ": " << "Number of SSNs from ARQ mechanism: " << ssns.size() << ", allowed data (bytes): " << dataAmount);

  //
  // TODO: calculate dataAmount with consideration of overhead from encoder below
  //
  while (!ssns.empty () && pushedData < dataAmount && (!m_indexedSegs.empty()))
    {
      GhnBuffer::iterator it = m_indexedSegs.begin ();
      while (it != m_indexedSegs.end ())
        {
          GhnPlcLpduHeader header;
          (*it)->PeekHeader (header);
          if (header.GetSsn () == *(ssns.begin ()))
            {
              pushedData += ((*it)->GetSize () + GHN_CRC_LENGTH);
              NS_LOG_DEBUG(
                      "Flow " << m_connId << ": " << "Amount of data to be pushed (bytes): " << pushedData << ", amount of allowed data (bytes): " << dataAmount << ", current SSN: " << (*ssns.begin()));
              if (pushedData >= dataAmount)
                {
                  break;
                }
              toTransmit.push_back ((*it)->Copy ());
              break;
            }
          it++;
        }
      ssns.pop_front ();
    }

  NS_ASSERT_MSG(ssns.empty (), "ssns size: " << ssns.size());

  NS_ASSERT_MSG(!toTransmit.empty (), "There is nothing to transmit");

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

  if (m_connId.dst == UanAddress::GetBroadcast ())
    {
      //
      // we expect no acknowledgment for broadcast packets
      // therefore we reset the Tx ARQ like if we would receive the complete ACK
      //
      NS_LOG_DEBUG("Flow " << m_connId << ": " << "Reseting Tx ARQ");
      m_txArq->Reset ();
      m_nonindexedSegs.clear ();
      m_indexedSegs.clear ();
    }
  return SendTuple (toTransmit, m_connId, UanAddress(nh));
}
bool
GhnPlcLlcFlow::IsQueueEmpty ()
{
  return m_frameBuffer.empty () && m_nonindexedSegs.empty() && m_indexedSegs.empty();
}

Ptr<Packet>
GhnPlcLlcFlow::ConvertApduToLlcFrame (Ptr<Packet> apdu, ConnId connId, int16_t ttl)
{
  GhnPlcLlcFrameHeader header;
  auto dll = m_dllMac->GetDllManagement ();
  header.SetFrameType (APDU_FRAME_TYPE);
  header.SetFrameSize (apdu->GetSize ());
  header.SetOrigNodeId (connId.src.GetAsInt ());
  header.SetTsmpI (TSMP_NOT_INCLUDED);
  header.SetTsmp (0);
  header.SetPriority (UNDEF_PRIORITY);
  header.SetBroadcastIndicator ((connId.dst == dll->GetBroadcast ()) ? BROADCAST_INDICATION_ON : BROADCAST_INDICATION_OFF);
  //
  // if this is not a relayed packet
  //
  if (ttl == -1)
    {
      ttl = (connId.dst == dll->GetBroadcast ()) ? dll->GetRoutingTable ()->GetMaxNumHops (connId.src) :
      dll->GetRoutingTable ()->GetNumHops (connId.src, connId.dst);
    }
  header.SetTtl (ttl);
  header.SetTsmp (m_llcFrameSeqNum++); //TSMP is used not as described in G.hn

  //  std::cout << "NEW >>>:\t" << header.GetOrigNodeId () << "\t" << (uint32_t) source.GetAsInt () << "\t"
  //          << (uint32_t) dll->GetAddress ().GetAsInt () << "\tTTL: " << header.GetTtl () << "\tPktSize: " << apdu->GetSize ()
  //          << std::endl;
  apdu->AddHeader (header);
  return apdu;
}

void
GhnPlcLlcFlow::CreateLogger ()
{
  m_aggr.push_back (
          CreateObject<FileAggregator> (
                  m_resDir + "llc_rcv_data_" + std::to_string (m_dllMac->GetDllManagement ()->GetAddress ().GetAsInt ())
                  + ".txt", FileAggregator::FORMATTED));
  auto aggr = *(m_aggr.end () - 1);
  aggr->Set4dFormat ("%.0f\t%.0f\t%.0f\t%.0f");
  aggr->Enable ();
  TraceConnect ("LlcRcvLog", "LlcRcvLogContext", MakeCallback (&FileAggregator::Write4d, aggr));

  m_aggr.push_back (
          CreateObject<FileAggregator> (
                  m_resDir + "llc_relayed_data_" + std::to_string (m_dllMac->GetDllManagement ()->GetAddress ().GetAsInt ())
                  + ".txt", FileAggregator::FORMATTED));
  aggr = *(m_aggr.end () - 1);
  aggr->Set4dFormat ("%.0f\t%.0f\t%.0f\t%.0f");
  aggr->Enable ();
  TraceConnect ("LlcRelayLog", "LlcRelayLogContext", MakeCallback (&FileAggregator::Write4d, aggr));

  m_aggr.push_back (
          CreateObject<FileAggregator> (
                  m_resDir + "llc_dropped_ttl_" + std::to_string (m_dllMac->GetDllManagement ()->GetAddress ().GetAsInt ())
                  + ".txt", FileAggregator::FORMATTED));
  aggr = *(m_aggr.end () - 1);
  aggr->Set4dFormat ("%.0f\t%.0f\t%.0f\t%.0f");
  aggr->Enable ();
  TraceConnect ("LlcTtlDroppedLog", "LlcTtlDroppedLogContext", MakeCallback (&FileAggregator::Write4d, aggr));

  m_aggr.push_back (
          CreateObject<FileAggregator> (
                  m_resDir + "llc_uncoded_" + std::to_string (m_dllMac->GetDllManagement ()->GetAddress ().GetAsInt ())
                  + ".txt", FileAggregator::FORMATTED));
  aggr = *(m_aggr.end () - 1);
  aggr->Set3dFormat ("%.0f\t%.0f\t%.0f");
  aggr->Enable ();
  TraceConnect ("LlcUncondedLog", "LlcUncondedLogContext", MakeCallback (&FileAggregator::Write3d, aggr));

}
}
} // namespace ns3
