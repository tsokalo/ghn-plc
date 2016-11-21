/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 TUD
 *
 *  Created on: 19.07.2013
 *      Author: Stanislav Mudriievskyi <stanislav.mudriievskyi@tu-dresden.de>
 */

#ifndef GDOTHN_LINK_PERFORMANCE_MODEL_H_
#define GDOTHN_LINK_PERFORMANCE_MODEL_H_

#include "ns3/plc-link-performance-model.h"

namespace ns3 {
namespace ghn {

class GhnPlcLinkPerformanceModel : public PLC_LinkPerformanceModel
{
public:
  struct McsInfo
  {
    Modulation mod;
    short cardinality;
//    double code_rate;
  };

  static TypeId GetTypeId (void);

  GhnPlcLinkPerformanceModel();

  void SetSymbolDuration (Time duration) { m_symbol_duration = duration; }
  Time GetSymbolDuration (void) { return m_symbol_duration; }
  void SetGuardIntervalDuration (Time duration) { m_guard_interval_duration = duration; }
  Time GetGuardIntervalDuration (void) { return m_symbol_duration; }
  void SetCodingOverhead (double overhead) { m_coding_overhead = overhead; }
  double GetCodingOverhead (void) { return m_coding_overhead; }

  double GetGatheredMutualInformation(void) { return m_gathered_information_bits; }

  double GetTransmissionRateLimit(Ptr<SpectrumValue> rxPsd, ModulationAndCodingScheme mcs);

private:
  static McsInfo s_mcs_info[13];

  void DoStartRx(double requiredInformationBits);
  void DoEvaluateChunk(void);
  bool DoEndRx(void);

  double CalculateChunkGuardIntervals (Time chunk_duration);

  Time m_symbol_duration;
  Time m_guard_interval_duration;
  int64_t m_last_symbol_residual;

  double m_required_information_bits;
  double m_gathered_information_bits;
  double m_coding_overhead;
};

}
}


#endif /* GDOTHN_LINK_PERFORMANCE_MODEL_H_ */
