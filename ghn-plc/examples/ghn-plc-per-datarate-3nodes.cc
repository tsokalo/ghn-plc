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
  Ptr<PLC_Node> n3 = CreateObject<PLC_Node> ();
  n1->SetPosition (0, 0, 0);
  n2->SetPosition (50, 0, 0);
  n3->SetPosition (100, 0, 0);

  PLC_NodeList nodes;
  nodes.push_back (n1);
  nodes.push_back (n2);
  nodes.push_back (n3);

  //
  // Link nodes
  //
  CreateObject<PLC_Line> (cable, n1, n2);
  CreateObject<PLC_Line> (cable, n2, n3);

  //
  // Set up channel
  //
  PLC_ChannelHelper channelHelper (sm);
  channelHelper.Install (nodes);
  Ptr<PLC_Channel> channel = channelHelper.GetChannel ();

  //
  // Create interfaces (usually done by the device helper)
  //
  auto add_txIf = [&] (Ptr<PLC_Node> node)
    {
      Ptr<PLC_TxInterface> txIf = CreateObject<PLC_TxInterface> (node, sm);
      channel->AddTxInterface (txIf);
      return txIf;
    };
  auto txIf1 = add_txIf (n1);
  auto txIf2 = add_txIf (n2);

  auto add_rxIf = [&] (Ptr<PLC_Node> node)
    {
      Ptr<PLC_RxInterface> rxIf = CreateObject<PLC_RxInterface> (node, sm);
      channel->AddRxInterface (rxIf);
      return rxIf;
    };
  auto rxIf2 = add_rxIf (n2);
  auto rxIf3 = add_rxIf (n3);

  channel->InitTransmissionChannels ();
  channel->CalcTransmissionChannels ();

  Ptr<SpectrumValue> noiseFloor = CreateWorstCaseBgNoise(sm)->GetNoisePsd ();

  auto get_sinr = [&] (Ptr<PLC_TxInterface> txIf, Ptr<PLC_RxInterface> rxIf)
    {
      Ptr<PLC_ChannelTransferImpl> chImpl = txIf->GetChannelTransferImpl (PeekPointer (rxIf));
      NS_ASSERT(chImpl);
      Ptr<SpectrumValue> rxPsd = chImpl->CalculateRxPowerSpectralDensity (txPsd);
      PLC_Interference interference;
      interference.SetNoiseFloor (noiseFloor);
      interference.StartRx (rxPsd);
      return interference.GetSinr ();
    };

  auto sinr12 = get_sinr (txIf1, rxIf2);
  auto sinr13 = get_sinr (txIf1, rxIf3);
  auto sinr23 = get_sinr (txIf2, rxIf3);

  //
  // Calculate BAT
  //
  Ptr<NcBlVarBatMap> bl = CreateObject<NcBlVarBatMap> ();

  std::vector<double> pers =
    { 0.00001, 0.01, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 0.95 };

  for (auto per : pers)
    {
      //
      // calculate reception data rate of the destination when the source sends
      //
      double e13 = per;
      BitAllocationTable bat = bl->CalculateBat (e13, *sinr13);
      double ofdm_symb_duration = PLC_Phy::GetSymbolDuration ().GetDouble ();
      uint32_t bits_per_ofdm_symb = bl->CalcBitsPerSymbol (bat);
      double d13 = (double) bits_per_ofdm_symb / ofdm_symb_duration;
      //
      // calculate PER for the relay
      //
      double cap = bl->GetOfdmSymbolCapacity (bat, sinr12);
      bits_per_ofdm_symb = bl->CalcBitsPerSymbol (bat);
      double ber = (cap > bits_per_ofdm_symb) ? 0 : ((double)bits_per_ofdm_symb - cap)  / (double)bits_per_ofdm_symb;
      double e12 = PbMappingList::get_per(ber);
      double d23 = d13;
      //
      // calculate reception data rate of the destination when the relay sends
      //
      double e23 = 0.00001;
      bat = bl->CalculateBat (e23, *sinr23);
      bits_per_ofdm_symb = bl->CalcBitsPerSymbol (bat);
      double d23 = (double) bits_per_ofdm_symb / ofdm_symb_duration;

      std::cout << e13 << "\t" << e12 << "\t" << e23 << "\t" << d13 << "\t" << d12 << "\t" << d23 << std::endl;
    }

  return EXIT_SUCCESS;
}
