/*
 * ghn-plc-data-rate-calculator.h
 *
 *  Created on: 14.02.2017
 *      Author: tsokalo
 */

#ifndef SRC_GHN_PLC_MODEL_GHN_PLC_DATA_RATE_CALCULATOR_H_
#define SRC_GHN_PLC_MODEL_GHN_PLC_DATA_RATE_CALCULATOR_H_

#include "ns3/nstime.h"

namespace ns3
{
namespace ghn
{

class DataRateCalculator
{
public:
  DataRateCalculator ()
  {
    m_last = Simulator::Now ();
    m_bits = 0;
  }
  ~DataRateCalculator ()
  {

  }
  double
  Get ()
  {
    Time duration = Simulator::Now () - m_last;

    return (duration == Seconds (0.0) ? 0 : (m_bits / duration.GetSeconds ()));
  }
  void
  Update (uint32_t bits)
  {
    m_bits = bits;
    m_last = Simulator::Now ();
  }
private:

  Time m_last;
  uint32_t m_bits;
};

}
}

#endif /* SRC_GHN_PLC_MODEL_GHN_PLC_DATA_RATE_CALCULATOR_H_ */
