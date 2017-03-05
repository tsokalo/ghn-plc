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

#define DELIMITER '\t'

using namespace ns3;
using namespace ghn;

std::string
ConstructResFoldName (int argc, char *argv[]);

void
CreateInhomeTopology (PLC_NodeList &node_list, Ptr<PLC_Cable> cable, uint16_t num_modems,
        uint16_t num_el_devs, double totalSquare, std::ostream &fo);
void
SaveBackgroundNoise (Ptr<SpectrumValue> noiseFloor, std::ostream &fo);
void
LoadInhomeTopology (PLC_NodeList &node_list, Ptr<PLC_Cable> cable, uint16_t &num_modems,
        uint16_t &num_el_devs, double &totalSquare, std::istream &fi);
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
      totalSquare = 60;

      CreateInhomeTopology (node_list, cable, num_modems, num_el_devs, totalSquare, fo);
      modem_list.insert (modem_list.begin (), node_list.begin (), node_list.begin () + num_modems);
      eldev_list.insert (eldev_list.begin (), node_list.begin () + num_modems, node_list.begin () + num_modems + num_el_devs);
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
      channel = channelHelper.GetChannel ();
      cout << "Created channel.." << endl;
      //
      // Create electrical devices
      //
      elDevHelper.SetNodeList (eldev_list);
      elDevHelper.SetChannel (channel);
      elDevHelper.Setup ();

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
  devHelper.Setup();
  cout << "Created communication devices.." << endl;

  channel->InitTransmissionChannels ();
  channel->CalcTransmissionChannels ();
  cout << "Channel is initialized.." << endl;

  auto get_sinr = [&](Ptr<PLC_TxInterface> txIf, Ptr<PLC_RxInterface> rxIf)
    {
      auto chImpl = txIf->GetChannelTransferImpl (PeekPointer (rxIf));
      auto rxPsd = chImpl->CalculateRxPowerSpectralDensity (txPsd);

      PLC_Interference interference;
      interference.SetNoiseFloor (noiseFloor);
      interference.StartRx (rxPsd);
      return interference.GetSinr ();
    };

  auto save_snr = [&](Ptr<PLC_TxInterface> txIf, Ptr<PLC_RxInterface> rxIf, uint16_t s_id, uint16_t r_id)
    {
      Ptr<const SpectrumValue> sinr = get_sinr(txIf, rxIf);
      SpectrumValue sinr_db = 20 * Log10 (*sinr);

      Bands::const_iterator bi = sinr_db.ConstBandsBegin ();
      Values::const_iterator vi = sinr_db.ConstValuesBegin ();

      std::string f_name = resDir + "/sinr_dB_" + "_" + std::to_string(s_id) + "_" + std::to_string(r_id) + ".txt";
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

  auto get_max_achievable_rate = [](double d0, double d1, double e1, double e2, double e3)
    {
      if(d1 * (1 - e2) < d0 * (1 - e3))
        {
          return d0 * (1 - e3);
        }
      else
        {
          auto a = d0 * d1 * (1 - e2)*(1 - e1*e3);
          auto b = d1 * (1 - e2) + d0 * e3 * (1 - e1);
          return a / b;
        }
    };

  auto get_lrp_achievalbe_rate = [](double d0, double d1, double e1, double e2, double e3)
    {
      auto v1 = d0 * (1 - e3);
      auto a = d0 * d1 * (1 - e1)*(1 - e2);
      auto b = d1 * (1 - e2) + d0 * (1 - e1);
      auto v2 = a / b;

      return (v1 > v2) ? v1 : v2;
    };

  auto get_gain = [&](double d0, double d1, double e1, double e2, double e3)
    {
      auto v1 = get_max_achievable_rate(d0, d1, e1, e2, e3);
      auto v2 = get_lrp_achievalbe_rate(d0, d1, e1, e2, e3);

      return (v1 - v2) / v2;
    };

  auto save_gains =
          [&](Ptr<const SpectrumValue> sinr12, Ptr<const SpectrumValue> sinr13, Ptr<const SpectrumValue> sinr23, uint16_t s_id, uint16_t r1_id, uint16_t r2_id)
            {
              std::string f_name = resDir + "/gains_" + "_" + std::to_string(s_id) + "_" + std::to_string(r1_id) + "_" + std::to_string(r2_id) + ".txt";
              std::cout << "Gains between " << s_id << " and " << r1_id << ", " << r2_id << " in " << f_name << std::endl;
              std::ofstream f(f_name, std::ios::out);

              Ptr<NcBlVarBatMap> bl = CreateObject<NcBlVarBatMap> ();

              std::vector<double> pers =
                { 0.00001, 0.01, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 0.95};

              for (auto per : pers)
                {
                  //
                  // calculate reception data rate of the destination when the source sends
                  //
                  double e1 = per;
                  double ber1 = PbMappingList::get_ber (e1);
                  BitAllocationTable bat = bl->CalculateBat (ber1, *sinr12);
                  double ofdm_symb_duration = PLC_Phy::GetSymbolDuration ().GetDouble ();
                  uint32_t bits_per_ofdm_symb = bl->CalcBitsPerSymbol (bat);
                  double d0 = (double) bits_per_ofdm_symb / ofdm_symb_duration;
                  //
                  // calculate PER for the relay
                  //
                  double cap13 = bl->GetOfdmSymbolCapacity (bat, *sinr13);
                  double ber3 = (cap13 > bits_per_ofdm_symb) ? 0 : ((double) bits_per_ofdm_symb - cap13) / (double) bits_per_ofdm_symb;
                  double e3 = PbMappingList::get_per (ber3);
                  //
                  // calculate reception data rate of the destination when the relay sends
                  //
                  double e2 = 0.00001;
                  double ber2 = PbMappingList::get_ber (e2);
                  bat = bl->CalculateBat (ber2, *sinr23);
                  bits_per_ofdm_symb = bl->CalcBitsPerSymbol (bat);
                  double d1 = (double) bits_per_ofdm_symb / ofdm_symb_duration;

                  f << e3 << "\t" << e1 << "\t" << e2 << "\t" << d0 << "\t" << d1 << "\t"
                  << get_max_achievable_rate (d0, d1, e1, e2, e3)<< "\t"
                  << get_lrp_achievalbe_rate (d0, d1, e1, e2, e3)<< "\t"
                  << get_gain (d0, d1, e1, e2, e3) << std::endl;
                }
              f.close();
            };

  auto choose_tx_if = [&](Ptr<PLC_Phy> phy)
    {
      return (macMode == CSMA_CD) ? phy->GetObject<GhnPlcPhyPmdFullD> ()->GetTxInterface () :
      phy->GetObject<GhnPlcPhyPmdHalfD> ()->GetTxInterface ();
    };
  auto choose_rx_if = [&](Ptr<PLC_Phy> phy)
    {
      return (macMode == CSMA_CD) ? phy->GetObject<GhnPlcPhyPmdFullD> ()->GetRxInterface () :
      phy->GetObject<GhnPlcPhyPmdHalfD> ()->GetRxInterface ();
    };

  for (auto s : modem_list)
    {
      auto phy_s = devHelper.GetDevice (s->GetName ())->GetPhy ();
      Ptr<PLC_TxInterface> txIf = choose_tx_if (phy_s);

      for (auto r : modem_list)
        {
          if (s->GetVertexId () == r->GetVertexId ()) continue;

          auto phy_r = devHelper.GetDevice (r->GetName ())->GetPhy ();
          Ptr<PLC_RxInterface> rxIf = choose_rx_if (phy_r);
          save_snr (txIf, rxIf, s->GetVertexId (), r->GetVertexId ());
        }
    }

  //
  // works ONLY for three-node scenario
  //
  assert(modem_list.size () == 3);
  //
  // select source
  //
  for (auto s : modem_list)
    {
      auto phy_s = devHelper.GetDevice (s->GetName ())->GetPhy ();
      Ptr<PLC_TxInterface> txIf1= choose_tx_if (phy_s);
      auto s_id = s->GetVertexId ();

      //
      // select relay
      //
      for (auto r : modem_list)
        {
          auto r_id = r->GetVertexId ();
          if (s_id == r_id) continue;

          auto phy_r = devHelper.GetDevice (r->GetName ())->GetPhy ();
          Ptr<PLC_TxInterface> txIf2 = choose_tx_if (phy_r);
          Ptr<PLC_RxInterface> rxIf2 = choose_rx_if (phy_r);
          //
          // select destination
          //
          for (auto d : modem_list)
            {
              auto d_id = d->GetVertexId ();
              if (s_id == d_id || r_id == d_id) continue;

              auto phy_d = devHelper.GetDevice (d->GetName ())->GetPhy ();
              Ptr<PLC_RxInterface> rxIf3 = choose_rx_if (phy_d);

              auto sinr12 = get_sinr (txIf1, rxIf2);
              auto sinr13 = get_sinr (txIf1, rxIf3);
              auto sinr23 = get_sinr (txIf2, rxIf3);

              save_gains(sinr12, sinr13, sinr23, s_id, r_id, d_id);
            }
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
CreateInhomeTopology (PLC_NodeList &node_list, Ptr<PLC_Cable> cable, uint16_t num_modems,
        uint16_t num_el_devs, double totalSquare, std::ostream &fo)
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
  topology.Create (node_list, cable);
  topology.Save(fo);

  //
  // Let the distribution boxes do not have any impedance and create no noise
  //
  for (uint16_t i = num_modems + num_el_devs; i < node_list.size (); i++)
    {
      node_list.at (i)->GetObject<PLC_Node> ()->OpenCircuit ();
    }
}
void
LoadInhomeTopology (PLC_NodeList &node_list, Ptr<PLC_Cable> cable, uint16_t &num_modems,
        uint16_t &num_el_devs, double &totalSquare, std::istream &fi)
{
  fi >> num_modems;
  fi >> num_el_devs;
  fi >> totalSquare;

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

  while(it != noiseFloor->ValuesEnd())
    {
      fo << *it << DELIMITER;
      it++;
    }
}

Ptr<SpectrumValue>
LoadInhomeBackgroundNoise (Ptr<const SpectrumModel> sm, std::istream &fi)
{
  Ptr<SpectrumValue> noiseFloor = Create<SpectrumValue>(sm);
  uint32_t n = sm->GetNumBands ();
  for(uint32_t i = 0; i < n; i++)
    {
      fi >> (*noiseFloor)[i];
    }
  return noiseFloor;
}
