/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef __PLC_ELECTRICAL_DEVICE_H__
#define __PLC_ELECTRICAL_DEVICE_H__

#include <set>
#include "ns3/object.h"
#include "ns3/plc-defs.h"
#include "ns3/plc-edge.h"
#include "ns3/plc-node.h"
#include "ns3/plc-backbone.h"
#include "ns3/plc-defs-extension.h"
#include "ns3/event-id.h"
#include "ns3/random-variable-stream.h"

namespace ns3
{

/**
 * \class PLC_Electrical_Device
 *
 */
/*
 * fist - intensity in ON state
 * second - intensity in OFF state
 */
typedef std::pair<double, double> SwitchingIntesity;
enum SwitchingState
{
  ON_SWITCHING_STATE, OFF_SWITCHING_STATE
};

class PLC_Electrical_Device : public PLC_Node
{

public:

  PLC_Electrical_Device ();
  ~PLC_Electrical_Device ()
  {
  }

  void
  OnStart (SwitchingIntesity switchingIntesity);
  void
  OnStop (void);
  void
  InitDevice (Ptr<const SpectrumModel> sm);

  void
  UrgeOff ();
  void
  UrgeOn ();

  Ptr<PLC_CustomImpulsiveNoiseSource>
  GetImpulsiveNoiseSource ()
  {
    return m_impNoise;
  }
  Ptr<PLC_TimeVariantFreqSelectiveImpedance>
  GetImpedance ()
  {
    return m_impedance;
  }

  static TypeId
  GetTypeId (void);

  void
  SetPhy (Ptr<PLC_Phy> phy);
  Ptr<PLC_Phy>
  GetPhy ();

  void
  Save (std::ostream &fo);
  void
  Load (Ptr<const SpectrumModel> sm, std::istream &fi);

private:

  void
  CreateImpedance (Ptr<const SpectrumModel> sm);
  void
  CreateImpulsiveNoiseValue (Ptr<const SpectrumModel> sm);
  void
  DefineInitialSwitchingState ();
  void
  SwitchState ();

  void
  SaveImpedance (std::ostream &fo);
  void
  SaveImpulsiveNoise (std::ostream &fo);
  void
  LoadImpedance (Ptr<const SpectrumModel> sm, std::istream &fi);
  void
  LoadImpulsiveNoise (Ptr<const SpectrumModel> sm, std::istream &fi);

  Ptr<PLC_CustomImpulsiveNoiseSource> m_impNoise;
  Ptr<PLC_TimeVariantFreqSelectiveImpedance> m_impedance;

  PLC_ImpedanceProperties m_impedanceProperties;
  PLC_ImpulseNoiseProperties m_impNoiseProperties;

  SwitchingIntesity m_switchingIntesity;
  SwitchingState m_actualState;

  Ptr<PLC_Phy> m_phy;
};

}

#endif /* __PLC_ELECTRICAL_DEVICE_H__ */

