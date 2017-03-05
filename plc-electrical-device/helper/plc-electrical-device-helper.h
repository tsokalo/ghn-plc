/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef __PLC_ELECTRICAL_DEVICE_HELPER_H__
#define __PLC_ELECTRICAL_DEVICE_HELPER_H__

#include "ns3/plc-defs-extension.h"
#include "ns3/plc-electrical-device.h"

#include "ns3/ptr.h"
#include "ns3/ghn-plc-helper.h"

namespace ns3
{

class PlcElectricalDeviceHelper : public ghn::GhnPlcHelper
{
public:
  static TypeId
  GetTypeId (void);
  PlcElectricalDeviceHelper (ghn::BandPlanType bandplan, Ptr<const SpectrumModel> sm);

  void
  Setup (void);
  void
  Save(std::ofstream &fo);
  void
  Load(std::ifstream &fi);
  void
  SetSwitchingIntesity(SwitchingIntesity si);

private:

  SwitchingIntesity m_switchingIntesity;

};
}

#endif /* __PLC_ELECTRICAL_DEVICE_HELPER_H__ */

