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

using namespace ns3;
using namespace ghn;
using namespace std;

//turn on all error logs
//CXXFLAGS="-std=c++0x" ./waf configure --disable-python --with-nsc /home/tsokalo/workspace/ns3sims/ns-3-dev/nsc
//CXXFLAGS="-std=c++0x" ./waf configure --disable-python --enable-examples --with-nsc /home/tsokalo/workspace/ns3sims/ns-3-dev/nsc
//export 'NS_LOG=*=level_info|prefix_time|prefix_node|prefix_func'
//export 'NS_LOG=GhnPlcNetDevice=level_debug|prefix_time|prefix_node|prefix_func:PLC_Electrical_Device=level_debug|prefix_time|prefix_node|prefix_func:GhnPlcMacBackoff=level_debug|prefix_time|prefix_node|prefix_func:GhnPlcDllMac=level_debug|prefix_time|prefix_node|prefix_func:GhnPlcDllLlc=level_debug|prefix_time|prefix_node|prefix_func:GhnPlcDllApc=level_debug|prefix_time|prefix_node|prefix_func:GhnPlcPhyPmd=level_debug|prefix_time|prefix_node|prefix_func:GhnPlcPhyPma=level_debug|prefix_time|prefix_node|prefix_func:GhnPlcPhyPcs=level_debug|prefix_time|prefix_node|prefix_func:GhnPlcPhyManagement=level_debug|prefix_time|prefix_node|prefix_func:PLC_INHOME_TOPOLOGY=level_debug|prefix_time|prefix_node|prefix_func'
//export 'NS_LOG=GhnPlcNetDevice=level_logic|prefix_time|prefix_node|prefix_func:PLC_Electrical_Device=level_logic|prefix_time|prefix_node|prefix_func:GhnPlcMacBackoff=level_logic|prefix_time|prefix_node|prefix_func:GhnPlcDllMac=level_logic|prefix_time|prefix_node|prefix_func:GhnPlcDllLlc=level_logic|prefix_time|prefix_node|prefix_func:GhnPlcDllApc=level_logic|prefix_time|prefix_node|prefix_func:GhnPlcPhyPmd=level_logic|prefix_time|prefix_node|prefix_func:GhnPlcPhyPma=level_logic|prefix_time|prefix_node|prefix_func:GhnPlcPhyPcs=level_logic|prefix_time|prefix_node|prefix_func:GhnPlcPhyManagement=level_logic|prefix_time|prefix_node|prefix_func:PLC_INHOME_TOPOLOGY=level_logic|prefix_time|prefix_node|prefix_func:PLC_Phy=level_logic|prefix_time|prefix_node|prefix_func:PLC_LinkPerformanceModel=level_logic|prefix_time|prefix_node|prefix_func:PLC_Channel=level_logic|prefix_time|prefix_node|prefix_func:NcHelper=level_logic|prefix_time|prefix_node|prefix_func'
//export 'NS_LOG=GhnPlcNetDevice=level_logic|prefix_time|prefix_node|prefix_func:PLC_Electrical_Device=level_logic|prefix_time|prefix_node|prefix_func:GhnPlcMacBackoff=level_logic|prefix_time|prefix_node|prefix_func:GhnPlcDllMac=level_logic|prefix_time|prefix_node|prefix_func:GhnPlcDllLlc=level_logic|prefix_time|prefix_node|prefix_func:GhnPlcDllApc=level_logic|prefix_time|prefix_node|prefix_func:GhnPlcPhyPmd=level_logic|prefix_time|prefix_node|prefix_func:GhnPlcPhyPma=level_logic|prefix_time|prefix_node|prefix_func:GhnPlcPhyPcs=level_logic|prefix_time|prefix_node|prefix_func:GhnPlcPhyManagement=level_logic|prefix_time|prefix_node|prefix_func:PLC_INHOME_TOPOLOGY=level_logic|prefix_time|prefix_node|prefix_func:PLC_Phy=level_logic|prefix_time|prefix_node|prefix_func:PLC_LinkPerformanceModel=level_logic|prefix_time|prefix_node|prefix_func'
//export 'NS_LOG=GhnPlcNetDevice=level_logic|prefix_time|prefix_node|prefix_func:PLC_Electrical_Device=level_logic|prefix_time|prefix_node|prefix_func:GhnPlcMacBackoff=level_logic|prefix_time|prefix_node|prefix_func:GhnPlcDllMac=level_logic|prefix_time|prefix_node|prefix_func:GhnPlcDllLlc=level_logic|prefix_time|prefix_node|prefix_func:GhnPlcDllApc=level_logic|prefix_time|prefix_node|prefix_func:PLC_INHOME_TOPOLOGY=level_logic|prefix_time|prefix_node|prefix_func:NcCutLog=level_logic|prefix_time|prefix_node|prefix_func'
//./waf --run nc-aware-routing --command-template="gdb --args %s bt"
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
ConstructResFoldName (int argc, char *argv[]);

int
main (int argc, char *argv[])
{
  std::string resDir = ConstructResFoldName (argc, argv);
  RemoveDirectory (resDir);
  CreateDirectory (resDir);
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
  TopologyType topologyType = LINE_TOPOLOGY_TYPE;
  std::vector<uint32_t> distance;
  uint32_t distance_ptp = 1000;
  uint16_t num_modems = 3;
  if (argc > 1)
    {
      num_modems = atoi (argv[1]);
    }

  for (uint16_t i = 0; i < num_modems - 1; i++)
    distance.push_back (distance_ptp); //unit [meters]
  double load = 1.0; //no units
  double dr = 200 * 1000 * 1000; //unit [bps]
  double nodeRate = load * dr / (double) (num_modems - 1); //unit [bps]
  double simDuration = 1.0; //unit [s]
  double minSimDuration = 0.1; //unit [s]
  uint16_t maxCwSize = 10;
  if (argc > 2)
    {
      maxCwSize = atoi (argv[2]);
    }
  BandPlanType bandplan = GDOTHN_BANDPLAN_25MHZ;
  GhnPlcDllMacProtocol macMode = CSMA_CA;
  if (argc > 3)
    {
      macMode = GhnPlcDllMacProtocol (atoi (argv[3]));
    }
  std::vector<double> vals;
  vals.push_back (num_modems - 1);
  vals.push_back (maxCwSize);
  vals.push_back (topologyType);
  vals.push_back (macMode);
  std::string logID = GetLogIdent (vals);
  cout << "Initialized the constants.." << endl;

  PLC_NodeList node_list;
  GhnPlcHelper devHelper (bandplan);
  Ptr<const SpectrumModel> sm = devHelper.GetSpectrumModel ();
  Ptr<PLC_Cable> cable = CreateObject<PLC_NAYY150SE_Cable> (sm);

  //
  // Create topology
  //
  if (topologyType == LINE_TOPOLOGY_TYPE)
    {
      CreateLineTopology (node_list, cable, sm, distance);
    }
  else if (topologyType == STAR_TOPOLOGY_TYPE)
    {
      CreateStarTopology (node_list, cable, sm, distance);
    }
  else
    {
      assert(0);
    }
  cout << "Created topology.." << endl;

  //
  // Select background noise
  //
  Ptr<SpectrumValue> noiseFloor = CreateWorstCaseBgNoise(sm)->GetNoisePsd ();
  //
  // Create channel
  //
  PLC_ChannelHelper channelHelper (sm);
  channelHelper.Install (node_list);
  cout << "Created channel.." << endl;

  Ptr<GhnPlcStats> ncStats = CreateObject<GhnPlcStats> ();

  //
  // Create communication devices
  //
  devHelper.SetNodeList (node_list);
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
  typedef std::pair<UanAddress, UanAddress> UanAdderssPair;
  typedef std::pair<Ptr<Node>, Ptr<Node> > Ns3NodePair;
  std::vector<NodeIndexPair> node_index_pairs;
  std::vector<UanAdderssPair> uan_address_pairs;
  NodeContainer dst_nodes;
  //
  // add address pairs HERE
  //
  node_index_pairs.push_back (NodeIndexPair (0, node_list.size () - 1));
  //
  // find correspondence of node indices and their UanAddresses
  //
  for (auto node_index_pair : node_index_pairs)
    {
      assert(addressMap.find (node_index_pair.first) != addressMap.end ());
      assert(addressMap.find (node_index_pair.second) != addressMap.end ());
      uan_address_pairs.push_back (UanAdderssPair (addressMap[node_index_pair.first], addressMap[node_index_pair.second]));
    }
  //
  // check if the communication between the selected pairs of sources and destinations is possible
  //
  auto devMap = devHelper.GetNetdeviceMap ();
  for (auto node_index_pair : node_index_pairs)
    {
      auto index_src = node_index_pair.first;
      auto index_dst = node_index_pair.second;
      auto address_src = devMap[node_list.at (index_src)->GetName ()]->GetObject<PLC_NetDevice> ()->GetAddress ();
      auto address_dst = devMap[node_list.at (index_dst)->GetName ()]->GetObject<PLC_NetDevice> ()->GetAddress ();

      cout << "Source: " << node_list.at (index_src)->GetName () << " with address " << address_src << endl;
      cout << "Destination: " << node_list.at (index_dst)->GetName () << " with address " << address_dst << endl;

      //
      // Check if at least one route between the selected source and destination exits
      //
      if (devHelper.IsCommunicationPossible (index_src, index_dst))
        {
          cout << "Ready to start! Starting.." << endl;
        }
      else
        {
          cout << "There is no route between the selected source and destination. Stopping.." << endl;
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
  for (auto node_index_pair : node_index_pairs)
    {
      auto index_src = node_index_pair.first;
      auto index_dst = node_index_pair.second;
      auto address_dst = devMap[node_list.at (index_dst)->GetName ()]->GetObject<PLC_NetDevice> ()->GetAddress ();
      auto ns3_node_src = devMap[node_list.at (index_src)->GetName ()]->GetObject<PLC_NetDevice> ()->GetNode ();

      GhnPlcUdpClientHelper clientHelper (address_dst, 9, true);
      clientHelper.SetAttribute ("PacketSize", UintegerValue (1000));
      clientHelper.SetResDirectory (resDir);
      auto clientApp = clientHelper.Install (ns3_node_src);

      appMap[addressMap[index_src]] = clientApp;
      clients.Add(clientApp);

      dst_nodes.Add (devMap[node_list.at (index_dst)->GetName ()]->GetObject<PLC_NetDevice> ()->GetNode ());
    }

  devHelper.SetAppMap (appMap);

  //
  // create sinks on the destinations
  //
  GhnPlcPacketSinkHelper packetSink ("ns3::UdpSocketFactory", Address (InetSocketAddress (Ipv4Address::GetAny (), 9)));
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

  rv_t data, loss, latency, iat, jitter;
  ReadFileContents (resDir + "app_data_0" + logID + ".txt", data, loss, latency, iat);
  jitter = GetJitter (latency);
  std::cout << std::setprecision (5);
  FlatterIats (iat);
  uint16_t uh = 0;
  for (auto d : iat)
    std::cout << uh++ << "\t" << d << "\n";
  std::cout << std::endl << std::endl << std::endl;
  bi_rv_t datarate = GetDatarate (iat, data);

  std::pair<double, double> ls, js, ds;
  ls = CalcStats (latency);
  js = CalcStats (jitter);
  ds = CalcStatsByDelta (datarate);

  std::stringstream ss;
  ss << nodeRate / 1000000 << "\t" << num_modems - 1 << "\t" << maxCwSize << "\t" << ls.first << "\t" << ls.second << "\t"
          << js.first << "\t" << js.second << "\t" << ds.first << "\t" << ds.second;
  std::cout << ss.str () << std::endl;
  std::string cmd = "echo \"" + ss.str () + "\" >> " + resDir + "/sim_res.txt";
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
ConstructResFoldName (int argc, char *argv[])
{
  std::stringstream ss;
  std::string path = argv[0]; // get path from argument 0
  path = path.substr (0, path.rfind ("/") + 1);
  ss << path << "Results";

  //  for (uint16_t i = 1; i < argc; i++)
  //    {
  //      ss << "_" << argv[1];
  //    }
  ss << "/";
  return ss.str ();
}

