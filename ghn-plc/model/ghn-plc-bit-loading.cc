/*
 * GhnPlcBitLoading.cpp
 *
 *  Created on: Jun 13, 2016
 *      Author: tsokalo
 */
#include <iostream>
#include <array>
#include <limits>
#include "ns3/log.h"
#include "ns3/spectrum-value.h"
#include "ns3/simulator.h"
#include "ns3/plc-dcmc-capacity.h"
#include "ns3/plc-header.h"
#include "ghn-plc-bit-loading.h"
#include "ns3/ghn-plc-utilities.h"
#include "ns3/ghn-plc-bit-loading-data.h"
#include "ghn-plc-net-device.h"
NS_LOG_COMPONENT_DEFINE("GhnPlcBitLoading");

namespace ns3
{
namespace ghn
{
NS_OBJECT_ENSURE_REGISTERED(GhnPlcBitLoading);

PLC_InformationRateModel::McsInfo GhnPlcBitLoading::s_mcs_info[13] =
  {
    { QAM, 0 }, // NOMOD
        { QAM, 2 }, // BPSK
        { QAM, 4 }, // QAM4
        { QAM, 8 }, // QAM8
        { QAM, 16 }, // QAM16
        { QAM, 32 }, // QAM32
        { QAM, 64 }, // QAM64
        { QAM, 128 }, // QAM128
        { QAM, 256 }, // QAM256
        { QAM, 512 }, // QAM512
        { QAM, 1024 }, // QAM1024
        { QAM, 2048 }, // QAM2048
        { QAM, 4096 }, // QAM4096
        };

TypeId
GhnPlcBitLoading::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::GhnPlcBitLoading").SetParent<Object> ()

  .AddTraceSource ("CapacityLog", "The capacity at the moment of the network start up",
          MakeTraceSourceAccessor (&GhnPlcBitLoading::m_capacityTrace), "ns3::CapacityLogTrace::TracedCallback")

          ;
  return tid;
}

GhnPlcBitLoading::GhnPlcBitLoading ()
{

}

GhnPlcBitLoading::~GhnPlcBitLoading ()
{

}
void
GhnPlcBitLoading::AddNode (Ptr<PLC_Node> node)
{
  NS_LOG_FUNCTION(this << node->GetVertexId());
  this->m_nodes.push_back (node);
}

ModulationAndCodingScheme
GhnPlcBitLoading::GetModulationAndCodingScheme (uint32_t src_id, uint32_t dst_id)
{
  NS_LOG_FUNCTION(this);
  NS_ASSERT_MSG(src_id <= m_mcs.size (), src_id);
  NS_ASSERT_MSG(dst_id <= m_mcs.at (src_id).size (), dst_id);

  return m_mcs[src_id][dst_id];
}
Ptr<SpectrumValue>
GhnPlcBitLoading::GetTxPsd (uint32_t src_id, uint32_t dst_id)
{
  NS_LOG_FUNCTION(this);
  if (dst_id == UanAddress::GetBroadcast ().GetAsInt ()) return m_txEnvelope;

  NS_ASSERT_MSG(src_id <= m_blts.size (), src_id);
  NS_ASSERT_MSG(dst_id <= m_blts.at (src_id).size (), dst_id);

  return m_blts[src_id][dst_id];
}
double
GhnPlcBitLoading::GetNumEffBits (uint32_t src_id, uint32_t dst_id)
{
  NS_ASSERT_MSG(src_id <= m_capacityPerSymbol.size (), src_id);
  NS_ASSERT_MSG(dst_id <= m_capacityPerSymbol.at (src_id).size (), dst_id);

  return m_capacityPerSymbol[src_id][dst_id];
}
void
GhnPlcBitLoading::PrintBitLoadingTable ()
{
  NS_ASSERT(!m_mcs.empty ());
  NS_ASSERT(!m_capacityPerSymbol.empty ());

  for (std::vector<std::vector<ModulationAndCodingScheme> >::iterator its = m_mcs.begin (); its < m_mcs.end (); its++)
    {
      for (std::vector<ModulationAndCodingScheme>::iterator itd = its->begin (); itd < its->end (); itd++)
        {
          if (m_capacityPerSymbol[std::distance (m_mcs.begin (), its)][std::distance (its->begin (), itd)] > 0)
            {
              std::cout << "From " << std::distance (m_mcs.begin (), its);
              std::cout << " to " << std::distance (its->begin (), itd);
              std::cout << " -> MCS: " << *itd << " capacity per symbol: ";
              std::cout << m_capacityPerSymbol[std::distance (m_mcs.begin (), its)][std::distance (its->begin (), itd)]
                      << std::endl;
            }
        }
    }
}
uint32_t
GhnPlcBitLoading::GetDataAmount (Time txTime, uint8_t src_id, uint8_t dst_id)
{
  NS_LOG_FUNCTION(this << txTime << (uint32_t)src_id << (uint32_t)dst_id);
  NS_ASSERT_MSG(m_bitsPerSymbol.find (src_id) != m_bitsPerSymbol.end(), src_id);
  NS_ASSERT_MSG(m_bitsPerSymbol[src_id].find (dst_id) != m_bitsPerSymbol[src_id].end(), dst_id);

  Time headerSymbDuration = PLC_Phy::GetHeaderSymbolDuration ();
  Time symbolDuration = PLC_Phy::GetSymbolDuration ();
  double payloadSymbols = (txTime.GetInteger () - headerSymbDuration.GetInteger () - PLC_Preamble::GetDuration ().GetInteger ())
          / symbolDuration.GetInteger ();
  NS_LOG_UNCOND(
          headerSymbDuration << " / " << PLC_Preamble::GetDuration () << " / " << (txTime.GetInteger () - headerSymbDuration.GetInteger () - PLC_Preamble::GetDuration ().GetInteger ()) << " / " << payloadSymbols << " / " << m_capacityPerSymbol[src_id][dst_id]);
  return payloadSymbols * m_bitsPerSymbol[src_id][dst_id] / 8;
}
Ptr<PLC_ChannelTransferImpl>
GhnPlcBitLoading::GetChannalTransferImpl (uint16_t src_id, uint16_t dst_id)
{
  auto find_phy = [&](uint16_t vertex_id)
    {
      for (uint32_t i = 0; i < m_channel->GetNDevices (); i++)
        {
          if (m_channel->GetDevice (i)->GetObject<GhnPlcNetDevice> ()->GetPlcNode ()->GetVertexId () == vertex_id)
            {
              return m_channel->GetDevice (i)->GetObject<GhnPlcNetDevice> ()->GetDllManagement ()->GetPhyManagement ();
            }
        }
      NS_ASSERT(0);
    };

  auto src_phy = find_phy (src_id);
  auto dst_phy = find_phy (dst_id);

  return src_phy->GetTxInterface ()->GetChannelTransferImpl (PeekPointer (dst_phy->GetRxInterface ()));
}

double
GhnPlcBitLoading::GetNumEffBits (ModulationAndCodingScheme mcs, Ptr<SpectrumValue> sinr)
{
  return GetNumEffBits (mcs, *sinr);
}
void
GhnPlcBitLoading::CreateLogger ()
{
  m_aggr = CreateObject<FileAggregator> (m_resDir + "capacity.txt", FileAggregator::FORMATTED);
  m_aggr->Set3dFormat ("%.0f\t%.0f\t%.0f");
  m_aggr->Enable ();
  TraceConnect ("CapacityLog", "CapacityLogContext", MakeCallback (&FileAggregator::Write3d, m_aggr));
}
void
GhnPlcBitLoading::PrintCapacity ()
{
  for (std::vector<Ptr<PLC_Node> >::iterator it1 = m_nodes.begin (); it1 != m_nodes.end (); it1++)
    {
      uint32_t src_id = (*it1)->GetVertexId ();

      for (std::vector<Ptr<PLC_Node> >::iterator it2 = m_nodes.begin (); it2 != m_nodes.end (); it2++)
        {
          if (it1 == it2) continue;
          uint32_t dst_id = (*it2)->GetVertexId ();

          NS_LOG_DEBUG(
                  "Between " << src_id << " and " << dst_id << ", capacity: " << (double)m_capacityPerSymbol[src_id][dst_id] / (double)(PLC_Phy::GetSymbolDuration().GetSeconds()));
          m_capacityTrace (src_id, dst_id,
                  (double) m_capacityPerSymbol[src_id][dst_id] / (double) (PLC_Phy::GetSymbolDuration ().GetSeconds ()));
        }
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////              NcBlVarTxPsd           ////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
NS_OBJECT_ENSURE_REGISTERED(NcBlVarTxPsd);
TypeId
NcBlVarTxPsd::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NcBlVarTxPsd").SetParent<GhnPlcBitLoading> ().AddConstructor<NcBlVarTxPsd> ();
  return tid;
}
NcBlVarTxPsd::NcBlVarTxPsd ()
{
}
NcBlVarTxPsd::~NcBlVarTxPsd ()
{
}
void
NcBlVarTxPsd::CalcBitLoadingTable ()
{
  NS_LOG_FUNCTION(this);

  NS_LOG_LOGIC("Calculation BAT for " << m_nodes.size() << " nodes");

  CreateLogger ();

  auto create_table = [&](SpectrumTable &tab)
    {
      tab.resize (m_nodes.size ());
      for (auto it = tab.begin (); it != tab.end (); it++)
        {
          it->resize (m_nodes.size ());
          for (auto it2 = it->begin (); it2 != it->end (); it2++)
          *it2 = Create<SpectrumValue> (m_txEnvelope->GetSpectrumModel ());
        }
    };

  create_table (m_blts);
  create_table (m_sinr);

  for (std::vector<Ptr<PLC_Node> >::iterator it1 = m_nodes.begin (); it1 != m_nodes.end (); it1++)
    {
      for (std::vector<Ptr<PLC_Node> >::iterator it2 = m_nodes.begin (); it2 != m_nodes.end (); it2++)
        {
          if (it1 == it2) continue;
          uint32_t src_id = (*it1)->GetVertexId ();
          uint32_t dst_id = (*it2)->GetVertexId ();

          NS_LOG_LOGIC("Getting BAT between " << src_id << " and " << dst_id);

          NS_ASSERT_MSG(src_id <= m_nodes.size () && dst_id <= m_nodes.size (), src_id << dst_id);

          //
          // calculate levels = N/ H^2
          //
          Ptr<PLC_ChannelTransferImpl> chImpl = GetChannalTransferImpl (src_id, dst_id);
          Ptr<SpectrumValue> hquatrat = chImpl->GetAbsSqrCtf (PLC_Time::GetTimeslot (Simulator::Now ()));
          SpectrumValue levels = *m_noiseEnvelope / *hquatrat;

          std::vector<double> nh2;
          for (Values::iterator it = levels.ValuesBegin (); it != levels.ValuesEnd (); it++)
            nh2.push_back (*it);

          //
          // calculate TX PSD; make calculation separate for each band with constant TX power envelope
          //
          double act_val = 0;
          double p = 0; // total power allocated
          auto l = m_txEnvelope->ValuesBegin ();
          std::vector<double> txD;
          for (Values::iterator it = m_txEnvelope->ValuesBegin (); it != m_txEnvelope->ValuesEnd (); it++)
            {
              if (act_val != *it || it + 1 == m_txEnvelope->ValuesEnd ())
                {
                  if (std::distance (l, it) > 0)
                    {
                      uint32_t band_length = std::distance (l, it);
                      uint32_t band_start = std::distance (m_txEnvelope->ValuesBegin (), l);
                      std::vector<double> nh2_part (nh2.begin () + band_start, nh2.begin () + band_start + band_length);
                      NS_LOG_DEBUG(
                              "Band start: " << band_start << ", band end: " << band_start + band_length << ", Tx power: " << p);
                      m_waterFiller = std::shared_ptr<GhnPlcWaterFiller> (new GhnPlcWaterFiller (nh2_part, p));
                      std::vector<double> txD_part = m_waterFiller->CreateFill ();
                      txD.insert (txD.end (), txD_part.begin (), txD_part.end ());
                    }
                  act_val = *it;
                  p = 0;
                  l = it;
                }
              p += *it;
            }

          //
          // save SNR and TX PSD
          //
          auto txD_it = txD.begin ();
          auto sinr_it = m_sinr[src_id][dst_id]->ValuesBegin ();
          auto levels_it = levels.ValuesBegin ();
          for (Values::iterator it = m_blts[src_id][dst_id]->ValuesBegin (); it != m_blts[src_id][dst_id]->ValuesEnd (); it++)
            {
              *it = *(txD_it);
              *sinr_it = *it / *levels_it;
              sinr_it++;
              levels_it++;
              txD_it++;
            }
          //          std::cout << "SINR between " << src_id << " and " << dst_id << ": " << *(m_sinr[src_id][dst_id]) << std::endl;
        }
    }
  CalcModulationAndCodingScheme ();

  PrintCapacity ();
}

void
NcBlVarTxPsd::CalcModulationAndCodingScheme ()
{
  NS_LOG_FUNCTION(this);

  NS_ASSERT(UanAddress::GetBroadcast ().GetAsInt () >= m_nodes.size ());
  m_mcs.resize (m_nodes.size (),
          std::vector<ModulationAndCodingScheme> (UanAddress::GetBroadcast ().GetAsInt () + 1, ModulationAndCodingScheme ()));
  m_capacityPerSymbol.resize (m_nodes.size (), std::vector<double> (UanAddress::GetBroadcast ().GetAsInt () + 1, 0));

  for (std::vector<Ptr<PLC_Node> >::iterator it1 = m_nodes.begin (); it1 != m_nodes.end (); it1++)
    {
      for (std::vector<Ptr<PLC_Node> >::iterator it2 = m_nodes.begin (); it2 != m_nodes.end (); it2++)
        {
          if (it1 == it2) continue;
          uint32_t src_id = (*it1)->GetVertexId ();
          uint32_t dst_id = (*it2)->GetVertexId ();

          NS_ASSERT_MSG(src_id <= m_nodes.size () && dst_id <= m_nodes.size (), src_id << dst_id);

          for (uint16_t modulation = 12; modulation > 0; modulation--)
            {
              for (uint16_t codingRate = 1; codingRate < 6; codingRate++)
                {
                  ModulationAndCodingScheme mcs (ModulationType (modulation),
                          ConvertGhnRateToPlcRate (FecRateType (codingRate)), 0);

                  double capacityPerSymbol = GetNumEffBits (mcs, *m_sinr[src_id][dst_id]);

                  if (capacityPerSymbol > m_capacityPerSymbol[src_id][dst_id])
                    {
                      NS_LOG_DEBUG(
                              "Between " << src_id << " and " << dst_id << " " << mcs << ", capacity per symbol: " << capacityPerSymbol);
                      m_capacityPerSymbol[src_id][dst_id] = capacityPerSymbol;
                      m_bitsPerSymbol[src_id][dst_id] = modulation * m_sinr[src_id][dst_id]->GetSpectrumModel()->GetNumBands();
                      m_mcs[src_id][dst_id].mt = ModulationType (modulation);
                      m_mcs[src_id][dst_id].ct = ConvertGhnRateToPlcRate (FecRateType (codingRate));
                    }
                }
            }
        }
    }

  //
  // add info for broadcast address
  //
  for (std::vector<Ptr<PLC_Node> >::iterator it1 = m_nodes.begin (); it1 != m_nodes.end (); it1++)
    {
      uint32_t src_id = (*it1)->GetVertexId ();
      uint32_t dst_id = UanAddress::GetBroadcast ().GetAsInt ();
      m_mcs[src_id][dst_id].mt = BPSK;
      m_mcs[src_id][dst_id].ct = CODING_RATE_1_2;

      m_bitsPerSymbol[src_id][dst_id] = BPSK * m_txEnvelope->GetSpectrumModel()->GetNumBands();
      m_capacityPerSymbol[src_id][dst_id] = GetNumEffBits (m_mcs[src_id][dst_id], *m_txEnvelope / *m_noiseEnvelope);
    }
}
double
NcBlVarTxPsd::GetNumEffBits (ModulationAndCodingScheme mcs, SpectrumValue sinr)
{
  NS_LOG_FUNCTION(this << mcs);
  SpectrumValue sinr_db = 20 * Log10 (sinr);
  SpectrumValue capacityPerHertz = GetCapacity (sinr_db, GhnPlcBitLoading::s_mcs_info[mcs.mt].mod,
          GhnPlcBitLoading::s_mcs_info[mcs.mt].cardinality);

  //
  // Assume ideal coder
  //
  std::array<double, 7> codingRates =
    { (4.0 / 1.0), (2.0 / 1.0), (3.0 / 2.0), (21.0 / 16.0), (6.0 / 5.0), (18.0 / 16.0), (21.0 / 20.0) };

  double sum_bit = 0;
  double c = 0;
  for (Values::iterator it = capacityPerHertz.ValuesBegin (); it != capacityPerHertz.ValuesEnd (); it++)
    {
      sum_bit += *it;
      c++;
    }

  return (sum_bit * codingRates.at ((uint16_t) mcs.ct - 1) < (double) mcs.mt * c) ? 0 :
          (double) mcs.mt * c / codingRates.at ((uint16_t) mcs.ct - 1);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////              NcBlVarBatMap           ///////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
NS_OBJECT_ENSURE_REGISTERED(NcBlVarBatMap);
TypeId
NcBlVarBatMap::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NcBlVarBatMap").SetParent<GhnPlcBitLoading> ().AddConstructor<NcBlVarBatMap> ();
  return tid;
}
NcBlVarBatMap::NcBlVarBatMap ()
{
}
NcBlVarBatMap::~NcBlVarBatMap ()
{
}
void
NcBlVarBatMap::SetPer (uint16_t src_id, double per)
{
  m_desiredPer[src_id] = per;
}
void
NcBlVarBatMap::CalcBitLoadingTable ()
{
  NS_LOG_FUNCTION(this);

  NS_LOG_LOGIC("Calculation BAT for " << m_nodes.size() << " nodes");

  CreateLogger ();

  auto create_table = [&](SpectrumTable &tab)
    {
      tab.resize (m_nodes.size ());
      for (auto it = tab.begin (); it != tab.end (); it++)
        {
          it->resize (m_nodes.size ());
          for (auto it2 = it->begin (); it2 != it->end (); it2++)
          *it2 = Create<SpectrumValue> (m_txEnvelope->GetSpectrumModel ());
        }
    };

  create_table (m_blts);
  create_table (m_sinr);

  for (std::vector<Ptr<PLC_Node> >::iterator it1 = m_nodes.begin (); it1 != m_nodes.end (); it1++)
    {
      for (std::vector<Ptr<PLC_Node> >::iterator it2 = m_nodes.begin (); it2 != m_nodes.end (); it2++)
        {
          if (it1 == it2) continue;
          uint32_t src_id = (*it1)->GetVertexId ();
          uint32_t dst_id = (*it2)->GetVertexId ();

          //
          // define TX PSD
          //
          m_blts[src_id][dst_id] = m_txEnvelope;

          //
          // calculate SNR
          //
          Ptr<PLC_ChannelTransferImpl> chImpl = GetChannalTransferImpl (src_id, dst_id);
          Ptr<SpectrumValue> hquatrat = chImpl->GetAbsSqrCtf (PLC_Time::GetTimeslot (Simulator::Now ()));
          *(m_sinr[src_id][dst_id]) = (*m_txEnvelope) * (*hquatrat) / *m_noiseEnvelope;
        }
    }
  CalcModulationAndCodingScheme ();

  PrintCapacity ();
}

void
NcBlVarBatMap::CalcModulationAndCodingScheme ()
{
  NS_LOG_FUNCTION(this);

  NS_ASSERT(UanAddress::GetBroadcast ().GetAsInt () >= m_nodes.size ());
  m_mcs.resize (m_nodes.size (),
          std::vector<ModulationAndCodingScheme> (UanAddress::GetBroadcast ().GetAsInt () + 1, ModulationAndCodingScheme ()));
  m_capacityPerSymbol.resize (m_nodes.size (), std::vector<double> (UanAddress::GetBroadcast ().GetAsInt () + 1, 0));

  for (std::vector<Ptr<PLC_Node> >::iterator it1 = m_nodes.begin (); it1 != m_nodes.end (); it1++)
    {
      for (std::vector<Ptr<PLC_Node> >::iterator it2 = m_nodes.begin (); it2 != m_nodes.end (); it2++)
        {
          if (it1 == it2) continue;
          uint32_t src_id = (*it1)->GetVertexId ();
          uint32_t dst_id = (*it2)->GetVertexId ();

          m_mcs[src_id][dst_id].use_bat = true;
          m_mcs[src_id][dst_id].mt = NOMOD;
          assert(m_desiredPer[src_id] > 0.00001);
          PbMapping pbm = PbMappingList::get_val (m_desiredPer[src_id]);
          std::cout << "Projected BER " << pbm.ber << std::endl;
          m_mcs[src_id][dst_id].bat = CalculateBat (pbm.ber, *m_sinr[src_id][dst_id]);
          m_mcs[src_id][dst_id].ct = pbm.ct;
          m_bitsPerSymbol[src_id][dst_id] = CalcBitsPerSymbol(m_mcs[src_id][dst_id].bat);
          m_capacityPerSymbol[src_id][dst_id] = GetNumEffBits (m_mcs[src_id][dst_id], *m_sinr[src_id][dst_id]);
        }
    }

  //
  // add info for broadcast address
  //
  for (std::vector<Ptr<PLC_Node> >::iterator it1 = m_nodes.begin (); it1 != m_nodes.end (); it1++)
    {
      uint32_t src_id = (*it1)->GetVertexId ();
      uint32_t dst_id = UanAddress::GetBroadcast ().GetAsInt ();
      m_mcs[src_id][dst_id].mt = BPSK;
      m_mcs[src_id][dst_id].ct = CODING_RATE_1_2;

      m_bitsPerSymbol[src_id][dst_id] = BPSK * m_txEnvelope->GetSpectrumModel()->GetNumBands();
      m_capacityPerSymbol[src_id][dst_id] = GetNumEffBits (m_mcs[src_id][dst_id], *m_txEnvelope / *m_noiseEnvelope);
    }
}
double
NcBlVarBatMap::GetNumEffBits (ModulationAndCodingScheme mcs, SpectrumValue sinr)
{
  NS_LOG_FUNCTION(this << mcs);

  if (!mcs.use_bat)
    {
      assert(mcs.mt != NOMOD);
      for (Values::iterator it = sinr.ValuesBegin (); it != sinr.ValuesEnd (); it++)
        mcs.bat.push_back (mcs.mt);
    }

  assert(mcs.bat.size () == sinr.GetSpectrumModel ()->GetNumBands ());

  double sum_bit = GetOfdmSymbolCapacity (mcs, sinr);
  double max_sum_bit = mcs.bat.get_total_bits ();

  //
  // Assume ideal coder
  //
  std::array<double, 7> codingRates =
    { (4.0 / 1.0), (2.0 / 1.0), (3.0 / 2.0), (21.0 / 16.0), (6.0 / 5.0), (18.0 / 16.0), (21.0 / 20.0) };

  return (sum_bit * codingRates.at ((uint16_t) mcs.ct - 1) < max_sum_bit) ? 0 :
          max_sum_bit / codingRates.at ((uint16_t) mcs.ct - 1);
}
BitAllocationTable
NcBlVarBatMap::CalculateBat (double P_t, SpectrumValue sinr)
{
  SpectrumValue sinr_db = 20 * Log10 (sinr); // - m_mcs.gap2Capacity_dB;
  //
  //  there is no force mask for modulation
  //

  //
  // initialize
  //
  typedef std::pair<double, uint16_t> carrier_ber_t;
  BitAllocationTable bat (sinr.GetSpectrumModel ()->GetNumBands (), NOMOD);
  std::priority_queue<carrier_ber_t> ber_set;
  double P_bar_nom = 0;
  double P_bar_denom = 0;
  for (uint16_t i = 0; i < sinr.GetSpectrumModel ()->GetNumBands (); i++)
    {
      ModulationType m = QAM4096;
      bat[i] = m;
      double b = m;
      double ber = (b - GetCapPerChannel (sinr_db[i], m)) / b;
      P_bar_nom += ber * b; // sum(P[i]*b[i])
      P_bar_denom += b; // sum(b[i])

      carrier_ber_t carrier_ber = std::make_pair (ber, i);
      ber_set.push (carrier_ber);
    }
  //
  // Incremental algorithm [1]
  //
  while (P_bar_nom / P_bar_denom > P_t && !ber_set.empty ())
    { // test if sum(P[i]*b[i])/sum(b[i]) > P_t)
      carrier_ber_t carrier_ber = ber_set.top (); // get the bitloading of the worst carrier
      double ber = carrier_ber.first;
      uint16_t i = carrier_ber.second;
      double new_ber = 0;
      ber_set.pop (); // remove it from the set
      ModulationType m = bat[i];
      double b = m;
      //            std::cout << "Top carrier: " << i << ", BER: " << ber << ", modulation: " << m << std::endl;
      P_bar_nom -= b * ber; // substract the removed SER from the sum
      P_bar_denom -= b;
      if (m != BPSK)
        {
          m = (ModulationType) ((uint16_t) m - 1); // decrease its bitloading and calculate its new BER
          b = m;
          new_ber = (b - GetCapPerChannel (sinr_db[i], m)) / b;
          carrier_ber_t carrier_ber (new_ber, i);
          ber_set.push (carrier_ber); // add the bitloading back to the set
          P_bar_nom += new_ber * b; // add the new BER to the sum
          P_bar_denom += b;
          bat[i] = m;
          //                std::cout << "Top carrier: " << i << ", new BER: " << new_ser/b << ", new modulation: " << m << std::endl;
        }
      else
        {
          bat[i] = NOMOD;
        }
    }

  SpectrumValue CapacityPerHertz = GetCapacity (sinr_db, bat);
//  NS_LOG_UNCOND ("(GHN module) BAT: " << bat);
//  std::cout << std::endl;
//  std::cout << std::endl;
//  std::cout << std::endl;
//  NS_LOG_UNCOND ("(GHN module) Capacity per hertz: " << CapacityPerHertz);

  if (1)
    {
      double projected = 0, loaded = 0;
//      std::cout << "Tone Map: ";
      for (uint16_t i = 0; i < bat.size (); i++)
        {
          auto cap = GetCapPerChannel (sinr_db[i], bat.at (i));
//          std::cout << i << "\t" << (uint16_t) bat.at (i) << "\t" << cap << "\n";
          loaded += (uint16_t) bat.at (i);
          projected += cap;
        }
      std::cout << std::endl;
      std::cout << "Projected: " << projected << ", Loaded: " << loaded << ", planned BER: " << (loaded - projected) / loaded
              << std::endl;
    }
  return bat;
}
ser_map_t
NcBlVarBatMap::CalculateSerMap (SpectrumValue sinr)
{
  ser_map_t ser_map;
  ser_map.resize (sinr.GetSpectrumModel ()->GetNumBands ());
  for (uint16_t i = 0; i < sinr.GetSpectrumModel ()->GetNumBands (); i++)
    {
      for (uint16_t mod = BPSK; mod <= QAM4096; mod++)
        {
          ser_map.at (i).push_back (CalcSer (ModulationType (mod), sinr[i]));
        }
    }
  return ser_map;
}
cap_map_t
NcBlVarBatMap::CalculateCapMap (SpectrumValue sinr)
{
  SpectrumValue sinr_db = 20 * Log10 (sinr); // - m_mcs.gap2Capacity_dB;
  cap_map_t cap_map;
  cap_map.resize (sinr.GetSpectrumModel ()->GetNumBands ());
  for (uint16_t i = 0; i < sinr.GetSpectrumModel ()->GetNumBands (); i++)
    {
      for (uint16_t mod = BPSK; mod <= QAM4096; mod++)
        {
          cap_map.at (i).push_back (GetCapPerChannel (sinr_db[i], ModulationType (mod)));
        }
    }
  return cap_map;
}
double
NcBlVarBatMap::GetOfdmSymbolCapacity (ModulationAndCodingScheme mcs, SpectrumValue sinr)
{
  SpectrumValue sinr_db = 20 * Log10 (sinr); // - m_mcs.gap2Capacity_dB;
  double sum_bit = 0;
  auto m_it = mcs.bat.begin ();
  auto sinr_db_it = sinr_db.ValuesBegin ();
  while (m_it != mcs.bat.end ())
    {
      sum_bit += GetCapPerChannel (*sinr_db_it, *m_it);
      m_it++;
      sinr_db_it++;
    }
  return sum_bit;
}
uint32_t
NcBlVarBatMap::CalcBitsPerSymbol (BitAllocationTable bat)
{
  uint32_t loaded = 0;
  for (uint16_t i = 0; i < bat.size (); i++)
    {
      loaded += (uint16_t) bat.at (i);
    }
  return loaded;
}
double
NcBlVarBatMap::CalcSer (ModulationType m, double sinr)
{
  NS_ASSERT(m != NOMOD);
  NS_ASSERT((uint16_t ) m < 16);

  double ser = 0;

  auto M = [](ModulationType m)->uint16_t
    {
      return 1 << (uint16_t) m;
    };

  auto scale = [&](ModulationType m)->double
    {
      return sqrt(3 / ((double)M(m) - 1) / 2);
    };

  switch (m)
    {
  case BPSK:
    {
      double f = std::erfc (sqrt (sinr));
      ser = f / 2;
      break;
    }
  case QAM8:
    {
      double f1 = std::erfc (scale (m) * sqrt (sinr));
      double f2 = std::erfc (scale (m) * 1.29 * sqrt (sinr));
      ser = 3 / 4 * f1 * (1 - f2) + f2 / 2;
      break;
    }
  case QAM4:
  case QAM16:
  case QAM32:
  case QAM64:
  case QAM128:
  case QAM256:
  case QAM512:
  case QAM1024:
  case QAM2048:
  case QAM4096:
    {
      double f = std::erfc (scale (m) * sqrt (sinr));
      double A = 1 - 1 / sqrt (M (m));
      ser = 2 * A * f * (1 - A * f / 2); // calculate symbol error rate (SER) for the carrier
      //                std::cout << "scale(m): " << scale(m) << ", sqrt(sinr): " << sqrt(sinr) << ", f: " << f << ", A: " << A << ", ser: " << ser << std::endl;
      break;
    }
    }
  //        std::cout << "SINR: " << sinr << ", modulation: " << m << ", SER: " << ser << std::endl;
  return ser;
}
}
}
