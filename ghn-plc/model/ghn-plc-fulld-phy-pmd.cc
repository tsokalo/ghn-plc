/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 TUD
 *
 *  Created on: 25.06.2013
 *      Author: Stanislav Mudriievskyi <stanislav.mudriievskyi@tu-dresden.de>
 */
#include "ns3/log.h"
#include "ns3/nstime.h"
#include "ns3/simulator.h"

#include "ghn-plc-fulld-phy-pmd.h"

NS_LOG_COMPONENT_DEFINE("GhnPlcPhyPmdFullD");

namespace ns3
{
namespace ghn
{
NS_OBJECT_ENSURE_REGISTERED(GhnPlcPhyPmdFullD);

TypeId
GhnPlcPhyPmdFullD::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::GhnPlcPhyPmdFullD").SetParent<PLC_InfRateFDPhy> ().AddConstructor<GhnPlcPhyPmdFullD> ()

  .AddTraceSource ("TxDurationLog", "Duration of TX frame", MakeTraceSourceAccessor (&GhnPlcPhyPmdFullD::m_txDurationLogTrace),
          "ns3::TxDurationLog::TracedCallback");
  return tid;
}

GhnPlcPhyPmdFullD::GhnPlcPhyPmdFullD ()
{
  NS_LOG_FUNCTION(this);
  SetBandplan (GDOTHN_BANDPLAN_25MHZ);
  SetHeaderModulationAndCodingScheme (ModulationAndCodingScheme (QAM4, CODING_RATE_1_2, 0));
  m_incommingFrameSize = 0;
}

GhnPlcPhyPmdFullD::~GhnPlcPhyPmdFullD ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
GhnPlcPhyPmdFullD::SetBandplan (BandPlanType bandplan)
{
  m_bandplan = bandplan;
}

BandPlanType
GhnPlcPhyPmdFullD::GetBandplan (void)
{
  return m_bandplan;
}

bool
GhnPlcPhyPmdFullD::Send (Ptr<Packet> txPhyFrame, GhnPlcPhyFrameType frameType, uint8_t headerSymbols, uint32_t payloadSymbols)
{
  NS_LOG_FUNCTION(this << txPhyFrame);

  if (m_aggr.empty ()) CreateLogger ();

  //Append preamble
  NS_LOG_LOGIC("Adding preamble...");
  txPhyFrame->AddHeader (PLC_Preamble ());

  // Create meta information object
  Ptr<PLC_TrxMetaInfo> metaInfo = Create<PLC_TrxMetaInfo> ();
  metaInfo->SetMessage (txPhyFrame);
  metaInfo->SetFrame (txPhyFrame);
  metaInfo->SetHeaderDuration (PLC_Phy::GetHeaderSymbolDuration () * headerSymbols);
  metaInfo->SetPayloadDuration (PLC_Phy::GetSymbolDuration () * payloadSymbols);
  metaInfo->SetHeaderMcs (GetHeaderModulationAndCodingScheme ());
  metaInfo->SetPayloadMcs (GetPayloadModulationAndCodingScheme ());

  Time tx_duration = metaInfo->GetHeaderDuration () + metaInfo->GetPayloadDuration () + PLC_Preamble::GetDuration ();
  NS_LOG_LOGIC("header_duration: " << metaInfo->GetHeaderDuration ());
  NS_LOG_LOGIC("payload_duration: " << metaInfo->GetPayloadDuration ());
  NS_LOG_LOGIC("preamble_duration: " << PLC_Preamble::GetDuration ());
  NS_LOG_UNCOND("tx_duration: " << tx_duration);

  auto src = m_ghnPhyPma->GetPhyManagement ()->GetAddress ().GetAsInt();
  m_txDurationLogTrace (src, tx_duration.GetMicroSeconds ());

  NS_LOG_UNCOND("Starting frame sending... State: " << GetState());

  return DoStartTx (metaInfo);
}

void
GhnPlcPhyPmdFullD::SetPhyPma (Ptr<GhnPlcPhyPma> ghnPhyPma)
{
  m_ghnPhyPma = ghnPhyPma;
}

Ptr<GhnPlcPhyPma>
GhnPlcPhyPmdFullD::GetPhyPma (void)
{
  return m_ghnPhyPma;
}

void
GhnPlcPhyPmdFullD::StartReception (uint32_t txId, Ptr<const SpectrumValue> rxPsd, Time duration,
        Ptr<const PLC_TrxMetaInfo> metaInfo)
{
  NS_LOG_FUNCTION(this << txId << rxPsd << duration << metaInfo);
  NS_LOG_UNCOND("Starting frame reception... State: " << GetState());

  // Determine uncoded header bits
  ModulationAndCodingScheme header_mcs = metaInfo->GetHeaderMcs ();
  size_t uncoded_header_bits = GDOTHN_PHY_HEADER * 8;

  // Receive header
  Time header_duration = metaInfo->GetHeaderDuration ();
  NS_LOG_LOGIC("header duration: " << header_duration);

  // header is not FEC encoded => no coding overhead
  m_information_rate_model->SetCodingOverhead (0);
  m_information_rate_model->StartRx (header_mcs, rxPsd, uncoded_header_bits);
  m_receptionEndEvent = Simulator::Schedule (header_duration, &GhnPlcPhyPmdFullD::EndRxHeader, this, txId, rxPsd, metaInfo);
}

void
GhnPlcPhyPmdFullD::EndRxHeader (uint32_t txId, Ptr<const SpectrumValue> rxPsd, Ptr<const PLC_TrxMetaInfo> metaInfo)
{
  NS_LOG_FUNCTION(this << metaInfo << GetState ());

  if (GetState () == IDLE) return;
  NS_ASSERT_MSG(GetState () == RX || GetState () == TXRX, "PHY state: " << GetState());

  ModulationAndCodingScheme payload_mcs = metaInfo->GetPayloadMcs ();
  Time payload_duration = metaInfo->GetPayloadDuration ();

  SetPayloadModulationAndCodingScheme (payload_mcs);

  if (m_information_rate_model->EndRx ())
    {
      NS_LOG_INFO("Successfully received header, starting payload reception");

      ModulationAndCodingScheme payload_mcs = metaInfo->GetPayloadMcs ();

      // Remove PHY header common core part
      m_incoming_frame->RemoveHeader (m_rxPhyHeaderCoreCommon);
      GhnPlcPhyFrameType frameType = m_rxPhyHeaderCoreCommon.GetFrameType ();

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

      switch (frameType)
        {
      case PHY_FRAME_MAP_RMAP:
        m_incoming_frame->RemoveHeader (header201);
        break;
      case PHY_FRAME_MSG:
        m_incoming_frame->RemoveHeader (header202);
        break;
      case PHY_FRAME_ACK:
        m_incoming_frame->RemoveHeader (header202);
        m_incoming_frame->RemoveHeader (header203);
        break;
      case PHY_FRAME_RTS:
        m_incoming_frame->RemoveHeader (header204);
        break;
      case PHY_FRAME_CTS:
        m_incoming_frame->RemoveHeader (header205);
        break;
      case PHY_FRAME_CTMG:
        m_incoming_frame->RemoveHeader (header206);
        break;
      case PHY_FRAME_PROBE:
        m_incoming_frame->RemoveHeader (header207);
        break;
      case PHY_FRAME_ACKRQ:
        m_incoming_frame->RemoveHeader (header208);
        break;
      case PHY_FRAME_BMSG:
        m_incoming_frame->RemoveHeader (header209);
        break;
      case PHY_FRAME_BACK:
        m_incoming_frame->RemoveHeader (header210);
        break;
      case PHY_FRAME_ACTMG:
        m_incoming_frame->RemoveHeader (header211);
        break;
      case PHY_FRAME_FTE:
        m_incoming_frame->RemoveHeader (header212);
        break;
        }

      GhnPlcPhyFrameHeaderCoreCommonHcs header3;
      m_incoming_frame->RemoveHeader (header3);

      // If header is rateless encoded take coding overhead into account for reception
      if (payload_mcs.ct >= CODING_RATE_RATELESS)
        {
          m_information_rate_model->SetCodingOverhead (GetRatelessCodingOverhead ());
        }
      else
        {
          m_information_rate_model->SetCodingOverhead (0);
        }

      NS_LOG_INFO("Remaining rx time: " << payload_duration);

      size_t uncoded_payload_bits = m_incoming_frame->GetSize () * 8;
      m_incommingFrameSize = uncoded_payload_bits;

      if ((frameType == PHY_FRAME_ACK) || (frameType == PHY_FRAME_RTS) || (frameType == PHY_FRAME_CTS)
              || (frameType == PHY_FRAME_CTMG) || (frameType == PHY_FRAME_ACKRQ) || (frameType == PHY_FRAME_ACTMG)
              || (frameType == PHY_FRAME_FTE)) uncoded_payload_bits = 0;

      NS_LOG_LOGIC("Starting payload reception: " << payload_duration << payload_mcs << uncoded_payload_bits);
      //      NS_LOG_UNCOND ("Starting payload reception: " << payload_duration << payload_mcs << uncoded_payload_bits);

      m_incoming_frame->AddHeader (header3);

      switch (frameType)
        {
      case PHY_FRAME_MAP_RMAP:
        m_incoming_frame->AddHeader (header201);
        break;
      case PHY_FRAME_MSG:
        m_incoming_frame->AddHeader (header202);
        break;
      case PHY_FRAME_ACK:
        m_incoming_frame->AddHeader (header203);
        m_incoming_frame->AddHeader (header202);
        break;
      case PHY_FRAME_RTS:
        m_incoming_frame->AddHeader (header204);
        break;
      case PHY_FRAME_CTS:
        m_incoming_frame->AddHeader (header205);
        break;
      case PHY_FRAME_CTMG:
        m_incoming_frame->AddHeader (header206);
        break;
      case PHY_FRAME_PROBE:
        m_incoming_frame->AddHeader (header207);
        break;
      case PHY_FRAME_ACKRQ:
        m_incoming_frame->AddHeader (header208);
        break;
      case PHY_FRAME_BMSG:
        m_incoming_frame->AddHeader (header209);
        break;
      case PHY_FRAME_BACK:
        m_incoming_frame->AddHeader (header210);
        break;
      case PHY_FRAME_ACTMG:
        m_incoming_frame->AddHeader (header211);
        break;
      case PHY_FRAME_FTE:
        m_incoming_frame->AddHeader (header212);
        break;
        }

      m_incoming_frame->AddHeader (m_rxPhyHeaderCoreCommon);

      m_information_rate_model->StartRx (payload_mcs, rxPsd, uncoded_payload_bits);
      m_receptionEndEvent = Simulator::Schedule (payload_duration, &GhnPlcPhyPmdFullD::EndRxPayload, this, metaInfo);
      m_stateChangeEvent = Simulator::Schedule (payload_duration, &PLC_InfRateFDPhy::ChangeState, this, IDLE);
    }
  else
    {
      NS_LOG_INFO("Header reception failed, remaining signal treated as interference");

      NoiseStart (txId, rxPsd, payload_duration);

      m_receptionEndEvent = Simulator::Schedule (payload_duration, &PLC_InfRateFDPhy::ReceptionFailure, this);
      ChangeState (IDLE);
    }
}
void
GhnPlcPhyPmdFullD::EndRxPayload (Ptr<const PLC_TrxMetaInfo> metaInfo)
{
  NS_LOG_FUNCTION(this);
  if (m_information_rate_model->EndRx ())
    {
      // Successful payload reception
      NS_LOG_INFO("Message successfully decoded");
      NS_LOG_LOGIC("Decoded packet: " << *m_incoming_frame);

      if (!m_receive_success_cb.IsNull ())
        {
          m_receive_success_cb (metaInfo->GetMessage (), m_rxFch.GetMessageId ());
        }

      NotifySuccessfulReception ();

      m_incoming_frame = 0;
    }
  else
    {
      NS_LOG_INFO("Not able to decode datagram");
      NotifyPayloadReceptionFailed (metaInfo);
      ReceptionFailure ();
    }
}

void
GhnPlcPhyPmdFullD::CreateLogger ()
{
  auto src = m_ghnPhyPma->GetPhyManagement ()->GetAddress ().GetAsInt();

  m_aggr.push_back (
          CreateObject<FileAggregator> (m_resDir + "phy_frame_tx_duration_" + std::to_string (src) + ".txt",
                  FileAggregator::FORMATTED));
  auto aggr = *(m_aggr.end () - 1);
  aggr->Set2dFormat ("%.0f\t%.0f");
  aggr->Enable ();
  TraceConnect ("TxDurationLog", "TxDurationLogContext", MakeCallback (&FileAggregator::Write2d, aggr));
}
}
} // namespace ns3
