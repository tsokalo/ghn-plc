/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <fstream>
#include <functional>

#include <ns3/node.h>
#include <ns3/object-factory.h>
#include <ns3/log.h>
#include <ns3/abort.h>
#include "ns3/plc-helper.h"
//#include "ns3/plc-defs-extension.h"

#include "ns3/ghn-plc-helper.h"
//#include "ns3/ghn-plc-module.h"
#include "ns3/ghn-plc-greedy-udp-client.h"

namespace ns3
{
namespace ghn
{
////////////////////////// GhnPlcHelper ////////////////////////////////////
NS_LOG_COMPONENT_DEFINE("GhnPlcHelper");
NS_OBJECT_ENSURE_REGISTERED(GhnPlcHelper);

TypeId
GhnPlcHelper::GetTypeId (void)
{
  static TypeId tid = ns3::TypeId ("ns3::GhnPlcHelper").SetParent<Object> ()

  .AddTraceSource ("CostLog", "The cost at the moment of the network start up",
          MakeTraceSourceAccessor (&GhnPlcHelper::m_costTrace), "ns3::CostLogTrace::TracedCallback");
  return tid;
}

GhnPlcHelper::GhnPlcHelper (Ptr<const SpectrumModel> sm, Ptr<SpectrumValue> txPsd, PLC_NodeList& deviceNodes) :
        m_spectrum_model (sm), m_txPsd (txPsd), m_node_list (deviceNodes)
{
  // Modulation and Coding Schemes
  m_header_mcs = ModulationAndCodingScheme (BPSK, CODING_RATE_1_2, 0);
  m_payload_mcs = ModulationAndCodingScheme (BPSK, CODING_RATE_1_2, 0);

  // Default phy model to be used
  m_phyTid = GhnPlcPhyPmdHalfD::GetTypeId ();
  // Default mac model to be used
  m_macTid = GhnPlcDllMacCsma::GetTypeId ();
  m_bitLoadingTid = NcBlVarTxPsd::GetTypeId ();

  // Create ns3::Node for this NetDevice by default
  m_create_nodes = true;
  m_allowCooperation = false;
}
GhnPlcHelper::GhnPlcHelper (BandPlanType bandplan)
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
  m_phyTid = GhnPlcPhyPmdHalfD::GetTypeId ();
  // Default mac model to be used
  m_macTid = GhnPlcDllMacCsma::GetTypeId ();

  // Create ns3::Node for this NetDevice by default
  m_create_nodes = true;
  m_allowCooperation = false;
}

AddressMap
GhnPlcHelper::Setup (void)
{
  NS_ASSERT_MSG(m_txPsd, "TX psd not set!");

  NS_LOG_LOGIC("Setting up " << m_node_list.size() << " nodes");

  CreateLogger ();

  static uint32_t sid = 1;

  // Test if payload mcs is compatible
  if (m_phyTid == PLC_IncrementalRedundancyPhy::GetTypeId ())
    {
      NS_ASSERT_MSG(m_payload_mcs.ct >= CODING_RATE_RATELESS,
              "Payload coding type (" << m_payload_mcs << ") not compatible with PLC_IncrementalRedundancyPhy");
    }

  if (m_noiseFloor == NULL)
    {
      m_noiseFloor = CreateBestCaseBgNoise(m_spectrum_model)->GetNoisePsd ();
    }

  ObjectFactory netdeviceFactory;
  netdeviceFactory.SetTypeId (GhnPlcNetDevice::GetTypeId ());

  ObjectFactory phyFactory;
  phyFactory.SetTypeId (m_phyTid);

  //
  // Create net devices
  //
  PLC_NodeList::iterator nit;
  for (nit = m_node_list.begin (); nit != m_node_list.end (); nit++)
    {
      //Create net device
      Ptr<GhnPlcNetDevice> dev = netdeviceFactory.Create<GhnPlcNetDevice> ();

      dev->SetSpectrumModel (m_spectrum_model);
      dev->SetNoiseFloor (m_noiseFloor);
      dev->SetTxPowerSpectralDensity (m_txPsd);

      //Create PHYs
      Ptr<GhnPlcPhyManagement> phyManager = CreateObject<GhnPlcPhyManagement> ();
      Ptr<GhnPlcDllManagement> dllManager = CreateObject<GhnPlcDllManagement> ();

      Ptr<PLC_Phy> phy;
      if (m_phyTid == GhnPlcPhyPmdHalfD::GetTypeId ())
        {
          NS_LOG_LOGIC("Half duplex PHY is chosen");
          Ptr<GhnPlcPhyPcs> pcs = CreateObject<GhnPlcPhyPcs> ();
          Ptr<GhnPlcPhyPma> pma = CreateObject<GhnPlcPhyPma> ();
          Ptr<GhnPlcPhyPmdHalfD> pmd = CreateObject<GhnPlcPhyPmdHalfD> ();
          phy = pmd;

          phyManager->SetPhyPcs (pcs);
          phyManager->SetPhyPma (pma);
          pcs->SetPhyManagement (phyManager);
          pma->SetPhyManagement (phyManager);

          pcs->SetPhyPma (pma);
          pma->SetPhyPcs (pcs);
          pmd->SetPhyPma (pma);

          phyManager->SetFrameSizeCallback (MakeCallback (&GhnPlcPhyPmdHalfD::GetFrameSize, pmd));
          phyManager->SetGatheredInfBitsCallback (MakeCallback (&GhnPlcPhyPmdHalfD::GetGatheredInformationBits, pmd));
          phyManager->SetTxInterfaceCallback (
                  MakeCallback (&PLC_HalfDuplexOfdmPhy::GetTxInterface, pmd->GetObject<PLC_HalfDuplexOfdmPhy> ()));
          phyManager->SetRxInterfaceCallback (
                  MakeCallback (&PLC_HalfDuplexOfdmPhy::GetRxInterface, pmd->GetObject<PLC_HalfDuplexOfdmPhy> ()));
          pma->SetSendCallback (MakeCallback (&GhnPlcPhyPmdHalfD::Send, pmd));
          pma->SetBandPlanType (m_bandplan);
          pma->SetGetPmcScheme (
                  MakeCallback (&PLC_InformationRatePhy::GetPayloadModulationAndCodingScheme,
                          pmd->GetObject<PLC_InformationRatePhy> ()));

          // Set the function to be called after successful packet reception by phy2
          pmd->SetReceiveSuccessCallback (MakeCallback (&GhnPlcPhyPcs::ReceiveSuccess, pcs));
          pmd->SetReceiveErrorCallback (MakeCallback (&GhnPlcPhyPcs::ReceiveFailure, pcs));

          NS_ASSERT(m_ncStats);
          pcs->TraceConnectWithoutContext ("PhyRcvFailureLog", MakeCallback (&GhnPlcStats::NotifyPhyFailure, m_ncStats));
          pcs->TraceConnectWithoutContext ("PhyRcvSuccessLog", MakeCallback (&GhnPlcStats::NotifyPhySuccess, m_ncStats));
          dllManager->SetPhyPcs (pcs);

          pmd->SetBandplan (m_bandplan);

          // Set modulation and coding scheme
          pmd->SetPayloadModulationAndCodingScheme (ModulationAndCodingScheme (QAM4, CODING_RATE_2_3, 0));

          //Set FEC block size
          phyManager->SetTxFecBlockSize (FEC_BLOCK_SIZE_540);

          //Set FEC rate
          phyManager->SetTxPayloadFecRate (ConvertPlcRateToGhnRate (CODING_RATE_2_3));

          //Set number of repetitions
          phyManager->SetTxRepetitionsNumber (ENCODING_REPETITIONS_1);

        }
      else if (m_phyTid == GhnPlcPhyPmdFullD::GetTypeId ())
        {
          NS_LOG_LOGIC("Full duplex PHY is chosen");
          Ptr<GhnPlcPhyPcs> pcs = CreateObject<GhnPlcPhyPcs> ();
          Ptr<GhnPlcPhyPma> pma = CreateObject<GhnPlcPhyPma> ();
          Ptr<GhnPlcPhyPmdFullD> pmd = CreateObject<GhnPlcPhyPmdFullD> ();
          phy = pmd;

          phyManager->SetPhyPcs (pcs);
          phyManager->SetPhyPma (pma);
          pcs->SetPhyManagement (phyManager);
          pma->SetPhyManagement (phyManager);

          pcs->SetPhyPma (pma);
          pma->SetPhyPcs (pcs);
          pmd->SetPhyPma (pma);

          phyManager->SetFrameSizeCallback (MakeCallback (&GhnPlcPhyPmdFullD::GetFrameSize, pmd));
          phyManager->SetGatheredInfBitsCallback (MakeCallback (&GhnPlcPhyPmdFullD::GetGatheredInformationBits, pmd));
          phyManager->SetTxInterfaceCallback (
                  MakeCallback (&PLC_FullDuplexOfdmPhy::GetTxInterface, pmd->GetObject<PLC_FullDuplexOfdmPhy> ()));
          phyManager->SetRxInterfaceCallback (
                  MakeCallback (&PLC_FullDuplexOfdmPhy::GetRxInterface, pmd->GetObject<PLC_FullDuplexOfdmPhy> ()));
          pma->SetSendCallback (MakeCallback (&GhnPlcPhyPmdFullD::Send, pmd));
          pma->SetBandPlanType (m_bandplan);
          pma->SetGetPmcScheme (
                  MakeCallback (&PLC_InfRateFDPhy::GetPayloadModulationAndCodingScheme, pmd->GetObject<PLC_InfRateFDPhy> ()));

          // Set the function to be called after successful packet reception by phy2
          pmd->SetReceiveSuccessCallback (MakeCallback (&GhnPlcPhyPcs::ReceiveSuccess, pcs));
          pmd->SetReceiveErrorCallback (MakeCallback (&GhnPlcPhyPcs::ReceiveFailure, pcs));

          pmd->GetObject<PLC_InfRateFDPhy> ()->SetEndRxHeaderCallback (MakeCallback (&GhnPlcPhyPmdFullD::EndRxHeader, pmd));

          NS_ASSERT(m_ncStats);
          pcs->TraceConnectWithoutContext ("PhyRcvFailureLog", MakeCallback (&GhnPlcStats::NotifyPhyFailure, m_ncStats));
          pcs->TraceConnectWithoutContext ("PhyRcvSuccessLog", MakeCallback (&GhnPlcStats::NotifyPhySuccess, m_ncStats));
          dllManager->SetPhyPcs (pcs);

          pmd->SetBandplan (m_bandplan);

          // Set modulation and coding scheme
          pmd->SetPayloadModulationAndCodingScheme (ModulationAndCodingScheme (QAM4, CODING_RATE_2_3, 0));

          //Set FEC block size
          phyManager->SetTxFecBlockSize (FEC_BLOCK_SIZE_540);

          //Set FEC rate
          phyManager->SetTxPayloadFecRate (ConvertPlcRateToGhnRate (CODING_RATE_2_3));

          //Set number of repetitions
          phyManager->SetTxRepetitionsNumber (ENCODING_REPETITIONS_1);
        }
      else
        {
          NS_ABORT_MSG("Incompatible PHY type!");
        }

      // Create DLL
      NS_ASSERT(m_macTid == GhnPlcDllMacCsma::GetTypeId () || m_macTid.IsChildOf (GhnPlcDllMacCsma::GetTypeId ()));
      if (m_macTid == GhnPlcDllMacCsmaCd::GetTypeId ())
        {
          NS_ASSERT(m_phyTid == GhnPlcPhyPmdFullD::GetTypeId ());
        }

      dllManager->DefineMacType (m_macTid);
      dllManager->CreateDllMac ();
      if (m_phyTid == GhnPlcPhyPmdHalfD::GetTypeId ())
        {
          dllManager->GetDllMac ()->SetSetMcsCallback (
                  MakeCallback (&PLC_InformationRatePhy::SetPayloadModulationAndCodingScheme,
                          StaticCast<PLC_InformationRatePhy, PLC_Phy> (phy)));
          dllManager->GetDllMac ()->SetSetTxPsdCallback (
                  MakeCallback (&PLC_HalfDuplexOfdmPhy::SetTxPowerSpectralDensity,
                          StaticCast<PLC_HalfDuplexOfdmPhy, PLC_Phy> (phy)));
        }
      else if (m_phyTid == GhnPlcPhyPmdFullD::GetTypeId ())
        {

          dllManager->GetDllMac ()->SetSetMcsCallback (
                  MakeCallback (&PLC_InfRateFDPhy::SetPayloadModulationAndCodingScheme,
                          StaticCast<PLC_InfRateFDPhy, PLC_Phy> (phy)));
          dllManager->GetDllMac ()->SetSetTxPsdCallback (
                  MakeCallback (&PLC_FullDuplexOfdmPhy::SetTxPowerSpectralDensity,
                          StaticCast<PLC_FullDuplexOfdmPhy, PLC_Phy> (phy)));

          StaticCast<PLC_InfRateFDPhy, PLC_Phy> (phy)->SetCollisionDetection (
                  MakeCallback (&GhnPlcDllMacCsmaCd::CollisionDetection,
                          (StaticCast<GhnPlcDllMacCsmaCd, GhnPlcDllMac> (dllManager->GetDllMac ()))));
        }
      else
        {
          NS_ABORT_MSG("Incompatible PHY type!");
        }

      Ptr<GhnPlcDllApc> apc = CreateObject<GhnPlcDllApc> ();
      Ptr<GhnPlcDllLlc> llc = CreateObject<GhnPlcDllLlc> ();
      llc->SetCreateFlowCallback (MakeCallback (&GhnPlcHelper::CreateFlow, this));
      llc->AllowCooperation (m_allowCooperation);

      dllManager->SetDllApc (apc);
      dllManager->SetDllLlc (llc);
      dllManager->SetPhyManagement (phyManager);
      dllManager->GetDllMac ()->SetMinCw (0);
      dllManager->GetDllMac ()->SetMaxCw (m_maxCwSize);
      dllManager->GetDllMac ()->SetBackoffSlotDuration (NanoSeconds (GDOTHN_IST));
      dllManager->GetDllMac ()->AllowCooperation (m_allowCooperation);
      if (!m_sp.mutualPhyLlcCoding) assert(!m_sp.mutualPhyLlcCoding);

      if (m_macTid == GhnPlcDllMacCsma::GetTypeId ())
        {
          NS_LOG_LOGIC("CSMA CA is chosen");
          dllManager->GetDllMac ()->SetCcaRequestCallback (
                  MakeCallback (&PLC_HalfDuplexOfdmPhy::CcaRequest, StaticCast<PLC_HalfDuplexOfdmPhy, PLC_Phy> (phy)));
          dllManager->GetDllMac ()->SetCcaCancelCallback (
                  MakeCallback (&PLC_HalfDuplexOfdmPhy::CancelCca, StaticCast<PLC_HalfDuplexOfdmPhy, PLC_Phy> (phy)));

          phy->SetFrameSentCallback (MakeCallback (&GhnPlcDllMac::NotifyTransmissionEnd, dllManager->GetDllMac ()));
          StaticCast<PLC_HalfDuplexOfdmPhy, PLC_Phy> (phy)->SetCcaConfirmCallback (
                  MakeCallback (&GhnPlcDllMacCsma::CcaConfirm, dllManager->GetDllMac ()));
        }
      else if (m_macTid == GhnPlcDllMacCsmaCd::GetTypeId ())
        {
          NS_LOG_LOGIC("CSMA CD is chosen");
          StaticCast<GhnPlcDllMacCsmaCd, GhnPlcDllMacCsma> (dllManager->GetDllMac ())->SetCcaRequestCallback (
                  MakeCallback (&PLC_FullDuplexOfdmPhy::CcaRequest, StaticCast<PLC_FullDuplexOfdmPhy, PLC_Phy> (phy)));
          StaticCast<GhnPlcDllMacCsmaCd, GhnPlcDllMacCsma> (dllManager->GetDllMac ())->SetCcaCancelCallback (
                  MakeCallback (&PLC_FullDuplexOfdmPhy::CancelCca, StaticCast<PLC_FullDuplexOfdmPhy, PLC_Phy> (phy)));

          phy->SetFrameSentCallback (MakeCallback (&GhnPlcDllMacCsma::NotifyTransmissionEnd, dllManager->GetDllMac ()));
          StaticCast<PLC_FullDuplexOfdmPhy, PLC_Phy> (phy)->SetCcaConfirmCallback (
                  MakeCallback (&GhnPlcDllMacCsmaCd::CcaConfirm,
                          StaticCast<GhnPlcDllMacCsmaCd, GhnPlcDllMacCsma> (dllManager->GetDllMac ())));
        }

      dev->SetDllManagement (dllManager);
      dev->SetDllApc (apc);
      dev->SetPhy (phy);
      dllManager->SetTxPsdCallback (MakeCallback (&PLC_NetDevice::SetTxPowerSpectralDensity, dev));

      apc->SetNetDevice (dev);
      apc->SetDllLlc (llc);
      apc->SetApduForwardDownCallback (MakeCallback (&GhnPlcDllLlc::SendFrom, llc));
      apc->SetAdpForwardUpCallback (MakeCallback (&GhnPlcNetDevice::Receive, dev));

      llc->SetDllApc (apc);
      llc->SetApduForwardUpCallback (MakeCallback (&GhnPlcDllApc::Receive, apc));
      llc->SetResDirectory (m_resDir);

      dllManager->GetDllMac ()->SetResDirectory (m_resDir);

      if (m_rxImpedance) dev->SetRxImpedance (m_rxImpedance);

      if (m_txImpedance) dev->SetTxImpedance (m_txImpedance);

      auto addr = UanAddress::Allocate ();
      m_addressMap[std::distance (m_node_list.begin (), nit)] = addr;
      dev->SetAddress (addr);

      // Setting PLC node here to complete config
      dev->SetPlcNode (*nit);

      if (m_create_nodes)
        {
          // Create ns-3 node
          Ptr<Node> node = CreateObject<Node> (sid++);

          // Bind device to ns-3 node
          node->AddDevice (dev);

          NS_ASSERT(dev->ConfigComplete ());
        }

      std::string name = (*nit)->GetName ();
      NS_ASSERT_MSG(m_netdeviceMap.find (name) == m_netdeviceMap.end (), "Duplicate netdevice name");
      m_netdeviceMap[name] = dev;

      dllManager->SetNNodes (m_node_list.size ());
    }

  NcNetdeviceMap::iterator dit;
  for (dit = m_netdeviceMap.begin (); dit != m_netdeviceMap.end (); dit++)
    {
      dit->second->SetVitualBroadcastCallback (MakeCallback (&GhnPlcHelper::VitualBroadcast, this));
    }

  AttachChannel ();
  CreateBitLoadingTable ();
  CreateRoutingTable ();

  return m_addressMap;
}

void
GhnPlcHelper::CreateRoutingTable ()
{
  m_routingTable = CreateObject<GhnPlcRoutingTable> ();

  PLC_NodeList::iterator nit;
  for (nit = m_node_list.begin (); nit != m_node_list.end (); nit++)
    {
      m_routingTable->AddNode (*nit);
    }

  m_routingTable->SetResDirectory (m_resDir);
  m_routingTable->SetChannel (m_channel);
  m_routingTable->SetBitLoadingTable (m_bitLoadingTable);
  m_routingTable->CalcRoutingTable ();

  for (auto p : m_netdeviceMap)
    p.second->GetDllManagement ()->SetRoutingTable (m_routingTable);

  m_routingTable->PrintRoutingTable ();
}
void
GhnPlcHelper::CreateBitLoadingTable ()
{
  NS_LOG_LOGIC("Creating bit loading table");
  ObjectFactory blFactory;
  blFactory.SetTypeId (m_bitLoadingTid);
  m_bitLoadingTable = blFactory.Create<GhnPlcBitLoading> ();

  m_bitLoadingTable->SetTxEnvelope (m_txPsd);
  m_bitLoadingTable->SetNoiseEnvelope (m_noiseFloor);

  PLC_NodeList::iterator nit;
  for (nit = m_node_list.begin (); nit != m_node_list.end (); nit++)
    {
      m_bitLoadingTable->AddNode (*nit);
    }

  if (m_bitLoadingTid == NcBlVarBatMap::GetTypeId ())
    {
      for (nit = m_node_list.begin (); nit != m_node_list.end (); nit++)
        {
          m_bitLoadingTable->GetObject<NcBlVarBatMap> ()->SetPer ((*nit)->GetVertexId (),
                  m_sp.mutualPhyLlcCoding ? m_sp.per : 0.01);
        }
    }

  m_bitLoadingTable->SetResDirectory (m_resDir);
  m_bitLoadingTable->SetChannel (m_channel);
  m_bitLoadingTable->CalcBitLoadingTable ();

  for (auto p : m_netdeviceMap)
    p.second->GetDllManagement ()->SetBitLoadingTable (m_bitLoadingTable);
  ;

  m_bitLoadingTable->PrintBitLoadingTable ();
}

void
GhnPlcHelper::SetNodeList (PLC_NodeList& deviceNodes)
{
  m_node_list = deviceNodes;
}

Ptr<const SpectrumModel>
GhnPlcHelper::GetSpectrumModel ()
{
  return m_spectrum_model;
}

void
GhnPlcHelper::DefinePhyType (TypeId tid)
{
  m_phyTid = tid;
}

void
GhnPlcHelper::DefineMacType (TypeId tid)
{
  m_macTid = tid;
}

void
GhnPlcHelper::DefineBitLoadingType (TypeId id)
{
  m_bitLoadingTid = id;
}
void
GhnPlcHelper::SetRxImpedance (Ptr<PLC_Impedance> rxImpedance)
{
  m_rxImpedance = rxImpedance->Copy ();

  if (m_netdeviceMap.size () > 0)
    {
      NcNetdeviceMap::iterator nit;
      for (nit = m_netdeviceMap.begin (); nit != m_netdeviceMap.end (); nit++)
        {
          nit->second->SetRxImpedance (m_rxImpedance);
        }
    }
}

void
GhnPlcHelper::SetTxImpedance (Ptr<PLC_Impedance> txImpedance)
{
  m_txImpedance = txImpedance->Copy ();

  if (m_netdeviceMap.size () > 0)
    {
      NcNetdeviceMap::iterator nit;
      for (nit = m_netdeviceMap.begin (); nit != m_netdeviceMap.end (); nit++)
        {
          nit->second->SetTxImpedance (m_txImpedance);
        }
    }
}

Ptr<GhnPlcNetDevice>
GhnPlcHelper::GetDevice (std::string name)
{
  NS_ASSERT_MSG(m_netdeviceMap.find (name) != m_netdeviceMap.end (), "Unknown net device");
  return m_netdeviceMap[name];
}

NodeContainer
GhnPlcHelper::GetNS3Nodes (void)
{
  NodeContainer c;
  NcNetdeviceMap::iterator dit;
  for (dit = m_netdeviceMap.begin (); dit != m_netdeviceMap.end (); dit++)
    {
      c.Add (dit->second->GetNode ());
    }

  return c;
}

NetDeviceContainer
GhnPlcHelper::GetNetDevices (void)
{
  NetDeviceContainer c;
  NcNetdeviceMap::iterator dit;
  for (dit = m_netdeviceMap.begin (); dit != m_netdeviceMap.end (); dit++)
    {
      c.Add (dit->second);
    }

  return c;
}

void
GhnPlcHelper::SetChannel (Ptr<PLC_Channel> channel)
{
  m_channel = channel;
}
bool
GhnPlcHelper::IsCommunicationPossible (uint32_t src_id, uint32_t dst_id)
{
  auto src_address = m_addressMap[src_id];
  auto dst_address = m_addressMap[dst_id];
  NS_LOG_UNCOND(
          "Route between " << src_id << "(" << (uint32_t)src_address.GetAsInt() << ") " << " and " << dst_id << "(" << (uint32_t)dst_address.GetAsInt() << ") " << ": " << (m_routingTable->DoesRouteExist (src_address, dst_address) ? "exists" : "does not exist"));
  return m_routingTable->DoesRouteExist (src_address, dst_address);
}
void
GhnPlcHelper::PrintCostTable (uint32_t dst_id)
{
  std::ofstream f (m_resDir + "cost.txt", std::ios_base::out);

  std::cout << "Cost table:" << std::endl;
  auto dst_address = m_addressMap[dst_id];
  NcNetdeviceMap::iterator dit;
  for (dit = m_netdeviceMap.begin (); dit != m_netdeviceMap.end (); dit++)
    {
      auto src_address = dit->second->GetDllManagement ()->GetAddress ();
      if (src_address == dst_address) continue;
      std::cout << std::setprecision (0) << (uint16_t) src_address.GetAsInt () << "\t" << (uint16_t) dst_address.GetAsInt ()
              << "\t" << std::setprecision (20) << m_routingTable->GetRouteCost (src_address, dst_address) << std::endl;

      f << std::setprecision (0) << (uint16_t) src_address.GetAsInt () << "\t" << (uint16_t) dst_address.GetAsInt () << "\t"
              << std::setprecision (20) << m_routingTable->GetRouteCost (src_address, dst_address) << std::endl;
    }
  f.close ();
}
FlowInterface
GhnPlcHelper::CreateFlow (ConnId connId, Ptr<GhnPlcDllMacCsma> mac, Ptr<GhnPlcDllApc> apc, Ptr<GhnPlcDllLlc> llc)
{
  ObjectFactory factory;
  if (m_allowCooperation && connId.dst != mac->GetDllManagement ()->GetBroadcast () && connId.flowId != MANAGMENT_CONN_ID)
    {
      auto own_addr = mac->GetDllManagement ()->GetAddress ();
      factory.SetTypeId (GhnPlcLlcCodedFlow::GetTypeId ());
      auto flow_o = factory.Create<GhnPlcLlcCodedFlow> ();
      flow_o->SetConnId (connId);
      flow_o->SetDllMac (mac);
      flow_o->SetDllApc (apc);
      flow_o->SetDllLlc (llc);
      flow_o->SetResDirectory (m_resDir);

      FlowInterface flow_i (MakeCallback (&GhnPlcLlcCodedFlow::SendFrom, flow_o),
              MakeCallback (&GhnPlcLlcCodedFlow::Receive, flow_o), MakeCallback (&GhnPlcLlcCodedFlow::ReceiveAck, flow_o),
              MakeCallback (&GhnPlcLlcCodedFlow::IsQueueEmpty, flow_o), MakeCallback (&GhnPlcLlcCodedFlow::SendDown, flow_o));

      ncr::NodeType type =
              (own_addr == connId.src && connId.dst != UanAddress::GetBroadcast ()) ? ncr::SOURCE_NODE_TYPE :
                      ncr::RELAY_NODE_TYPE;

      auto app = m_appMap.find (own_addr);
      if (type == ncr::SOURCE_NODE_TYPE) assert(app != m_appMap.end ());
      Callback<void, uint32_t> cb;
      cb.Nullify ();
      if (type == ncr::SOURCE_NODE_TYPE) cb = MakeCallback (&GhnPlcGreedyUdpClient::SendBatch,
              app->second->GetObject<GhnPlcGreedyUdpClient> ());

      m_sp.sendRate = 5000;
      flow_o->Configure (type, connId.dst.GetAsInt (), m_sp, cb);

      m_flowStack.push_back (flow_o);

      return flow_i;
    }
  else
    {
      factory.SetTypeId (GhnPlcLlcFlow::GetTypeId ());
      auto flow_o = factory.Create<GhnPlcLlcFlow> ();
      flow_o->SetConnId (connId);
      flow_o->SetDllMac (mac);
      flow_o->SetDllApc (apc);
      flow_o->SetDllLlc (llc);
      flow_o->SetResDirectory (m_resDir);

      FlowInterface flow_i (MakeCallback (&GhnPlcLlcFlow::SendFrom, flow_o), MakeCallback (&GhnPlcLlcFlow::Receive, flow_o),
              MakeCallback (&GhnPlcLlcFlow::ReceiveAck, flow_o), MakeCallback (&GhnPlcLlcFlow::IsQueueEmpty, flow_o),
              MakeCallback (&GhnPlcLlcFlow::SendDown, flow_o));

      m_flowStack.push_back (flow_o);

      return flow_i;
    }
}
void
GhnPlcHelper::VitualBroadcast (Ptr<Packet> packet, const UanAddress& source, const UanAddress& dest)
{
  NcNetdeviceMap::iterator dit;
  for (dit = m_netdeviceMap.begin (); dit != m_netdeviceMap.end (); dit++)
    {
      if (UanAddress::ConvertFrom (dit->second->GetAddress ()) == source) continue;
      dit->second->Receive (packet, source, dest);
    }
}
void
GhnPlcHelper::SetResDirectory (std::string resFolder)
{
  m_resDir = resFolder;
}
void
GhnPlcHelper::AllowCooperation (bool v)
{
  m_allowCooperation = v;
}
void
GhnPlcHelper::SetAppMap (std::map<UanAddress, Ptr<Application> > appMap)
{
  m_appMap = appMap;
}

Ptr<const SpectrumModel>
GhnPlcHelper::GetGhnSpectrumModel (BandPlanType bandplan)
{
  PLC_SpectrumModelHelper smHelper;
  Ptr<const SpectrumModel> sm;
  double fl = 0;
  double fh = m_ofdmParameters[bandplan].m_fUs * 2 * 1000000;
  sm = smHelper.GetSpectrumModel (fl, fh, m_ofdmParameters[bandplan].m_subcarriersNumber);
  return sm;
}
Ptr<SpectrumValue>
GhnPlcHelper::GetGhnTransmitPSD (BandPlanType bandplan, Ptr<const SpectrumModel> sm)
{
  // Define transmit power spectral density
  double FL3 = 2000; //in kHz
  double FH1 = 30000; //in kHz
  Ptr<SpectrumValue> txPsd = Create<SpectrumValue> (sm);
  uint16_t iFL3 = ceil (
          m_ofdmParameters[bandplan].m_subcarriersNumber / 2
                  - (0 + m_ofdmParameters[bandplan].m_fUs * 1000 - FL3) / m_ofdmParameters[bandplan].m_subcarrierSpacing);
  for (uint16_t i = 0; i < PERMANENTLY_MASKED_SUBCARRIERS; i++)
    (*txPsd)[i] = -150; //dBm/Hz
  for (uint16_t i = PERMANENTLY_MASKED_SUBCARRIERS; i <= iFL3; i++)
    (*txPsd)[i] = -85; //dBm/Hz
  uint16_t iFH1 = floor (
          m_ofdmParameters[bandplan].m_subcarriersNumber / 2
                  - (0 + m_ofdmParameters[bandplan].m_fUs * 1000 - FH1) / m_ofdmParameters[bandplan].m_subcarrierSpacing);
  if (bandplan == GDOTHN_BANDPLAN_25MHZ)
    {
      for (uint16_t i = iFL3 + 1; i < m_ofdmParameters[bandplan].m_subcarriersNumber; i++)
        (*txPsd)[i] = -55; //dBm/Hz
    }
  else
    {
      for (uint16_t i = iFL3 + 1; i <= iFH1; i++)
        (*txPsd)[i] = -55; //dBm/Hz
      for (uint16_t i = iFH1 + 1; i < m_ofdmParameters[bandplan].m_subcarriersNumber; i++)
        (*txPsd)[i] = -85; //dBm/Hz
    }

  // convert to W/Hz
  (*txPsd) = Pow (10.0, ((*txPsd) - 30) / 10.0);
  //  NS_LOG_LOGIC("Transmission PSD: " << *txPsd);
  return txPsd;
}
void
GhnPlcHelper::SetPhyParameters (BandPlanType bandPlanType)
{
  PLC_Phy::SetHeaderSymbolDuration (
          Time::FromInteger (
                  (uint64_t) ((m_ofdmParameters[bandPlanType].m_subcarriersNumber
                          + m_ofdmParameters[bandPlanType].m_headerGuardInterval)
                          * m_ofdmParameters[bandPlanType].m_sampleDuration), Time::NS));
  PLC_Phy::SetSymbolDuration (NanoSeconds (42240));
  PLC_HalfDuplexOfdmPhy::SetGuardIntervalDuration (NanoSeconds (1280));
  PLC_FullDuplexOfdmPhy::SetGuardIntervalDuration (NanoSeconds (1280));
  //Duration of the preamble is defined for PLC in samples: beta + N1 * N / k1 + N2 * N / k2
  //Constants: N1 = 7, N2 = 2, k1 = k2 = 8
  Time preambleDuration = Time::FromInteger (
          (uint64_t) ((m_ofdmParameters[bandPlanType].m_windowSize
                  + GDOTHN_PREAMBLE_N1 * (uint64_t) m_ofdmParameters[bandPlanType].m_subcarriersNumber / GDOTHN_PREAMBLE_K1
                  + GDOTHN_PREAMBLE_N2 * (uint64_t) m_ofdmParameters[bandPlanType].m_subcarriersNumber / GDOTHN_PREAMBLE_K2)
                  * m_ofdmParameters[bandPlanType].m_sampleDuration), Time::NS);
  PLC_Preamble::SetDuration (preambleDuration);

  NS_LOG_UNCOND("Header symbol duration: " << PLC_Phy::GetHeaderSymbolDuration());
  NS_LOG_UNCOND("Preamble duration: " << preambleDuration);
}
void
GhnPlcHelper::AttachChannel ()
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
GhnPlcHelper::CreateLogger ()
{
  m_aggr = CreateObject<FileAggregator> (m_resDir + "cost.txt", FileAggregator::FORMATTED);
  m_aggr->Set3dFormat ("%.0f\t%.0f\t%.15f");
  m_aggr->Enable ();
  TraceConnect ("CostLog", "CostLogContext", MakeCallback (&FileAggregator::Write3d, m_aggr));
}
}
} // namespace ns3
