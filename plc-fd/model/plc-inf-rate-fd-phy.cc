/*
 * PLC_InfRateFDPhy.cpp
 *
 *  Created on: 07.09.2016
 *      Author: tsokalo
 */
#include <algorithm>
#include <ns3/simulator.h>
#include <ns3/abort.h>
#include <ns3/random-variable-stream.h>
#include <ns3/callback.h>
#include <ns3/log.h>
#include <ns3/abort.h>
#include <ns3/object-factory.h>
#include "ns3/plc-simulator-impl.h"
#include "plc-inf-rate-fd-phy.h"
NS_LOG_COMPONENT_DEFINE("PLC_InfRateFDPhy");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(PLC_InfRateFDPhy);

size_t PLC_InfRateFDPhy::modulation_symbols_per_code_block = 1;
double PLC_InfRateFDPhy::rateless_coding_overhead = 0.2;

TypeId
PLC_InfRateFDPhy::GetTypeId (void)
{
  static TypeId tid =
          TypeId ("ns3::PLC_InfRateFDPhy").SetParent<PLC_FullDuplexOfdmPhy> ().AddConstructor<PLC_InfRateFDPhy> ().AddAttribute (
                  "MaxRxQueueSize", "Maximum size of RX queue for redundancy reception.", UintegerValue (5),
                  MakeUintegerAccessor (&PLC_InfRateFDPhy::m_maxRxQueueSize), MakeUintegerChecker<size_t> ());
  return tid;
}

PLC_InfRateFDPhy::PLC_InfRateFDPhy (void)
{
  NS_LOG_FUNCTION(this);
  m_header_mcs = ModulationAndCodingScheme (BPSK, CODING_RATE_1_2, 0);
  m_payload_mcs = ModulationAndCodingScheme (BPSK, CODING_RATE_1_2, 0);
  m_information_rate_model = CreateObject<PLC_InformationRateModel> ();
  m_information_rate_model->SetSymbolDuration (GetSymbolDuration ());
  m_information_rate_model->SetGuardIntervalDuration (GetGuardIntervalDuration ());

  Ptr<UniformRandomVariable> u = CreateObject<UniformRandomVariable> ();
  m_txMessageId = u->GetInteger (1, 65535);

  m_txEnd = Simulator::Now ();
  m_rxEnd = Simulator::Now ();
}

void
PLC_InfRateFDPhy::SetOfdmSymbolsPerCodeBlock (size_t symbols)
{
  modulation_symbols_per_code_block = symbols;
}

size_t
PLC_InfRateFDPhy::GetOfdmSymbolsPerCodeBlock (void)
{
  return modulation_symbols_per_code_block;
}

void
PLC_InfRateFDPhy::SetRatelessCodingOverhead (double overhead)
{
  NS_ASSERT(overhead >= 0);
  rateless_coding_overhead = overhead;
}

size_t
PLC_InfRateFDPhy::RequiredChunks (size_t num_blocks)
{
  NS_LOG_FUNCTION(this);
  size_t ret = ceil (num_blocks * (1 + rateless_coding_overhead));
  NS_LOG_INFO("minimum required chunks to decode datagramm: " << ret);
  return ret;
}

size_t
PLC_InfRateFDPhy::ChunksInByte (size_t num_chunks, size_t raw_bits_per_symbol)
{
  NS_LOG_FUNCTION(this);
  return ceil (GetOfdmSymbolsPerCodeBlock () * raw_bits_per_symbol * num_chunks / (double) 8);
}
Ptr<SpectrumValue>
PLC_InfRateFDPhy::ComputeSelfInterference (Ptr<SpectrumValue> channelImp, Ptr<const SpectrumValue> txPsd)
{
  Ptr<const SpectrumModel> sm = txPsd->GetSpectrumModel ();
  Ptr<SpectrumValue> si = Create<SpectrumValue> (sm);
  SpectrumValue lcI = 10 * Log10 (*channelImp);
  SpectrumValue ltP = *txPsd;

  /*
   * HERE MUST BE NOT txPsd BUT ALL SIGNALS PSD
   */

  uint16_t i = 0;
  for (Bands::const_iterator band = sm->Begin (); band != sm->End (); band++)
    {
      uint32_t l = ((lcI[i] > 29) ? 0 : round (-lcI[i])) + 29;
      if (lcI[i] > 29 || l >= sca_ecg.size ())
        {
          (*si)[i] = ltP[i];
          NS_LOG_LOGIC(
                  "No cancellation for channel " << i << ": txPsd -> " << ltP[i] << " W/Hz, CTF -> " << lcI[i] << " dB, RCTF -> " << l << " dB");
        }
      else
        {
          NS_LOG_LOGIC(
                  "Apply cancellation for channel " << i << ": txPsd -> " << ltP[i] << " W/Hz, CTF -> " << lcI[i] << " dB, RCTF -> " << l << " dB, cancellation -> " << sca_ecg[l] << " dB");

          (*si)[i] = ltP[i] / std::pow (10, sca_ecg[l] / 10);
        }
      i++;
    }

  NS_LOG_LOGIC("Remaining interference (W/Hz) " << *si);

  return si;
  //  Ptr<const SpectrumModel> sm = txPsd->GetSpectrumModel ();
  //  Ptr<SpectrumValue> si = Create<SpectrumValue> (sm);
  //  SpectrumValue lcI = 10 * Log10 (*channelImp);
  //  SpectrumValue ltP = *txPsd;
  //
  //  /*
  //   * HERE MUST BE NOT txPsd BUT ALL SIGNALS PSD
  //   */
  //
  //  uint16_t i = 0;
  //  for (Bands::const_iterator band = sm->Begin (); band != sm->End (); band++)
  //    {
  //      uint32_t l = ((lcI[i] > 0) ? 0 : round (-lcI[i]));
  //      if (lcI[i] > 0 || l >= sca_ecg.size ())
  //        {
  //          (*si)[i] = ltP[i];
  //          NS_LOG_LOGIC ("No cancellation for channel " << i << ": txPsd -> " << ltP[i] << " W/Hz, CTF -> " << lcI[i]
  //                  << " dB, RCTF -> " << l << " dB");
  //        }
  //      else
  //        {
  //          NS_LOG_LOGIC ("Apply cancellation for channel " << i << ": txPsd -> " << ltP[i] << " W/Hz, CTF -> " << lcI[i]
  //                  << " dB, RCTF -> " << l << " dB, cancellation -> " << sca_ecg[l] << " dB");
  //
  //          (*si)[i] = ltP[i] / std::pow (10, sca_ecg[l] / 10);
  //        }
  //      i++;
  //    }
  //
  //  NS_LOG_LOGIC ("Remaining interference (W/Hz) " << *si);
  //
  //  return si;
}
void
PLC_InfRateFDPhy::ReceptionFailure (void)
{
  NS_LOG_FUNCTION(this);
  if (!m_receive_error_cb.IsNull ())
    {
      m_receive_error_cb ();
    }
}

double
PLC_InfRateFDPhy::GetTransmissionRateLimit (Ptr<SpectrumValue> rxPsd)
{
  NS_LOG_FUNCTION(rxPsd);
  return m_information_rate_model->GetTransmissionRateLimit (rxPsd, GetPayloadModulationAndCodingScheme ());
}
void
PLC_InfRateFDPhy::AbortReception ()
{
  NS_LOG_DEBUG(m_rxInterface->GetNode ()->GetName () << " Aborting reception");
  if (m_preambleDetectionEvent.IsRunning ()) m_preambleDetectionEvent.Cancel ();
  if (m_stateChangeEvent.IsRunning ()) m_stateChangeEvent.Cancel ();
  if (m_receptionEndEvent.IsRunning ()) m_receptionEndEvent.Cancel ();
  //
  // remove RX PSD of preamble
  //
  if (m_information_rate_model->IsReceiving ()) m_information_rate_model->EndRx ();
  ChangeState (IDLE);
  if (!m_notifyMacAbort.IsNull ()) m_notifyMacAbort ();
}

void
PLC_InfRateFDPhy::DoStart (void)
{
  NS_LOG_FUNCTION(this);
  PLC_FullDuplexOfdmPhy::DoStart ();
}

void
PLC_InfRateFDPhy::DoDispose (void)
{
  NS_LOG_FUNCTION(this);
  ChangeState (IDLE);
  m_information_rate_model = 0;
  m_frame_sent_callback = MakeNullCallback<void> ();
  PLC_FullDuplexOfdmPhy::DoDispose ();
}

Ptr<PLC_LinkPerformanceModel>
PLC_InfRateFDPhy::DoGetLinkPerformanceModel (void)
{
  NS_LOG_FUNCTION(this);
  return m_information_rate_model;
}

bool
PLC_InfRateFDPhy::DoStartTx (Ptr<PLC_TrxMetaInfo> metaInfo)
{
  NS_LOG_FUNCTION(this << metaInfo << m_rxInterface->GetNode ()->GetName () << GetState());

  NS_ASSERT_MSG(m_txPsd, "TxPsd not set!");

  // Start sending
  if (GetState () == IDLE || GetState () == RX)
    {
      NS_LOG_INFO("Device state: " << GetState() << ", sending frame: " << *metaInfo);
      SendFrame (metaInfo);

      //
      // add self-interference if receiving
      //
      SiNoiseStart ();

      m_collisionCheckEvent = Simulator::Schedule (PLC_Preamble::GetDuration (), &PLC_InfRateFDPhy::CheckCollision, this);

      return true;
    }

  PLC_LOG_INFO("Phy busy, cannot send frame");
  return false;
}
bool
PLC_InfRateFDPhy::DoStartTx (Ptr<const Packet> p)
{
  NS_ABORT_MSG("This funtion is not implemented");
  return false;
}

bool
PLC_InfRateFDPhy::SendRedundancy (void)
{
  NS_LOG_FUNCTION(this);
  return false;
}

void
PLC_InfRateFDPhy::DoStartRx (uint32_t txId, Ptr<const SpectrumValue> rxPsd, Time duration, Ptr<const PLC_TrxMetaInfo> metaInfo)
{
  NS_LOG_FUNCTION(
          this << txId << rxPsd << duration << metaInfo << m_rxInterface->GetNode ()->GetName () << m_txInterface->GetTxIfIdx ());

  NS_LOG_LOGIC("PHY frame transmission time: " << duration);

  NS_LOG_DEBUG(m_rxInterface->GetNode ()->GetName () << " PHY frame transmission time: " << duration);

  if (txId == m_txInterface->GetTxIfIdx ()) return;

  NS_ASSERT_MSG(m_information_rate_model,
          "PLC_FullDuplexOfdmPhy: an error model has to be assigned to the Phy previous to data reception!");

  NS_LOG_LOGIC("Receive Power: " << Pwr (*rxPsd));

  *m_chTrF = *rxPsd / *m_txPsd;

  m_rxEnd = Simulator::Now () + duration;

  if (metaInfo
          && // meta information present
          (GetState () == IDLE || GetState () == TX)
          && metaInfo->GetHeaderMcs ().mt == GetHeaderModulationAndCodingScheme ().mt&& // same modulation and coding type (preamble detection is supposed to be always successful)
                  metaInfo->GetHeaderMcs ().ct == GetHeaderModulationAndCodingScheme ().ct &&// same modulation and coding type (preamble detection is supposed to be always successful)
                  W2dBm (Pwr ((*rxPsd)/* / GetTotalNoisePower()*/)) >= PLC_RECEIVER_SENSIVITY)// power is sufficient for synchronization
    {
      PLC_LOG_LOGIC(this << "Starting preamble detection...");

      m_locked_txId = 0;
      if (GetState () == IDLE)
        {
          ChangeState (RX);
        }
      else if (GetState () == TX)
        {
          ChangeState (TXRX);
        }
      //
      // adding self-interference if transmitting
      //
      SiNoiseStart ();

      NS_ASSERT(!m_preambleDetectionEvent.IsRunning ());
      m_preambleDetectionEvent = Simulator::Schedule (PLC_Preamble::GetDuration (), &PLC_InfRateFDPhy::PreambleDetection, this,
              txId, rxPsd, duration, metaInfo);

      ModulationAndCodingScheme header_mcs = metaInfo->GetHeaderMcs ();

      //
      // just let the information rate model know
      // that the power of the signal being received should be counted
      // for preamble collision detection
      //
      NS_ASSERT(!m_information_rate_model->IsReceiving());
      m_information_rate_model->StartRx (header_mcs, rxPsd, 1);

    }
  else
    {
      NoiseStart (txId, rxPsd, duration);
    }
}
void
PLC_InfRateFDPhy::PreambleDetection (uint32_t txId, Ptr<const SpectrumValue> rxPsd, Time duration,
        Ptr<const PLC_TrxMetaInfo> metaInfo)
{
  if (GetState () == IDLE) return;
  //
  // assuming that the period of channel assessment completely fits in preamble duration
  // check if the received signal is not just our signal; CollisionDetection() returns instantly
  //
  if (GetState () == RX)
    {
      Simulator::ScheduleNow (&PLC_InfRateFDPhy::PreambleDetectionSuccessful, this, txId, rxPsd, duration, metaInfo);
    }
  else
    {
      NS_ASSERT_MSG(GetState () == TXRX, "Current state: " << GetState());

      Simulator::ScheduleNow (&PLC_InfRateFDPhy::PreambleCollisionDetection, this);
    }
  //
  // end the preamble reception
  //
  m_information_rate_model->EndRx ();
}

void
PLC_InfRateFDPhy::PreambleDetectionSuccessful (uint32_t txId, Ptr<const SpectrumValue> rxPsd, Time duration,
        Ptr<const PLC_TrxMetaInfo> metaInfo)
{
  NS_LOG_FUNCTION(this << txId << rxPsd << duration << metaInfo);

  NS_LOG_INFO("Preamble detection successful");
  NS_LOG_DEBUG("Preamble detection successful");

  if (GetState () != RX && GetState () != TXRX)
    {
      NS_LOG_INFO("No reception is continued.. " << GetState() << " state");
      return;
    }
  NS_ASSERT(metaInfo && metaInfo->GetFrame ());

  m_locked_txId = txId;
  m_incoming_frame = metaInfo->GetFrame ()->Copy ();

  NS_LOG_LOGIC("Incoming frame: " << *m_incoming_frame);

  // Remove preamble
  PLC_Preamble preamble;
  m_incoming_frame->RemoveHeader (preamble);

  // Start frame reception
  StartReception (txId, rxPsd, duration, metaInfo);
}
void
PLC_InfRateFDPhy::PreambleCollisionDetection ()
{
  NS_LOG_FUNCTION(this);

  NS_LOG_INFO("Collision detection");
  NS_LOG_DEBUG("Collision detection");

  //
  // stop other events for collision check in future
  //
  if (m_collisionCheckEvent.IsRunning ()) m_collisionCheckEvent.Cancel ();
  //
  // abort the frame reception
  //
  AbortReception ();
  //
  // stop SI noise
  //
  NoiseStop (m_txInterface->GetTxIfIdx ());

  //
  // abort current transmission events on the current node initiated by the frame reception start
  //
  if (m_changeStateEvent.IsRunning ()) m_changeStateEvent.Cancel ();
  if (m_txFrameEvent.IsRunning ()) m_txFrameEvent.Cancel ();
  Simulator::ScheduleNow (&PLC_Channel::CancelTransmission, m_txInterface->GetChannel ());
  m_txEnd = Simulator::Now ();
  m_rxEnd = Simulator::Now ();
  ChangeState (IDLE);

  //
  // abort current transmission events on other nodes
  // In reality it happens automatically after the previous step; but in the simulation it should be done implicitly
  //
  NS_LOG_LOGIC("See " << m_txInterface->GetChannel ()->GetNRxInterfaces () << " RX interface");
  for (uint16_t i = 1; i <= m_txInterface->GetChannel ()->GetNRxInterfaces (); i++)
    {
      auto rxInterface = m_txInterface->GetChannel ()->GetRxInterface (i);
      if (rxInterface != m_rxInterface)
        {
          NS_LOG_DEBUG(
                  "I am " << m_rxInterface->GetNode ()->GetName () << " do abort reception for " << rxInterface->GetNode ()->GetName ());
          Ptr<PLC_Phy> phy = rxInterface->GetPhy ();
          phy->GetObject<PLC_FullDuplexOfdmPhy> ()->NoiseStop (m_txInterface->GetTxIfIdx ());
          if (phy->GetObject<PLC_InfRateFDPhy> ()->GetState () == RX) phy->GetObject<PLC_InfRateFDPhy> ()->AbortReception ();
        }
    }

  //
  // Report about the collision
  //
  if (!m_collisionDetection.IsNull ())
    {
      m_collisionDetection ();
    }
}
void
PLC_InfRateFDPhy::StartReception (uint32_t txId, Ptr<const SpectrumValue> rxPsd, Time duration,
        Ptr<const PLC_TrxMetaInfo> metaInfo)
{
  NS_LOG_FUNCTION(this << txId << rxPsd << duration << metaInfo);
  NS_LOG_INFO("Starting frame reception...");

  // Determine uncoded header bits
  ModulationAndCodingScheme header_mcs = metaInfo->GetHeaderMcs ();
  NS_LOG_INFO("Header MCS: " << header_mcs);
  size_t uncoded_header_bits = PLC_PhyFrameControlHeader ().GetSerializedSize () * 8;

  // Receive header
  Time header_duration = metaInfo->GetHeaderDuration ();
  NS_LOG_LOGIC("header duration: " << header_duration);

  // header is not FEC encoded => no coding overhead
  m_information_rate_model->SetCodingOverhead (0);
  m_information_rate_model->StartRx (header_mcs, rxPsd, uncoded_header_bits);
  m_receptionEndEvent = Simulator::Schedule (header_duration, &PLC_InfRateFDPhy::EndRxHeader, this, txId, rxPsd, metaInfo);

}

void
PLC_InfRateFDPhy::EndRxHeader (uint32_t txId, Ptr<const SpectrumValue> rxPsd, Ptr<const PLC_TrxMetaInfo> metaInfo)
{
  NS_LOG_FUNCTION(this << metaInfo);
  if (!m_endRxHeaderCallback.IsNull ())
    {
      m_endRxHeaderCallback (txId, rxPsd, metaInfo);
      return;
    }

  NS_ASSERT(GetState () == RX || GetState () == TXRX);

  ModulationAndCodingScheme payload_mcs = metaInfo->GetPayloadMcs ();
  Time payload_duration = metaInfo->GetPayloadDuration ();
  NS_LOG_LOGIC("payload duration: " << payload_duration);

  SetPayloadModulationAndCodingScheme (payload_mcs);

  if (m_information_rate_model->EndRx ())
    {
      NS_LOG_INFO("Successfully received header, starting payload reception");

      ModulationAndCodingScheme payload_mcs = metaInfo->GetPayloadMcs ();
      NS_LOG_INFO("Payload MCS: " << payload_mcs);

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

      NS_LOG_INFO("Remaining rx time: " << payload_duration);

      size_t uncoded_payload_bits = m_incoming_frame->GetSize () * 8;

      NS_LOG_LOGIC("Starting payload reception: " << payload_duration << payload_mcs << uncoded_payload_bits);

      m_information_rate_model->StartRx (payload_mcs, rxPsd, uncoded_payload_bits);
      m_receptionEndEvent = Simulator::Schedule (payload_duration, &PLC_InfRateFDPhy::EndRxPayload, this, metaInfo);
      m_stateChangeEvent = Simulator::Schedule (payload_duration, &PLC_FullDuplexOfdmPhy::ChangeState, this, IDLE);
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
PLC_InfRateFDPhy::EndRxPayload (Ptr<const PLC_TrxMetaInfo> metaInfo)
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
    }
  else
    {
      NS_LOG_INFO("Not able to decode datagram");
      NotifyPayloadReceptionFailed (metaInfo);
      ReceptionFailure ();
    }

  m_incoming_frame = 0;
}

void
PLC_InfRateFDPhy::SetHeaderModulationAndCodingScheme (ModulationAndCodingScheme mcs)
{
  NS_LOG_FUNCTION(this);
  NS_ASSERT_MSG(mcs.ct < CODING_RATE_RATELESS, "Not a fixed rate modulation and coding scheme!");
  m_header_mcs = mcs;
}

ModulationAndCodingScheme
PLC_InfRateFDPhy::GetHeaderModulationAndCodingScheme (void)
{
  NS_LOG_FUNCTION(this);
  return m_header_mcs;
}

void
PLC_InfRateFDPhy::SetPayloadModulationAndCodingScheme (ModulationAndCodingScheme mcs)
{
  NS_LOG_FUNCTION(this);
  m_payload_mcs = mcs;
}

ModulationAndCodingScheme
PLC_InfRateFDPhy::GetPayloadModulationAndCodingScheme (void)
{
  NS_LOG_FUNCTION(this);
  return m_payload_mcs;
}

void
PLC_InfRateFDPhy::SetPayloadReceptionFailedCallback (PLC_PayloadReceptionFailedCallback c)
{
  NS_LOG_FUNCTION(this);
  m_payload_reception_failed_cb = c;
}

PLC_PhyCcaResult
PLC_InfRateFDPhy::ClearChannelAssessment (void)
{
  NS_LOG_FUNCTION(this);

  double mean_rx_pwr = m_information_rate_model->GetMeanRxPower ();
  PLC_LOG_LOGIC("Mean RX power: " << mean_rx_pwr);

  if (GetState () == IDLE && mean_rx_pwr <= CCA_THRESHOLD_POWER)
    {
      PLC_LOG_LOGIC("IDLE state and mean Rx power is less then threshold");
      return CHANNEL_CLEAR;
    }
  else
    {
      PLC_LOG_LOGIC("IDLE state and mean Rx power is greater then threshold");
      return CHANNEL_OCCUPIED;
    }
}
PLC_PhyCcaResult
PLC_InfRateFDPhy::CollisionDetection ()
{
  NS_LOG_FUNCTION(this);

  double mean_rx_pwr = m_information_rate_model->GetMeanRxPower ();
  PLC_LOG_LOGIC("Mean RX power: " << mean_rx_pwr << ", threshold: " << CCA_THRESHOLD_POWER);

  if (mean_rx_pwr <= CCA_THRESHOLD_POWER)
    {
      PLC_LOG_LOGIC("Mean Rx power is less then threshold");
      return CHANNEL_CLEAR;
    }
  else
    {
      PLC_LOG_LOGIC("Mean Rx power is greater then threshold");
      return CHANNEL_OCCUPIED;
    }
}
double
PLC_InfRateFDPhy::GetMeanSelfInterferencePower (Ptr<const SpectrumValue> allSignals)
{
  NS_LOG_FUNCTION(this);

  if (round (Sum (*m_chTrF)) == 0) return 0;

  Ptr<SpectrumValue> selfInterferencePsd = ComputeSelfInterference (m_chTrF, m_txPsd);

  Ptr<SpectrumValue> remainingSignal = Create<SpectrumValue> (m_txPsd->GetSpectrumModel ());
  *remainingSignal = *allSignals - *m_txPsd + *selfInterferencePsd;

  double meanPwr = Sum (*remainingSignal) / (remainingSignal->GetSpectrumModel ()->GetNumBands ());

  NS_LOG_LOGIC("Mean power: " << meanPwr);
  NS_LOG_DEBUG("remainingSignal: " << (*remainingSignal));

  return meanPwr;
}
void
PLC_InfRateFDPhy::NotifySuccessfulReception (void)
{
  NS_LOG_FUNCTION(this);
}

void
PLC_InfRateFDPhy::NotifyPayloadReceptionFailed (Ptr<const PLC_TrxMetaInfo> metaInfo)
{
  NS_LOG_FUNCTION(this << metaInfo);
}
void
PLC_InfRateFDPhy::SiNoiseStart ()
{
  NS_LOG_FUNCTION(this << GetState());
  if (GetState () == TX || GetState () == RX)
    {
      if (GetState () == RX)
        {
          NS_ASSERT(m_rxEnd > Simulator::Now ());
        }
      if (GetState () == TX)
        {
          NS_ASSERT(m_txEnd > Simulator::Now ());
        }
      NS_LOG_LOGIC("No signals are being received and transmitted simultaneously. No SI noise will be created");
      return;
    }

  NS_ASSERT_MSG(GetState () == TXRX,
          "RX end: " << m_rxEnd << ", TX end: " << m_txEnd << ", now: " << Simulator::Now () << ", current state: " << GetState());

  NS_ASSERT(m_txEnd > Simulator::Now ());

  Time interferenceDuration = ((m_rxEnd < m_txEnd) ? m_rxEnd : m_txEnd) - Simulator::Now ();
  Ptr<SpectrumValue> siPsd = ComputeSelfInterference (m_chTrF, m_txPsd);

  NS_LOG_LOGIC("SI noise is being started. PSD: " << *siPsd);

  NoiseStart (m_txInterface->GetTxIfIdx (), siPsd, interferenceDuration);
}
void
PLC_InfRateFDPhy::CheckCollision ()
{
  if (CollisionDetection () == CHANNEL_OCCUPIED)
    {
      PreambleCollisionDetection ();
    }
}

}
