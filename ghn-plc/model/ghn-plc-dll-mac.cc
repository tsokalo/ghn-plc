/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 TUD
 *
 *  Created on: 25.08.2015
 *      Author: Stanislav Mudriievskyi <stanislav.mudriievskyi@tu-dresden.de>
 */
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/uan-header-common.h"
#include "ghn-plc-dll-management.h"
#include "ghn-plc-utilities.h"
#include "ghn-plc-dll-mac.h"
NS_LOG_COMPONENT_DEFINE ("GhnPlcDllMac");

namespace ns3
{
namespace ghn
{
/////////////////////////////////////////////////////////////////////////////////////////////////
NS_OBJECT_ENSURE_REGISTERED (GhnPlcDllMac);
/////////////////////////////////////////////////////////////////////////////////////////////////

TypeId
GhnPlcDllMac::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::GhnPlcDllMac") .SetParent<Object> ()

  .AddTraceSource ("MpduBitsLog", "Number of bits received in the MPDU", MakeTraceSourceAccessor (&GhnPlcDllMac::m_mpduBytes),
          "ns3::MpduBitsLog::TracedCallback");
  return tid;
}

GhnPlcDllMac::GhnPlcDllMac ()
{
  NS_LOG_FUNCTION (this);
  m_txAllowed = true;
  m_askedForAck = false;
  m_sentAck = false;
  m_transPacket = 0;
  m_nodeState = READY;
  m_blockSize = GHN_BLKSZ_540;
  m_allowCooperation = false;
}

GhnPlcDllMac::~GhnPlcDllMac ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
GhnPlcDllMac::AllowCooperation (bool v)
{
  m_allowCooperation = v;
}
void
GhnPlcDllMac::SetPhyManagement (Ptr<GhnPlcPhyManagement> ghnPhyManagement)
{
  m_phyMan = ghnPhyManagement;
}

Ptr<GhnPlcPhyManagement>
GhnPlcDllMac::GetPhyManagement (void)
{
  return m_phyMan;
}

void
GhnPlcDllMac::SetDllManagement (Ptr<GhnPlcDllManagement> ncDllManagement)
{
  NS_LOG_FUNCTION (this);
  m_dllMan = ncDllManagement;
}

Ptr<GhnPlcDllManagement>
GhnPlcDllMac::GetDllManagement (void)
{
  return m_dllMan;
}

void
GhnPlcDllMac::SetDllLlc (Ptr<GhnPlcDllLlc> ncDllLlc)
{
  m_ncDllLlc = ncDllLlc;
}

Ptr<GhnPlcDllLlc>
GhnPlcDllMac::GetDllLlc (void)
{
  return m_ncDllLlc;
}

void
GhnPlcDllMac::SetMpduForwardDownCallback (MpduForwardDownCallback cb)
{
  m_forwardDown = cb;
}

void
GhnPlcDllMac::SetLpduForwardUpCallback (LpduForwardUpCallback cb)
{
  m_forwardUp = cb;
}

void
GhnPlcDllMac::SetMacCycleBegin (Time macBegin)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("MAC cycle begin: " << macBegin << " ns");
  m_macBegin = macBegin;
}

bool
GhnPlcDllMac::SendAck (ConnId connId)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT(!m_setMcsCallback.IsNull() && !m_setTxPsdCallback.IsNull());

  NS_LOG_UNCOND ("ACK packet size: " << m_transPacket->GetSize () << ", connId: " << connId);

  m_sentAck = true;
  SetState (SEND_ACK);

  auto rt = m_dllMan->GetRoutingTable ();
  auto bl = m_dllMan->GetBitLoadingTable ();
  uint32_t dId = rt->GetIdByAddress (connId.src);
  uint32_t sId = rt->GetIdByAddress (connId.dst);
  uint32_t ownId = rt->GetIdByAddress (m_dllMan->GetAddress ());

  m_phyMan->SetTxSourceId (sId);
  m_phyMan->SetTxDestinationId (dId);
  m_phyMan->SetTxConnectionIdentifier (connId.flowId);

  ModulationAndCodingScheme mcs (bl->GetModulationAndCodingScheme (ownId, dId));

  m_setMcsCallback (mcs);

  NS_LOG_DEBUG("Setting MCS: " << mcs);
  m_phyMan->SetTxPayloadFecRate (ConvertPlcRateToGhnRate (mcs.ct));
  m_phyMan->SetTxRepetitionsNumber (ENCODING_REPETITIONS_1);

  m_dllMan->SetTxPsd (bl->GetTxPsd (ownId, dId));

  m_setTxPsdCallback (bl->GetTxPsd (ownId, dId));

  m_forwardDown (PHY_FRAME_ACK, m_transPacket);

  return true;
}

bool
GhnPlcDllMac::StartTx (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("Simulation time: " << Simulator::Now ().GetNanoSeconds () << " ns");

  m_setTimeCallback (Simulator::Now ().GetMilliSeconds ());

  return DoStartTx ();
}

void
GhnPlcDllMac::NotifyTransmissionEnd (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("Simulation time: " << Simulator::Now ().GetNanoSeconds () << " ns");

  DoNotifyTransmissionEnd ();
}

bool
GhnPlcDllMac::Receive (GhnPlcPhyFrameType frameType, Ptr<Packet> packet, const UanAddress& source, const UanAddress& dest)
{
  m_blockSize = GHN_BLKSZ_540;
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC ("Packet size: " << packet->GetSize ());

  switch (frameType)
    {
  case PHY_FRAME_MAP_RMAP:
    break;
  case PHY_FRAME_MSG:
    break;
  case PHY_FRAME_ACK:
    break;
  case PHY_FRAME_RTS:
    break;
  case PHY_FRAME_CTS:
    break;
  case PHY_FRAME_CTMG:
    break;
  case PHY_FRAME_PROBE:
    break;
  case PHY_FRAME_ACKRQ:
    break;
  case PHY_FRAME_BMSG:
    break;
  case PHY_FRAME_BACK:
    break;
  case PHY_FRAME_ACTMG:
    break;
  case PHY_FRAME_FTE://should not be used in this simulation (is only for new/extended frame types)
    break;
    }

  return DoReceive (frameType, packet, source, dest, m_phyMan->GetRxConnectionIdentifier ());
}

Ptr<Packet>
GhnPlcDllMac::AssembleMpdu (GhnBuffer buffer)
{
  GhnBuffer::iterator it = buffer.begin ();
  Ptr<Packet> bigpacket = Create<Packet> ();
  while (it != buffer.end ())
    {
      bigpacket->AddAtEnd ((*it)->Copy ());
      it++;
    }
  return bigpacket;
}
GhnBuffer
GhnPlcDllMac::DisassembleMpdu (Ptr<Packet> mpdu)
{
  NS_LOG_FUNCTION (this << mpdu->GetSize());
  NS_ASSERT(mpdu->GetSize() % m_blockSize == 0 || mpdu->GetSize() % m_blockSize == m_blockSize);

  GhnBuffer buffer;

  while (mpdu->GetSize () != 0)
    {
      NS_LOG_DEBUG("Adding the segment: " << 0 << " " << m_blockSize);
      NS_ASSERT(mpdu->GetSize() >= m_blockSize);
      buffer.push_back ((mpdu->GetSize () == m_blockSize) ? mpdu : mpdu->CreateFragment (0, m_blockSize));
      if (mpdu->GetSize () == m_blockSize) break;
      mpdu->RemoveAtStart (m_blockSize);
    }
  NS_LOG_DEBUG("MPDU is disassembled into " << buffer.size() << " segments");
  return buffer;
}
void
GhnPlcDllMac::SetState (GhnPlcCsmaNodeState s)
{
  NS_LOG_FUNCTION(this << m_dllMan->GetAddress() << s);
  m_nodeState = s;
}
void
GhnPlcDllMac::CreateLogger ()
{
  m_aggr.push_back (CreateObject<FileAggregator> (m_resDir + "mac_rcv_bits_" + std::to_string (
          m_dllMan->GetAddress ().GetAsInt ()) + ".txt", FileAggregator::FORMATTED));
  auto aggr = *(m_aggr.end () - 1);
  aggr->Set5dFormat ("%.0f\t%.0f\t%.0f\t%.0f\t%.0f");
  aggr->Enable ();
  TraceConnect ("MpduBitsLog", "MpduBitsLogContext", MakeCallback (&FileAggregator::Write5d, aggr));
}

/////////////////////////////////////////////////////////////////////////////////////////////////
NS_OBJECT_ENSURE_REGISTERED (GhnPlcDllMacCsma);
/////////////////////////////////////////////////////////////////////////////////////////////////

TypeId
GhnPlcDllMacCsma::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::GhnPlcDllMacCsma") .SetParent<GhnPlcDllMac> () .AddConstructor<GhnPlcDllMacCsma> ();
  return tid;
}

GhnPlcDllMacCsma::GhnPlcDllMacCsma ()
{
  NS_LOG_FUNCTION (this);
  m_backoff = CreateObject<GhnPlcMacBackoff> ();
  m_mpduSeqNum = 0;
  m_uniformVar = CreateObject<UniformRandomVariable> ();
  m_minCw = 1;
  m_minCw = 5;
  m_allowCooperation = false;
}

GhnPlcDllMacCsma::~GhnPlcDllMacCsma ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
GhnPlcDllMacCsma::SetCcaRequestCallback (GhnPlcCcaRequestCallback c)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("Simulation time: " << Simulator::Now ());
  m_ccaRequest = c;
}

void
GhnPlcDllMacCsma::SetCcaCancelCallback (GhnPlcCcaCancelCallback c)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("Simulation time: " << Simulator::Now ().GetNanoSeconds () << " ns");
  m_ccaCancel = c;
}

void
GhnPlcDllMacCsma::CcaRequest (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("Simulation time: " << Simulator::Now ().GetNanoSeconds () << " ns");
  SetState (CCA);
  m_ccaRequest ();
}

void
GhnPlcDllMacCsma::CcaConfirm (PLC_PhyCcaResult status)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC (Simulator::Now () << " -> CcaConfirm");
  if (status == CHANNEL_CLEAR)
    {
      NS_LOG_DEBUG ("Channel is free");
      SetState (TX);
      StartTx ();
    }
  else
    {
      NS_LOG_DEBUG ("Channel is busy");
      if (m_nodeState != RX) SetState (READY);
      CheckStart ();
    }
}

void
GhnPlcDllMacCsma::DoNotifyTransmissionEnd (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("Simulation time: " << Simulator::Now ().GetNanoSeconds () << " ns");

  if (m_sentAck)
    {
      SetState (GAP);
      m_sentAck = false;
      m_lastEndTxEvent = Simulator::Schedule (MicroSeconds (GDOTHN_MIN_TIFG_DURATION), &GhnPlcDllMacCsma::EndTx, this);
    }
  else if (m_askedForAck)
    {
      SetState (WAIT_ACK);
      //      m_askedForAck = false;
      NS_LOG_DEBUG ("Wait for ACK");
      m_lastEndTxEvent = Simulator::Schedule (NanoSeconds (GDOTHN_TAIFGD) + NanoSeconds (GDOTHN_ACK_DURATION) + MicroSeconds (
              GDOTHN_MIN_TIFG_DURATION), &GhnPlcDllMacCsma::EndTx, this);
    }
  else
    {
      SetState (GAP);
      m_lastEndTxEvent = Simulator::Schedule (MicroSeconds (GDOTHN_MIN_TIFG_DURATION), &GhnPlcDllMacCsma::EndTx, this);
    }
  m_transPacket = 0;
}
void
GhnPlcDllMacCsma::DoNotifyTransmissionFailure (void)
{
  NS_LOG_FUNCTION (this << GetState());
  NS_LOG_DEBUG ("Simulation time: " << Simulator::Now ().GetNanoSeconds () << " ns");
  NS_LOG_DEBUG ("Flags: <sentAck> " << m_sentAck << " / <askedForAck> " << m_askedForAck << " / <isPacketZero> " << (m_transPacket == 0));
  if (m_sentAck)
    {
      NS_LOG_DEBUG ("No Ack retransmission after transmission failure");
      m_sentAck = false;
    }
  else if (m_askedForAck)
    {
      NS_LOG_DEBUG ("No Ack is waited after transmission failure");
      m_askedForAck = false;
    }
  else
    {
      NS_LOG_DEBUG ("The transmitting packet is not zeroed after the transmission failure");
    }

  SetState (GAP);
  m_lastEndTxEvent = Simulator::Schedule (MicroSeconds (GDOTHN_MIN_TIFG_DURATION), &GhnPlcDllMacCsma::EndTxFailure, this);
}

void
GhnPlcDllMacCsma::EndTx (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("Simulation time: " << Simulator::Now ().GetNanoSeconds () << " ns");

  NS_LOG_DEBUG("Simulation time: " << Simulator::Now () << ", Node " << (uint16_t)GetDllManagement()->GetAddress().GetAsInt() << " end TX");
  if (!m_dllMan->GetDllLlc ()->IsQueueEmpty ())
    {
      //increase backoff if not acknowledged
      SetState (BACKOFF);
      StartBackoff ();
    }
  else
    {
      NS_LOG_LOGIC ("GhnPlcDllMacCsma::EndTx: Queue is empty");
      SetState (READY);
    }
}
void
GhnPlcDllMacCsma::EndTxFailure (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("Simulation time: " << Simulator::Now ().GetNanoSeconds () << " ns");

  NS_LOG_UNCOND("Simulation time: " << Simulator::Now () << ", Node " << (uint16_t)GetDllManagement()->GetAddress().GetAsInt() << " end TX failure");
  if (!m_dllMan->GetDllLlc ()->IsQueueEmpty () || m_transPacket != 0)
    {
      //increase backoff if not acknowledged
      SetState (BACKOFF);
      StartBackoff ();
    }
  else
    {
      NS_LOG_LOGIC ("GhnPlcDllMacCsma::EndTx: Queue is empty");
      SetState (READY);
    }
}

void
GhnPlcDllMacCsma::DoNotifyReceiptionEnd (Time time)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("Simulation time: " << Simulator::Now ().GetNanoSeconds () << " ns");
  SetState (GAP);
  NS_LOG_LOGIC ("Cancel backoff event with evend id:" << m_lastBackoffEvent.GetUid ());
  Simulator::Cancel (m_lastBackoffEvent);
  Simulator::Cancel (m_lastEndTxEvent);
  NS_LOG_LOGIC ("Cancel last CCA request");
  m_ccaCancel ();

  //
  // by the reception end the reaction is the same as by the transmission end
  //
  m_lastEndTxEvent = Simulator::Schedule (time, &GhnPlcDllMacCsma::EndTx, this);
}

void
GhnPlcDllMacCsma::StopCsma (void)
{
  NS_LOG_LOGIC ("Cancel last backoff, last EndTx, and CCA request in order to stop CSMA");
  Simulator::Cancel (m_lastBackoffEvent);
  Simulator::Cancel (m_lastEndTxEvent);
  m_ccaCancel ();
}
uint64_t
GhnPlcDllMacCsma::GetBackoffSlots ()
{
  NS_LOG_FUNCTION (this);

  uint64_t backoffPeriod = m_uniformVar->GetValue (m_minCw, m_maxCw);
  NS_LOG_LOGIC ("CSMA/CA backoff period /slots: " << backoffPeriod);

  return backoffPeriod;
}

void
GhnPlcDllMacCsma::StartBackoff (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("Simulation time: " << Simulator::Now ().GetNanoSeconds () << " ns");

  if (!m_backoffCallback.IsNull ())
    {
      m_lastBackoffTime = m_backoffCallback () * m_backoffSlotDuration;
    }
  else
    {
      m_lastBackoffTime = m_backoff->GetBackoffTime ();
    }
  NS_LOG_UNCOND (Simulator::Now() << " -> Start contention, backing off for " << m_lastBackoffTime);
  m_lastBackoffEvent = Simulator::Schedule (m_lastBackoffTime, &GhnPlcDllMacCsma::CcaRequest, this);
  NS_LOG_LOGIC ("Schedule backoff event with event id:" << m_lastBackoffEvent.GetUid ());
}

void
GhnPlcDllMacCsma::DoCancelEvents (void)
{
  NS_LOG_LOGIC ("Cancel events");
  Simulator::Cancel (m_lastBackoffEvent);
  Simulator::Cancel (m_lastEndTxEvent);
  m_ccaCancel ();
}

bool
GhnPlcDllMacCsma::DoReceive (GhnPlcPhyFrameType frameType, Ptr<Packet> packet, const UanAddress& source,
        const UanAddress& dest, uint8_t flowId)
{
  ConnId connId (source, dest, flowId);
  NS_LOG_FUNCTION (this << connId << packet->GetSize ());

  if (m_aggr.empty ()) CreateLogger ();

  m_txAllowed = false;
  SetState (RX);

  m_backoff->RecalculateCw (NanoSeconds (m_phyMan->GetRxDomainId () * GDOTHN_IST));

  uint16_t padlen;

  switch (frameType)
    {
  case PHY_FRAME_MAP_RMAP:
    break;
  case PHY_FRAME_MSG:
    {
      GhnBuffer buffer = DisassembleMpdu (packet);

      if (dest == UanAddress::GetBroadcast ())
        {
          NS_LOG_LOGIC ("Received broadcast packet");
          DoNotifyReceiptionEnd (MicroSeconds (GDOTHN_MIN_TIFG_DURATION));
          m_forwardUp (buffer, connId);
          NS_LOG_LOGIC ("Send no ACK for broadcast packet");
        }
      else
        {
          auto get_bytes = [](GhnBuffer buffer)->uint32_t
            {
              uint32_t s = 0;
              for(auto p : buffer) s += p->GetSize();
              return s;
            };;
          m_mpduBytes (Simulator::Now ().GetMicroSeconds (), source.GetAsInt (), dest.GetAsInt (), flowId, get_bytes (buffer));
          //
          // the LLC layer decides if the ACK should be sent
          //
          GroupEncAckInfo info (m_forwardUp (buffer, connId));

          if (m_phyMan->GetRxReplyRequired () != 1)
            {
              DoNotifyReceiptionEnd (MicroSeconds (GDOTHN_MIN_TIFG_DURATION));
            }
          else
            {
              if (info.invalid == false)
                {
                  m_transPacket = GroupEncAckInfoToPkt (info);
                  DoCancelEvents ();
                  NS_LOG_LOGIC ("Scheduled GhnPlcDllMac::SendAck ()");
                  Simulator::Schedule (NanoSeconds (GDOTHN_TAIFGD), &GhnPlcDllMac::SendAck, this, connId);
                }
              else
                {
                  DoNotifyReceiptionEnd (NanoSeconds (GDOTHN_TAIFGD) + NanoSeconds (GDOTHN_ACK_DURATION) + MicroSeconds (
                          GDOTHN_MIN_TIFG_DURATION));
                }
            }
        }
      break;
    }
  case PHY_FRAME_ACK:
    {
      if (dest == m_dllMan->GetAddress ())
        {
          m_transPacket = 0;
          m_askedForAck = false;
          NS_LOG_UNCOND ("GhnPlcDllMacCsma: Received my ACK.");
          m_ncDllLlc->ReceiveAck (PktToGroupEncAckInfo (packet), connId);
        }
      else
        {
          NS_LOG_UNCOND ("GhnPlcDllMacCsma: Received not my ACK.");
        }
      DoNotifyReceiptionEnd (MicroSeconds (GDOTHN_MIN_TIFG_DURATION));
      break;
    }
  case PHY_FRAME_RTS:
  case PHY_FRAME_CTS:
  case PHY_FRAME_CTMG:
  case PHY_FRAME_PROBE:
  case PHY_FRAME_ACKRQ:
  case PHY_FRAME_BMSG:
  case PHY_FRAME_BACK:
  case PHY_FRAME_ACTMG:
  case PHY_FRAME_FTE:
  default:
    break;
    }

  return true;
}

bool
GhnPlcDllMacCsma::DoStartTx (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("Simulation time: " << Simulator::Now ().GetNanoSeconds () << " ns");

  m_txAllowed = false;
  SetState (TX);

  if (m_transPacket == 0)
    {
      m_sendTuple = m_ncDllLlc->SendDown ();

      NS_LOG_UNCOND("Node " << m_dllMan->GetAddress() << " TX buffer size: " << m_sendTuple.get_buffer().size()
              << ", connId: " << m_sendTuple.get_conn_id() << ", next hop: " << m_sendTuple.GetNextHopAddress());

      m_transPacket = AssembleMpdu (m_sendTuple.get_buffer ());

      NS_LOG_LOGIC ("Packet size: " << m_transPacket->GetSize());
      NS_ASSERT_MSG (m_transPacket != 0, "GhnPlcDllMac::StartTx(): IsEmpty false but no Packet on queue?");
    }
  else
    {
      NS_LOG_DEBUG("Re-sending the MPDU after the transmission failure");
      NS_ASSERT(m_sendTuple.get_buffer().size() != 0);
    }
  uint16_t fecBlockSize = m_phyMan->GetTxFecBlockSizeFromHeader () / 8;
  ConnId connId = m_sendTuple.get_conn_id ();

  if (connId.dst != UanAddress::GetBroadcast ())
    {
      m_lastTxMulticastIndication = 0;

      if (!m_allowCooperation || connId.flowId == MANAGMENT_CONN_ID)
        {
          m_askedForAck = true;
          m_lastTxReplyRequired = 1;
          NS_LOG_LOGIC ("Send unicast packet");
          NS_LOG_LOGIC ("Ask for ACK.");
        }
      else
        {
          m_askedForAck = false;
          m_lastTxReplyRequired = 0;
          NS_LOG_LOGIC ("Cooperation is allows. Not ACK is acquired");
        }
    }
  else
    {
      m_lastTxMulticastIndication = 1;
      m_askedForAck = false;
      m_lastTxReplyRequired = 0;
      NS_LOG_LOGIC ("Send broadcast packet");
    }

  m_backoff->RecalculateCw (m_lastBackoffTime);

  ConfigurePhy (m_sendTuple);

  return m_forwardDown (PHY_FRAME_MSG, m_transPacket);
}
void
GhnPlcDllMacCsma::TriggerSend ()
{
  NS_LOG_FUNCTION (this << m_nodeState << m_askedForAck);
  NS_LOG_DEBUG ("Simulation time: " << Simulator::Now ().GetNanoSeconds () << " ns");
  if (m_nodeState == READY)
    {
      SetState (BACKOFF);
      StartBackoff ();
    }
  else
    {
      NS_LOG_DEBUG ("Current node sate is NOT READY");
    }
}
void
GhnPlcDllMacCsma::ConfigurePhy (SendTuple st)
{
  NS_ASSERT(!m_setMcsCallback.IsNull() && !m_setTxPsdCallback.IsNull());

  auto connId = st.get_conn_id();
  auto rt = m_dllMan->GetRoutingTable ();
  auto bl = m_dllMan->GetBitLoadingTable ();
  auto nh = st.GetNextHopAddress();

  m_lastTxSourceId = rt->GetIdByAddress (connId.src);
  m_destinationId = rt->GetIdByAddress (connId.dst);

  m_phyMan->SetTxMulticastIndication (m_lastTxMulticastIndication);
  m_phyMan->SetTxReplyRequired (m_lastTxReplyRequired);
  m_phyMan->SetTxDomainId (m_lastBackoffTime.GetNanoSeconds () / GDOTHN_IST); //used to send the current winner backoff
  m_phyMan->SetTxSourceId (m_lastTxSourceId);
  m_phyMan->SetTxDestinationId (m_destinationId);
  m_phyMan->SetTxConnectionIdentifier (connId.flowId);
  m_phyMan->SetMcAckSlotsNumber (m_mpduSeqNum++);

  uint32_t nextId = rt->GetIdByAddress (nh);
  ModulationAndCodingScheme mcs (bl->GetModulationAndCodingScheme (m_lastTxSourceId, nextId));
  m_setMcsCallback (mcs);

  NS_LOG_UNCOND("Setting MCS: " << mcs);
  m_phyMan->SetTxPayloadFecRate (ConvertPlcRateToGhnRate (mcs.ct));
  m_phyMan->SetTxRepetitionsNumber (ENCODING_REPETITIONS_1);

  m_dllMan->SetTxPsd (bl->GetTxPsd (m_lastTxSourceId, nextId));
  m_setTxPsdCallback (bl->GetTxPsd (m_lastTxSourceId, nextId));
}

/////////////////////////////////////////////////////////////////////////////////////////////////
NS_OBJECT_ENSURE_REGISTERED (GhnPlcDllMacCsmaCd);
/////////////////////////////////////////////////////////////////////////////////////////////////
TypeId
GhnPlcDllMacCsmaCd::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::GhnPlcDllMacCsmaCd") .SetParent<GhnPlcDllMacCsma> () .AddConstructor<GhnPlcDllMacCsmaCd> ();
  return tid;
}

GhnPlcDllMacCsmaCd::GhnPlcDllMacCsmaCd ()
{

}

GhnPlcDllMacCsmaCd::~GhnPlcDllMacCsmaCd ()
{

}

void
GhnPlcDllMacCsmaCd::CollisionDetection ()
{
  NS_LOG_FUNCTION (this << GetState());

  NS_LOG_LOGIC ("Collision detected");

  NS_ASSERT_MSG(GetState () == TX || GetState() == SEND_ACK, "MAC State: " << GetState ());

  DoNotifyTransmissionFailure ();
}
uint64_t
GhnPlcDllMacCsmaCd::GetBackoffSlots ()
{
  NS_LOG_FUNCTION (this);

  uint64_t backoffPeriod = m_uniformVar->GetValue (m_minCw, m_maxCw);
  NS_LOG_LOGIC ("CSMA/CA backoff period /slots: " << backoffPeriod);

  return backoffPeriod;
}

std::ostream&
operator<< (std::ostream& os, GhnPlcCsmaNodeState state)
{
  switch (state)
    {
  case (GhnPlcCsmaNodeState::READY):
    {
      os << "READY";
      break;
    }
  case (GhnPlcCsmaNodeState::TX):
    {
      os << "TX";
      break;
    }
  case (GhnPlcCsmaNodeState::RX):
    {
      os << "RX";
      break;
    }
  case (GhnPlcCsmaNodeState::BACKOFF):
    {
      os << "BACKOFF";
      break;
    }
  case (GhnPlcCsmaNodeState::SEND_ACK):
    {
      os << "SEND_ACK";
      break;
    }
  case (GhnPlcCsmaNodeState::WAIT_ACK):
    {
      os << "WAIT_ACK";
      break;
    }
  case (GhnPlcCsmaNodeState::GAP):
    {
      os << "GAP";
      break;
    }
  default:
    {
      os << "INVALID STATE";
      break;
    }
    }
  return os;
}

}
} // namespace ns3
