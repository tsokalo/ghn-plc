/*
 *      Author: tsokalo
 */

#include <iostream>
#include <sstream>
#include <fstream>
#include <time.h>
#include <vector>
#include <string.h>

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

std::string
ConstructResFoldName (int argc, char *argv[]);

int
main (int argc, char *argv[])
{
  std::string resDir = ConstructResFoldName (argc, argv);
  ghn::RemoveDirectory (resDir);
  ghn::CreateDirectory (resDir);

  BandPlanType bandplan = GDOTHN_BANDPLAN_25MHZ;
  GhnPlcHelper helper (bandplan);
  Ptr<const SpectrumModel> sm = helper.GetSpectrumModel ();
  Ptr<PLC_Cable> cable = CreateObject<PLC_NAYY150SE_Cable> (sm);
  Ptr<SpectrumValue> txPsd = helper.GetTxPsd ();

  //
  // Create nodes
  //
  std::vector<uint32_t> distance;
  uint32_t distance_ptp = 50;
  uint16_t num_modems = 30;
  for (uint16_t i = 0; i < num_modems - 1; i++)
    distance.push_back (distance_ptp); //unit [meters]
  PLC_NodeList node_list;
  CreateLineTopology (node_list, cable, sm, distance);

  //
  // Set up channel
  //
  PLC_ChannelHelper channelHelper (sm);
  channelHelper.Install (node_list);
  Ptr<PLC_Channel> channel = channelHelper.GetChannel ();

  //
  // Define background noise
  //
  Ptr<SpectrumValue> noiseFloor = CreateWorstCaseBgNoise(sm)->GetNoisePsd ();
  PLC_Interference interference;
  interference.SetNoiseFloor (noiseFloor);

  //
  // Create interfaces (usually done by the device helper)
  //
  auto sender = node_list.at(0);
  Ptr<PLC_TxInterface> txIf = CreateObject<PLC_TxInterface> (sender, sm);
  channel->AddTxInterface (txIf);

  std::vector<Ptr<PLC_RxInterface> > rxIfs;
  for(auto i = 1 ; i < node_list.size(); i++)
    {
      auto receiver = node_list.at(i);
      auto rxIf = CreateObject<PLC_RxInterface> (receiver, sm);
      rxIfs.push_back(rxIf);
      channel->AddRxInterface (rxIf);
    }

  //
  // Add interfaces to the channel (usually done by the device helper)
  //
  channel->InitTransmissionChannels ();
  channel->CalcTransmissionChannels ();

  //
  // The receive power spectral density computation is done by the channel
  // transfer implementation from TX interface to RX interface
  //
  for(auto i = 1 ; i < node_list.size(); i++)
    {
      auto rxIf = rxIfs.at(i - 1);
      auto chImpl = txIf->GetChannelTransferImpl (PeekPointer (rxIf));
      auto rxPsd = chImpl->CalculateRxPowerSpectralDensity (txPsd);

      interference.StartRx (rxPsd);
      Ptr<const SpectrumValue> sinr = interference.GetSinr ();
      SpectrumValue sinr_db = 20 * Log10 (*sinr);

      Bands::const_iterator bi = sinr_db.ConstBandsBegin ();
      Values::const_iterator vi = sinr_db.ConstValuesBegin ();

      std::string f_name = resDir + "sinr_dB_0_" + std::to_string(i) + ".txt";
      std::cout << "SINR between 0 and " << i << " in " << f_name << std::endl;

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
