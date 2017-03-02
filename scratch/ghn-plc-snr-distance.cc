/*
 *      Author: tsokalo
 */

#include <iostream>
#include <sstream>
#include <fstream>
#include <time.h>
#include <vector>

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
  Ptr<SpectrumValue> noiseFloor = CreateBestCaseBgNoise(sm)->GetNoisePsd ();
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
      // TODO: convert to dB and format output with carriers and write to files
      NS_LOG_UNCOND("SINR between 0 and " << i << ":\n" << *sinr);
    }

  return EXIT_SUCCESS;
}
