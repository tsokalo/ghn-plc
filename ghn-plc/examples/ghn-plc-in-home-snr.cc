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

std::string
ConstructResFoldName (int argc, char *argv[]);

void
CreateInhomeTopology (PLC_NodeList &node_list, Ptr<PLC_Cable> cable, Ptr<const SpectrumModel> sm, uint16_t num_modems,
        uint16_t num_el_devs, double totalSquare, std::ofstream &fo);
void
WriteBackgroundNoise (Ptr<SpectrumValue> noiseFloor, std::ofstream &fo);
void
LoadInhomeTopology (PLC_NodeList &node_list, Ptr<PLC_Cable> cable, Ptr<const SpectrumModel> sm, uint16_t &num_modems,
        uint16_t &num_el_devs, double &totalSquare, std::ifstream &fi);
Ptr<SpectrumValue>
LoadInhomeBackgroundNoise (Ptr<const SpectrumModel> sm, std::ifstream &fi);

int
main (int argc, char *argv[])
{
  std::string resDir = ConstructResFoldName (argc, argv);
  ghn::RemoveDirectory (resDir);
  ghn::CreateDirectory (resDir);

  bool create_new = true;
  std::string path = resDir + "/scenario.txt";
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
  PlcElectricalDeviceHelper elDevHelper (bandplan);
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
      totalSquare = 60;

      CreateInhomeTopology (node_list, cable, sm, num_modems, num_el_devs, totalSquare, fo);
      modem_list.insert (modem_list.begin (), node_list.begin (), node_list.begin () + num_modems);
      eldev_list.insert (eldev_list.begin (), node_list.begin () + num_modems, node_list.end ());
      std::cout << "Created topology.." << std::endl;

      //
      // Select background noise
      //
      noiseFloor = CreateInhomeBackgroundNoise (sm);
      WriteBackgroundNoise (noiseFloor, fo);
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
      elDevHelper.SetChannel (channel);
      elDevHelper.Setup ();
      SwitchingIntesity switchingIntesity;
      switchingIntesity.first = 0.1;
      switchingIntesity.second = 0.1;
      for (auto &eldev : eldev_list)
        eldev->GetObject<PLC_Electrical_Device> ()->OnStart (switchingIntesity);

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
      LoadInhomeTopology (node_list, cable, sm, num_modems, num_el_devs, totalSquare, fi);
      modem_list.insert (modem_list.begin (), node_list.begin (), node_list.begin () + num_modems);
      eldev_list.insert (eldev_list.begin (), node_list.begin () + num_modems, node_list.end ());
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
      channel = channelHelper.GetChannel ();
      cout << "Created channel.." << endl;
      //
      // Load electrical devices
      //
      elDevHelper.SetNodeList (eldev_list);
      elDevHelper.SetChannel (channel);
      elDevHelper.Load (fi);
      cout << "Loaded electrical devices.." << endl;

      fi.close ();
    }

  //
  // Create communication devices
  //
  devHelper.SetNodeList (modem_list);
  devHelper.SetNoiseFloor (noiseFloor);
  devHelper.SetChannel (channel);
  devHelper.SetTxImpedance (CreateObject<PLC_ConstImpedance> (sm, PLC_Value (50, 0)));
  devHelper.SetRxImpedance (CreateObject<PLC_ConstImpedance> (sm, PLC_Value (200000, 0)));
  cout << "Created communication devices.." << endl;

  channel->InitTransmissionChannels ();
  channel->CalcTransmissionChannels ();
  cout << "Channel is initialized.." << endl;

  auto save_snr = [&](Ptr<PLC_TxInterface> txIf, Ptr<PLC_TxInterface> rxIf, uint16_t s_id, uint16_t r_id)
    {
      auto chImpl = txIf->GetChannelTransferImpl (PeekPointer (rxIf));
      auto rxPsd = chImpl->CalculateRxPowerSpectralDensity (txPsd);

      PLC_Interference interference;
      interference.SetNoiseFloor (noiseFloor);
      interference.StartRx (rxPsd);
      Ptr<const SpectrumValue> sinr = interference.GetSinr ();
      SpectrumValue sinr_db = 20 * Log10 (*sinr);

      Bands::const_iterator bi = sinr_db.ConstBandsBegin ();
      Values::const_iterator vi = sinr_db.ConstValuesBegin ();

      std::string f_name = resDir + "sinr_dB_" + "_" + std::to_string(s_id) + "_" + std::to_string(r_id) + ".txt";
      std::cout << "SINR between " << s_id << " and " << r_id << " in " << f_name << std::endl;

      std::ofstream f(f_name, std::ios::out);
      while (bi != sinr_db.ConstBandsEnd ())
        {
          NS_ASSERT (vi != sinr_db.ConstValuesEnd ());
          double carrier = (bi->fh + bi->fl) / 2;
          double sinr_db_val = *vi;
          f << carrier << "\t" << sinr_db_val << std::endl;
          ++bi;
          ++vi;
        }
      f.close();
    };

  for (auto s : modem_list)
    {
      auto phy_s = devHelper.GetDevice (s->GetName ())->GetPhy ();
      Ptr<PLC_TxInterface> txIf =
              (macMode == CSMA_CD) ? phy_s->GetObject<GhnPlcPhyPmdFullD> ()->GetTxInterface () :
                      phy_s->GetObject<GhnPlcPhyPmdHalfD> ()->GetTxInterface ();

      for (auto r : modem_list)
        {
          if (s->GetVertexId () == r->GetVertexId ()) continue;

          auto phy_r = devHelper.GetDevice (r->GetName ())->GetPhy ();
          Ptr<PLC_TxInterface> rxIf =
                  (macMode == CSMA_CD) ? phy_r->GetObject<GhnPlcPhyPmdFullD> ()->GetRxInterface () :
                          phy_r->GetObject<GhnPlcPhyPmdHalfD> ()->GetRxInterface ();
          save_snr(txIf, rxIf, s->GetVertexId (), r->GetVertexId ());
        }
    }

  return EXIT_SUCCESS;
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

void
CreateInhomeTopology (PLC_NodeList &node_list, Ptr<PLC_Cable> cable, Ptr<const SpectrumModel> sm, uint16_t num_modems,
        uint16_t num_el_devs, double totalSquare, std::ofstream &fo)
{
  node_list.resize (num_modems + num_el_devs);

  //
  // Create node_list
  //
  for (uint16_t i = 0; i < num_modems; i++)
    {
      NS_LOG_DEBUG("Creating a modem " << i);
      node_list.at (i) = CreateObject<PLC_Node> ();
      node_list.at (i)->SetName (std::string ("Node ") + std::to_string (i + 1));
    }

  //
  // Create electrical devices
  //
  for (uint16_t i = num_modems; i < num_modems + num_el_devs; i++)
    {
      NS_LOG_DEBUG("Creating an electrical device " << i);
      node_list.at (i) = CreateObject<PLC_Electrical_Device> ();
      node_list.at (i)->SetName (std::string ("El.Dev. ") + std::to_string (i + 1 - num_modems));
    }

  //
  // Set node_list positions and connect them with cables
  // Here are additional node_list created - distribution boxes
  //
  InhomeTopology topology (totalSquare, num_modems, num_el_devs);
  topology.Create (node_list, cable);

  //
  // Let the distribution boxes do not have any impedance and create no noise
  //
  for (uint16_t i = num_modems + num_el_devs; i < node_list.size (); i++)
    {
      node_list.at (i)->GetObject<PLC_Node> ()->OpenCircuit ();
    }
}

void
WriteBackgroundNoise (Ptr<SpectrumValue> noiseFloor, std::ofstream &fo)
{

}
void
LoadInhomeTopology (PLC_NodeList &node_list, Ptr<PLC_Cable> cable, Ptr<const SpectrumModel> sm, uint16_t &num_modems,
        uint16_t &num_el_devs, double &totalSquare, std::ifstream &fi)
{

}
Ptr<SpectrumValue>
LoadInhomeBackgroundNoise (Ptr<const SpectrumModel> sm, std::ifstream &fi)
{

}
