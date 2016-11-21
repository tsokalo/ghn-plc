/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016 TUD
 *
 *  Created on: 31.05.2016
 *      Author: Ievgenii Tsokalo
 */

#ifndef GHN_PLC_DLL_APC_H_
#define GHN_PLC_DLL_APC_H_

#include "ns3/object.h"

#include "ns3/ghn-plc-dll-llc.h"
#include "ns3/ghn-plc-net-device.h"

namespace ns3 {
namespace ghn {
class GhnPlcDllLlc;
class GhnPlcNetDevice;

class GhnPlcDllApc : public Object
{
public:
  static TypeId GetTypeId (void);
  GhnPlcDllApc ();
  virtual ~GhnPlcDllApc ();
  void SetDllLlc (Ptr<GhnPlcDllLlc> ncDllLlc);
  Ptr<GhnPlcDllLlc> GetDllLlc (void);
  void SetNetDevice (Ptr<GhnPlcNetDevice> ghnNetDevice);
  Ptr<GhnPlcNetDevice> GetNetDevice (void);

  /**
   * \param packet packet sent from DLL APC down to DLL LLC
   * \param source source mac address
   * \param dest mac address of the destination (already resolved)
   *
   *  Called from net device to send packet into DLL APC
   *  with the specified source and destination Addresses.
   *
   * \return whether the Send operation succeeded
   */
  bool SendFrom (Ptr<Packet> packet, const UanAddress& source, const UanAddress& dest);
  bool Receive (Ptr<Packet> packet, const UanAddress& source, const UanAddress& dest);

  typedef Callback<bool, Ptr<Packet>, const UanAddress &, const UanAddress &, int16_t> ApduForwardDownCallback;
  typedef Callback<bool, Ptr<Packet>, const UanAddress &, const UanAddress &> AdpForwardUpCallback;

  void SetApduForwardDownCallback (ApduForwardDownCallback cb);
  void SetAdpForwardUpCallback (AdpForwardUpCallback cb);

protected:
  Ptr<GhnPlcDllLlc> m_dllLlc;
  Ptr<GhnPlcNetDevice> m_ghnNetDevice;

  ApduForwardDownCallback m_forwardDown;
  AdpForwardUpCallback m_forwardUp;
};

} // namespace ns3
}
#endif /* GHN_PLC_DLL_APC_H_ */
