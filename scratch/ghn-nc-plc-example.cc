#include <iostream>
#include <sstream>
#include <fstream>
#include <time.h>
#include <chrono>
#include <array>
#include <cstdlib>

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

using namespace ns3;
using namespace ghn;
using namespace std;

//turn on all error logs
//CXXFLAGS="-std=c++0x" ./waf configure --disable-python --with-nsc /home/tsokalo/workspace/ns3sims/ns-3-dev/nsc
//CXXFLAGS="-std=c++0x" ./waf configure --disable-python --enable-examples --with-nsc /home/tsokalo/workspace/ns3sims/ns-3-dev/nsc
//export 'NS_LOG=*=level_info|prefix_time|prefix_node|prefix_func'
//export 'NS_LOG=GhnPlcDllMac=level_all|prefix_time|prefix_node|prefix_func'
//export 'NS_LOG=GhnPlcPhyPmdFullD=level_all|prefix_time|prefix_node:PLC_FullDuplexOfdmPhy=level_all|prefix_time|prefix_node:PLC_InfRateFDPhy=level_all|prefix_time|prefix_node|prefix_func'
//export 'NS_LOG=GhnPlcLlcFlow=level_all|prefix_time|prefix_node|prefix_func:GhnPlcLlcCodedFlow=level_all|prefix_time|prefix_node|prefix_func:GhnPlcPhyManagement=level_all|prefix_time|prefix_node|prefix_func'
//export 'NS_LOG=GhnPlcNetDevice=level_debug|prefix_time|prefix_node|prefix_func:PLC_Electrical_Device=level_debug|prefix_time|prefix_node|prefix_func:GhnPlcMacBackoff=level_debug|prefix_time|prefix_node|prefix_func:GhnPlcDllMac=level_debug|prefix_time|prefix_node|prefix_func:GhnPlcDllLlc=level_debug|prefix_time|prefix_node|prefix_func:GhnPlcDllApc=level_debug|prefix_time|prefix_node|prefix_func:GhnPlcPhyPmd=level_debug|prefix_time|prefix_node|prefix_func:GhnPlcPhyPma=level_debug|prefix_time|prefix_node|prefix_func:GhnPlcPhyPcs=level_debug|prefix_time|prefix_node|prefix_func:GhnPlcPhyManagement=level_debug|prefix_time|prefix_node|prefix_func:PLC_INHOME_TOPOLOGY=level_debug|prefix_time|prefix_node|prefix_func'
//export 'NS_LOG=GhnPlcNetDevice=level_logic|prefix_time|prefix_node|prefix_func:PLC_Electrical_Device=level_logic|prefix_time|prefix_node|prefix_func:GhnPlcMacBackoff=level_logic|prefix_time|prefix_node|prefix_func:GhnPlcDllMac=level_logic|prefix_time|prefix_node|prefix_func:GhnPlcDllLlc=level_logic|prefix_time|prefix_node|prefix_func:GhnPlcDllApc=level_logic|prefix_time|prefix_node|prefix_func:GhnPlcPhyPmd=level_logic|prefix_time|prefix_node|prefix_func:GhnPlcPhyPma=level_logic|prefix_time|prefix_node|prefix_func:GhnPlcPhyPcs=level_logic|prefix_time|prefix_node|prefix_func:GhnPlcPhyManagement=level_logic|prefix_time|prefix_node|prefix_func:PLC_INHOME_TOPOLOGY=level_logic|prefix_time|prefix_node|prefix_func:PLC_Phy=level_logic|prefix_time|prefix_node|prefix_func:PLC_LinkPerformanceModel=level_logic|prefix_time|prefix_node|prefix_func:PLC_Channel=level_logic|prefix_time|prefix_node|prefix_func:NcHelper=level_logic|prefix_time|prefix_node|prefix_func'
//export 'NS_LOG=GhnPlcNetDevice=level_logic|prefix_time|prefix_node|prefix_func:PLC_Electrical_Device=level_logic|prefix_time|prefix_node|prefix_func:GhnPlcMacBackoff=level_logic|prefix_time|prefix_node|prefix_func:GhnPlcDllMac=level_logic|prefix_time|prefix_node|prefix_func:GhnPlcDllLlc=level_logic|prefix_time|prefix_node|prefix_func:GhnPlcDllApc=level_logic|prefix_time|prefix_node|prefix_func:GhnPlcPhyPmd=level_logic|prefix_time|prefix_node|prefix_func:GhnPlcPhyPma=level_logic|prefix_time|prefix_node|prefix_func:GhnPlcPhyPcs=level_logic|prefix_time|prefix_node|prefix_func:GhnPlcPhyManagement=level_logic|prefix_time|prefix_node|prefix_func:PLC_INHOME_TOPOLOGY=level_logic|prefix_time|prefix_node|prefix_func:PLC_Phy=level_logic|prefix_time|prefix_node|prefix_func:PLC_LinkPerformanceModel=level_logic|prefix_time|prefix_node|prefix_func'
//export 'NS_LOG=GhnPlcNetDevice=level_logic|prefix_time|prefix_node|prefix_func:PLC_Electrical_Device=level_logic|prefix_time|prefix_node|prefix_func:GhnPlcMacBackoff=level_logic|prefix_time|prefix_node|prefix_func:GhnPlcDllMac=level_logic|prefix_time|prefix_node|prefix_func:GhnPlcDllLlc=level_logic|prefix_time|prefix_node|prefix_func:GhnPlcDllApc=level_logic|prefix_time|prefix_node|prefix_func:PLC_INHOME_TOPOLOGY=level_logic|prefix_time|prefix_node|prefix_func:NcCutLog=level_logic|prefix_time|prefix_node|prefix_func'
//./waf --run ghn-nc-plc-example --command-template="gdb --args %s bt"
//export 'NS_LOG=NcCutLog=level_logic|prefix_time|prefix_node|prefix_func:TimeProbe=level_logic|prefix_time|prefix_node|prefix_func:FileHelper=level_logic|prefix_time|prefix_node|prefix_func:NcDllLlc=level_logic|prefix_time|prefix_node|prefix_func:NcRoutingTable=level_logic|prefix_time|prefix_node|prefix_func:NcDllLlc=level_logic|prefix_time|prefix_node|prefix_func:NcDllMac=level_logic|prefix_time|prefix_node|prefix_func:NcDllApc=level_logic|prefix_time|prefix_node|prefix_func'
//export 'NS_LOG=NcCutLog=level_logic|prefix_time|prefix_node|prefix_func:NcDllLlc=level_logic|prefix_time|prefix_node|prefix_func:NcLlcFlow=level_logic|prefix_time|prefix_node|prefix_func:NcRoutingTable=level_logic|prefix_time|prefix_node|prefix_func:NcDllMac=level_logic|prefix_time|prefix_node|prefix_func:NcDllApc=level_logic|prefix_time|prefix_node|prefix_func:NcNetDevice=level_logic|prefix_time|prefix_node|prefix_func:GhnSegmenter=level_logic|prefix_time|prefix_node|prefix_func:GhnPlcPhyManagement=level_debug|prefix_time|prefix_node|prefix_func'
//export 'NS_LOG=NcBitLoading=level_logic|prefix_time|prefix_node|prefix_func:NcHelper=level_logic|prefix_time|prefix_node|prefix_func:PLC_InfRateFDPhy=level_all|prefix_time|prefix_node|prefix_func:NcDllMac=level_logic|prefix_time|prefix_node|prefix_func:NcPhyPmd=level_logic|prefix_time|prefix_node|prefix_func'

void
ReceivedACK (void)
{
  NS_LOG_UNCOND(Simulator::Now() << ": ACK received!");
}

void
ConfigureTrafficGenerator (GhnPlcOnOffHelper &onOff);
void
ConfigureIpRouting (Ipv4ListRoutingHelper &list, uint16_t numNodes);

std::string
ConstructScenarioFolderName (int argc, char *argv[]);
void
LoadInhomeTopology (PLC_NodeList &node_list, Ptr<PLC_Cable> cable, uint16_t &num_modems, uint16_t &num_el_devs,
        double &totalSquare, std::istream &fi);
Ptr<SpectrumValue>
LoadInhomeBackgroundNoise (Ptr<const SpectrumModel> sm, std::istream &fi);
std::string
GetScenarioFileName (int argc, char *argv[]);
void
SaveBackgroundNoise (Ptr<SpectrumValue> noiseFloor, std::ostream &fo);
void
CreateInhomeTopology (PLC_NodeList &node_list, Ptr<PLC_Cable> cable, uint16_t num_modems, uint16_t num_el_devs,
        double totalSquare, std::ostream &fo);

int
main (int argc, char *argv[])
{
  std::string resDir = ConstructResFoldName (argc, argv);
//  ghn::RemoveDirectory (resDir);
  ghn::CreateDirectory (resDir);
  //
  // Set random seed value
  //
  typedef std::chrono::high_resolution_clock myclock;
  myclock::time_point beginning = myclock::now ();
  myclock::duration d = myclock::now () - beginning;
  RngSeedManager::SetSeed (d.count ());

  //
  // Set constant parameters
  //
  TopologyType topologyType = INHOME_TOPOLOGY_TYPE;
  std::vector<uint32_t> distance;
  uint32_t distance_ptp = 700;
  uint16_t num_modems = 2;
  if (argc > 1)
    {
      num_modems = atoi (argv[1]);
    }

  for (uint16_t i = 0; i < num_modems - 1; i++)
    distance.push_back (distance_ptp); //unit [meters]
  double load = 1.0; //no units
  double dr = 200 * 1000 * 1000; //unit [bps]
  double nodeRate = load * dr / (double) (num_modems - 1); //unit [bps]
  double simDuration = 2.0 + 2 * GHN_WARMUP_PERIOD; //unit [s]
  double minSimDuration = 0.1; //unit [s]
  uint16_t maxCwSize = 20;
  if (argc > 2)
    {
      maxCwSize = atoi (argv[2]);
    }

  BandPlanType bandplan = GDOTHN_BANDPLAN_25MHZ;
  GhnPlcDllMacProtocol macMode = CSMA_CD;
  if (argc > 3)
    {
      macMode = GhnPlcDllMacProtocol (atoi (argv[3]));
    }
  uint16_t num_el_devs = 0;
  double totalSquare = 60;
  std::vector<double> vals;
  vals.push_back (num_modems - 1);
  vals.push_back (maxCwSize);
  vals.push_back (topologyType);
  vals.push_back (macMode);
  std::string logID = GetLogIdent (vals);
  cout << "Initialized the constants.." << endl;

  PLC_NodeList node_list, modem_list, eldev_list;
  GhnPlcHelper devHelper (bandplan);
  Ptr<const SpectrumModel> sm = devHelper.GetSpectrumModel ();
  Ptr<PLC_Cable> cable = CreateObject<PLC_NAYY150SE_Cable> (sm);
  PLC_ChannelHelper channelHelper (sm);
  PlcElectricalDeviceHelper elDevHelper (bandplan, sm);
  //
  // Select background noise
  //
  Ptr<SpectrumValue> noiseFloor = CreateWorstCaseBgNoise(sm)->GetNoisePsd ();

  //
  // Create topology
  //
  if (topologyType == LINE_TOPOLOGY_TYPE)
    {
      CreateLineTopology (node_list, cable, sm, distance);
      channelHelper.Install (node_list);
      modem_list.insert (modem_list.begin (), node_list.begin (), node_list.begin () + num_modems);
    }
  else if (topologyType == STAR_TOPOLOGY_TYPE)
    {
      CreateStarTopology (node_list, cable, sm, distance);
      channelHelper.Install (node_list);
      modem_list.insert (modem_list.begin (), node_list.begin (), node_list.begin () + num_modems);
    }
  else if (topologyType == INHOME_TOPOLOGY_TYPE)
    {
      std::string scenDir = ConstructScenarioFolderName (argc, argv);
      ghn::CreateDirectory (scenDir);

      bool create_new = true;
      std::string path = scenDir + "/" + GetScenarioFileName (argc, argv);
      if (create_new)
        {
          //
          // Create and save the scenario to the file
          //
          std::ofstream fo (path, std::ios::out);

          CreateInhomeTopology (node_list, cable, num_modems, num_el_devs, totalSquare, fo);
          modem_list.insert (modem_list.begin (), node_list.begin (), node_list.begin () + num_modems);
          eldev_list.insert (eldev_list.begin (), node_list.begin () + num_modems,
                  node_list.begin () + num_modems + num_el_devs);
          std::cout << "Created topology.." << std::endl;

          //
          // Select background noise
          //
          noiseFloor = CreateInhomeBackgroundNoise (sm);
          SaveBackgroundNoise (noiseFloor, fo);
          cout << "Created background noise.." << endl;

          //
          // Create channel
          //
          channelHelper.Install (node_list);
          cout << "Created channel.." << endl;
          if (num_el_devs > 0)
            {
              //
              // Create electrical devices
              //
              elDevHelper.SetNodeList (eldev_list);
              devHelper.SetNoiseFloor (noiseFloor);
              elDevHelper.SetChannel (channelHelper.GetChannel ());
              elDevHelper.GetObject<PlcElectricalDeviceHelper> ()->Setup ();

              elDevHelper.Save (fo);
            }
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
          eldev_list.insert (eldev_list.begin (), node_list.begin () + num_modems,
                  node_list.begin () + num_modems + num_el_devs);
          std::cout << "Loaded topology.." << std::endl;
          //
          // Load background noise
          //
          noiseFloor = LoadInhomeBackgroundNoise (sm, fi);
          cout << "Loaded background noise.." << endl;
          //
          // Create channel
          //
          channelHelper.Install (node_list);
          cout << "Created channel.." << endl;
          if (num_el_devs > 0)
            {
              //
              // Load electrical devices
              //
              elDevHelper.SetNodeList (eldev_list);
              devHelper.SetNoiseFloor (noiseFloor);
              elDevHelper.SetChannel (channelHelper.GetChannel ());
              elDevHelper.Load (fi);
              cout << "Loaded electrical devices.." << endl;
            }
          fi.close ();
        }
    }
  else
    {
      assert(0);
    }
  cout << "Created topology.." << endl;

  Ptr<GhnPlcStats> ncStats = CreateObject<GhnPlcStats> ();

  //
  // Create communication devices
  //
  devHelper.SetNodeList (modem_list);
  if (macMode == CSMA_CD)
    {
      devHelper.DefinePhyType (GhnPlcPhyPmdFullD::GetTypeId ());
      devHelper.DefineMacType (GhnPlcDllMacCsmaCd::GetTypeId ());
    }
  devHelper.SetNoiseFloor (noiseFloor);
  devHelper.SetChannel (channelHelper.GetChannel ());
  devHelper.SetTxImpedance (CreateObject<PLC_ConstImpedance> (sm, PLC_Value (50, 0)));
  devHelper.SetRxImpedance (CreateObject<PLC_ConstImpedance> (sm, PLC_Value (200000, 0)));
  devHelper.SetGhnPlcStats (ncStats);
  devHelper.DefineBitLoadingType (NcBlVarBatMap::GetTypeId ());
  devHelper.SetResDirectory (resDir);
  devHelper.SetMaxCwSize (maxCwSize);
  devHelper.AllowCooperation ();
  devHelper.SetForcePer ();
  devHelper.StickToMainPath ();
  devHelper.SetImmediateFeedback ();
  devHelper.SetLowerSrcPriority ();
  auto addressMap = devHelper.Setup ();
  cout << "Created communication devices.." << endl;

  //
  // Add IP routing
  //
  Ipv4ListRoutingHelper list;
  ConfigureIpRouting (list, num_modems);
  cout << "Configured ARP.." << endl;

  //
  // Add IP stack
  //
  InternetStackHelper stack;
  //  stack.SetRoutingHelper (list); // has effect on the next Install ()
  stack.Install (devHelper.GetNS3Nodes ());
  Ipv4AddressHelper address ("192.168.10.0", "255.255.255.0");
  Ipv4InterfaceContainer plcInterfaces;
  plcInterfaces = address.Assign (devHelper.GetNetDevices ());
  cout << "Added IP stack.." << endl;

  //
  // Add transport layer
  //
  // Select and configure the source and destination nodes
  //
  typedef std::pair<uint8_t, uint8_t> NodeIndexPair;
  std::vector<NodeIndexPair> node_index_pairs;
  NodeContainer dst_nodes;
  //
  // add address pairs HERE
  //
  node_index_pairs.push_back (NodeIndexPair (0, modem_list.size () - 1));
  //
  // check if the communication between the selected pairs of sources and destinations is possible
  //
  auto devMap = devHelper.GetNetdeviceMap ();
  for (auto node_index_pair : node_index_pairs)
    {
      auto index_src = node_index_pair.first;
      auto index_dst = node_index_pair.second;
      assert(addressMap.find (index_src) != addressMap.end ());
      assert(addressMap.find (index_dst) != addressMap.end ());
      assert(modem_list.size () > index_src);
      assert(modem_list.size () > index_dst);
      auto address_src = addressMap[index_src];
      auto address_dst = addressMap[index_dst];

      cout << "Source: " << modem_list.at (index_src)->GetName () << " with address " << address_src << endl;
      cout << "Destination: " << modem_list.at (index_dst)->GetName () << " with address " << address_dst << endl;

      //
      // Check if at least one route between the selected source and destination exits
      //
      if (!devHelper.IsCommunicationPossible (index_src, index_dst))
        {
          cout << "Stopping.." << endl;
          // Cleanup simulation
          simDuration = 0;
          break;
        }
      //
      // Print communication cost table
      //
      devHelper.PrintCostTable (index_dst);
    }
  //
  // create and install traffic generators on source nodes; collect destination nodes
  //
  std::map<UanAddress, Ptr<Application> > appMap;
  ApplicationContainer clients;
  uint16_t port = 9;
  for (auto node_index_pair : node_index_pairs)
    {
      auto index_src = node_index_pair.first;
      auto index_dst = node_index_pair.second;
      assert(addressMap.find (index_src) != addressMap.end ());
      assert(addressMap.find (index_dst) != addressMap.end ());
      assert(modem_list.size () > index_src);
      assert(modem_list.size () > index_dst);
      auto ns3_node_src = devMap[modem_list.at (index_src)->GetName ()]->GetObject<PLC_NetDevice> ()->GetNode ();
      auto inet_addr_src = plcInterfaces.GetAddress (index_src);
      auto inet_addr_dst = plcInterfaces.GetAddress (index_dst);

      cout << "Installing client with address " << inet_addr_src << " and sink address " << inet_addr_dst << endl;

      GhnPlcUdpClientHelper clientHelper (inet_addr_dst, port, true);
      clientHelper.SetAttribute ("PacketSize", UintegerValue (750));
      clientHelper.SetResDirectory (resDir);
      auto clientApp = clientHelper.Install (ns3_node_src);

      appMap[addressMap[index_src]] = clientApp;
      clients.Add (clientApp);

      cout << "Adding sink with address " << inet_addr_dst << " to the list of sinks" << endl;
      assert(devMap.find (modem_list.at (index_dst)->GetName ()) != devMap.end ());
      dst_nodes.Add (devMap[modem_list.at (index_dst)->GetName ()]->GetObject<PLC_NetDevice> ()->GetNode ());
    }

  devHelper.SetAppMap (appMap);

  //
  // create sinks on the destinations
  //
  GhnPlcPacketSinkHelper packetSink ("ns3::UdpSocketFactory", Address (InetSocketAddress (Ipv4Address::GetAny (), port)));
  packetSink.SetResDirectory (resDir);
  packetSink.SetLogId (logID);
  ApplicationContainer servers = packetSink.Install (dst_nodes);

  cout << "Added Transport layer.." << endl;
  cout << "Configured traffic generators.." << endl;

  Time startTime = Seconds (0.0);
  servers.Start (startTime);
  servers.Stop (startTime + Seconds (simDuration + minSimDuration));
  clients.Start (startTime + Seconds (0.0001));
  clients.Stop (startTime + Seconds (simDuration + minSimDuration));

  Simulator::Stop (startTime + Seconds (simDuration + minSimDuration));

  // Start simulation
  Simulator::Run ();

  // Cleanup simulation
  Simulator::Destroy ();

  std::cout << "Finished the simulation" << std::endl;

  std::cout << "Start evaluation for the file: " << resDir + "app_data_" + std::to_string (num_modems - 1) + logID + ".txt"
          << std::endl;

  rv_t data, loss, latency, iat, jitter;
  ReadFileContents (resDir + "app_data_" + std::to_string (num_modems - 1) + logID + ".txt", data, loss, latency, iat);
  jitter = GetJitter (latency);
  std::cout << std::setprecision (5);
  FlatterIats (iat);
  uint16_t uh = 0;
//  for (auto d : iat)
//    std::cout << uh++ << "\t" << d << "\n";
//  std::cout << std::endl << std::endl << std::endl;
  bi_rv_t datarate = GetDatarate (iat, data);

  std::pair<double, double> ls, js, ds;
  ls = CalcStats (latency);
  js = CalcStats (jitter);
  ds = CalcStatsByDelta (datarate);

  std::stringstream ss;
  bool arq_win_size_max = 1;  //1 - gives no limit on ARQ win size, 0 - limited through original ARQ
  uint16_t gen_size = 64;
  bool allow_coop = true;
  ss << allow_coop << "\t" << arq_win_size_max << "\t" << gen_size << "\t" << num_modems << "\t" << maxCwSize << "\t"
          << ls.first << "\t" << ls.second << "\t" << js.first << "\t" << js.second << "\t" << ds.first << "\t" << ds.second;
  std::cout << ss.str () << std::endl;
  std::string fin_path = argv[0]; // get path from argument 0
  fin_path = fin_path.substr (0, fin_path.rfind ("/") + 1);
  std::string cmd = "echo \"" + ss.str () + "\" >> " + fin_path + "/sim_res.txt";
  system (cmd.c_str ());

  std::cout << "Finished the evaluation" << std::endl;

  return EXIT_SUCCESS;
}

void
ConfigureTrafficGenerator (GhnPlcOnOffHelper &onOff)
{
  //
  // Constant rate traffic
  //
  onOff.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  onOff.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  onOff.SetAttribute ("DataRate", DataRateValue (DataRate ("1MBps")));
  onOff.SetAttribute ("PacketSize", UintegerValue (490));

  //
  //Poisson traffic
  //
  //  char string_duration_on[300];
  //  double duration_on = (double) ((packetSize ) * 8.0 / (double) (datarate));
  //  sprintf (string_duration_on, "ns3::ConstantRandomVariable[Constant=" "%.9f" "]", duration_on);
  //  std::cout << "Const duration of On State: " << string_duration_on << std::endl;
  //
  //  onOff.SetAttribute ("OnTime", StringValue (string_duration_on));
  //
  //  char string_duration_off[300];
  //  double mean_duration_off ;
  //  mean_duration_off = (double) (packetSize) * 8.0 / nodeRate - duration_on;
  //  sprintf(string_duration_off,  "ns3::ExponentialRandomVariable[Mean=" "%.9f" "]" , mean_duration_off );
  //  std::cout << "Mean duration of Off State: " << string_duration_off << std::endl;
  //
  //  onOff.SetAttribute ("OffTime", StringValue (string_duration_off));
  //
  //  char datarateStr [50];
  //  sprintf (datarateStr, "%.0f", datarate);
  //  onOff.SetAttribute ("DataRate", StringValue (datarateStr));
  //  onOff.SetAttribute ("PacketSize", UintegerValue (packetSize));

}
void
ConfigureIpRouting (Ipv4ListRoutingHelper &list, uint16_t numNodes)
{
  //  //
  //   // Enable OLSR
  //   //
  //   OlsrHelper olsr;
  //   Ipv4StaticRoutingHelper staticRouting;
  //
  //   list.Add (staticRouting, 0);
  //   list.Add (olsr, 10);
  //
  //   Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("ghn.routes", std::ios::out);
  //   list.PrintRoutingTableAllAt (Seconds (30), routingStream);
  //
  //   Ptr<OutputStreamWrapper> arpCache = Create<OutputStreamWrapper> ("ghn.arp", std::ios::out);
  //   list.PrintNeighborCacheAllAt (Seconds (31.5), arpCache);
  //
  //   //
  //   // specify TTL
  //   //
  //   Config::SetDefault ("ns3::Ipv4L3Protocol::DefaultTtl", UintegerValue (numNodes - 1));
  //
  //   //
  //   // ARP configuration
  //   //
  //   Config::SetDefault ("ns3::ArpCache::AliveTimeout", TimeValue (Seconds (1000000)));
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
SaveBackgroundNoise (Ptr<SpectrumValue> noiseFloor, std::ostream &fo)
{
  auto it = noiseFloor->ValuesBegin ();

  while (it != noiseFloor->ValuesEnd ())
    {
      fo << *it << DELIMITER;
      it++;
    }
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
