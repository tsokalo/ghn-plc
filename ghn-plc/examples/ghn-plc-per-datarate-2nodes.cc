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
  Ptr<PLC_Node> n1 = CreateObject<PLC_Node> ();
  Ptr<PLC_Node> n2 = CreateObject<PLC_Node> ();
  n1->SetPosition (0, 0, 0);
  n2->SetPosition (50, 0, 0);

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

  //
  // Calculate BAT
  //
  Ptr<NcBlVarBatMap> bl = CreateObject<NcBlVarBatMap> ();

  std::vector<double> pers =
    { 0.0001, 0.01, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 0.95 };

  for (auto per : pers)
    {
      BitAllocationTable bat = bl->CalculateBat (per, *sinr);

      double ofdm_symb_duration = PLC_Phy::GetSymbolDuration ().GetDouble();
      uint32_t bits_per_ofdm_symb = bl->CalcBitsPerSymbol (bat);
      double d = (double) bits_per_ofdm_symb / ofdm_symb_duration;

      std::cout << per << "\t" << d << std::endl;
    }

  return EXIT_SUCCESS;
}
