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
ConstructResFoldName (int argc, char *argv[]);

//export 'NS_LOG=InhomeTopology=level_all|prefix_time|prefix_node|prefix_func'

int
main (int argc, char *argv[])
{
  std::string resDir = ConstructResFoldName (argc, argv);
  ghn::RemoveDirectory (resDir);
  ghn::CreateDirectory (resDir);

  BandPlanType bandplan = GDOTHN_BANDPLAN_25MHZ;
  GhnPlcDllMacProtocol macMode = CSMA_CD;

  GhnPlcHelper devHelper (bandplan);
  Ptr<const SpectrumModel> sm = devHelper.GetSpectrumModel ();
  Ptr<PLC_Cable> cable = CreateObject<PLC_NAYY50SE_Cable> (sm);
  Ptr<SpectrumValue> txPsd = devHelper.GetTxPsd ();
  cout << *txPsd << endl;

  //
  // Create nodes
  //
  std::vector<uint32_t> distance;
  uint32_t distance_ptp = 600;
  uint16_t num_modems = 3;
  for (uint16_t i = 0; i < num_modems - 1; i++)
    distance.push_back (distance_ptp); //unit [meters]
  PLC_NodeList node_list;
  CreateLineTopology (node_list, cable, sm, distance);
  std::cout << "Created topology.." << std::endl;

  //
  // Set up channel
  //
  PLC_ChannelHelper channelHelper (sm);
  channelHelper.Install (node_list);
  auto channel = channelHelper.GetChannel ();
  cout << "Created channel.." << endl;

  //
  // Define background noise
  //
  Ptr<SpectrumValue> noiseFloor = CreateWorstCaseBgNoise(sm)->GetNoisePsd ();
  cout << *noiseFloor << endl;
  cout << "Created background noise.." << endl;

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

      std::string f_name = resDir + "/sinr_dB_" + std::to_string(s_id) + "_" + std::to_string(r_id) + ".txt";
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

  bool use_fec = false;

  auto get_ber =
          use_fec ? std::bind (&PbMappingList::get_ber, std::placeholders::_1) :
                  std::bind (&PbMappingList::get_ber_nofec, std::placeholders::_1);
  auto get_per =
          use_fec ? std::bind (&PbMappingList::get_per, std::placeholders::_1) :
                  std::bind (&PbMappingList::get_per_nofec, std::placeholders::_1);
  auto get_ct =
          use_fec ? std::bind (&PbMappingList::get_ct, std::placeholders::_1) :
                  std::bind (&PbMappingList::get_ct_nofec, std::placeholders::_1);

  auto save_gains =
          [&](Ptr<const SpectrumValue> sinr12, Ptr<const SpectrumValue> sinr13, Ptr<const SpectrumValue> sinr23, uint16_t s_id, uint16_t r1_id, uint16_t r2_id)
            {
              std::string f_name = resDir + "/gains_" + std::to_string(s_id) + "_" + std::to_string(r1_id) + "_" + std::to_string(r2_id) + ".txt";
              std::cout << "Gains between " << s_id << " and " << r1_id << ", " << r2_id << " in " << f_name << std::endl;
              std::ofstream f(f_name, std::ios::out);

              Ptr<NcBlVarBatMap> bl = CreateObject<NcBlVarBatMap> ();

              std::vector<double> pers =
                { 0.0001, 0.01, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 0.95};

              for (auto per : pers)
                {
                  //
                  // calculate reception data rate of the destination when the source sends
                  //
                  double e3 = per;
                  double ber3 = get_ber (e3);
                  BitAllocationTable bat = bl->CalculateBat (ber3, *sinr13);
                  double ofdm_symb_duration = PLC_Phy::GetSymbolDuration ().GetDouble ();
                  uint32_t bits_per_ofdm_symb = bl->CalcBitsPerSymbol (bat);
                  double d0 = (double) bits_per_ofdm_symb / ofdm_symb_duration * get_ct(e3);
                  //
                  // calculate PER for the relay
                  //
                  double cap12 = bl->GetOfdmSymbolCapacity (bat, *sinr12);
                  assert(bits_per_ofdm_symb != 0);
                  double ber1 = (cap12 > bits_per_ofdm_symb) ? 0 : ((double) bits_per_ofdm_symb - cap12) / (double) bits_per_ofdm_symb;
                  double e1 = get_per (ber1);
                  //
                  // calculate reception data rate of the destination when the relay sends
                  //
                  double e2 = 0.0001;
                  double ber2 = get_ber (e2);
                  bat = bl->CalculateBat (ber2, *sinr23);
                  bits_per_ofdm_symb = bl->CalcBitsPerSymbol (bat);
                  double d1 = (double) bits_per_ofdm_symb / ofdm_symb_duration * get_ct(e2);

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

  for (auto s : node_list)
    {
      auto phy_s = devHelper.GetDevice (s->GetName ())->GetPhy ();
      Ptr<PLC_TxInterface> txIf = choose_tx_if (phy_s);

      for (auto r : node_list)
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
  assert(node_list.size () == 3);
  //
  // select source
  //
  for (auto s : node_list)
    {
      auto phy_s = devHelper.GetDevice (s->GetName ())->GetPhy ();
      Ptr<PLC_TxInterface> txIf1 = choose_tx_if (phy_s);
      auto s_id = s->GetVertexId ();

      //
      // select relay
      //
      for (auto r : node_list)
        {
          auto r_id = r->GetVertexId ();
          if (s_id == r_id) continue;

          auto phy_r = devHelper.GetDevice (r->GetName ())->GetPhy ();
          Ptr<PLC_TxInterface> txIf2 = choose_tx_if (phy_r);
          Ptr<PLC_RxInterface> rxIf2 = choose_rx_if (phy_r);
          //
          // select destination
          //
          for (auto d : node_list)
            {
              auto d_id = d->GetVertexId ();
              if (s_id == d_id || r_id == d_id) continue;

              auto phy_d = devHelper.GetDevice (d->GetName ())->GetPhy ();
              Ptr<PLC_RxInterface> rxIf3 = choose_rx_if (phy_d);

              auto sinr12 = get_sinr (txIf1, rxIf2);
              auto sinr13 = get_sinr (txIf1, rxIf3);
              auto sinr23 = get_sinr (txIf2, rxIf3);

              save_gains (sinr12, sinr13, sinr23, s_id, r_id, d_id);
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

