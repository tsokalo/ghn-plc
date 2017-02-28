/*
 * PLC_FullDuplexOfdmPhy.cpp
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
#include "plc-full-duplex-ofdm-phy.h"
NS_LOG_COMPONENT_DEFINE("PLC_FullDuplexOfdmPhy");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(PLC_FullDuplexOfdmPhy);

Time PLC_FullDuplexOfdmPhy::guard_interval_duration = MicroSeconds (20);

TypeId
PLC_FullDuplexOfdmPhy::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PLC_FullDuplexOfdmPhy").SetParent<PLC_Phy> ();
  return tid;
}

PLC_FullDuplexOfdmPhy::PLC_FullDuplexOfdmPhy ()
{
  NS_LOG_FUNCTION(this);
  m_state = IDLE;
}

PLC_FullDuplexOfdmPhy::~PLC_FullDuplexOfdmPhy ()
{
  NS_LOG_FUNCTION(this);
}

void
PLC_FullDuplexOfdmPhy::DoStart (void)
{
  NS_LOG_FUNCTION(this);
  assert(0);
  m_state = IDLE;
  PLC_Phy::DoInitialize ();
}

void
PLC_FullDuplexOfdmPhy::DoDispose ()
{
  m_txImpedance = 0;
  m_rxImpedance = 0;
  m_outlet = 0;
  m_txPsd = 0;
  m_txInterface = 0;
  m_rxInterface = 0;
  m_shuntImpedance = 0;
  m_txImpedance = 0;
  m_rxImpedance = 0;
  m_eqRxImpedance = 0;
  m_eqTxImpedance = 0;
  PLC_Phy::DoDispose ();
}

PLC_ChannelTransferImpl *
PLC_FullDuplexOfdmPhy::DoGetChannelTransferImpl (Ptr<PLC_Phy> rxPhy)
{
  NS_LOG_FUNCTION(this << rxPhy);
  NS_ASSERT_MSG(rxPhy->GetInstanceTypeId ().IsChildOf (PLC_FullDuplexOfdmPhy::GetTypeId ()),
          "RX Phy not of type PLC_FullDuplexOfdmPhy");
  Ptr<PLC_RxInterface> rxIf = (StaticCast<PLC_FullDuplexOfdmPhy, PLC_Phy> (rxPhy))->GetRxInterface ();
  return GetTxInterface ()->GetChannelTransferImpl (PeekPointer (rxIf));
}

Ptr<PLC_ChannelTransferImpl>
PLC_FullDuplexOfdmPhy::DoGetChannelTransferImplPtr (Ptr<PLC_Phy> rxPhy)
{
  NS_LOG_FUNCTION(this << rxPhy);
  NS_ASSERT_MSG(rxPhy->GetInstanceTypeId ().IsChildOf (PLC_FullDuplexOfdmPhy::GetTypeId ()),
          "RX Phy not of type PLC_FullDuplexOfdmPhy");
  Ptr<PLC_RxInterface> rxIf = (StaticCast<PLC_FullDuplexOfdmPhy, PLC_Phy> (rxPhy))->GetRxInterface ();
  return Ptr<PLC_ChannelTransferImpl> (GetTxInterface ()->GetChannelTransferImpl (PeekPointer (rxIf)));
}

void
PLC_FullDuplexOfdmPhy::SetGuardIntervalDuration (Time duration)
{
  guard_interval_duration = duration;
}

Time
PLC_FullDuplexOfdmPhy::GetGuardIntervalDuration (void)
{
  return guard_interval_duration;
}

void
PLC_FullDuplexOfdmPhy::CreateInterfaces (Ptr<PLC_Outlet> outlet, Ptr<SpectrumValue> txPsd, Ptr<PLC_Impedance> rxImpedance,
        Ptr<PLC_Impedance> txImpedance)
{
  NS_LOG_FUNCTION(this << outlet << txPsd << rxImpedance << txImpedance);
  m_node = outlet->GetNode ();

  NS_ASSERT_MSG(m_node, "PLC_FullDuplexOfdmPhy::CreateInterfaces: outlet has to be bound to a PLC_Node");
  m_outlet = outlet;

  Ptr<PLC_Graph> graph = m_node->GetGraph ();
  NS_ASSERT_MSG(graph,
          "PLC_FullDuplexOfdmPhy::CreateInterfaces: PLC_Node the outlet is located on is not attached to a PLC_Graph");

  Ptr<PLC_Channel> ch = graph->GetChannel ();
  NS_ASSERT_MSG(ch,
          "PLC_FullDuplexOfdmPhy::CreateInterfaces: the graph the outlet's node is connected to is not bound to a PLC_Channel");

  if (!m_txInterface)
    {
      m_txInterface = CreateObject<PLC_TxInterface> (m_node, txPsd->GetSpectrumModel ());
    }
  m_txInterface->SetPhy (Ptr<PLC_Phy> (this));

  if (!m_rxInterface)
    {
      m_rxInterface = CreateObject<PLC_RxInterface> (m_node, txPsd->GetSpectrumModel ());
    }
  m_rxInterface->SetPhy (Ptr<PLC_Phy> (this));
  m_rxInterface->SetOutlet (m_outlet);

  m_txInterface->SetTxIfIdx (ch->AddTxInterface (m_txInterface));
  m_txInterface->SetNoiseIfIdx (ch->AddTxInterface (m_txInterface));
  m_rxInterface->SetRxIfIdx (ch->AddRxInterface (m_rxInterface));

  if (m_node->GetImpedancePtr ())
    {
      PLC_LOG_LOGIC("Node has shunt impedance");
      m_shuntImpedance = m_node->GetImpedancePtr ();
    }

  SetTxPowerSpectralDensity (txPsd);
  SetRxImpedance (rxImpedance);
  SetTxImpedance (txImpedance);
  ComputeEquivalentImpedances ();
}

Ptr<PLC_TxInterface>
PLC_FullDuplexOfdmPhy::GetTxInterface (void)
{
  PLC_LOG_FUNCTION(this);
  NS_ASSERT_MSG(m_txInterface, "TX interface not yet created!");
  return m_txInterface;
}

Ptr<PLC_RxInterface>
PLC_FullDuplexOfdmPhy::GetRxInterface (void)
{
  PLC_LOG_FUNCTION(this);
  return m_rxInterface;
}

void
PLC_FullDuplexOfdmPhy::SetTxPowerSpectralDensity (Ptr<SpectrumValue> txPsd)
{
  NS_ASSERT_MSG(m_txInterface, "Phy interfaces have to be created first before changing txPsd");
  if (m_txPsd)
  NS_ASSERT_MSG(txPsd->GetSpectrumModel () == m_txPsd->GetSpectrumModel (),
          "PLC_FullDuplexOfdmPhy::SetTxPowerSpectralDensity: new txPsd uses different SpectrumModel");

  m_txPsd = txPsd;
  m_numSubcarriers = m_txPsd->GetSpectrumModel ()->GetNumBands ();
  m_chTrF = Create<SpectrumValue> (m_txPsd->GetSpectrumModel ());
}

void
PLC_FullDuplexOfdmPhy::SetShuntImpedance (Ptr<PLC_Impedance> shuntImpedance)
{
  NS_LOG_FUNCTION(this << shuntImpedance);

#ifdef PLC_USE_IMPEDANCE_HASHING
  if (shuntImpedance)
    {
      m_shuntImpedance = shuntImpedance->Copy ();
    }
  else
    {
      m_shuntImpedance = NULL;
    }
#else
  m_shuntImpedance = shuntImpedance;
#endif

  ComputeEquivalentImpedances ();

  if (m_outlet)
    {
      if (GetState () == TX || GetState () == TXRX)
        {
          m_outlet->SetImpedance (m_eqTxImpedance);
        }
      else
        {
          m_outlet->SetImpedance (m_eqRxImpedance);
        }
    }
}

void
PLC_FullDuplexOfdmPhy::SetRxImpedance (Ptr<PLC_Impedance> rxImpedance)
{
  NS_LOG_FUNCTION(this << rxImpedance);

#ifdef PLC_USE_IMPEDANCE_HASHING
  if (rxImpedance)
    {
      m_rxImpedance = rxImpedance->Copy ();
    }
  else
    {
      m_rxImpedance = NULL;
    }
#else
  m_rxImpedance = rxImpedance;
#endif

  ComputeEquivalentImpedances ();

  if (m_outlet)
    {
      if (GetState () == TX || GetState () == TXRX)
        {
          m_outlet->SetImpedance (m_eqTxImpedance);
        }
      else
        {
          m_outlet->SetImpedance (m_eqRxImpedance);
        }
    }
}

void
PLC_FullDuplexOfdmPhy::SetTxImpedance (Ptr<PLC_Impedance> txImpedance)
{
  NS_LOG_FUNCTION(this << txImpedance);

#ifdef PLC_USE_IMPEDANCE_HASHING
  if (txImpedance)
    {
      m_txImpedance = txImpedance->Copy ();
    }
  else
    {
      m_txImpedance = NULL;
    }
#else
  m_txImpedance = txImpedance;
#endif

  ComputeEquivalentImpedances ();

  if (m_outlet)
    {
      if (GetState () == TX || GetState () == TXRX)
        {
          m_outlet->SetImpedance (m_eqTxImpedance);
        }
      else
        {
          m_outlet->SetImpedance (m_eqRxImpedance);
        }
    }
}

void
PLC_FullDuplexOfdmPhy::SetNoiseFloor (Ptr<const SpectrumValue> noiseFloor)
{
  NS_LOG_FUNCTION(this << noiseFloor);
  GetLinkPerformanceModel ()->SetNoiseFloor (noiseFloor);
}

void
PLC_FullDuplexOfdmPhy::ComputeEquivalentImpedances (void)
{
  NS_LOG_FUNCTION(this);
  if (m_shuntImpedance)
    {
      if (m_rxImpedance)
        {
          NS_LOG_INFO("Node has shunt impedance. Using equivalent value of the parallel circuit as RX access impedance");
          m_eqRxImpedance = Divide (Multiply (m_rxImpedance, m_shuntImpedance), Add (m_rxImpedance, m_shuntImpedance));
          PLC_LOG_LOGIC("Equivalent RX impedance: " << *m_eqRxImpedance);
        }
      else
        {
          m_eqRxImpedance = m_shuntImpedance;
        }
      if (m_txImpedance)
        {
          NS_LOG_INFO("Node has shunt impedance. Using equivalent value of the parallel circuit as TX access impedance");
          m_eqTxImpedance = Divide (Multiply (m_txImpedance, m_shuntImpedance), Add (m_txImpedance, m_shuntImpedance));
          PLC_LOG_LOGIC("Equivalent TX impedance: " << *m_eqTxImpedance);
        }
      else
        {
          m_eqTxImpedance = m_shuntImpedance;
        }
    }
  else
    {
      m_eqRxImpedance = m_rxImpedance;
      m_eqTxImpedance = m_txImpedance;
    }
}

void
PLC_FullDuplexOfdmPhy::SwitchImpedance (State state)
{
  NS_LOG_FUNCTION(this);

  if (GetState () == state) return;

  PLC_LOG_LOGIC("m_eqRxImpedance: " << m_eqRxImpedance);
  PLC_LOG_LOGIC("m_eqTxImpedance: " << m_eqRxImpedance);

  if (state == TX)
    {
      m_outlet->SetImpedance (m_eqTxImpedance);
    }
  else if (GetState () == TX)
    {
      m_outlet->SetImpedance (m_eqRxImpedance);
    }
}

void
PLC_FullDuplexOfdmPhy::CcaRequest (void)
{
  NS_LOG_FUNCTION(this);
  static Time ccaTime = Seconds (CCA_NUM_SYMBOLS * GetSymbolDuration ().GetSeconds ());
  m_ccaEndEvent = Simulator::Schedule (ccaTime, &PLC_FullDuplexOfdmPhy::EndCca, this);
}

void
PLC_FullDuplexOfdmPhy::CancelCca (void)
{
  NS_LOG_FUNCTION(this);
  m_ccaEndEvent.Cancel ();
}

void
PLC_FullDuplexOfdmPhy::EndCca (void)
{
  NS_LOG_FUNCTION(this);
  if (!m_ccaConfirmCallback.IsNull ())
    {
      m_ccaConfirmCallback (ClearChannelAssessment ());
    }
}

void
PLC_FullDuplexOfdmPhy::SetCcaConfirmCallback (PLC_PhyCcaConfirmCallback c)
{
  PLC_LOG_FUNCTION(this);
  m_ccaConfirmCallback = c;
}

void
PLC_FullDuplexOfdmPhy::ChangeState (State newState)
{
  NS_LOG_FUNCTION(this);
  if (newState != GetState ())
    {
      NS_LOG_UNCOND(this << " state: " << GetState () << " -> " << newState);
      NS_ASSERT_MSG(m_outlet, "PHY's outlet is not set!");

      if (newState == TX || newState == TXRX)
        SwitchImpedance (TX);
      else
        SwitchImpedance (RX);

      m_PhyStateLogger (Simulator::Now (), newState);
      m_state = newState;
    }
}

PLC_FullDuplexOfdmPhy::State
PLC_FullDuplexOfdmPhy::GetState (void)
{
//  NS_LOG_FUNCTION (this);
  return m_state;
}

Time
PLC_FullDuplexOfdmPhy::CalculateTransmissionDuration (size_t symbols)
{
  return Time::FromInteger (
          ((PLC_Phy::GetSymbolDuration ().GetInteger () + GetGuardIntervalDuration ().GetInteger ()) * symbols),
          Time::GetResolution ());
}

void
PLC_FullDuplexOfdmPhy::SendFrame (Ptr<PLC_TrxMetaInfo> metaInfo)
{
  NS_LOG_FUNCTION(this << metaInfo);
  NS_ASSERT_MSG(m_txInterface, "Phy has no tx interface");
  NS_ASSERT(metaInfo && metaInfo->GetFrame ());
  NS_ASSERT(GetState () != TXRX);

  Ptr<Packet> frame = metaInfo->GetFrame ();

  Time tx_duration = metaInfo->GetHeaderDuration () + metaInfo->GetPayloadDuration () + PLC_Preamble::GetDuration ();

  NS_LOG_LOGIC(Simulator::Now() << " -> SendFrame on PHY. Frame duration: " << tx_duration);

  m_txEnd = Simulator::Now () + tx_duration;
  NS_LOG_LOGIC("Start sending frame...");
  if (GetState () == RX)
    {
      ChangeState (TXRX);
    }
  else
    {
      ChangeState (TX);
    }
  m_txInterface->StartTx (m_txPsd, tx_duration, metaInfo);
  m_changeStateEvent = Simulator::Schedule (tx_duration, &PLC_FullDuplexOfdmPhy::ChangeState, this, IDLE);
  m_txFrameEvent = Simulator::Schedule (tx_duration, &PLC_Phy::NotifyFrameSent, this);
}

void
PLC_FullDuplexOfdmPhy::NoiseStart (uint32_t txId, Ptr<const SpectrumValue> psd, Time duration)
{
  NS_LOG_FUNCTION(this << txId << psd);

  EventId noiseRemoveEvent;
  if (m_rxNoisePsdMap.find (txId) != m_rxNoisePsdMap.end ())
    {
      noiseRemoveEvent = m_rxNoisePsdMap[txId].first;
      noiseRemoveEvent.Cancel ();
      GetLinkPerformanceModel ()->RemoveNoiseSignal (m_rxNoisePsdMap[txId].second);
    }

  PLC_LOG_LOGIC("Adding " << txId << " to noise psd map");
  Ptr<SpectrumValue> v = Create<SpectrumValue> (psd->GetSpectrumModel ());
  if (m_rxNoisePsdMap[txId].second) *v += *(m_rxNoisePsdMap[txId].second);
  *v += *psd;
  m_rxNoisePsdMap[txId].second = v;
  PLC_LOG_LOGIC("Mean Rx power before adding noise signal: " << GetLinkPerformanceModel ()->GetMeanRxPower());
  GetLinkPerformanceModel ()->AddNoiseSignal (psd);
  PLC_LOG_LOGIC("Interference signal: " << *psd);

  PLC_LOG_LOGIC("Mean Rx power after adding noise signal: " << GetLinkPerformanceModel ()->GetMeanRxPower());

  noiseRemoveEvent = Simulator::Schedule (duration, &PLC_FullDuplexOfdmPhy::NoiseStop, this, txId);
  m_rxNoisePsdMap[txId].first = noiseRemoveEvent;
}

void
PLC_FullDuplexOfdmPhy::NoiseStop (uint32_t txId)
{
  NS_LOG_FUNCTION(this << txId);

  if (m_rxNoisePsdMap.find (txId) == m_rxNoisePsdMap.end ())
    {
      PLC_LOG_LOGIC("There is no noise from " << txId);
      return;
    }

  if (m_rxNoisePsdMap[txId].first.IsRunning ()) m_rxNoisePsdMap[txId].first.Cancel ();

  PLC_LOG_LOGIC("Removing " << txId << " from noise psd map");

  GetLinkPerformanceModel ()->RemoveNoiseSignal (m_rxNoisePsdMap[txId].second);
  m_rxNoisePsdMap.erase (txId);
}

void
PLC_FullDuplexOfdmPhy::DoUpdateRxPsd (uint32_t txId, Ptr<const SpectrumValue> rxSignal)
{
  NS_LOG_FUNCTION(this << txId << rxSignal);

  if ((GetState () == RX || GetState () == TXRX) && m_locked_txId == txId)
    {
      NS_ASSERT_MSG(m_rxNoisePsdMap.find (txId) == m_rxNoisePsdMap.end (),
              "TxId also registered as interfering source, something went wrong...");
      GetLinkPerformanceModel ()->AlterRxSignal (rxSignal);
    }
  else
    {
      if (m_rxNoisePsdMap.find (txId) != m_rxNoisePsdMap.end ())
        {
          GetLinkPerformanceModel ()->RemoveNoiseSignal (m_rxNoisePsdMap[txId].second);
          GetLinkPerformanceModel ()->AddNoiseSignal (rxSignal);

          PLC_LOG_LOGIC("Adding " << txId << " to noise psd map");
          m_rxNoisePsdMap[txId].second = rxSignal;
        }
    }
}

std::ostream&
operator<< (std::ostream& os, PLC_FullDuplexOfdmPhy::State state)
{
  switch (state)
    {
  case (PLC_FullDuplexOfdmPhy::IDLE):
    {
      os << "IDLE";
      break;
    }
  case (PLC_FullDuplexOfdmPhy::TX):
    {
      os << "TX";
      break;
    }
  case (PLC_FullDuplexOfdmPhy::RX):
    {
      os << "RX";
      break;
    }
  case (PLC_FullDuplexOfdmPhy::TXRX):
    {
      os << "TXRX";
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
