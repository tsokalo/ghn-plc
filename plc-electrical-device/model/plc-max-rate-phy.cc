/*
 * PlcMaxRatePhy.cpp
 *
 *  Created on: Oct 6, 2015
 *      Author: tsokalo
 */

#include "plc-max-rate-phy.h"
#include <ns3/log.h>
#include <ns3/random-variable-stream.h>
#include <ns3/simulator.h>

NS_LOG_COMPONENT_DEFINE ("PlcMaxRatePhy");

namespace ns3
{

#define PLC_PHY_CUSTOM_LOGIC(msg)\
        do {\
                if (m_node) {\
                        PLC_LOG_LOGIC("PLC_Phy[" << m_node->GetName() << "]: " << msg);\
                }\
                else {\
                        PLC_LOG_LOGIC("PLC_Phy: " << msg);\
                }\
        } while(0)

#define PLC_PHY_CUSTOM_FUNCTION(msg)\
        do {\
                if (m_node) {\
                        NS_LOG_FUNCTION(m_node->GetName() << msg);\
                }\
                else {\
                        NS_LOG_FUNCTION(msg);\
                }\
        } while(0)

#define PLC_PHY_CUSTOM_INFO(msg)\
        do {\
                if (m_node) {\
                        NS_LOG_INFO("PLC_Phy[" << m_node->GetName() << "]: " << msg);\
                }\
                else {\
                        NS_LOG_INFO("PLC_Phy: " << msg);\
                }\
        } while(0)

#define PLC_PHY_CUSTOM_DEBUG(msg)\
        do {\
                if (m_node) {\
                        PLC_LOG_DEBUG(m_node->GetName() << msg);\
                }\
                else {\
                        PLC_LOG_DEBUG(msg);\
                }\
        } while(0)


NS_OBJECT_ENSURE_REGISTERED (PlcMaxRatePhy);


size_t PlcMaxRatePhy::modulation_symbols_per_code_block = 1;
double PlcMaxRatePhy::rateless_coding_overhead = 0.2;

TypeId
PlcMaxRatePhy::GetTypeId (void)
{
  static TypeId tid =
          TypeId ("ns3::PlcMaxRatePhy") .SetParent<PLC_HalfDuplexOfdmPhy> () .AddConstructor<PlcMaxRatePhy> () .AddAttribute (
                  "MaxRxQueueSize", "Maximum size of RX queue for redundancy reception.", UintegerValue (5),
                  MakeUintegerAccessor (&PlcMaxRatePhy::m_maxRxQueueSize), MakeUintegerChecker<size_t> ());
  return tid;
}

PlcMaxRatePhy::PlcMaxRatePhy (void)
{
  PLC_PHY_CUSTOM_FUNCTION (this);
  m_header_mcs = ModulationAndCodingScheme (BPSK, CODING_RATE_1_2, 0);
  m_payload_mcs = ModulationAndCodingScheme (BPSK, CODING_RATE_1_2, 0);
  m_information_rate_model = CreateObject<PlcMaxRate> ();
  m_information_rate_model->SetSymbolDuration (GetSymbolDuration ());
  m_information_rate_model->SetGuardIntervalDuration (GetGuardIntervalDuration ());

  UniformRandomVariable u;
  m_txMessageId = u.GetInteger (1, 65535);
}

void
PlcMaxRatePhy::SetOfdmSymbolsPerCodeBlock (size_t symbols)
{
  modulation_symbols_per_code_block = symbols;
}

size_t
PlcMaxRatePhy::GetOfdmSymbolsPerCodeBlock (void)
{
  return modulation_symbols_per_code_block;
}

void
PlcMaxRatePhy::SetRatelessCodingOverhead (double overhead)
{
  NS_ASSERT (overhead >= 0);
  rateless_coding_overhead = overhead;
}

size_t
PlcMaxRatePhy::RequiredChunks (size_t num_blocks)
{
  PLC_PHY_CUSTOM_FUNCTION (this);
  size_t ret = ceil (num_blocks * (1 + rateless_coding_overhead));
  PLC_PHY_CUSTOM_INFO ("minimum required chunks to decode datagramm: " << ret);
  return ret;
}

size_t
PlcMaxRatePhy::ChunksInByte (size_t num_chunks, size_t raw_bits_per_symbol)
{
  PLC_PHY_CUSTOM_FUNCTION (this);
  return ceil (GetOfdmSymbolsPerCodeBlock () * raw_bits_per_symbol * num_chunks / (double) 8);
}

void
PlcMaxRatePhy::ReceptionFailure (void)
{
  PLC_PHY_CUSTOM_FUNCTION (this);
  if (!m_receive_error_cb.IsNull ())
    {
      m_receive_error_cb ();
    }
}

double
PlcMaxRatePhy::GetTransmissionRateLimit (Ptr<SpectrumValue> rxPsd)
{
  PLC_PHY_CUSTOM_FUNCTION (rxPsd);
  m_information_rate_model->Evaluate(rxPsd);
  return m_information_rate_model->GetTransmissionRate();
}
Ptr<SpectrumValue>
PlcMaxRatePhy::GetBpc ()
{
  return m_information_rate_model->GetBitLoadingTable();
}

void
PlcMaxRatePhy::DoStart (void)
{
  PLC_PHY_CUSTOM_FUNCTION (this);
  PLC_HalfDuplexOfdmPhy::DoStart ();
}

void
PlcMaxRatePhy::DoDispose (void)
{
  PLC_PHY_CUSTOM_FUNCTION (this);
  ChangeState( IDLE);
  m_information_rate_model = 0;
  m_frame_sent_callback = MakeNullCallback<void> ();
  PLC_HalfDuplexOfdmPhy::DoDispose ();
}

Ptr<PLC_LinkPerformanceModel>
PlcMaxRatePhy::DoGetLinkPerformanceModel (void)
{
  PLC_PHY_CUSTOM_FUNCTION (this);
  return m_information_rate_model;
}

bool
PlcMaxRatePhy::DoStartTx (Ptr<const Packet> ppdu)
{
  PLC_PHY_CUSTOM_FUNCTION (this << ppdu);
  NS_ASSERT_MSG(m_txPsd, "TxPsd not set!"); NS_ASSERT_MSG(m_header_mcs.ct < CODING_RATE_RATELESS, "Header cannot be encoded rateless");

  NS_LOG_LOGIC ("Phy Protocol Data Unit: " << *ppdu);

  // Create meta information object
  Ptr < PLC_TrxMetaInfo > metaInfo = Create<PLC_TrxMetaInfo> ();
  metaInfo->SetMessage (ppdu);
  metaInfo->SetHeaderMcs (GetHeaderModulationAndCodingScheme ());
  metaInfo->SetPayloadMcs (GetPayloadModulationAndCodingScheme ());

  // (Virtually encode frame)
  PrepareTransmission ( metaInfo);

  // Start sending
  if (GetState () == IDLE)
    {
      PLC_PHY_CUSTOM_INFO ("Device idle, sending frame: " << *metaInfo);
      SendFrame (metaInfo);
      return true;
    }

  PLC_LOG_INFO("Phy busy, cannot send frame");
  return false;
}

void
PlcMaxRatePhy::PrepareTransmission (Ptr<PLC_TrxMetaInfo> metaInfo)
{
  PLC_PHY_CUSTOM_FUNCTION (this << metaInfo);
  NS_ASSERT (metaInfo && metaInfo->GetMessage ());

  if (metaInfo->GetPayloadMcs ().ct < CODING_RATE_RATELESS)
    {
      // Fixed rate payload encoding
      PrepareFixedRateTransmission (metaInfo);
    }
  else
    {
      // Rateless payload encoding
      PrepareRatelessTransmission (metaInfo);
    }
}

void
PlcMaxRatePhy::PrepareFixedRateTransmission (Ptr<PLC_TrxMetaInfo> metaInfo)
{
  PLC_PHY_CUSTOM_FUNCTION (this << metaInfo);
  NS_ASSERT (metaInfo && metaInfo->GetMessage ()); NS_ASSERT (metaInfo->GetHeaderMcs().ct < CODING_RATE_RATELESS && metaInfo->GetPayloadMcs ().ct < CODING_RATE_RATELESS);

  Ptr<Packet> frame = metaInfo->GetMessage ()->Copy ();
  ModulationAndCodingScheme header_mcs = metaInfo->GetHeaderMcs ();
  ModulationAndCodingScheme payload_mcs = metaInfo->GetPayloadMcs ();

  // (Virtually) encode packet
  size_t uncoded_header_bits = m_txFch.GetSerializedSize () * 8;
  size_t encoded_header_bits = EncodedBits (uncoded_header_bits, header_mcs.ct);
  size_t header_symbols = RequiredSymbols (encoded_header_bits, header_mcs.mt, GetNumSubcarriers ());
  Time header_duration = CalculateTransmissionDuration (header_symbols);

  NS_LOG_LOGIC ("symbol duration: " << GetSymbolDuration ());
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
  PLC_PHY_CUSTOM_LOGIC ("Adding preamble...");
  frame->AddHeader (PLC_Preamble ());

  // Complete metaInfo
  metaInfo->SetFrame (frame);
  metaInfo->SetHeaderDuration (header_duration);
  metaInfo->SetPayloadDuration (payload_duration);
}

void
PlcMaxRatePhy::PrepareRatelessTransmission (Ptr<PLC_TrxMetaInfo> metaInfo)
{
  PLC_PHY_CUSTOM_FUNCTION (this << metaInfo);
  NS_ASSERT (metaInfo && metaInfo->GetMessage ()); NS_ASSERT (metaInfo->GetHeaderMcs().ct < CODING_RATE_RATELESS && metaInfo->GetPayloadMcs ().ct >= CODING_RATE_RATELESS);

  // Read mcs info
  ModulationAndCodingScheme header_mcs = metaInfo->GetHeaderMcs ();
  ModulationAndCodingScheme payload_mcs = metaInfo->GetPayloadMcs ();

  m_txFch.SetMessageId (m_txMessageId++);
  m_txFch.SetPayloadMt (payload_mcs.mt);
  m_txFch.SetPayloadCt (payload_mcs.ct);

  // (Virtually) encode frame

  // Get number of raw bits an OFDM header/payload symbol is able to carry
  // (if not all subcarriers are used for transmission m_numSubcarriers should be adjusted)
  size_t bitsPerOfdmHeaderSymbol = GetBitsPerSymbol (header_mcs.mt) * GetNumSubcarriers ();
  size_t bitsPerOfdmPayloadSymbol = GetBitsPerSymbol (payload_mcs.mt) * GetNumSubcarriers ();
  PLC_LOG_LOGIC("Raw bits per ofdm header symbol: " << bitsPerOfdmHeaderSymbol);
  PLC_LOG_LOGIC("Raw bits per ofdm payload symbol: " << bitsPerOfdmPayloadSymbol);

  // Segment ppdu into blocks
  size_t num_blocks = ceil (metaInfo->GetMessage ()->GetSize () * 8 / (double) (GetOfdmSymbolsPerCodeBlock ()
          * bitsPerOfdmPayloadSymbol));
  m_txFch.SetNumBlocks (num_blocks);
  PLC_LOG_LOGIC ("Number of uncoded blocks: " << num_blocks);

  // Estimate how many encoded blocks (chunks) are at least required for successful decoding
  size_t required_chunks = RequiredChunks (num_blocks);
  PLC_LOG_LOGIC("Required chunks: " << required_chunks);

  // Calculate length of the encoded packet
  size_t encoded_payload_size = ChunksInByte (required_chunks, bitsPerOfdmPayloadSymbol);
  PLC_LOG_LOGIC("Encoded payload size: " << encoded_payload_size);

  // Create frame
  // The frame is composed of the uncoded header and rateless encoded payload blocks
  Ptr<Packet> frame = Create<Packet> (encoded_payload_size);

  // Determine header symbols
  size_t uncoded_header_bits = m_txFch.GetSerializedSize () * 8;
  size_t encoded_header_bits = EncodedBits (uncoded_header_bits, header_mcs.ct);
  size_t header_symbols = RequiredSymbols (encoded_header_bits, header_mcs.mt, GetNumSubcarriers ());

  // Determine payload symbols
  size_t payload_symbols = RequiredSymbols (encoded_payload_size * 8, payload_mcs.mt, GetNumSubcarriers ());
  m_txFch.SetPayloadSymbols (payload_symbols);

  // Calculate tx duration of packet segments
  Time header_duration = CalculateTransmissionDuration (header_symbols);
  Time payload_duration = CalculateTransmissionDuration (payload_symbols);

  NS_LOG_LOGIC ("symbol duration: " << GetSymbolDuration ());
  NS_LOG_LOGIC ("encoded_header_bits: " << encoded_header_bits);
  NS_LOG_LOGIC ("header_mcs: " << header_mcs);
  NS_LOG_LOGIC ("header_duration: " << header_duration);
  NS_LOG_LOGIC ("header_symbols: " << header_symbols);

  // Add frame control header
  frame->AddHeader (m_txFch);

  //Append preamble
  PLC_PHY_CUSTOM_LOGIC ("Adding preamble...");
  frame->AddHeader (PLC_Preamble ());

  // Complete meta information
  metaInfo->SetFrame (frame);
  metaInfo->SetHeaderDuration (header_duration);
  metaInfo->SetPayloadDuration (payload_duration);
}

bool
PlcMaxRatePhy::SendRedundancy (void)
{
  PLC_PHY_CUSTOM_FUNCTION (this);
  return false;
}

void
PlcMaxRatePhy::DoStartRx (uint32_t txId, Ptr<const SpectrumValue> rxPsd, Time duration, Ptr<const PLC_TrxMetaInfo> metaInfo)
{
  PLC_PHY_CUSTOM_FUNCTION (this << txId << rxPsd << duration << metaInfo);
  NS_ASSERT_MSG(m_information_rate_model, "PLC_HalfDuplexOfdmPhy: an error model has to be assigned to the Phy previous to data reception!");

  PLC_PHY_CUSTOM_LOGIC ("Receive Power: " << Pwr (*rxPsd));

  if (metaInfo && // meta information present
          GetState () == IDLE && // PHY will only start receiving in IDLE state (TODO: conditional resynchronization to the new signal if PHY is currently in receiving state)
          metaInfo->GetHeaderMcs ().mt == GetHeaderModulationAndCodingScheme ().mt && // same modulation and coding type (preamble detection is supposed to be always successful)
          metaInfo->GetHeaderMcs ().ct == GetHeaderModulationAndCodingScheme ().ct && // same modulation and coding type (preamble detection is supposed to be always successful)
          W2dBm (Pwr ((*rxPsd)/* / GetTotalNoisePower()*/)) >= PLC_RECEIVER_SENSIVITY) // power is sufficient for synchronization
    {
      PLC_LOG_LOGIC (this << "Starting preamble detection...");
      Simulator::Schedule (PLC_Preamble::GetDuration (), &PlcMaxRatePhy::PreambleDetectionSuccessful, this, txId, rxPsd,
              duration, metaInfo);
      m_locked_txId = 0;
      ChangeState( RX);
    }
  else
    {
      NoiseStart (txId, rxPsd, duration);
    }
}

void
PlcMaxRatePhy::PreambleDetectionSuccessful (uint32_t txId, Ptr<const SpectrumValue> rxPsd, Time duration, Ptr<
        const PLC_TrxMetaInfo> metaInfo)
{
  PLC_PHY_CUSTOM_FUNCTION (this << txId << rxPsd << duration << metaInfo);
  PLC_PHY_CUSTOM_INFO ("Preamble detection successful");

  NS_ASSERT (GetState() == RX); NS_ASSERT (metaInfo && metaInfo->GetFrame ());

  m_locked_txId = txId;
  m_incoming_frame = metaInfo->GetFrame ()->Copy ();

  //      PLC_PHY_CUSTOM_LOGIC ("Locked on incoming signal");
  PLC_PHY_CUSTOM_LOGIC ("Incoming frame: " << *m_incoming_frame);

  // Remove preamble
  PLC_Preamble preamble;
  m_incoming_frame->RemoveHeader (preamble);

  // Start frame reception
  StartReception (txId, rxPsd, duration, metaInfo);
}

void
PlcMaxRatePhy::StartReception (uint32_t txId, Ptr<const SpectrumValue> rxPsd, Time duration,
        Ptr<const PLC_TrxMetaInfo> metaInfo)
{
  PLC_PHY_CUSTOM_FUNCTION (this << txId << rxPsd << duration << metaInfo);
  PLC_PHY_CUSTOM_INFO ("Starting frame reception...");

  // Determine uncoded header bits
  ModulationAndCodingScheme header_mcs = metaInfo->GetHeaderMcs ();
  size_t uncoded_header_bits = PLC_PhyFrameControlHeader ().GetSerializedSize () * 8;

  // Receive header
  Time header_duration = metaInfo->GetHeaderDuration ();
  NS_LOG_LOGIC ("header duration: " << header_duration);

  // header is not FEC encoded => no coding overhead
  m_information_rate_model->SetCodingOverhead (0);
  m_information_rate_model->StartRx (header_mcs, rxPsd, uncoded_header_bits);
  Simulator::Schedule (header_duration, &PlcMaxRatePhy::EndRxHeader, this, txId, rxPsd, metaInfo);
}

void
PlcMaxRatePhy::EndRxHeader (uint32_t txId, Ptr<const SpectrumValue> rxPsd, Ptr<const PLC_TrxMetaInfo> metaInfo)
{
  PLC_PHY_CUSTOM_FUNCTION (this << metaInfo);
  NS_ASSERT(GetState () == RX);

  ModulationAndCodingScheme payload_mcs = metaInfo->GetPayloadMcs ();
  Time payload_duration = metaInfo->GetPayloadDuration ();

  if ((payload_mcs.mt == GetPayloadModulationAndCodingScheme ().mt) && (payload_mcs.ct
          == GetPayloadModulationAndCodingScheme ().ct))
    {
      if (m_information_rate_model->EndRx ())
        {
          PLC_PHY_CUSTOM_INFO ("Successfully received header, starting payload reception");

          ModulationAndCodingScheme payload_mcs = metaInfo->GetPayloadMcs ();

          // Remove frame control header
          m_incoming_frame->RemoveHeader (m_rxFch);

          // If header is rateless encoded take coding overhead into account for reception
          if (payload_mcs.ct >= CODING_RATE_RATELESS)
            {
              m_information_rate_model->SetCodingOverhead (GetRatelessCodingOverhead ());
            }
          else
            {
              m_information_rate_model->SetCodingOverhead (0);
            }

          PLC_PHY_CUSTOM_INFO ("Remaining rx time: " << payload_duration);

          size_t uncoded_payload_bits = m_incoming_frame->GetSize () * 8;

          NS_LOG_LOGIC ("Starting payload reception: " << payload_duration << payload_mcs << uncoded_payload_bits);

          m_information_rate_model->StartRx (payload_mcs, rxPsd, uncoded_payload_bits);
          Simulator::Schedule (payload_duration, &PlcMaxRatePhy::EndRxPayload, this, metaInfo);
          Simulator::Schedule (payload_duration, &PLC_HalfDuplexOfdmPhy::ChangeState, this, IDLE);
        }
      else
        {
          PLC_PHY_CUSTOM_INFO ("Header reception failed, remaining signal treated as interference");

          NoiseStart (txId, rxPsd, payload_duration);

          Simulator::Schedule (payload_duration, &PlcMaxRatePhy::ReceptionFailure, this);
          ChangeState ( IDLE);
        }
    }
  else
    {
      PLC_PHY_CUSTOM_INFO ("Different modulation and coding scheme: PHY (" << GetPayloadModulationAndCodingScheme () << "), message ("
              << payload_mcs << "), remaining signal treated as interference");

      NoiseStart (txId, rxPsd, payload_duration);

      Simulator::Schedule (payload_duration, &PlcMaxRatePhy::ReceptionFailure, this);
      ChangeState ( IDLE);
    }
}

void
PlcMaxRatePhy::EndRxPayload (Ptr<const PLC_TrxMetaInfo> metaInfo)
{
  PLC_PHY_CUSTOM_FUNCTION (this);
  if (m_information_rate_model->EndRx ())
    {
      // Successful payload reception
      PLC_PHY_CUSTOM_INFO ("Message successfully decoded");
      PLC_PHY_CUSTOM_LOGIC ("Decoded packet: " << *m_incoming_frame);

      if (!m_receive_success_cb.IsNull ())
        {
          m_receive_success_cb (metaInfo->GetMessage (), m_rxFch.GetMessageId ());
        }

      NotifySuccessfulReception ();
    }
  else
    {
      PLC_PHY_CUSTOM_INFO ("Not able to decode datagram");
      NotifyPayloadReceptionFailed (metaInfo);
      ReceptionFailure ();
    }

  m_incoming_frame = 0;
}

void
PlcMaxRatePhy::SetHeaderModulationAndCodingScheme (ModulationAndCodingScheme mcs)
{
  PLC_PHY_CUSTOM_FUNCTION (this);
  NS_ASSERT_MSG(mcs.ct < CODING_RATE_RATELESS, "Not a fixed rate modulation and coding scheme!");
  m_header_mcs = mcs;
}

ModulationAndCodingScheme
PlcMaxRatePhy::GetHeaderModulationAndCodingScheme (void)
{
  PLC_PHY_CUSTOM_FUNCTION (this);
  return m_header_mcs;
}

void
PlcMaxRatePhy::SetPayloadModulationAndCodingScheme (ModulationAndCodingScheme mcs)
{
  PLC_PHY_CUSTOM_FUNCTION (this);
  m_payload_mcs = mcs;
}

ModulationAndCodingScheme
PlcMaxRatePhy::GetPayloadModulationAndCodingScheme (void)
{
  PLC_PHY_CUSTOM_FUNCTION (this);
  return m_payload_mcs;
}

void
PlcMaxRatePhy::SetPayloadReceptionFailedCallback (PLC_PayloadReceptionFailedCallback c)
{
  PLC_PHY_CUSTOM_FUNCTION (this);
  m_payload_reception_failed_cb = c;
}

PLC_PhyCcaResult
PlcMaxRatePhy::ClearChannelAssessment (void)
{
  PLC_PHY_CUSTOM_FUNCTION (this);

  double mean_rx_pwr = m_information_rate_model->GetMeanRxPower ();
  PLC_LOG_LOGIC ("Mean RX power: " << mean_rx_pwr);

  if (GetState () == IDLE && mean_rx_pwr <= CCA_THRESHOLD_POWER)
    return CHANNEL_CLEAR;
  else
    return CHANNEL_OCCUPIED;
}

void
PlcMaxRatePhy::NotifySuccessfulReception (void)
{
  PLC_PHY_CUSTOM_FUNCTION (this);
}

void
PlcMaxRatePhy::NotifyPayloadReceptionFailed (Ptr<const PLC_TrxMetaInfo> metaInfo)
{
  PLC_PHY_CUSTOM_FUNCTION (this << metaInfo);
}
}
