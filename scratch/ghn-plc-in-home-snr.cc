#include <iostream>
#include <sstream>
#include <fstream>
#include <time.h>
#include <chrono>
#include <array>
#include <cstdlib>
#include <functional>

#include <ns3/core-module.h>
#include <ns3/nstime.h>
#include <ns3/simulator.h>
#include <ns3/output-stream-wrapper.h>
#include "ns3/random-variable-stream.h"
#include "ns3/network-module.h"
#include "ns3/stats-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/olsr-module.h"

#include "ns3/plc-module.h"
#include "ns3/ghn-plc-module.h"
#include "ns3/in-home-topology-module.h"
#include "ns3/plc-electrical-device-module.h"

#define DELIMITER '\t'

using namespace ns3;
using namespace ghn;

std::string
ConstructScenarioFolderName (int argc, char *argv[]);
std::string
GetScenarioFileName (int argc, char *argv[]);

void
CreateInhomeTopology (PLC_NodeList &node_list, Ptr<PLC_Cable> cable, uint16_t num_modems, uint16_t num_el_devs,
        double totalSquare, std::ostream &fo);
void
SaveBackgroundNoise (Ptr<SpectrumValue> noiseFloor, std::ostream &fo);
void
LoadInhomeTopology (PLC_NodeList &node_list, Ptr<PLC_Cable> cable, uint16_t &num_modems, uint16_t &num_el_devs,
        double &totalSquare, std::istream &fi);
Ptr<SpectrumValue>
LoadInhomeBackgroundNoise (Ptr<const SpectrumModel> sm, std::istream &fi);

//export 'NS_LOG=InhomeTopology=level_all|prefix_time|prefix_node|prefix_func'

int
main (int argc, char *argv[])
{
  //
  // Set random seed value
  //
  typedef std::chrono::high_resolution_clock myclock;
  myclock::time_point beginning = myclock::now ();
  myclock::duration d = myclock::now () - beginning;
  RngSeedManager::SetSeed (d.count ());

  std::string resDir = ConstructResFoldName (argc, argv);
  ghn::RemoveDirectory (resDir);
  ghn::CreateDirectory (resDir);

  std::string scenDir = ConstructScenarioFolderName (argc, argv);
  ghn::CreateDirectory (scenDir);

  bool create_new = true;
  std::string path = scenDir + "/" + GetScenarioFileName (argc, argv);
  std::cout << "Using scenario file " << path << std::endl;
  uint16_t num_modems = 0;
  uint16_t num_el_devs = 0;
  double totalSquare = 0;
  BandPlanType bandplan = GDOTHN_BANDPLAN_25MHZ;
  GhnPlcDllMacProtocol macMode = CSMA_CD;
  PLC_NodeList node_list, modem_list, eldev_list;
  GhnPlcHelper devHelper (bandplan);
  Ptr<const SpectrumModel> sm = devHelper.GetSpectrumModel ();
  Ptr<PLC_Cable> cable = CreateObject<PLC_NAYY50SE_Cable> (sm);
  Ptr<PLC_Channel> channel;
  Ptr<SpectrumValue> noiseFloor;
  PlcElectricalDeviceHelper elDevHelper (bandplan, sm);
  PLC_ChannelHelper channelHelper (sm);
  Ptr<SpectrumValue> txPsd = devHelper.GetTxPsd ();

  if (create_new)
    {
      //
      // Create and save the scenario to the file
      //
      std::ofstream fo (path, std::ios::out);
      num_modems = 3;
      num_el_devs = 3;
      if (argc > 1)
        {
          num_el_devs = atoi (argv[1]);
        }
      totalSquare = 60;

      CreateInhomeTopology (node_list, cable, num_modems, num_el_devs, totalSquare, fo);
      modem_list.insert (modem_list.begin (), node_list.begin (), node_list.begin () + num_modems);
      eldev_list.insert (eldev_list.begin (), node_list.begin () + num_modems, node_list.begin () + num_modems + num_el_devs);
      std::cout << "Created topology.." << std::endl;

      //
      // Select background noise
      //
      noiseFloor = CreateInhomeBackgroundNoise (sm);
//      std::cout << *noiseFloor << std::endl;
      SaveBackgroundNoise (noiseFloor, fo);
      cout << "Created background noise.." << endl;
      //
      // Create channel
      //
      channelHelper.Install (node_list);
      channel = channelHelper.GetChannel ();
      cout << "Created channel.." << endl;
      //
      // Create electrical devices
      //
      elDevHelper.SetNodeList (eldev_list);
      devHelper.SetNoiseFloor (noiseFloor);
      elDevHelper.SetChannel (channel);
      elDevHelper.GetObject<PlcElectricalDeviceHelper> ()->Setup ();

      elDevHelper.Save (fo);
      cout << "Created electrical devices.." << endl;

      fo.close ();
    }
  else
    {
      //
      // Load scenario from the file
      //
      std::ifstream fi (path, std::ios::in);
      LoadInhomeTopology (node_list, cable, num_modems, num_el_devs, totalSquare, fi);
      modem_list.insert (modem_list.begin (), node_list.begin (), node_list.begin () + num_modems);
      eldev_list.insert (eldev_list.begin (), node_list.begin () + num_modems, node_list.begin () + num_modems + num_el_devs);
      std::cout << "Loaded topology.." << std::endl;
      //
      // Load background noise
      //
      noiseFloor = LoadInhomeBackgroundNoise (sm, fi);
//      *noiseFloor *= 10000;
      cout << "Loaded background noise.." << endl;
      //
      // Create channel
      //
      channelHelper.Install (node_list);
      channel = channelHelper.GetChannel ();
      cout << "Created channel.." << endl;
      //
      // Load electrical devices
      //
      elDevHelper.SetNodeList (eldev_list);
      devHelper.SetNoiseFloor (noiseFloor);
      elDevHelper.SetChannel (channel);
      elDevHelper.Load (fi);
      cout << "Loaded electrical devices.." << endl;

      fi.close ();
    }

  //
  // Create communication devices
  //
  if (macMode == CSMA_CD)
    {
      devHelper.DefinePhyType (GhnPlcPhyPmdFullD::GetTypeId ());
      devHelper.DefineMacType (GhnPlcDllMacCsmaCd::GetTypeId ());
    }
  devHelper.SetNodeList (modem_list);
  devHelper.SetNoiseFloor (noiseFloor);
  devHelper.SetChannel (channel);
  devHelper.SetTxImpedance (CreateObject<PLC_ConstImpedance> (sm, PLC_Value (50, 0)));
  devHelper.SetRxImpedance (CreateObject<PLC_ConstImpedance> (sm, PLC_Value (200000, 0)));
//  devHelper.SetGhnPlcStats (ncStats);
//  devHelper.DefineBitLoadingType (NcBlVarBatMap::GetTypeId ());
  devHelper.SetResDirectory (resDir);
//  devHelper.SetMaxCwSize (maxCwSize);
//  devHelper.AllowCooperation ();
//  devHelper.StickToMainPath ();
//  devHelper.SetImmediateFeedback ();
//  devHelper.SetLowerSrcPriority ();
//  auto addressMap = devHelper.Setup ();

  cout << "Created communication devices.." << endl;

//  Utility::Evaluate (devHelper, resDir, macMode);

  return EXIT_SUCCESS;
}

std::string
ConstructScenarioFolderName (int argc, char *argv[])
{
  std::stringstream ss;
  std::string path = argv[0]; // get path from argument 0
  path = path.substr (0, path.rfind ("/") + 1);
  ss << path << "Scenario";

  //  for (uint16_t i = 1; i < argc; i++)
  //    {
  //      ss << "_" << argv[1];
  //    }
  ss << "/";
  return ss.str ();
}
std::string
GetScenarioFileName (int argc, char *argv[])
{
  UniformRandomVariable uniRV;
  auto v = uniRV.GetValue (0, 1000000);

  std::stringstream ss;
  ss << "Scenario";

  for (uint16_t i = 1; i < argc; i++)
    {
      ss << "_" << argv[i];
    }
  ss << "_" << v;
  ss << "_.txt";

//  return "scenario.txt";
  return ss.str ();
}
void
CreateInhomeTopology (PLC_NodeList &node_list, Ptr<PLC_Cable> cable, uint16_t num_modems, uint16_t num_el_devs,
        double totalSquare, std::ostream &fo)
{
  fo << num_modems << DELIMITER;
  fo << num_el_devs << DELIMITER;
  fo << totalSquare << DELIMITER;

  node_list.resize (num_modems + num_el_devs);

  //
  // Create node_list
  //
  for (uint16_t i = 0; i < num_modems; i++)
    {
      node_list.at (i) = CreateObject<PLC_Node> ();
      node_list.at (i)->SetName (std::string ("Node ") + std::to_string (i + 1));
      std::cout << "Creating " << node_list.at (i)->GetName () << std::endl;
    }

  //
  // Create electrical devices
  //
  for (uint16_t i = num_modems; i < num_modems + num_el_devs; i++)
    {
      node_list.at (i) = CreateObject<PLC_Electrical_Device> ();
      node_list.at (i)->SetName (std::string ("El.Dev. ") + std::to_string (i + 1 - num_modems));
      std::cout << "Creating " << node_list.at (i)->GetName () << std::endl;
    }

  //
  // Set node_list positions and connect them with cables
  // Here are additional node_list created - distribution boxes
  //
  InhomeTopology topology (totalSquare, num_modems, num_el_devs);
  topology.Create (node_list, cable);
  topology.Save (fo);
  topology.Draw ("/home/tsokalo/workspace/ns-allinone-3.25/ns-3.25/src/in-home-topology/Latex/");

  //
  // Let the distribution boxes do not have any impedance and create no noise
  //
  for (uint16_t i = num_modems + num_el_devs; i < node_list.size (); i++)
    {
      node_list.at (i)->GetObject<PLC_Node> ()->OpenCircuit ();
    }
}
void
LoadInhomeTopology (PLC_NodeList &node_list, Ptr<PLC_Cable> cable, uint16_t &num_modems, uint16_t &num_el_devs,
        double &totalSquare, std::istream &fi)
{
  fi >> num_modems;
  fi >> num_el_devs;
  fi >> totalSquare;

  std::cout << "Loading.. number of modems: " << num_modems << ", number of el.devs: " << num_el_devs << ", total square: "
          << totalSquare << std::endl;

  node_list.resize (num_modems + num_el_devs);

  //
  // Create node_list
  //
  for (uint16_t i = 0; i < num_modems; i++)
    {
      std::cout << "Creating a modem " << i << std::endl;
      node_list.at (i) = CreateObject<PLC_Node> ();
      node_list.at (i)->SetName (std::string ("Node ") + std::to_string (i + 1));
    }

  //
  // Create electrical devices
  //
  for (uint16_t i = num_modems; i < num_modems + num_el_devs; i++)
    {
      std::cout << "Creating an electrical device " << i << std::endl;
      node_list.at (i) = CreateObject<PLC_Electrical_Device> ();
      node_list.at (i)->SetName (std::string ("El.Dev. ") + std::to_string (i + 1 - num_modems));
    }

  //
  // Set node_list positions and connect them with cables
  // Here are additional node_list created - distribution boxes
  //
  InhomeTopology topology (totalSquare, num_modems, num_el_devs);
  topology.Load (node_list, cable, fi);

  //
  // Let the distribution boxes do not have any impedance and create no noise
  //
  for (uint16_t i = num_modems + num_el_devs; i < node_list.size (); i++)
    {
      node_list.at (i)->GetObject<PLC_Node> ()->OpenCircuit ();
    }
}

void
SaveBackgroundNoise (Ptr<SpectrumValue> noiseFloor, std::ostream &fo)
{
  auto it = noiseFloor->ValuesBegin ();

  while (it != noiseFloor->ValuesEnd ())
    {
      fo << *it << DELIMITER;
      it++;
    }
}

Ptr<SpectrumValue>
LoadInhomeBackgroundNoise (Ptr<const SpectrumModel> sm, std::istream &fi)
{
  Ptr<SpectrumValue> noiseFloor = Create<SpectrumValue> (sm);
  uint32_t n = sm->GetNumBands ();
  for (uint32_t i = 0; i < n; i++)
    {
      fi >> (*noiseFloor)[i];
    }
  return noiseFloor;
}
