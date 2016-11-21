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
#include "ns3/nc-module.h"
#include "ns3/plc-electrical-device-module.h"

#include "ns3/csma-module.h"
#include "ns3/plc-fd-module.h"

using namespace ns3;
using namespace ghn;
using namespace std;

//CXXFLAGS="-std=c++0x" ./waf configure --disable-python --with-nsc /home/tsokalo/workspace/ns3sims/ns-3-dev/nsc
//./waf --run csma-cd-example --command-template="gdb --args %s bt"
//export 'NS_LOG=PLC_FullDuplexOfdmPhy=level_logic|prefix_time|prefix_node|prefix_func:PLC_InfRateFDPhy=level_logic|prefix_time|prefix_node|prefix_func:PLC_Interference=level_logic|prefix_time|prefix_node|prefix_func:PLC_Channel=level_logic|prefix_time|prefix_node|prefix_func'
//export 'NS_LOG=PLC_Mac=level_logic|prefix_time|prefix_node|prefix_func:PLC_InfRateFDPhy=level_logic|prefix_time|prefix_node|prefix_func:CsmaCdMac=level_logic|prefix_time|prefix_node|prefix_func'
//export 'NS_LOG=PLC_FullDuplexOfdmPhy=level_logic|prefix_time|prefix_node|prefix_func:PLC_InfRateFDPhy=level_logic|prefix_time|prefix_node|prefix_func:PLC_Mac=level_logic|prefix_time|prefix_node|prefix_func'

void
ReceivedACK (void)
{
  NS_LOG_UNCOND(Simulator::Now() << ": ACK received!");
}
void
ConfigureIpRouting (Ipv4ListRoutingHelper &list, uint16_t numNodes);

void
ConfigureTrafficGenerator (CustomOnOffHelper &clientHelper);
std::string
ConstructResFoldName (int argc, char *argv[]);

int
main (int argc, char *argv[])
{
  //  LogComponentEnable("PLC_Mac", LOG_LEVEL_FUNCTION);

  //  Packet::EnablePrinting ();
  std::string resDir = ConstructResFoldName (argc, argv);
  //  RemoveDirectory (resDir);
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
  std::vector<uint32_t> distance;
  uint32_t distance_ptp = 500;
  uint16_t num_modems = 2;
  if (argc > 1)
    {
      num_modems = atoi (argv[1]);
    }
  for (uint16_t i = 0; i < num_modems - 1; i++)
    distance.push_back (distance_ptp); //unit [meters]
  double load = 1.0; //no units
  double dr = 3420000;//unit [bps]
  double nodeRate = load * dr / (double) (num_modems - 1); //unit [bps]
  double simDuration = 2; //unit [s]
  double minSimDuration = 0.1; //unit [s]
  uint16_t maxCwSize = 10;
  if (argc > 2)
    {
      maxCwSize = atoi (argv[2]);
    }
  BandPlanType bandplan = GDOTHN_BANDPLAN_25MHZ;
  GdothnDllMacProtocol macMode = CSMA_CA;
  std::vector<double> vals;
  vals.push_back (num_modems - 1);
  vals.push_back (maxCwSize);
  std::string logID = GetLogIdent (vals);
  cout << "Initialized the constants.." << endl;

  CsmaCdHelper devHelper (bandplan);
  Ptr<const SpectrumModel> sm = devHelper.GetSpectrumModel ();
  Ptr<PLC_Cable> cable = CreateObject<PLC_NAYY150SE_Cable> (sm);

  //
  // Create topology
  //
  PLC_NodeList node_list;
  CreateStarTopology (node_list, cable, sm, distance);
  cout << "Created topology.." << endl;

  //
  // Select background noise
  //
  Ptr<SpectrumValue> noiseFloor = CreateBestCaseBgNoise(sm)->GetNoisePsd ();

  //
  // Create channel
  //
  PLC_ChannelHelper channelHelper (sm);
  channelHelper.Install (node_list);
  cout << "Created channel.." << endl;

  //
  // Create communication devices
  //
  devHelper.SetNodeList (node_list);
  //  devHelper.DefinePhyType (PLC_InformationRatePhy::GetTypeId ());
  devHelper.DefineMacType (CsmaCdMac::GetTypeId ());
  devHelper.SetNoiseFloor (noiseFloor);
  devHelper.SetTxImpedance (CreateObject<PLC_ConstImpedance> (sm, PLC_Value (50, 0)));
  devHelper.SetRxImpedance (CreateObject<PLC_ConstImpedance> (sm, PLC_Value (200000, 0)));
  devHelper.SetChannel (channelHelper.GetChannel ());
  devHelper.SetResDirectory (resDir);
  devHelper.SetMaxCwSize (maxCwSize);
  devHelper.Setup ();
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
  stack.SetRoutingHelper (list); // has effect on the next Install ()
  stack.Install (devHelper.GetNS3Nodes ());
  Ipv4AddressHelper address ("192.168.10.0", "255.255.255.0");
  Ipv4InterfaceContainer plcInterfaces;
  plcInterfaces = address.Assign (devHelper.GetNetDevices ());
  cout << "Added IP stack.." << endl;

  //
  // Add transport layer
  //
  CustomUdpClientHelper clientHelper (plcInterfaces.GetAddress (0), 9);
  clientHelper.SetAttribute ("DataRate", DataRateValue (DataRate (nodeRate)));
  clientHelper.SetAttribute ("PacketSize", UintegerValue (490));
  clientHelper.SetResDirectory (resDir);
  cout << "Added Transport layer.." << endl;
  cout << "Configured traffic generators.." << endl;

  //
  // Select and configure the source and destination nodes
  //

  uint16_t dst_id = 0;
  auto devMap = devHelper.GetNetdeviceMap();
  cout << "Destination: " << node_list.at (dst_id)->GetName () << " with address "
          << devMap[node_list.at (dst_id)->GetName ()]->GetObject<PLC_NetDevice> ()->GetAddress () << endl;
  Ptr<Node> destinationNode = devMap[node_list.at (dst_id)->GetName ()]->GetObject<PLC_NetDevice> ()->GetNode ();

  NodeContainer c;
  for (uint16_t id = 1; id < node_list.size (); id++)
    {
      cout << "Source " << id << ": " << node_list.at (id)->GetName () << " with address "
              << devMap[node_list.at (id)->GetName ()]->GetObject<PLC_NetDevice> ()->GetAddress () << endl;
      Ptr<Node> sourceNode = devMap[node_list.at (id)->GetName ()]->GetObject<PLC_NetDevice> ()->GetNode ();
      c.Add (sourceNode);
    }

  ApplicationContainer clientApp = clientHelper.Install (c);
  CustomPacketSinkHelper packetSink ("ns3::UdpSocketFactory", Address (InetSocketAddress (Ipv4Address::GetAny (), 9)));
  packetSink.SetResDirectory (resDir);
  packetSink.SetLogId (logID);
  ApplicationContainer serverApp = packetSink.Install (destinationNode);

  Time startTime = Seconds (0.0);
  serverApp.Start (startTime);
  serverApp.Stop (startTime + Seconds (simDuration + minSimDuration));
  clientApp.Start (startTime + Seconds (0.0001));
  clientApp.Stop (startTime + Seconds (simDuration + minSimDuration));

  Simulator::Stop (startTime + Seconds (simDuration + minSimDuration));

  // Start simulation
  Simulator::Run ();

  // Cleanup simulation
  Simulator::Destroy ();

  rv_t data, loss, latency, iat, jitter;
  ReadFileContents (resDir + "app_data_0" + logID + ".txt", data, loss, latency, iat);
  jitter = GetJitter (latency);
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

  return EXIT_SUCCESS;
}

void
ConfigureIpRouting (Ipv4ListRoutingHelper &list, uint16_t numNodes)
{
  //
  // Enable OLSR
  //
  OlsrHelper olsr;
  Ipv4StaticRoutingHelper staticRouting;

  list.Add (staticRouting, 0);
  list.Add (olsr, 10);

  Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("gdothn.routes", std::ios::out);
  list.PrintRoutingTableAllAt (Seconds (30), routingStream);

  Ptr<OutputStreamWrapper> arpCache = Create<OutputStreamWrapper> ("gdothn.arp", std::ios::out);
  list.PrintNeighborCacheAllAt (Seconds (31.5), arpCache);

  //
  // specify TTL
  //
  Config::SetDefault ("ns3::Ipv4L3Protocol::DefaultTtl", UintegerValue (numNodes - 1));

  //
  // ARP configuration
  //
  Config::SetDefault ("ns3::ArpCache::AliveTimeout", TimeValue (Seconds (1000000)));
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
//      ss << "_" << argv[i];
//    }
  ss << "/";
  return ss.str ();
}

