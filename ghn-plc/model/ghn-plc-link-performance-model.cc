/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 TUD
 *
 *  Created on: 19.07.2013
 *      Author: Stanislav Mudriievskyi <stanislav.mudriievskyi@tu-dresden.de>
 */

#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ghn-plc-link-performance-model.h"

NS_LOG_COMPONENT_DEFINE ("GhnPlcLinkPerformanceModel");

namespace ns3 {
namespace ghn {

NS_OBJECT_ENSURE_REGISTERED (GhnPlcLinkPerformanceModel);

GhnPlcLinkPerformanceModel::McsInfo
GhnPlcLinkPerformanceModel::s_mcs_info[13] =
{
                {QAM, 0},       // NOMOD
                {QAM, 2},       // BPSK
                {QAM, 4},       // QAM4
                {QAM, 8},       // QAM8
                {QAM, 16},      // QAM16
                {QAM, 32},      // QAM32
                {QAM, 64},      // QAM64
                {QAM, 128},     // QAM128
                {QAM, 256},     // QAM256
                {QAM, 512},     // QAM512
                {QAM, 1024},    // QAM1024
                {QAM, 2048},    // QAM2048
                {QAM, 4096},    // QAM4096
};

TypeId
GhnPlcLinkPerformanceModel::GetTypeId (void)
{
        static TypeId tid = TypeId ("ns3::GhnPlcLinkPerformanceModel")
        .SetParent<PLC_LinkPerformanceModel> ()
        .AddConstructor<GhnPlcLinkPerformanceModel> ()
        ;
        return tid;
}

GhnPlcLinkPerformanceModel::GhnPlcLinkPerformanceModel()
{
        NS_LOG_FUNCTION(this);
        m_gathered_information_bits = 0;
        m_required_information_bits = 0;
        m_last_symbol_residual = 0;
        m_symbol_duration = Seconds (0);
        m_guard_interval_duration = Seconds (0);
        m_coding_overhead = 0;
}

void
GhnPlcLinkPerformanceModel::DoStartRx(double requiredInformationBits)
{
        NS_LOG_FUNCTION(this);

        m_required_information_bits = requiredInformationBits;
        m_gathered_information_bits = 0;
        m_lastChangeTime = Now ();
}

double
GhnPlcLinkPerformanceModel::CalculateChunkGuardIntervals (Time chunk_duration)
{
        NS_LOG_FUNCTION(this);
        NS_ASSERT (m_symbol_duration > m_guard_interval_duration);

        int64_t symbol_residual = chunk_duration.GetInteger() % m_symbol_duration.GetInteger();

        NS_LOG_LOGIC ("Chunk duration: " << chunk_duration  << " (" << chunk_duration.GetInteger() << ")");
        NS_LOG_LOGIC ("Symbol duration: " << m_symbol_duration << " (" << m_symbol_duration.GetInteger() << ")");
        NS_LOG_LOGIC ("Guard interval duration: " << m_guard_interval_duration << " (" << m_guard_interval_duration.GetInteger() << ")");
        NS_LOG_LOGIC ("Last symbol residual: " << m_last_symbol_residual);
        NS_LOG_LOGIC ("New symbol residual: " << symbol_residual);

        double chunk_guard_intervals = 0;
        if (m_last_symbol_residual < m_guard_interval_duration.GetInteger())
        {
                NS_LOG_LOGIC ("Consider remaining guard interval fraction of first symbol");

                int64_t first_guard_interval_residual = m_guard_interval_duration.GetInteger() - m_last_symbol_residual;
                NS_LOG_LOGIC ("First guard interval residual: " << first_guard_interval_residual);

                if (first_guard_interval_residual > chunk_duration.GetInteger())
                {
                        NS_LOG_LOGIC ("Guard interval of first symbol is not completed with current chunk");
                        chunk_guard_intervals += (double) chunk_duration.GetInteger() / m_guard_interval_duration.GetInteger();
                }
                else
                {
                        NS_LOG_LOGIC ("Considering guard intervals of completed chunk symbols");
                        chunk_guard_intervals += (double) first_guard_interval_residual / m_guard_interval_duration.GetInteger();
                        // tsokalo
                        if(chunk_duration >= m_symbol_duration)
                          {
                            chunk_guard_intervals += (chunk_duration.GetInteger() / m_symbol_duration.GetInteger() - 1);
                          }
                        // original
//                      chunk_guard_intervals += chunk_duration.GetInteger() / m_symbol_duration.GetInteger();

                        if (symbol_residual >= m_guard_interval_duration.GetInteger())
                        {
                                NS_LOG_LOGIC ("Guard interval of last chunk symbol complete");
                                chunk_guard_intervals += 1;
                        }
                        else
                        {
                                NS_LOG_LOGIC ("Guard interval of last chunk not complete");
                                chunk_guard_intervals += (double) symbol_residual / m_guard_interval_duration.GetInteger();
                        }
                }
        }
        else if (chunk_duration > m_symbol_duration - m_guard_interval_duration)
        {
                NS_LOG_LOGIC ("Considering guard intervals of completed chunk symbols");
                chunk_guard_intervals += chunk_duration.GetInteger() / m_symbol_duration.GetInteger();

                if (symbol_residual >= m_guard_interval_duration.GetInteger())
                {
                        NS_LOG_LOGIC ("Guard interval of last chunk symbol complete");
                        chunk_guard_intervals += 1;
                }
                else
                {
                        NS_LOG_LOGIC ("Guard interval of last chunk not complete");
                        chunk_guard_intervals += (double) symbol_residual / m_guard_interval_duration.GetInteger();
                }

        }
        m_last_symbol_residual = symbol_residual;

        NS_LOG_LOGIC ("chunk_duration_s: " << chunk_duration.GetSeconds());
        NS_LOG_LOGIC ("guard_interval_duration_s: " << m_guard_interval_duration.GetSeconds());
        NS_LOG_LOGIC ("chunk guard intervals: " << chunk_guard_intervals);

        return chunk_guard_intervals;
}

void
GhnPlcLinkPerformanceModel::DoEvaluateChunk(void)
{
        NS_LOG_FUNCTION(this);

        Time chunk_duration = Now () - m_lastChangeTime;
        double effective_duration_s = chunk_duration.GetSeconds();

        if (m_guard_interval_duration > Seconds(0))
        {
                // Determine effective rx duration, i.e. (virtually) remove guard intervals
                double chunk_guard_intervals = CalculateChunkGuardIntervals (chunk_duration);
                NS_LOG_LOGIC ("chunk_guard_intervals: " << chunk_guard_intervals);
                NS_LOG_LOGIC ("guard_inteval_duration: " << m_guard_interval_duration.GetSeconds());

                effective_duration_s -= m_guard_interval_duration.GetSeconds() * chunk_guard_intervals;
        }

        NS_LOG_LOGIC ("Effective duration: " << effective_duration_s);

        Ptr<SpectrumValue> sinr = m_interference->GetSinr();
        NS_LOG_LOGIC ("SINR: " << *sinr);

        SpectrumValue sinr_db = 20*Log10(*sinr) - m_mcs.gap2Capacity_dB;

//        SpectrumValue CapacityPerHertz = GetCapacity (sinr_db, s_mcs_info[m_mcs.mt].mod, s_mcs_info[m_mcs.mt].cardinality);
        //tsokalo
        SpectrumValue CapacityPerHertz = m_mcs.use_bat ? GetCapacity (sinr_db, m_mcs.bat) : GetCapacity (sinr_db, s_mcs_info[m_mcs.mt].mod, s_mcs_info[m_mcs.mt].cardinality);
        NS_LOG_LOGIC ("Capacity per hertz: " << CapacityPerHertz);

        Bands::const_iterator bi = CapacityPerHertz.ConstBandsBegin ();
        Values::const_iterator vi = CapacityPerHertz.ConstValuesBegin ();

        double chunk_bit_information = 0;
        while (bi != CapacityPerHertz.ConstBandsEnd ())
        {
                NS_ASSERT (vi != CapacityPerHertz.ConstValuesEnd ());
                double subchannel_capacity = (bi->fh - bi->fl) * (*vi);
                chunk_bit_information += subchannel_capacity * effective_duration_s / (1+GetCodingOverhead());
                ++bi;
                ++vi;
        }
        NS_ASSERT (vi == CapacityPerHertz.ConstValuesEnd ());

        NS_LOG_LOGIC("Chunk information in bits: " << chunk_bit_information);

        m_gathered_information_bits += chunk_bit_information;

        NS_LOG_LOGIC("Bits required for decoding: " << m_required_information_bits);
        NS_LOG_LOGIC("Collected bits: " << m_gathered_information_bits);

        m_lastChangeTime = Now();
}

bool
GhnPlcLinkPerformanceModel::DoEndRx(void)
{
        NS_LOG_FUNCTION(this);

        m_last_symbol_residual = 0;

        NS_LOG_INFO("Required bits: " << m_required_information_bits);
        NS_LOG_INFO("Gathered bits: " << m_gathered_information_bits);

        if (m_gathered_information_bits >= m_required_information_bits)
        {
                NS_LOG_INFO("PLC_InformationRateModel: reception successful");
                return true;
        }
        else
        {
                NS_LOG_INFO("PLC_InformationRateModel: reception failed");
                return false;
        }
}

double
GhnPlcLinkPerformanceModel::GetTransmissionRateLimit(Ptr<SpectrumValue> rxPsd, ModulationAndCodingScheme mcs)
{
        NS_LOG_FUNCTION (this << rxPsd << mcs);

        m_interference->StartRx(rxPsd);
        Ptr<SpectrumValue> sinr = m_interference->GetSinr();
        SpectrumValue sinr_db = 20*Log10(*sinr);
        // SpectrumValue CapacityPerHertz = GetCapacity (sinr_db, s_mcs_info[mcs.mt].mod, s_mcs_info[mcs.mt].cardinality);
        //tsokalo
        SpectrumValue CapacityPerHertz = mcs.use_bat ? GetCapacity (sinr_db, mcs.bat) : GetCapacity (sinr_db, s_mcs_info[mcs.mt].mod, s_mcs_info[mcs.mt].cardinality);
        NS_LOG_LOGIC ("rxPsd: " << *rxPsd);
        NS_LOG_LOGIC ("SINR (dB): " << *sinr);
        NS_LOG_LOGIC ("Capacity per hertz: " << CapacityPerHertz);
        m_interference->EndRx();

        double max_rate = 0;
        Bands::const_iterator bit = CapacityPerHertz.ConstBandsBegin ();
        Values::iterator vit = CapacityPerHertz.ValuesBegin ();
        while (bit != CapacityPerHertz.ConstBandsEnd())
        {
                double bw = (bit->fh) - (bit->fl);
                max_rate += bw*(*vit);
                ++bit;
                ++vit;
        }


        return max_rate;
}
}
}
