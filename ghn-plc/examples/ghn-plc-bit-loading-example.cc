/*
 *      Author: tsokalo
 */

#include <iostream>
#include <sstream>
#include <fstream>
#include <time.h>

#include <ns3/core-module.h>
#include <ns3/nstime.h>
#include <ns3/simulator.h>
#include <ns3/output-stream-wrapper.h>
#include "ns3/plc.h"
#include "ns3/plc-electrical-device.h"
#include <boost/assign/list_of.hpp>
#include "ns3/in-home-topology.h"
#include "ns3/nc-module.h"

using namespace ns3;
NS_LOG_COMPONENT_DEFINE("NC_BIT_LOADING_EXAMPLE");

int
main (int argc, char *argv[])
{
  BandPlanType bandplan = GDOTHN_BANDPLAN_25MHZ;
  Ptr<NcHelper> helper = CreateObject<NcHelper> (bandplan);
  Ptr<const SpectrumModel> sm = helper->GetSpectrumModel ();
  Ptr<SpectrumValue> txPsd = helper->GetTxPsd ();
  Ptr<PLC_Cable> cable = CreateObject<PLC_NAYY150SE_Cable> (sm);

  //
  // Create nodes
  //
  Ptr<PLC_Node> n1 = CreateObject<PLC_Node> ();
  Ptr<PLC_Node> n2 = CreateObject<PLC_Node> ();
  n1->SetPosition (0, 0, 0);
  n2->SetPosition (800, 0, 0);

  PLC_NodeList nodes;
  nodes.push_back (n1);
  nodes.push_back (n2);

  //
  // Link nodes
  //
  CreateObject<PLC_Line> (cable, n1, n2);

  //
  // Set up channel
  //
  PLC_ChannelHelper channelHelper (sm);
  channelHelper.Install (nodes);
  Ptr<PLC_Channel> channel = channelHelper.GetChannel ();

  //
  // Create interfaces (usually done by the device helper)
  //
  Ptr<PLC_TxInterface> txIf = CreateObject<PLC_TxInterface> (n1, sm);
  Ptr<PLC_RxInterface> rxIf = CreateObject<PLC_RxInterface> (n2, sm);

  //
  // Add interfaces to the channel (usually done by the device helper)
  //
  channel->AddTxInterface (txIf);
  channel->AddRxInterface (rxIf);

  channel->InitTransmissionChannels ();
  channel->CalcTransmissionChannels ();
  //
  // The receive power spectral density computation is done by the channel
  // transfer implementation from TX interface to RX interface
  //
  Ptr<PLC_ChannelTransferImpl> chImpl = txIf->GetChannelTransferImpl (PeekPointer (rxIf));
  NS_ASSERT(chImpl);
  Ptr<SpectrumValue> rxPsd = chImpl->CalculateRxPowerSpectralDensity (txPsd);

  //
  // Set background noise
  //
  Ptr<SpectrumValue> noiseFloor = CreateBestCaseBgNoise(sm)->GetNoisePsd ();

  //
  // Calculate SINR
  //
  PLC_Interference interference;
  interference.SetNoiseFloor (noiseFloor);
  interference.StartRx (rxPsd);
  Ptr<const SpectrumValue> sinr = interference.GetSinr ();

  //  NS_LOG_UNCOND("Transmit power spectral density:\n" << *txPsd << "\n");
  //  NS_LOG_UNCOND("Receive power spectral density:\n" << *rxPsd << "\n");
  //  NS_LOG_UNCOND("Noise power spectral density:\n" << *noiseFloor << "\n");
  //  NS_LOG_UNCOND("Signal to interference and noise ratio:\n" << *sinr);

  Ptr<NcBlVarBatMap> bl = CreateObject<NcBlVarBatMap> ();
  //
  // BER
  //
  double p = 0.029323;
  BitAllocationTable m = bl->CalculateBat (p, *sinr);

  //  NS_LOG_UNCOND("Tone map:\n" << m);
  SpectrumValue sinr_v = *sinr;
  for (uint16_t s = 0; s < m.size (); s++)
    {
      double b = m.at (s);
      double cap = GetCapPerChannel (sinr_v[s], m.at (s));
      std::cout << s << "\t" << (uint16_t) m.at (s) << "\t" <<(b == 0 ? 0 : (b - cap) / b) << std::endl;
    }

  cap_map_t cap_map = bl->CalculateCapMap (*sinr);
  for (uint16_t s = 0; s < cap_map.size (); s++)
    for (uint16_t i = 0; i < cap_map.at (s).size (); i++)
      {
        double b = i + 1;
        double cap = cap_map.at (s).at (i);
        std::cout << (*sinr)[s] << "\t" << i + 1 << "\t" << (b - cap) / b << std::endl;
      }

  return EXIT_SUCCESS;
}
