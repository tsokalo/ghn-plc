/*
 * PlcMaxRate.h
 *
 *  Created on: Oct 6, 2015
 *      Author: tsokalo
 */

#ifndef PLCMAXRATE_H_
#define PLCMAXRATE_H_

#include "ns3/plc-link-performance-model.h"
#include "ns3/plc-dcmc-capacity.h"
#include <vector>
#include <ns3/ptr.h>

namespace ns3
{

SpectrumValue
GetSelectiveCapacity (const SpectrumValue& SINR_dB, Ptr<SpectrumValue> bitPerCarrier);

class PlcMaxRate : public PLC_InformationRateModel
{
public:

  static TypeId
  GetTypeId (void);

  PlcMaxRate (void);
  void
  Evaluate (Ptr<SpectrumValue> rxPsd);
  double
  GetTransmissionRate ();
  Ptr<SpectrumValue>
  GetBitLoadingTable ();

private:

  double m_transRate;
  Ptr<SpectrumValue> m_bitLoadingTable;

};

}
#endif /* PLCMAXRATE_H_ */
