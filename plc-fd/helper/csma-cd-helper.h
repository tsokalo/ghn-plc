/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef CSMA_CD_HELPER_H
#define CSMA_CD_HELPER_H

#include <string>
#include <ns3/object.h>
#include "ns3/object-factory.h"
#include <ns3/random-variable-stream.h>
#include <ns3/node-container.h>
#include "ns3/net-device-container.h"
#include "ns3/plc-node.h"
#include "ns3/plc-outlet.h"
#include "ns3/plc-noise.h"
#include "ns3/plc-edge.h"
#include "ns3/plc-net-device.h"
#include "ns3/plc-channel.h"
#include "ns3/plc-link-performance-model.h"
#include "ns3/plc-phy.h"
#include "ns3/plc-mac.h"
#include "ns3/plc-spectrum-helper.h"


#include "ns3/uan-address.h"
#include <map>
#include "ns3/file-aggregator.h"

#include "ns3/plc-fd.h"

namespace ns3
{

typedef std::map<std::string, Ptr<PLC_NetDevice> > PLC_NetDeviceMap;

class CsmaCdHelper : public Object
{
public:
  static TypeId
  GetTypeId (void);

  CsmaCdHelper (Ptr<const SpectrumModel> sm, Ptr<SpectrumValue> txPsd, PLC_NodeList& netdevices);

  CsmaCdHelper (ghn::BandPlanType bandplan);
  void
  SetNodeList (PLC_NodeList& deviceNodes);
  Ptr<const SpectrumModel>
  GetSpectrumModel ();

  void
  DefinePhyType (TypeId tid);
  void
  DefineMacType (TypeId tid);

  void
  SetNoiseFloor (Ptr<const SpectrumValue> psd)
  {
    m_noiseFloor = psd;
  }
  void
  SetTxPowerSpectralDensity (Ptr<SpectrumValue> txPsd)
  {
    m_txPsd = txPsd;
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

  void
  Setup (void);

  void
  CreateNodes (bool create)
  {
    m_create_nodes = create;
  }

  Ptr<PLC_NetDevice>
  GetDevice (std::string name);

  NodeContainer
  GetNS3Nodes (void);
  NetDeviceContainer
  GetNetDevices (void);

  PLC_NetDeviceMap
  GetNetdeviceMap (void)
  {
    return m_netdeviceMap;
  }

  void
  SetChannel (Ptr<PLC_Channel> channel);

  void
  SetResDirectory (std::string resFolder);

  void
  SetMaxCwSize(uint16_t maxCwSize)
  {
    m_maxCwSize = maxCwSize;
  }

protected:

  Ptr<const SpectrumModel>
  GetGhnSpectrumModel (ghn::BandPlanType bandplan);
  Ptr<SpectrumValue>
  GetGhnTransmitPSD (ghn::BandPlanType bandplan, Ptr<const SpectrumModel> sm);
  void
  SetPhyParameters (ghn::BandPlanType bandPlanType);
  void
  AttachChannel ();
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
  bool m_create_nodes;
  ghn::BandPlanType m_bandplan;
  PLC_NetDeviceMap m_netdeviceMap;
  uint16_t m_maxCwSize;
  std::string m_resDir;

  Ptr<FileAggregator> m_aggr;
};

}

#endif /* CSMA_CD_HELPER_H */

