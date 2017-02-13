/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 TUD
 *
 *  Created on: 26.06.2013
 *      Author: Stanislav Mudriievskyi <stanislav.mudriievskyi@tu-dresden.de>
 */

#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/plc-header.h"

#include "ghn-plc-phy-pcs.h"

NS_LOG_COMPONENT_DEFINE ("GhnPlcPhyPcs");

namespace ns3
{
namespace ghn {
NS_OBJECT_ENSURE_REGISTERED (GhnPlcPhyPcs);

TypeId
GhnPlcPhyPcs::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::GhnPlcPhyPcs") .SetParent<Object> () .AddConstructor<GhnPlcPhyPcs> ()

  .AddTraceSource ("PhyRcvFailureLog", "Receive failure either of PHY Preamble or PHY header or PHY payload",
          MakeTraceSourceAccessor (&GhnPlcPhyPcs::m_phyRcvFailureLog), "ns3::PhyRcvFailureLog::TracedCallback")

  .AddTraceSource ("PhyRcvSuccessLog", "Successful receiving of PHY Preamble, header and payload", MakeTraceSourceAccessor (
          &GhnPlcPhyPcs::m_phyRcvSuccessLog), "ns3::PhyRcvSuccessLog::TracedCallback");
  return tid;
}

GhnPlcPhyPcs::GhnPlcPhyPcs ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

GhnPlcPhyPcs::~GhnPlcPhyPcs ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

bool
GhnPlcPhyPcs::StartTx (GhnPlcPhyFrameType frameType, Ptr<const Packet> packet)
{
  NS_LOG_FUNCTION_NOARGS ();

  Ptr<Packet> mpdu = Create<Packet> ();
  mpdu = packet->Copy ();

  NS_LOG_DEBUG ("Tx frame size without header: " << mpdu->GetSize ());
  NS_ASSERT_MSG (mpdu->GetSize () * 8 % m_ghnPhyManagement->GetTxFecBlockSizeFromHeader() == 0, "Wrong packet size! Must be multiple of FEC block!");

  m_ghnPhyManagement->SetTxPacketSize (mpdu->GetSize ());

  GhnPlcPhyFrameHeaderCoreCommonHcs header3;
  mpdu->AddHeader (header3);

  GhnPlcPhyFrameHeaderCoreVariableMapRmap header201;
  GhnPlcPhyFrameHeaderCoreVariableMsg header202;
  GhnPlcPhyFrameHeaderCoreVariableAck header203;
  GhnPlcPhyFrameHeaderCoreVariableRts header204;
  GhnPlcPhyFrameHeaderCoreVariableCts header205;
  GhnPlcPhyFrameHeaderCoreVariableCtmg header206;
  GhnPlcPhyFrameHeaderCoreVariableProbe header207;
  GhnPlcPhyFrameHeaderCoreVariableAckrq header208;
  GhnPlcPhyFrameHeaderCoreVariableBmsg header209;
  GhnPlcPhyFrameHeaderCoreVariableBack header210;
  GhnPlcPhyFrameHeaderCoreVariableActmg header211;
  GhnPlcPhyFrameHeaderCoreVariableFte header212;

  GhnPlcPhyFrameHeaderCoreCommon header1;
  header1.SetFrameType (frameType);
  header1.SetDomainId (m_ghnPhyManagement->GetTxDomainId ());
  header1.SetSourceId (m_ghnPhyManagement->GetTxSourceId ());
  header1.SetDestinationId (m_ghnPhyManagement->GetTxDestinationId ());
  header1.SetMulticastIndication (m_ghnPhyManagement->GetTxMulticastIndication ());
  header1.SetHeaderSegmentationIndication (m_ghnPhyManagement->GetTxHeaderSegmentationIndication ());
  header1.SetExtendedHeaderIndication (m_ghnPhyManagement->GetTxExtendedHeaderIndication ());

  Time txTime = m_ghnPhyManagement->GetTxTime (m_ghnPhyManagement->GetTxPacketSize (), 0, 0);
  NS_LOG_LOGIC ("Tx time, us: " << txTime.GetMicroSeconds ());

  switch (frameType)
    {
  case PHY_FRAME_MAP_RMAP:
    header1.SetDurationIndication (1);
    header201.SetFecBlockSize (m_ghnPhyManagement->GetTxFecBlockSize ());
    header201.SetRepetitionsNumber (m_ghnPhyManagement->GetTxRepetitionsNumber ());
    mpdu->AddHeader (header201);
    break;
  case PHY_FRAME_MSG:
    header1.SetDurationIndication (1);
    header202.SetFecBlockSize (m_ghnPhyManagement->GetTxFecBlockSize ());
    header202.SetFecCodingRate (m_ghnPhyManagement->GetTxPayloadFecRate ());
    header202.SetRepetitionsNumber (m_ghnPhyManagement->GetTxRepetitionsNumber ());
    header202.SetMsgFrameDuration (m_ghnPhyManagement->GetTxFrameDuration ());
    header202.SetBrurqStartSsn (txTime.GetMicroSeconds ()); //used for transmission time currently
    header202.SetReplyRequired (m_ghnPhyManagement->GetTxReplyRequired ());
    header202.SetConnectionIdentifier (m_ghnPhyManagement->GetTxConnectionIdentifier ());
    m_ghnPhyManagement->SetRxTime (m_ghnPhyManagement->GetRxTime () + MicroSeconds (header202.GetBrurqStartSsn ()));
    mpdu->AddHeader (header202);
    break;
  case PHY_FRAME_ACK:
    header1.SetDurationIndication (0);
    mpdu->AddHeader (header203);

    header202.SetFecBlockSize (m_ghnPhyManagement->GetTxFecBlockSize ());
    header202.SetFecCodingRate (m_ghnPhyManagement->GetTxPayloadFecRate ());
    header202.SetRepetitionsNumber (m_ghnPhyManagement->GetTxRepetitionsNumber ());
    header202.SetConnectionIdentifier (m_ghnPhyManagement->GetTxConnectionIdentifier ());
    mpdu->AddHeader (header202);
    break;
  case PHY_FRAME_RTS:
    header1.SetDurationIndication (1);
    mpdu->AddHeader (header204);
    break;
  case PHY_FRAME_CTS:
    header1.SetDurationIndication (1);
    mpdu->AddHeader (header205);
    break;
  case PHY_FRAME_CTMG:
    header1.SetDurationIndication (0);
    header206.SetCtmgData16 (m_ghnPhyManagement->GetTxCtmgData16 ());
    mpdu->AddHeader (header206);
    break;
  case PHY_FRAME_PROBE:
    header1.SetDurationIndication (1);
    mpdu->AddHeader (header207);
    break;
  case PHY_FRAME_ACKRQ:
    header1.SetDurationIndication (0);
    mpdu->AddHeader (header208);
    break;
  case PHY_FRAME_BMSG:
    header1.SetDurationIndication (1);
    mpdu->AddHeader (header209);
    break;
  case PHY_FRAME_BACK:
    header1.SetDurationIndication (1);
    mpdu->AddHeader (header210);
    break;
  case PHY_FRAME_ACTMG:
    header1.SetDurationIndication (0);
    mpdu->AddHeader (header211);
    break;
  case PHY_FRAME_FTE://should not be used in this simulation (is only for new/extended frame types)
    header1.SetDurationIndication (0);//may be also 1
    mpdu->AddHeader (header212);
    break;
    }

  mpdu->AddHeader (header1);

  NS_LOG_DEBUG ("Tx frame size with header: " << mpdu->GetSize ());

  return m_ghnPhyPma->StartTx (mpdu, frameType);
}

void
GhnPlcPhyPcs::ReceiveSuccess (Ptr<const Packet> txPhyFrame, uint16_t msgId)
{
  NS_LOG_FUNCTION (this << Simulator::Now () << ": Packet received!");

//  m_phyRcvSuccessLog (this->GetPhyManagement()->GetPhyPmd()->GetNode()->GetVertexId());

  m_rxPacket = txPhyFrame->Copy ();
  //  PLC_Preamble preamble;
  //  m_rxPacket->RemoveHeader(preamble);

  GhnPlcPhyFrameHeaderCoreCommon header1;
  m_rxPacket->RemoveHeader (header1);
  GhnPlcPhyFrameType frameType = header1.GetFrameType ();
  NS_LOG_LOGIC ("Frame type: " << frameType);
  uint8_t sourceId = header1.GetSourceId ();
  NS_LOG_LOGIC ("Source ID: " << sourceId);
  uint8_t destinationId = header1.GetDestinationId ();
  NS_LOG_LOGIC ("Destination ID: " << destinationId);
  uint8_t domainId = header1.GetDomainId (); //used for winner backoff here
  NS_LOG_LOGIC ("Domain ID: " << domainId);
  m_ghnPhyManagement->SetRxDomainId (domainId);

  GhnPlcPhyFrameHeaderCoreVariableMapRmap header201;
  GhnPlcPhyFrameHeaderCoreVariableMsg header202;
  GhnPlcPhyFrameHeaderCoreVariableAck header203;
  GhnPlcPhyFrameHeaderCoreVariableRts header204;
  GhnPlcPhyFrameHeaderCoreVariableCts header205;
  GhnPlcPhyFrameHeaderCoreVariableCtmg header206;
  GhnPlcPhyFrameHeaderCoreVariableProbe header207;
  GhnPlcPhyFrameHeaderCoreVariableAckrq header208;
  GhnPlcPhyFrameHeaderCoreVariableBmsg header209;
  GhnPlcPhyFrameHeaderCoreVariableBack header210;
  GhnPlcPhyFrameHeaderCoreVariableActmg header211;
  GhnPlcPhyFrameHeaderCoreVariableFte header212;

  uint8_t currentTs;
  currentTs = 1;
  if (currentTs == 1) currentTs = 3;

  switch (frameType)
    {
  case PHY_FRAME_MAP_RMAP:
    m_rxPacket->RemoveHeader (header201);
    break;
  case PHY_FRAME_MSG:
    m_rxPacket->RemoveHeader (header202);
    currentTs = header202.GetCurrentTs ();
    m_ghnPhyManagement->SetRxFrameDuration (header202.GetMsgFrameDuration ());
    m_ghnPhyManagement->SetRxTime (m_ghnPhyManagement->GetRxTime () + MicroSeconds (header202.GetBrurqStartSsn ()));
    m_ghnPhyManagement->SetRxReplyRequired (header202.GetReplyRequired ());
    m_ghnPhyManagement->SetRxConnectionIdentifier (header202.GetConnectionIdentifier ());
    break;
  case PHY_FRAME_ACK:
    m_rxPacket->RemoveHeader (header202);
    m_ghnPhyManagement->SetRxConnectionIdentifier (header202.GetConnectionIdentifier ());

    m_rxPacket->RemoveHeader (header203);
    m_ghnPhyManagement->SetRxAckReceived (true);
    break;
  case PHY_FRAME_RTS:
    m_rxPacket->RemoveHeader (header204);
    break;
  case PHY_FRAME_CTS:
    m_rxPacket->RemoveHeader (header205);
    break;
  case PHY_FRAME_CTMG:
    m_rxPacket->RemoveHeader (header206);
    m_ghnPhyManagement->SetRxCtmgData16 (header206.GetCtmgData16 ());
    break;
  case PHY_FRAME_PROBE:
    m_rxPacket->RemoveHeader (header207);
    break;
  case PHY_FRAME_ACKRQ:
    m_rxPacket->RemoveHeader (header208);
    break;
  case PHY_FRAME_BMSG:
    m_rxPacket->RemoveHeader (header209);
    break;
  case PHY_FRAME_BACK:
    m_rxPacket->RemoveHeader (header210);
    break;
  case PHY_FRAME_ACTMG:
    m_rxPacket->RemoveHeader (header211);
    break;
  case PHY_FRAME_FTE://should not be used in this simulation (is only for new/extended frame types)
    m_rxPacket->RemoveHeader (header212);
    break;
    }

  GhnPlcPhyFrameHeaderCoreCommonHcs header3;
  m_rxPacket->RemoveHeader (header3);
  NS_LOG_LOGIC (m_rxPacket->GetSize () * 8.0 / MicroSeconds (header202.GetBrurqStartSsn ()).GetSeconds () << " bps");

  //here call the Receive from MAC with (m_rxPacket, frameType, sourceId, destinationId, currentTs)
  m_forwardUp (frameType, m_rxPacket, UanAddress (sourceId), UanAddress (destinationId));
}
void
GhnPlcPhyPcs::ReceiveFailure ()
{
//  m_phyRcvFailureLog (this->GetPhyManagement()->GetPhyPmd()->GetNode()->GetVertexId());
}

void
GhnPlcPhyPcs::SetPhyManagement (Ptr<GhnPlcPhyManagement> ghnPhyManagement)
{
  m_ghnPhyManagement = ghnPhyManagement;
}

Ptr<GhnPlcPhyManagement>
GhnPlcPhyPcs::GetPhyManagement (void)
{
  return m_ghnPhyManagement;
}

void
GhnPlcPhyPcs::SetPhyPma (Ptr<GhnPlcPhyPma> ghnPhyPma)
{
  m_ghnPhyPma = ghnPhyPma;
}

Ptr<GhnPlcPhyPma>
GhnPlcPhyPcs::GetPhyPma (void)
{
  return m_ghnPhyPma;
}

void
GhnPlcPhyPcs::SetCurrentTs (uint8_t currentTs)
{
  m_currentTs = currentTs;
}
uint8_t
GhnPlcPhyPcs::GetCurrentTs (void)
{
  return m_currentTs;
}

void
GhnPlcPhyPcs::SetMpduForwardUpCallback (MpduForwardUpCallback cb)
{
  m_forwardUp = cb;
}
uint32_t
GhnPlcPhyPcs::GetDataAmount (Time txTime, uint8_t sourceId, uint8_t destinationId)
{
  uint32_t payloadSymbols =
          (txTime.GetInteger () - (m_ghnPhyManagement->GetTxHeaderSegmentationIndication () + 1)
                  * (PLC_Phy::GetHeaderSymbolDuration()).GetInteger ()
                  - PLC_Preamble::GetDuration ().GetInteger ())
                  / (PLC_Phy::GetSymbolDuration()).GetInteger ();
  uint32_t payloadEncodedBits = payloadSymbols * m_ghnPhyPma->GetBitsPerSymbol ();
  uint8_t repetitionsNumber = m_ghnPhyManagement->GetTxRepetitionsNumber ();
  uint32_t payloadUncodedBits = payloadEncodedBits / repetitionsNumber;
  switch (m_ghnPhyManagement->GetTxPayloadFecRate ())
    {
  case FEC_RATE_1_2:
    payloadUncodedBits = payloadUncodedBits * 1 / 2;
    break;
  case FEC_RATE_2_3:
    payloadUncodedBits = payloadUncodedBits * 2 / 3;
    break;
  case FEC_RATE_5_6:
    payloadUncodedBits = payloadUncodedBits * 5 / 6;
    break;
  case FEC_RATE_16_18:
    payloadUncodedBits = payloadUncodedBits * 16 / 18;
    break;
  case FEC_RATE_16_21:
    payloadUncodedBits = payloadUncodedBits * 16 / 21;
    break;
  case FEC_RATE_20_21:
    payloadUncodedBits = payloadUncodedBits * 20 / 21;
    break;
    }
  return payloadUncodedBits / 8;
}
}
} // namespace ns3
