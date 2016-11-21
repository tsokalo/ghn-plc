/*
 * nc-routing-rules.h
 *
 *  Created on: 02.10.2016
 *      Author: tsokalo
 */

#ifndef GHN_PLC_ROUTINGRULES_H_
#define GHN_PLC_ROUTINGRULES_H_

#include "ns3/object.h"

#include "ghn-plc-stat-filter.h"

namespace ns3
{
namespace ghn {
class NcRoutingRules : public Object
{
public:
  static TypeId
  GetTypeId (void);
  NcRoutingRules ();
  virtual
  ~NcRoutingRules ();

  /*
   * INPUTS
   */


  /*
   * OUTPUS
   */
};
}
}
#endif /* GHN_PLC_ROUTINGRULES_H_ */
