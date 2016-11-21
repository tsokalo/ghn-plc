/*
 * GhnPlcLlcCodedFlow.h
 *
 *  Created on: Jul 26, 2016
 *      Author: tsokalo
 */

#ifndef GHN_PLC_LLCCODEDFLOW_H_
#define GHN_PLC_LLCCODEDFLOW_H_

#include "ghn-plc-llc-flow.h"

namespace ns3
{
namespace ghn {
class GhnPlcLlcCodedFlow : public GhnPlcLlcFlow
{

public:
  static TypeId
  GetTypeId (void);
  GhnPlcLlcCodedFlow ();
  virtual
  ~GhnPlcLlcCodedFlow ();

};
}
}
#endif /* GHN_PLC_LLCCODEDFLOW_H_ */
