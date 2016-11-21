/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 TUD
 *
 *  Created on: 25.06.2013
 *      Author: Stanislav Mudriievskyi <stanislav.mudriievskyi@tu-dresden.de>
 */
#include "ns3/log.h"
#include "ghn-plc-halfd-phy-pmd.h"
#include "ns3/nstime.h"
#include "ns3/simulator.h"
NS_LOG_COMPONENT_DEFINE ("GhnPlcPhyPmdHalfD");

namespace ns3
{
namespace ghn
{
NS_OBJECT_ENSURE_REGISTERED (GhnPlcPhyPmdHalfD);

TypeId
GhnPlcPhyPmdHalfD::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::GhnPlcPhyPmdHalfD") .SetParent<PLC_InformationRatePhy> () .AddConstructor<GhnPlcPhyPmdHalfD> ();
  return tid;
}

GhnPlcPhyPmdHalfD::GhnPlcPhyPmdHalfD ()
{
  NS_LOG_FUNCTION (this);
  SetBandplan (GDOTHN_BANDPLAN_25MHZ);
  SetHeaderModulationAndCodingScheme (ModulationAndCodingScheme (QAM4, CODING_RATE_1_2, 0));
  m_gdothnLinkPerformanceModel = CreateObject<GhnPlcLinkPerformanceModel> ();
  m_incommingFrameSize = 0;
}

GhnPlcPhyPmdHalfD::~GhnPlcPhyPmdHalfD ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
GhnPlcPhyPmdHalfD::SetBandplan (BandPlanType bandplan)
{
  m_bandplan = bandplan;
}

BandPlanType
GhnPlcPhyPmdHalfD::GetBandplan (void)
{
  return m_bandplan;
}

bool
GhnPlcPhyPmdHalfD::DoStartTx (Ptr<const Packet> ppdu)
{
  NS_LOG_FUNCTION(this << ppdu);
  NS_ASSERT_MSG(m_txPsd, "TxPsd not set!");
  NS_ASSERT_MSG(m_header_mcs.ct < CODING_RATE_RATELESS, "Header cannot be encoded rateless");

  NS_LOG_LOGIC ("Phy Protocol Data Unit: " << *ppdu);

  // Create meta information object
  Ptr<PLC_TrxMetaInfo> metaInfo = Create<PLC_TrxMetaInfo> ();
  metaInfo->SetMessage (ppdu);
  metaInfo->SetHeaderMcs (GetHeaderModulationAndCodingScheme ());
  metaInfo->SetPayloadMcs (GetPayloadModulationAndCodingScheme ());

  // (Virtually encode frame)
  PrepareTransmission (metaInfo);

  // Start sending
  if (GetState () == IDLE)
    {
      NS_LOG_INFO("Device idle, sending frame: " << *metaInfo);
      SendFrame (metaInfo);
      return true;
    }

  NS_LOG_INFO("Phy busy, cannot send frame");
  return false;
}

void
GhnPlcPhyPmdHalfD::PreparePhyFrame (Ptr<PLC_TrxMetaInfo> metaInfo)
{
  NS_LOG_FUNCTION (this << metaInfo);
  NS_ASSERT (metaInfo && metaInfo->GetMessage ());
  NS_ASSERT (metaInfo->GetHeaderMcs().ct < CODING_RATE_RATELESS && metaInfo->GetPayloadMcs ().ct < CODING_RATE_RATELESS);

  Ptr<Packet> frame = metaInfo->GetMessage ()->Copy ();
  ModulationAndCodingScheme header_mcs = metaInfo->GetHeaderMcs ();
  ModulationAndCodingScheme payload_mcs = metaInfo->GetPayloadMcs ();

  // (Virtually) encode packet
  size_t uncoded_header_bits = m_txFch.GetSerializedSize () * 8;
  size_t encoded_header_bits = EncodedBits (uncoded_header_bits, header_mcs.ct);
  size_t header_symbols = RequiredSymbols (encoded_header_bits, header_mcs.mt, GetNumSubcarriers ());
  Time header_duration = CalculateTransmissionDuration (header_symbols);

  NS_LOG_LOGIC ("symbol duration: " << GetSymbolDuration());
  NS_LOG_LOGIC ("encoded_header_bits: " << encoded_header_bits);
  NS_LOG_LOGIC ("header mcs: " << header_mcs);
  NS_LOG_LOGIC ("header symbols: " << header_symbols);
  NS_LOG_LOGIC ("header duration: " << header_duration);

  size_t uncoded_payload_bits = frame->GetSize () * 8;
  size_t encoded_payload_bits = EncodedBits (uncoded_payload_bits, payload_mcs.ct);
  size_t payload_symbols = RequiredSymbols (encoded_payload_bits, header_mcs.mt, GetNumSubcarriers ());
  Time payload_duration = CalculateTransmissionDuration (payload_symbols);

  NS_LOG_LOGIC ("encoded_payload_bits: " << encoded_payload_bits);
  NS_LOG_LOGIC ("payload_mcs: " << payload_mcs);
  NS_LOG_LOGIC ("payload symbols: " << header_symbols);
  NS_LOG_LOGIC ("payload duration: " << header_duration);

  // Write FCH
  m_txFch.SetMessageId (m_txMessageId++);
  m_txFch.SetPayloadMt (payload_mcs.mt);
  m_txFch.SetPayloadCt (payload_mcs.ct);
  m_txFch.SetPayloadSymbols (payload_symbols);

  // Append FCH
  frame->AddHeader (m_txFch);

  //Append preamble
  NS_LOG_LOGIC ("Adding preamble...");
  frame->AddHeader (PLC_Preamble ());

  // Complete metaInfo
  metaInfo->SetFrame (frame);
  metaInfo->SetHeaderDuration (header_duration);
  metaInfo->SetPayloadDuration (payload_duration);
}

bool
GhnPlcPhyPmdHalfD::Send (Ptr<Packet> txPhyFrame, GhnPlcPhyFrameType frameType, uint8_t headerSymbols, uint32_t payloadSymbols)
{
  NS_LOG_FUNCTION (this << txPhyFrame);

  //Append preamble
  NS_LOG_LOGIC ("Adding preamble...");
  txPhyFrame->AddHeader (PLC_Preamble ());

  // Create meta information object
  Ptr<PLC_TrxMetaInfo> metaInfo = Create<PLC_TrxMetaInfo> ();
  metaInfo->SetMessage (txPhyFrame);
  metaInfo->SetFrame (txPhyFrame); //or SetMessage() ?
  metaInfo->SetHeaderDuration (NanoSeconds (GetHeaderSymbolDuration ().GetNanoSeconds () * headerSymbols));

  //set symbol duration for header and payload in the scratch file
  if (payloadSymbols <= 2)
    metaInfo->SetPayloadDuration (NanoSeconds (GetHeaderSymbolDuration ().GetNanoSeconds () * payloadSymbols));
  else
    metaInfo->SetPayloadDuration (NanoSeconds (GetHeaderSymbolDuration ().GetNanoSeconds () * 2
            + GetSymbolDuration ().GetNanoSeconds () * (payloadSymbols - 2)));
  //  if (payloadSymbols >= 2)
  //    metaInfo->SetPayloadDuration (Time::FromInteger((uint64_t) ((m_ofdmParameters[m_bandplan].m_subcarriersNumber +
  //          m_ofdmParameters[m_bandplan].m_payloadDefaultGuardInterval) *
  //          m_ofdmParameters[m_bandplan].m_sampleDuration * 2 +
  //          (m_ofdmParameters[m_bandplan].m_subcarriersNumber +
  //           m_ofdmParameters[m_bandplan].m_payloadDefaultGuardInterval) *
  //           m_ofdmParameters[m_bandplan].m_sampleDuration * (payloadSymbols - 2)), Time::NS));
  //  else if (payloadSymbols == 1)
  //    metaInfo->SetPayloadDuration (Time::FromInteger((uint64_t) ((m_ofdmParameters[m_bandplan].m_subcarriersNumber +
  //          m_ofdmParameters[m_bandplan].m_payloadDefaultGuardInterval) *
  //          m_ofdmParameters[m_bandplan].m_sampleDuration), Time::NS));
  //  else
  //    metaInfo->SetPayloadDuration (Time::FromInteger((uint64_t) (0), Time::NS));

  metaInfo->SetHeaderMcs (GetHeaderModulationAndCodingScheme ());
  metaInfo->SetPayloadMcs (GetPayloadModulationAndCodingScheme ());

  // Start sending
  if (GetState () == IDLE)
    {
      //          NS_LOG_INFO("Device idle, sending frame: " << *metaInfo);

      NS_ASSERT_MSG (m_txInterface, "Phy has no tx interface");
      NS_ASSERT (metaInfo && metaInfo->GetFrame ());

      Time tx_duration = metaInfo->GetHeaderDuration () + metaInfo->GetPayloadDuration () + PLC_Preamble::GetDuration ();
      NS_LOG_LOGIC ("header_duration: " << metaInfo->GetHeaderDuration ().GetNanoSeconds ());
      NS_LOG_LOGIC ("payload_duration: " << metaInfo->GetPayloadDuration ().GetNanoSeconds ());
      NS_LOG_LOGIC ("preamble_duration: " << PLC_Preamble::GetDuration ().GetNanoSeconds ());
      NS_LOG_LOGIC ("tx_duration: " << tx_duration.GetNanoSeconds ());

      NS_LOG_LOGIC ("Start sending frame...");
      ChangeState (TX);
      m_txInterface->StartTx (m_txPsd, tx_duration, metaInfo);
      Simulator::Schedule (tx_duration, &PLC_HalfDuplexOfdmPhy::ChangeState, this, IDLE);
      Simulator::Schedule (tx_duration, &PLC_Phy::NotifyFrameSent, this);

      NS_LOG_LOGIC ("Transmission PSD: " << *m_txPsd);

      return true;
    }

  NS_LOG_INFO("Phy busy, cannot send frame");

  return false;
}

void
GhnPlcPhyPmdHalfD::SetPhyPma (Ptr<GhnPlcPhyPma> gdothnPhyPma)
{
  m_gdothnPhyPma = gdothnPhyPma;
}

Ptr<GhnPlcPhyPma>
GhnPlcPhyPmdHalfD::GetPhyPma (void)
{
  return m_gdothnPhyPma;
}

void
GhnPlcPhyPmdHalfD::StartReception (uint32_t txId, Ptr<const SpectrumValue> rxPsd, Time duration, Ptr<const PLC_TrxMetaInfo> metaInfo)
{
  NS_LOG_FUNCTION (this << txId << rxPsd << duration << metaInfo);
  NS_LOG_INFO ("Starting frame reception...");

  // Determine uncoded header bits
  ModulationAndCodingScheme header_mcs = metaInfo->GetHeaderMcs ();
  size_t uncoded_header_bits = GDOTHN_PHY_HEADER * 8;

  // Receive header
  Time header_duration = metaInfo->GetHeaderDuration ();
  NS_LOG_LOGIC ("header duration: " << header_duration);

  // header is not FEC encoded => no coding overhead
  m_information_rate_model->SetCodingOverhead (0);
  m_information_rate_model->StartRx (header_mcs, rxPsd, uncoded_header_bits);
  Simulator::Schedule (header_duration, &GhnPlcPhyPmdHalfD::EndRxHeader, this, txId, rxPsd, metaInfo);
}

void
GhnPlcPhyPmdHalfD::EndRxHeader (uint32_t txId, Ptr<const SpectrumValue> rxPsd, Ptr<const PLC_TrxMetaInfo> metaInfo)
{
  NS_LOG_FUNCTION (this << metaInfo);
  NS_ASSERT(GetState () == RX);

  ModulationAndCodingScheme payload_mcs = metaInfo->GetPayloadMcs ();
  Time payload_duration = metaInfo->GetPayloadDuration ();

  SetPayloadModulationAndCodingScheme (payload_mcs);

  if (m_information_rate_model->EndRx ())
    {
      NS_LOG_INFO ("Successfully received header, starting payload reception");

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

      NS_LOG_INFO ("Remaining rx time: " << payload_duration);

      size_t uncoded_payload_bits = m_incoming_frame->GetSize () * 8;
      m_incommingFrameSize = uncoded_payload_bits;

      if ((frameType == PHY_FRAME_ACK) || (frameType == PHY_FRAME_RTS) || (frameType == PHY_FRAME_CTS) || (frameType
              == PHY_FRAME_CTMG) || (frameType == PHY_FRAME_ACKRQ) || (frameType == PHY_FRAME_ACTMG) || (frameType
              == PHY_FRAME_FTE)) uncoded_payload_bits = 0;

      NS_LOG_LOGIC ("Starting payload reception: " << payload_duration << payload_mcs << uncoded_payload_bits);
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
      Simulator::Schedule (payload_duration, &GhnPlcPhyPmdHalfD::EndRxPayload, this, metaInfo);
      Simulator::Schedule (payload_duration, &PLC_HalfDuplexOfdmPhy::ChangeState, this, IDLE);
    }
  else
    {
      NS_LOG_INFO ("Header reception failed, remaining signal treated as interference");

      NoiseStart (txId, rxPsd, payload_duration);

      Simulator::Schedule (payload_duration, &PLC_InformationRatePhy::ReceptionFailure, this);
      ChangeState (IDLE);
    }
}
void
GhnPlcPhyPmdHalfD::EndRxPayload(Ptr<const PLC_TrxMetaInfo> metaInfo)
{
	NS_LOG_FUNCTION(this);
	bool endsuccess = m_information_rate_model->EndRx();

	// Successful payload reception
	NS_LOG_INFO("Message received: average status -> " << endsuccess);

	if (!m_receive_success_cb.IsNull())
	{
		m_receive_success_cb(metaInfo->GetMessage (), m_rxFch.GetMessageId());
	}

	NotifySuccessfulReception();

	m_incoming_frame = 0;
}
}
} // namespace ns3
