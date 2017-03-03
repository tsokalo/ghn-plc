/*
 * PlcMaxRate.cpp
 *
 *  Created on: Oct 6, 2015
 *      Author: tsokalo
 */
#include "plc-max-rate.h"
#include "ns3/log.h"
#include <iostream>
NS_LOG_COMPONENT_DEFINE ("PlcMaxRate");

namespace ns3
{

SpectrumValue
GetSelectiveCapacity (const SpectrumValue& SINR_dB, Ptr<SpectrumValue> bitPerCarrier)
{
  SpectrumValue capacityPerHertz (SINR_dB.GetSpectrumModel ());

  //
  // concerning only QAM
  //
  Values::const_iterator SINR_dB_it = SINR_dB.ConstValuesBegin ();
  Values::iterator cit = capacityPerHertz.ValuesBegin ();
  Values::iterator bitPerCarrier_it = bitPerCarrier->ValuesBegin ();
  uint16_t minBpc = 1, maxBpc = 12;

//  uint16_t carriers = 0;
  while (SINR_dB_it != SINR_dB.ConstValuesEnd () && cit != capacityPerHertz.ValuesEnd ())
    {
      // subchannel SINR_dB
      double sc_SINR_dB = *SINR_dB_it;

      double maxCap = 0, currCap = 0;
      uint16_t usedBpc = minBpc;

//      std::cout << "Carrier " << carriers++ << ": ";

      for (uint16_t currBpc = minBpc; currBpc <= maxBpc; currBpc++)
        {
          const double *cap = qamCap[currBpc - minBpc];
          if (sc_SINR_dB < -10)
            {
              currCap = log2 (1 + pow (10, sc_SINR_dB / 10));
            }
          else if (sc_SINR_dB >= 40)
            {
              currCap = currBpc;
            }
          else
            {
              short x1 = floor (sc_SINR_dB + 10);
              short x2 = ceil (sc_SINR_dB + 10);

              if (x1 == x2)
                {
                  currCap = cap[x1];
                }
              else
                {
                  double y1 = cap[x1];
                  double y2 = cap[x2];

                  // linear interpolation
                  currCap = y1 + (sc_SINR_dB - x1 + 10) * ((y2 - y1) / (x2 - x1));
                }
            }
//          std::cout << "<" << currBpc << ", " << currCap << ", " << sc_SINR_dB << "> ";
          if (maxCap < currCap)
            {
              maxCap = currCap;
              usedBpc = currBpc;
            }
        }
//      std::cout << std::endl << std::endl << "max <" << usedBpc << ", " << maxCap << "> " << std::endl << std::endl;
      *cit = maxCap;
      *bitPerCarrier_it = usedBpc;
      ++SINR_dB_it;
      ++cit;
    }

  return capacityPerHertz;
}

NS_OBJECT_ENSURE_REGISTERED (PlcMaxRate);

TypeId
PlcMaxRate::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PlcMaxRate") .SetParent<PLC_InformationRateModel> () .AddConstructor<PlcMaxRate> ();
  return tid;
}

PlcMaxRate::PlcMaxRate ()
{
  m_transRate = 0;
}
void
PlcMaxRate::Evaluate (Ptr<SpectrumValue> rxPsd)
{
  NS_LOG_FUNCTION (this << rxPsd);
  m_transRate = 0;
  //m_bitLoadingTable->Cleanup();

  m_interference->StartRx (rxPsd);
  Ptr<SpectrumValue> sinr = m_interference->GetSinr ();
  SpectrumValue sinr_db = 20 * Log10 (*sinr);
  m_bitLoadingTable = Create<SpectrumValue> (rxPsd->GetSpectrumModel ());
  SpectrumValue CapacityPerHertz = GetSelectiveCapacity (sinr_db, m_bitLoadingTable);
  NS_LOG_LOGIC ("rxPsd: " << *rxPsd);NS_LOG_LOGIC ("SINR (dB): " << *sinr);NS_LOG_LOGIC ("Capacity per hertz: " << CapacityPerHertz);
  m_interference->EndRx ();

  Bands::const_iterator bit = CapacityPerHertz.ConstBandsBegin ();
  Values::iterator vit = CapacityPerHertz.ValuesBegin ();
  while (bit != CapacityPerHertz.ConstBandsEnd ())
    {
      double bw = (bit->fh) - (bit->fl);
      m_transRate += bw * (*vit);
      ++bit;
      ++vit;
    }

}
double
PlcMaxRate::GetTransmissionRate ()
{
  return m_transRate;
}
Ptr<SpectrumValue>
PlcMaxRate::GetBitLoadingTable ()
{
  return m_bitLoadingTable;
}

}
