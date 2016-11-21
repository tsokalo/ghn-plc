/*
 * CsmaCdMac.h
 *
 *  Created on: 08.09.2016
 *      Author: tsokalo
 */

#ifndef CSMACDMAC_H_
#define CSMACDMAC_H_

#include "ns3/plc-mac.h"
namespace ns3
{

class CsmaCdMac : public PLC_ArqMac
{
public:

  static TypeId GetTypeId (void);


  CsmaCdMac ();
  virtual
  ~CsmaCdMac ();

  void
  CollisionDetection();

  uint64_t
  GetBackoffSlots();

};
}
#endif /* CSMACDMAC_H_ */
