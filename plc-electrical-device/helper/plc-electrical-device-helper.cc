/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/node.h"
#include "ns3/object-factory.h"
#include "ns3/log.h"
#include "ns3/abort.h"
#include "ns3/ghn-plc-module.h"
#include "ns3/plc-helper.h"
#include "ns3/assert.h"

#include "plc-electrical-device-helper.h"

namespace ns3
{

using namespace ghn;

NS_OBJECT_ENSURE_REGISTERED(PlcElectricalDeviceHelper);

TypeId
PlcElectricalDeviceHelper::GetTypeId (void)
{
  static TypeId tid = ns3::TypeId ("ns3::PlcElectricalDeviceHelper").SetParent<GhnPlcHelper> ();
  return tid;
}
PlcElectricalDeviceHelper::PlcElectricalDeviceHelper (ghn::BandPlanType bandplan, Ptr<const SpectrumModel> sm) :
        GhnPlcHelper (bandplan, sm)
{
  m_phyTid == PLC_ChaseCombiningPhy::GetTypeId ();
  m_switchingIntesity = SwitchingIntesity(1.0,0.0);
}

void
PlcElectricalDeviceHelper::Setup (void)
{
  if (m_node_list.empty ()) return;

  ObjectFactory phyFactory;
  phyFactory.SetTypeId (m_phyTid);

  for (auto &node : m_node_list)
    {
      std::cout << "Init (electrical device) " << node->GetName() << std::endl;
      node->GetObject<PLC_Electrical_Device> ()->InitDevice (m_spectrum_model);

      Ptr<PLC_Phy> phy = phyFactory.Create<PLC_Phy> ();
      phy->GetObject<PLC_HalfDuplexOfdmPhy> ()->CreateInterfaces (CreateObject<PLC_Outlet> (node), m_txPsd);
      phy->GetObject<PLC_HalfDuplexOfdmPhy> ()->GetRxInterface ()->AggregateObject (CreateObject<Node> ());
      node->GetObject<PLC_Electrical_Device> ()->SetPhy (phy);
      node->GetObject<PLC_Electrical_Device> ()->OnStart (m_switchingIntesity);
    }

  AttachChannel ();
}

void
PlcElectricalDeviceHelper::Save (std::ofstream &fo)
{
  for (auto &node : m_node_list)
    {
      node->GetObject<PLC_Electrical_Device> ()->Save (fo);
    }
}
void
PlcElectricalDeviceHelper::Load (std::ifstream &fi)
{
  if (m_node_list.empty ()) return;

  ObjectFactory phyFactory;
  phyFactory.SetTypeId (m_phyTid);

  for (auto &node : m_node_list)
    {
      node->GetObject<PLC_Electrical_Device> ()->Load (m_spectrum_model, fi);

      Ptr<PLC_Phy> phy = phyFactory.Create<PLC_Phy> ();
      phy->GetObject<PLC_HalfDuplexOfdmPhy> ()->CreateInterfaces (CreateObject<PLC_Outlet> (node), m_txPsd);
      phy->GetObject<PLC_HalfDuplexOfdmPhy> ()->GetRxInterface ()->AggregateObject (CreateObject<Node> ());
      node->GetObject<PLC_Electrical_Device> ()->SetPhy (phy);
      node->GetObject<PLC_Electrical_Device> ()->OnStart (m_switchingIntesity);
    }

  AttachChannel ();
}
void
PlcElectricalDeviceHelper::SetSwitchingIntesity(SwitchingIntesity si)
{
  m_switchingIntesity = si;
}

}

