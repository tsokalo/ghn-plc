/*
 *      Author: tsokalo
 */

#include <iostream>
#include <sstream>
#include <fstream>
#include <time.h>
#include <vector>
#include <string.h>
#include <chrono>

#include <ns3/core-module.h>
#include <ns3/nstime.h>
#include <ns3/simulator.h>
#include <ns3/output-stream-wrapper.h>
#include "ns3/plc.h"
#include <boost/assign/list_of.hpp>

#include "ns3/ghn-plc-module.h"
#include "ns3/plc-module.h"

using namespace ns3;
using namespace ghn;

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

  uint16_t num_modems = 10;
  BandPlanType bandplan = GDOTHN_BANDPLAN_25MHZ;
  GhnPlcDllMacProtocol macMode = CSMA_CD;
  PLC_NodeList node_list;
  GhnPlcHelper devHelper (bandplan);
  Ptr<const SpectrumModel> sm = devHelper.GetSpectrumModel ();
  Ptr<PLC_Cable> cable = CreateObject<PLC_NAYY50SE_Cable> (sm);
  Ptr<SpectrumValue> txPsd = devHelper.GetTxPsd ();

  std::vector<uint32_t> distance;
  uint32_t distance_ptp = 350;
  for (uint16_t i = 0; i < num_modems - 1; i++)
    distance.push_back (distance_ptp); //unit [meters]
  CreateLineTopology (node_list, cable, sm, distance);
  std::cout << "Created topology.." << std::endl;

  //
  // Select background noise
  //
  Ptr<SpectrumValue> noiseFloor = CreateWorstCaseBgNoise(sm)->GetNoisePsd ();
  cout << "Created background noise.." << endl;
  //
  // Create channel
  //
  PLC_ChannelHelper channelHelper (sm);
  channelHelper.Install (node_list);
  auto channel = channelHelper.GetChannel ();
  cout << "Created channel.." << endl;
  //
  // Create communication devices
  //
  if (macMode == CSMA_CD)
    {
      devHelper.DefinePhyType (GhnPlcPhyPmdFullD::GetTypeId ());
      devHelper.DefineMacType (GhnPlcDllMacCsmaCd::GetTypeId ());
    }
  devHelper.SetNodeList (node_list);
  devHelper.SetNoiseFloor (noiseFloor);
  devHelper.SetChannel (channel);
  devHelper.SetTxImpedance (CreateObject<PLC_ConstImpedance> (sm, PLC_Value (50, 0)));
  devHelper.SetRxImpedance (CreateObject<PLC_ConstImpedance> (sm, PLC_Value (200000, 0)));
  devHelper.SetResDirectory (resDir);
  auto addressMap = devHelper.Setup ();

  cout << "Created communication devices.." << endl;

  Utility::Evaluate(devHelper, resDir, macMode);

  return EXIT_SUCCESS;
}
