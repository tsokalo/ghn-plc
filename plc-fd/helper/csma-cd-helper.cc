/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <fstream>

#include <ns3/node.h>
#include <ns3/object-factory.h>
#include <ns3/log.h>
#include <ns3/abort.h>
#include "ns3/nstime.h"

#include "ns3/plc-helper.h"
#include "ns3/plc-fd-module.h"

#include "csma-cd-helper.h"

#include "ns3/ghn-plc-dll-management.h"

namespace ns3
{

using namespace ghn;

////////////////////////// CsmaCdHelper ////////////////////////////////////
NS_LOG_COMPONENT_DEFINE("CsmaCdHelper");
NS_OBJECT_ENSURE_REGISTERED(CsmaCdHelper);

TypeId
CsmaCdHelper::GetTypeId (void)
  {
    static TypeId tid = ns3::TypeId ("ns3::CsmaCdHelper") .SetParent<Object> ();
    return tid;
  }

CsmaCdHelper::CsmaCdHelper (Ptr<const SpectrumModel> sm, Ptr<SpectrumValue> txPsd, PLC_NodeList& deviceNodes) :
m_spectrum_model (sm), m_txPsd (txPsd), m_node_list (deviceNodes)
  {
    // Modulation and Coding Schemes
    m_header_mcs = ModulationAndCodingScheme (BPSK, CODING_RATE_1_2, 0);
    m_payload_mcs = ModulationAndCodingScheme (BPSK, CODING_RATE_1_2, 0);

    // Default phy model to be used
    m_phyTid = PLC_InfRateFDPhy::GetTypeId ();
    // Default mac model to be used
    m_macTid = PLC_ArqMac::GetTypeId ();

    // Create ns3::Node for this NetDevice by default
    m_create_nodes = true;
  }
CsmaCdHelper::CsmaCdHelper (BandPlanType bandplan)
  {
    m_bandplan = bandplan;
    PLC_Time::SetTimeModel (50, 50);
    //  NS_LOG_DEBUG("Using following slot duration: " << PLC_Time::GetResolutionS());
    //  NS_LOG_DEBUG("Number of slots per cycle: " << PLC_Time::GetNumTimeslots());

    m_spectrum_model = GetGhnSpectrumModel (m_bandplan);
    m_txPsd = GetGhnTransmitPSD (m_bandplan, m_spectrum_model);
    SetPhyParameters (m_bandplan);
    // Modulation and Coding Schemes
    m_header_mcs = ModulationAndCodingScheme (BPSK, CODING_RATE_1_2, 0);
    m_payload_mcs = ModulationAndCodingScheme (BPSK, CODING_RATE_1_2, 0);

    // Default phy model to be used
    m_phyTid = PLC_InfRateFDPhy::GetTypeId ();
    // Default mac model to be used
    m_macTid = PLC_ArqMac::GetTypeId ();

    // Create ns3::Node for this NetDevice by default
    m_create_nodes = true;
  }

void
CsmaCdHelper::SetRxImpedance (Ptr<PLC_Impedance> rxImpedance)
  {
    m_rxImpedance = rxImpedance->Copy ();

    if (m_netdeviceMap.size () > 0)
      {
        PLC_NetdeviceMap::iterator nit;
        for (nit = m_netdeviceMap.begin (); nit != m_netdeviceMap.end (); nit++)
          {
            nit->second->SetRxImpedance (m_rxImpedance);
          }
      }
  }

void
CsmaCdHelper::SetTxImpedance (Ptr<PLC_Impedance> txImpedance)
  {
    m_txImpedance = txImpedance->Copy ();

    if (m_netdeviceMap.size () > 0)
      {
        PLC_NetdeviceMap::iterator nit;
        for (nit = m_netdeviceMap.begin (); nit != m_netdeviceMap.end (); nit++)
          {
            nit->second->SetTxImpedance (m_txImpedance);
          }
      }
  }

void
CsmaCdHelper::Setup (void)
  {
    NS_ASSERT_MSG(m_txPsd, "TX psd not set!");

    CreateLogger ();

    static uint32_t sid = 1;

    if (m_noiseFloor == NULL)
      {
        m_noiseFloor = CreateBestCaseBgNoise(m_spectrum_model)->GetNoisePsd ();
      }

    ObjectFactory netdeviceFactory;
    netdeviceFactory.SetTypeId (PLC_NetDevice::GetTypeId ());

    ObjectFactory phyFactory;
    phyFactory.SetTypeId (m_phyTid);

    ObjectFactory macFactory;
    macFactory.SetTypeId (m_macTid);

    //
    // Create net devices
    //
    PLC_NodeList::iterator nit;
    for (nit = m_node_list.begin (); nit != m_node_list.end (); nit++)
      {
        //Create net device
        Ptr<PLC_NetDevice> dev = netdeviceFactory.Create<PLC_NetDevice> ();

        dev->SetSpectrumModel (m_spectrum_model);
        dev->SetNoiseFloor (m_noiseFloor);
        dev->SetTxPowerSpectralDensity (m_txPsd);

        //Create PHYs
        Ptr<PLC_Phy> phy = phyFactory.Create<PLC_Phy> ();

        if (phy->GetInstanceTypeId () == PLC_InfRateFDPhy::GetTypeId ())
          {
            (StaticCast<PLC_InfRateFDPhy, PLC_Phy> (phy))->SetHeaderModulationAndCodingScheme (m_header_mcs);
            (StaticCast<PLC_InfRateFDPhy, PLC_Phy> (phy))->SetPayloadModulationAndCodingScheme (m_payload_mcs);
          }
        else if (phy->GetInstanceTypeId () == PLC_InformationRatePhy::GetTypeId ())
          {
            (StaticCast<PLC_InformationRatePhy, PLC_Phy> (phy))->SetHeaderModulationAndCodingScheme (m_header_mcs);
            (StaticCast<PLC_InformationRatePhy, PLC_Phy> (phy))->SetPayloadModulationAndCodingScheme (m_payload_mcs);
          }
        else
          {
            NS_ABORT_MSG("Incompatible PHY type!");
          }

        // Create MAC
        Ptr<PLC_Mac> mac = macFactory.Create<PLC_Mac> ();

        dev->SetPhy (phy);
        dev->SetMac (mac);
        mac->SetMinBE(1);
        mac->SetMaxBE(m_maxCwSize);
        mac->SetBackoffSlotDuration(NanoSeconds(GDOTHN_IST));
        mac->SetUnitBackoffPeriod(0);//just disable it
        if (mac->GetInstanceTypeId () == PLC_ArqMac::GetTypeId () || mac->GetInstanceTypeId ().IsChildOf(PLC_ArqMac::GetTypeId ()))
          {
            (StaticCast<PLC_ArqMac, PLC_Mac> (mac))->SetAcknowledgementTimeout (MilliSeconds(200));
            (StaticCast<PLC_ArqMac, PLC_Mac> (mac))->SetMaxReplays (1000);
          }
        if (phy->GetInstanceTypeId () == PLC_InfRateFDPhy::GetTypeId () && mac->GetInstanceTypeId() == CsmaCdMac::GetTypeId())
          {
            (StaticCast<PLC_InfRateFDPhy, PLC_Phy> (phy))->SetCollisionDetection (MakeCallback (
                            &CsmaCdMac::CollisionDetection, (StaticCast<CsmaCdMac, PLC_Mac> (mac))));
            mac->SetBackoffValueCallback(MakeCallback(&CsmaCdMac::GetBackoffSlots,(StaticCast<CsmaCdMac, PLC_Mac> (mac))));
          }

        dev->SetAddress (Mac48Address::Allocate ());
        if (m_rxImpedance) dev->SetRxImpedance (m_rxImpedance);

        if (m_txImpedance) dev->SetTxImpedance (m_txImpedance);

        // Setting PLC node here to complete config
        dev->SetPlcNode (*nit);

        if (m_create_nodes)
          {
            // Create ns-3 node
            Ptr<Node> node = CreateObject<Node> (sid++);

            // Bind device to ns-3 node
            node->AddDevice (dev);

            NS_ASSERT(dev->ConfigComplete());
          }

        std::string name = (*nit)->GetName ();
        NS_ASSERT_MSG(m_netdeviceMap.find(name) == m_netdeviceMap.end(), "Duplicate netdevice name");
        m_netdeviceMap[name] = dev;
      }

    AttachChannel ();
  }

void
CsmaCdHelper::SetNodeList (PLC_NodeList& deviceNodes)
  {
    m_node_list = deviceNodes;
  }

Ptr<const SpectrumModel>
CsmaCdHelper::GetSpectrumModel ()
  {
    return m_spectrum_model;
  }

void
CsmaCdHelper::DefinePhyType (TypeId tid)
  {
    m_phyTid = tid;
  }

void
CsmaCdHelper::DefineMacType (TypeId tid)
  {
    m_macTid = tid;
  }
Ptr<PLC_NetDevice>
CsmaCdHelper::GetDevice (std::string name)
  {
    NS_ASSERT_MSG(m_netdeviceMap.find(name) != m_netdeviceMap.end(), "Unknown net device");
    return m_netdeviceMap[name];
  }

NodeContainer
CsmaCdHelper::GetNS3Nodes (void)
  {
    NodeContainer c;
    PLC_NetDeviceMap::iterator dit;
    for (dit = m_netdeviceMap.begin (); dit != m_netdeviceMap.end (); dit++)
      {
        c.Add (dit->second->GetNode ());
      }

    return c;
  }

NetDeviceContainer
CsmaCdHelper::GetNetDevices (void)
  {
    NetDeviceContainer c;
    PLC_NetDeviceMap::iterator dit;
    for (dit = m_netdeviceMap.begin (); dit != m_netdeviceMap.end (); dit++)
      {
        c.Add (dit->second);
      }

    return c;
  }

void
CsmaCdHelper::SetChannel (Ptr<PLC_Channel> channel)
  {
    m_channel = channel;
  }

void
CsmaCdHelper::SetResDirectory (std::string resFolder)
  {
    m_resDir = resFolder;
  }
Ptr<const SpectrumModel>
CsmaCdHelper::GetGhnSpectrumModel (BandPlanType bandplan)
  {
    PLC_SpectrumModelHelper smHelper;
    Ptr<const SpectrumModel> sm;
    double fl = 0;
    double fh = m_ofdmParameters[bandplan].m_fUs * 2 * 1000000;
    sm = smHelper.GetSpectrumModel (fl, fh, m_ofdmParameters[bandplan].m_subcarriersNumber);
    return sm;
  }
Ptr<SpectrumValue>
CsmaCdHelper::GetGhnTransmitPSD (BandPlanType bandplan, Ptr<const SpectrumModel> sm)
  {
    // Define transmit power spectral density
    double FL3 = 2000; //in kHz
    double FH1 = 30000; //in kHz
    Ptr<SpectrumValue> txPsd = Create<SpectrumValue> (sm);
    uint16_t iFL3 = ceil (m_ofdmParameters[bandplan].m_subcarriersNumber / 2
            - (0 + m_ofdmParameters[bandplan].m_fUs * 1000 - FL3) / m_ofdmParameters[bandplan].m_subcarrierSpacing);
    for (uint16_t i = 0; i < PERMANENTLY_MASKED_SUBCARRIERS; i++)
    (*txPsd)[i] = -150; //dBm/Hz
    for (uint16_t i = PERMANENTLY_MASKED_SUBCARRIERS; i <= iFL3; i++)
    (*txPsd)[i] = -85; //dBm/Hz
    uint16_t iFH1 = floor (m_ofdmParameters[bandplan].m_subcarriersNumber / 2 - (0 + m_ofdmParameters[bandplan].m_fUs * 1000
                    - FH1) / m_ofdmParameters[bandplan].m_subcarrierSpacing);
    if (bandplan == GDOTHN_BANDPLAN_25MHZ)
    for (uint16_t i = iFL3 + 1; i < m_ofdmParameters[bandplan].m_subcarriersNumber; i++)
    (*txPsd)[i] = -55; //dBm/Hz

    else
      {
        for (uint16_t i = iFL3 + 1; i <= iFH1; i++)
        (*txPsd)[i] = -55; //dBm/Hz
        for (uint16_t i = iFH1 + 1; i < m_ofdmParameters[bandplan].m_subcarriersNumber; i++)
        (*txPsd)[i] = -85; //dBm/Hz
      }

    // convert to W/Hz
    (*txPsd) = Pow (10.0, ((*txPsd) - 30) / 10.0);
    NS_LOG_LOGIC("Transmission PSD: " << *txPsd);
    return txPsd;
  }
void
CsmaCdHelper::SetPhyParameters (BandPlanType bandPlanType)
  {
    PLC_Phy::SetHeaderSymbolDuration (Time::FromInteger ((uint64_t) ((m_ofdmParameters[bandPlanType].m_subcarriersNumber
                                    + m_ofdmParameters[bandPlanType].m_headerGuardInterval) * m_ofdmParameters[bandPlanType].m_sampleDuration), Time::NS));
    PLC_Phy::SetSymbolDuration (NanoSeconds (42240));
    PLC_HalfDuplexOfdmPhy::SetGuardIntervalDuration (NanoSeconds (1280));
    PLC_FullDuplexOfdmPhy::SetGuardIntervalDuration (NanoSeconds (1280));
    //Duration of the preamble is defined for PLC in samples: beta + N1 * N / k1 + N2 * N / k2
    //Constants: N1 = 7, N2 = 2, k1 = k2 = 8
    Time preambleDuration = Time::FromInteger ((uint64_t) ((m_ofdmParameters[bandPlanType].m_windowSize + GDOTHN_PREAMBLE_N1
                            * (uint64_t) m_ofdmParameters[bandPlanType].m_subcarriersNumber / GDOTHN_PREAMBLE_K1 + GDOTHN_PREAMBLE_N2
                            * (uint64_t) m_ofdmParameters[bandPlanType].m_subcarriersNumber / GDOTHN_PREAMBLE_K2)
                    * m_ofdmParameters[bandPlanType].m_sampleDuration), Time::NS);
    NS_LOG_LOGIC("Preamble duration: " << preambleDuration);
    PLC_Preamble::SetDuration (preambleDuration);
  }
void
CsmaCdHelper::AttachChannel ()
  {
    NetDeviceContainer plcDevices = GetNetDevices ();

    Ptr<NetDevice> netDevice;
    for (NetDeviceContainer::Iterator i = plcDevices.Begin (); i != plcDevices.End (); ++i)
      {
        netDevice = (*i);
        m_channel->AddDevice (netDevice);
      }
    // Since we do not run an ns-3 simulation, the channel computation has to be triggered manually
    m_channel->InitTransmissionChannels ();
    m_channel->CalcTransmissionChannels ();
  }
void
CsmaCdHelper::CreateLogger ()
  {
    m_aggr = CreateObject<FileAggregator> (m_resDir + "cost.txt", FileAggregator::FORMATTED);
    m_aggr->Set3dFormat ("%.0f\t%.0f\t%.15f");
    m_aggr->Enable ();
    TraceConnect ("CostLog", "CostLogContext", MakeCallback (&FileAggregator::Write3d, m_aggr));
  }

}

