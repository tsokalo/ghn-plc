/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef GHN_PLC_HELPER_H
#define GHN_PLC_HELPER_H

#include <string>
#include <map>
#include <memory>

#include <ns3/object.h>
#include <ns3/random-variable-stream.h>
#include <ns3/node-container.h>
#include "ns3/net-device-container.h"
#include "ns3/file-aggregator.h"
#include "ns3/uan-address.h"

#include "ns3/plc-node.h"
#include "ns3/plc-outlet.h"
#include "ns3/plc-noise.h"
#include "ns3/plc-edge.h"
#include "ns3/plc-channel.h"
#include "ns3/plc-link-performance-model.h"
#include "ns3/plc-phy.h"
#include "ns3/plc-mac.h"
#include "ns3/plc-spectrum-helper.h"

#include "ns3/ghn-plc-net-device.h"
#include "ns3/ghn-plc-routing-table.h"
#include "ns3/ghn-plc-defs.h"
#include "ns3/ghn-plc-bit-loading.h"
#include "ns3/ghn-plc-stats.h"
#include "ns3/ghn-plc-halfd-phy-pmd.h"
#include "ns3/ghn-plc-fulld-phy-pmd.h"
#include "ns3/ghn-plc-llc-coded-flow.h"

#include "logger.h"

namespace ns3
{
namespace ghn
{
typedef std::map<std::string, Ptr<GhnPlcNetDevice> > NcNetdeviceMap;
typedef std::map<uint32_t, UanAddress> AddressMap;
typedef std::shared_ptr<ncr::SimParameters> sim_par_ptr;

class GhnPlcHelper : public Object
{
  typedef std::shared_ptr<ncr::Logger> logger_ptr;

public:
  static TypeId
  GetTypeId (void);

  GhnPlcHelper (BandPlanType bandplan, Ptr<const SpectrumModel> sm);
  GhnPlcHelper (BandPlanType bandplan);
  void
  SetNodeList (PLC_NodeList& deviceNodes);
  PLC_NodeList
  GetNodeList ()
  {
    return m_node_list;
  }
  Ptr<const SpectrumModel>
  GetSpectrumModel ();

  void
  DefinePhyType (TypeId tid);
  void
  DefineMacType (TypeId tid);
  void
  DefineBitLoadingType (TypeId id);

  void
  SetNoiseFloor (Ptr<const SpectrumValue> psd)
  {
    m_noiseFloor = psd;
  }
  Ptr<const SpectrumValue>
  GetNoiseFloor ()
  {
    return m_noiseFloor;
  }
  void
  SetTxPowerSpectralDensity (Ptr<SpectrumValue> txPsd)
  {
    m_txPsd = txPsd;
  }
  Ptr<SpectrumValue>
  GetTxPsd ()
  {
    return m_txPsd;
  }
  void
  SetRxImpedance (Ptr<PLC_Impedance> rxImpedance);
  void
  SetTxImpedance (Ptr<PLC_Impedance> txImpedance);
  Ptr<PLC_Impedance>
  GetRxImpedance (void)
  {
    return m_rxImpedance;
  }
  Ptr<PLC_Impedance>
  GetTxImpedance (void)
  {
    return m_txImpedance;
  }
  void
  SetHeaderModulationAndCodingScheme (ModulationAndCodingScheme mcs)
  {
    m_header_mcs = mcs;
  }
  void
  SetPayloadModulationAndCodingScheme (ModulationAndCodingScheme mcs)
  {
    m_payload_mcs = mcs;
  }

  AddressMap
  Setup (void);

  void
  CreateNodes (bool create)
  {
    m_create_nodes = create;
  }

  Ptr<GhnPlcNetDevice>
  GetDevice (std::string name);

  NodeContainer
  GetNS3Nodes (void);
  NetDeviceContainer
  GetNetDevices (void);

  NcNetdeviceMap
  GetNetdeviceMap (void)
  {
    return m_netdeviceMap;
  }

  void
  SetChannel (Ptr<PLC_Channel> channel);

  void
  SetGhnPlcStats (Ptr<GhnPlcStats> ncStats)
  {
    m_ncStats = ncStats;
  }
  bool
  IsCommunicationPossible (uint32_t src_id, uint32_t dst_id);
  void
  PrintCostTable (uint32_t dst_id);

  FlowInterface
  CreateFlow (ConnId connId, Ptr<GhnPlcDllMacCsma> mac, Ptr<GhnPlcDllApc> apc, Ptr<GhnPlcDllLlc> llc);

  void
  VitualBroadcast (Ptr<Packet> packet, const UanAddress& source, const UanAddress& dest);

  void
  SetResDirectory (std::string resFolder);
  void
  SetMaxCwSize (uint16_t maxCwSize)
  {
    m_maxCwSize = maxCwSize;
  }
  void
  AllowCooperation (bool v = true);
  void
  StickToMainPath (bool v = true);
  void
  SetImmediateFeedback (bool v = true);
  void
  SetLowerSrcPriority (bool v = true);
  void
  SetForcePer (bool v = true);
  void
  SetSimParamFileName (std::string name)
  {
    m_simParamFileName = name;
  }

  void
  SetAppMap (std::map<UanAddress, Ptr<Application> > appMap);
  double
  GetAveMpduSize ();
  std::string
  GetLinDepRatios (uint16_t &num, UanAddress addr);
  std::string
  GetSuccessDeliveryRatio (uint16_t &num, UanAddress addr);
  double
  GetAveCoalitionSize (UanAddress addr);

protected:

  Ptr<const SpectrumModel>
  GetGhnSpectrumModel (BandPlanType bandplan);
  Ptr<SpectrumValue>
  GetGhnTransmitPSD (BandPlanType bandplan, Ptr<const SpectrumModel> sm);
  void
  SetPhyParameters (BandPlanType bandPlanType);
  void
  AttachChannel ();
  void
  CreateRoutingTable ();
  void
  CreateBitLoadingTable ();
  void
  CreateLogger ();

  Ptr<const SpectrumModel> m_spectrum_model;
  Ptr<PLC_Channel> m_channel;
  Ptr<SpectrumValue> m_txPsd;
  PLC_NodeList m_node_list;
  Ptr<const SpectrumValue> m_noiseFloor;
  Ptr<PLC_Impedance> m_rxImpedance;
  Ptr<PLC_Impedance> m_txImpedance;
  ModulationAndCodingScheme m_header_mcs;
  ModulationAndCodingScheme m_payload_mcs;
  TypeId m_phyTid;
  TypeId m_macTid;
  TypeId m_bitLoadingTid;
  bool m_create_nodes;
  BandPlanType m_bandplan;

  NcNetdeviceMap m_netdeviceMap;
  Ptr<GhnPlcRoutingTable> m_routingTable;
  Ptr<GhnPlcBitLoading> m_bitLoadingTable;
  Ptr<GhnPlcStats> m_ncStats;
  sim_par_ptr m_sp;
  //
  // flags
  //
  bool m_allowCooperation;
  bool m_stickToMainPath;
  bool m_immediateFeedback;
  bool m_useLowerSrcPriority;
  bool m_forcePer;

  std::string m_simParamFileName;

  AddressMap m_addressMap;

  std::map<uint16_t, std::map<UanAddress, Ptr<GhnPlcLlcFlow> > > m_flowStack;
  std::string m_resDir;
  uint16_t m_maxCwSize;

  Ptr<FileAggregator> m_aggr;
  logger_ptr m_logger;
  //
  // <from ID> <to ID> <cost>
  //
  TracedCallback<double, double, double> m_costTrace;

  std::map<UanAddress, Ptr<Application> > m_appMap;
};
}
}

#endif /* GHN_PLC_HELPER_H */

