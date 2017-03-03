/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/plc-electrical-device.h"
#include "ns3/plc-channel.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/plc-time.h"
#include "ns3/plc-noise.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("PLC_Electrical_Device");
NS_OBJECT_ENSURE_REGISTERED(PLC_Electrical_Device);

TypeId
PLC_Electrical_Device::GetTypeId (void)
{
  static TypeId tid = ns3::TypeId ("ns3::PLC_Electrical_Device").SetParent<PLC_Node> ();
  return tid;
}

PLC_Electrical_Device::PLC_Electrical_Device () :
        PLC_Node ()
{
  NS_LOG_DEBUG("Electrical device Node Id: " << this->GetVertexId ());
  m_impedance = 0;
  m_impNoise = 0;
}
void
PLC_Electrical_Device::OnStart (SwitchingIntesity switchingIntesity)
{
  m_switchingIntesity = switchingIntesity;
  DefineInitialSwitchingState ();
  this->SetImpedance (m_impedance);

  Simulator::Schedule (Seconds (0.0), &PLC_Electrical_Device::SwitchState, this);
}
void
PLC_Electrical_Device::OnStop (void)
{
  m_impNoise->Stop ();
}
void
PLC_Electrical_Device::InitDevice (Ptr<const SpectrumModel> sm)
{
  NS_LOG_DEBUG("Creating impedance model for Node Id: " << this->GetVertexId ());
  CreateImpedance (sm);
  NS_LOG_DEBUG("Creating noise model for Node Id: " << this->GetVertexId ());
  CreateImpulsiveNoiseValue (sm);

  this->SetImpedance (0);
  m_impNoise->Stop ();
  m_actualState = OFF_SWITCHING_STATE;
}
void
PLC_Electrical_Device::Save (std::ostream &fo)
{
  SaveImpedance (fo);
  SaveImpulsiveNoise (fo);
}
void
PLC_Electrical_Device::Load (Ptr<const SpectrumModel> sm, std::istream &fi)
{
  LoadImpedance (sm, fi);
  LoadImpulsiveNoise (sm, fi);
}
void
PLC_Electrical_Device::SaveImpedance (std::ostream &fo)
{
  fo << m_impedanceProperties;
}
void
PLC_Electrical_Device::SaveImpulsiveNoise (std::ostream &fo)
{
  fo << m_impNoiseProperties;
}
void
PLC_Electrical_Device::LoadImpedance (Ptr<const SpectrumModel> sm, std::istream &fi)
{
  fi >> m_impedanceProperties;

  m_impedance = CreateObject < PLC_TimeVariantFreqSelectiveImpedance > (sm);

  NS_LOG_DEBUG("Loaded impedance type: " << m_impedanceProperties.impedanceType);

  for (uint16_t slotIndex = 0; slotIndex < PLC_Time::GetNumTimeslots (); slotIndex++)
    {
      uint16_t carrierIndex = 0;
      for (Bands::const_iterator band = sm->Begin (); band != sm->End (); band++)
        {
          PLC_Value value = GetImpedanceValue (m_impedanceProperties, band->fc,
                  Seconds (slotIndex * PLC_Time::GetResolutionS ()));
          //                    NS_LOG_UNCOND("Setting impedance t<" << slotIndex << "> f<" << carrierIndex << "> : r<" << value.real() << "> i<" << value.imag() << ">");
          m_impedance->Set (slotIndex, carrierIndex++, value.real (), value.imag ());
        }
    }
}
void
PLC_Electrical_Device::LoadImpulsiveNoise (Ptr<const SpectrumModel> sm, std::istream &fi)
{
  fi >> m_impNoiseProperties;

  PLC_TimeVaryingSpectrumValue impulsiveNoiseValue;

  for (uint16_t slotIndex = 0; slotIndex < PLC_Time::GetNumTimeslots (); slotIndex++)
    {
      //      NS_LOG_DEBUG("Calculating impulsive noise for time slot<" << slotIndex << ">");
      Ptr < SpectrumValue > noiseValue = Create < SpectrumValue > (sm);
      Values::iterator value = noiseValue->ValuesBegin ();
      //                        uint16_t carrierIndex = 0;

      for (Bands::const_iterator band = sm->Begin (); band != sm->End (); band++)
        {
          double noiseWHz = GetImpulseNoiseValue (m_impNoiseProperties, m_impedanceProperties, band,
                  Seconds (slotIndex * PLC_Time::GetResolutionS ()));
          (*value) = noiseWHz;
          value++;
          //          NS_LOG_DEBUG("Setting impulsive noise t<" << slotIndex << "> f<" << carrierIndex++ << "> : " << 30 + 10 * log(noiseWHz) << " dBm/Hz>");
        }
      impulsiveNoiseValue.push_back (noiseValue);
    }
  m_impNoise = Create < PLC_CustomImpulsiveNoiseSource > (this, impulsiveNoiseValue);
  m_impNoise->Init ();
}
void
PLC_Electrical_Device::UrgeOff ()
{
  NS_LOG_DEBUG("Turn off by demand");
  m_impNoise->Stop ();
  PLC_ImpedanceProperties impProp;
  this->OpenCircuit ();
  this->GetOutlet ()->SetImpedance (0, false);
  m_actualState = OFF_SWITCHING_STATE;
}
void
PLC_Electrical_Device::UrgeOn ()
{
  NS_LOG_DEBUG("Turn on by demand");
  this->CloseCircuit ();
  this->GetOutlet ()->SetImpedance (m_impedance, false);
  m_impNoise->Start ();
  m_actualState = ON_SWITCHING_STATE;
}
void
PLC_Electrical_Device::SetPhy (Ptr<PLC_Phy> phy)
{
  NS_LOG_FUNCTION(this << phy);
  m_phy = phy;
}
void
PLC_Electrical_Device::CreateImpedance (Ptr<const SpectrumModel> sm)
{
  m_impedance = CreateObject < PLC_TimeVariantFreqSelectiveImpedance > (sm);

  InitImpedanceProperties (m_impedanceProperties);

  NS_LOG_DEBUG("Created impedance type: " << m_impedanceProperties.impedanceType);

  for (uint16_t slotIndex = 0; slotIndex < PLC_Time::GetNumTimeslots (); slotIndex++)
    {
      uint16_t carrierIndex = 0;
      for (Bands::const_iterator band = sm->Begin (); band != sm->End (); band++)
        {
          PLC_Value value = GetImpedanceValue (m_impedanceProperties, band->fc,
                  Seconds (slotIndex * PLC_Time::GetResolutionS ()));
          //                    NS_LOG_UNCOND("Setting impedance t<" << slotIndex << "> f<" << carrierIndex << "> : r<" << value.real() << "> i<" << value.imag() << ">");
          m_impedance->Set (slotIndex, carrierIndex++, value.real (), value.imag ());
        }
    }
}

void
PLC_Electrical_Device::CreateImpulsiveNoiseValue (Ptr<const SpectrumModel> sm)
{
  InitImpulseNoiseProperties (m_impNoiseProperties, m_impedanceProperties);

  PLC_TimeVaryingSpectrumValue impulsiveNoiseValue;

  for (uint16_t slotIndex = 0; slotIndex < PLC_Time::GetNumTimeslots (); slotIndex++)
    {
      //      NS_LOG_DEBUG("Calculating impulsive noise for time slot<" << slotIndex << ">");
      Ptr < SpectrumValue > noiseValue = Create < SpectrumValue > (sm);
      Values::iterator value = noiseValue->ValuesBegin ();
      //            		uint16_t carrierIndex = 0;

      for (Bands::const_iterator band = sm->Begin (); band != sm->End (); band++)
        {
          double noiseWHz = GetImpulseNoiseValue (m_impNoiseProperties, m_impedanceProperties, band,
                  Seconds (slotIndex * PLC_Time::GetResolutionS ()));
          (*value) = noiseWHz;
          value++;
          //          NS_LOG_DEBUG("Setting impulsive noise t<" << slotIndex << "> f<" << carrierIndex++ << "> : " << 30 + 10 * log(noiseWHz) << " dBm/Hz>");
        }
      impulsiveNoiseValue.push_back (noiseValue);
    }
  m_impNoise = Create < PLC_CustomImpulsiveNoiseSource > (this, impulsiveNoiseValue);
  m_impNoise->Init ();

}
void
PLC_Electrical_Device::DefineInitialSwitchingState ()
{
  double probOnState = m_switchingIntesity.first / (m_switchingIntesity.first + m_switchingIntesity.second);
  UniformRandomVariable uniRV;
  m_actualState = (uniRV.GetValue (0, 1) < probOnState) ? ON_SWITCHING_STATE : OFF_SWITCHING_STATE;
  NS_LOG_DEBUG("Electrical device starts with the following initial state: " << m_actualState);
}
void
PLC_Electrical_Device::SwitchState ()
{
  //  if (m_actualState == ON_SWITCHING_STATE)
  //    {
  //      ExponentialVariable expRV (m_switchingIntesity.second);
  //      Time duration = Seconds (expRV.GetValue ());
  //      Simulator::Schedule (duration, &PLC_Electrical_Device::SwitchState, this);
  //      NS_LOG_DEBUG("Electrical device switches to OFF state. Planned duration in this state: " << duration);
  //
  //      m_impNoise->Stop ();
  //      PLC_ImpedanceProperties impProp;
  //      this->OpenCircuit ();
  //      //      this->SetImpedance (CreateObject<PLC_ConstImpedance> (this->GetImpedancePtr ()->GetSpectrumModel (),
  //      //              impProp.offStateImpedanceValue));
  //      m_actualState = OFF_SWITCHING_STATE;
  //    }
  //  else
  //    {
  //      ExponentialVariable expRV (m_switchingIntesity.first);
  //      Time duration = Seconds (expRV.GetValue ());
  //      Simulator::Schedule (duration, &PLC_Electrical_Device::SwitchState, this);
  //      NS_LOG_DEBUG("Electrical device switches to ON state. Planned duration in this state: " << duration);
  //
  //      this->CloseCircuit ();
  //      m_impNoise->Start ();
  //      m_actualState = ON_SWITCHING_STATE;
  //    }

}

}

