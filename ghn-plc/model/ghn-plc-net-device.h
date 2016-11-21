/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 TUD
 *
 *  Created on: 21.02.2013
 *      Author: Stanislav Mudriievskyi <stanislav.mudriievskyi@tu-dresden.de>
 */

#ifndef GHN_PLC_NET_DEVICE_H_
#define GHN_PLC_NET_DEVICE_H_

#include "ns3/callback.h"
#include "ns3/plc-net-device.h"
#include "ns3/traced-callback.h"

#include "ghn-plc-dll-management.h"
#include "ghn-plc-dll-apc.h"

namespace ns3
{
namespace ghn {
class GhnPlcDllApc;
class GhnPlcDllManagement;

class GhnPlcNetDevice : public PLC_NetDevice
{
public:
  static TypeId
  GetTypeId (void);
  GhnPlcNetDevice ();
  virtual
  ~GhnPlcNetDevice ();

  void
  SetDllManagement (Ptr<GhnPlcDllManagement> ncDllManagement);
  Ptr<GhnPlcDllManagement>
  GetDllManagement (void) const;

  void
  SetDllApc (Ptr<GhnPlcDllApc> ncDllApc);
  Ptr<GhnPlcDllApc>
  GetDllApc (void);

  /**
   * Set the address of this interface
   * \param address address to set
   */
  virtual void
  SetAddress (Address address);

  /**
   * \return the current Address of this interface.
   */
  virtual Address
  GetAddress (void) const;

  /**
   * \return the broadcast address supported by
   *         this netdevice.
   *
   * Calling this method is invalid if IsBroadcast returns
   * not true.
   */
  virtual Address
  GetBroadcast (void) const;

  /**
   * Is the send side of the network device enabled?
   *
   * \returns True if the send side is enabled, otherwise false.
   */
  bool
  IsSendEnabled (void);

  /**
   * Enable or disable the send side of the network device.
   *
   * \param enable Enable the send side if true, otherwise disable.
   */
  void
  SetSendEnable (bool enable);

  /**
   * Is the receive side of the network device enabled?
   *
   * \returns True if the receiver side is enabled, otherwise false.
   */
  bool
  IsReceiveEnabled (void);

  /**
   * Enable or disable the receive side of the network device.
   *
   * \param enable Enable the receive side if true, otherwise disable.
   */
  void
  SetReceiveEnable (bool enable);

  /**
   * Start sending a packet down the channel.
   * \param packet packet to send
   * \param dest layer 2 destination address
   * \param protocolNumber protocol number
   * \return true if successfull, false otherwise (drop, ...)
   */
  virtual bool
  Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber);

  /**
   * Start sending a packet down the channel, with MAC spoofing
   * \param packet packet to send
   * \param source layer 2 source address
   * \param dest layer 2 destination address
   * \param protocolNumber protocol number
   * \return true if successfull, false otherwise (drop, ...)
   */
  virtual bool
  SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber);

  bool
  Receive (Ptr<Packet> packet, const UanAddress& source, const UanAddress& dest);

  virtual bool
  ConfigComplete (void);

  /**
   * \return true if this interface supports a bridging mode, false otherwise.
   */
  virtual bool
  SupportsSendFrom (void) const;

  Ptr<SpectrumValue>
  GetTxPowerSpectralDensity ()
  {
    return m_txPsd;
  }

  void
  NotifyPhyReceptionFailure ();
  void
  NotifyPhyReceptionSuccess ();

  typedef Callback<void, Ptr<Packet>, const UanAddress& , const UanAddress&> VitualBroadcastCallback;

  void
  SetVitualBroadcastCallback(VitualBroadcastCallback cb)
  {
    m_virtualBroadcast = cb;
  }

protected:
  virtual void
  CompleteConfig (void);

private:
  /**
   * Enable net device to send packets. True by default
   */
  bool m_sendEnable;

  /**
   * Enable net device to receive packets. True by default
   */
  bool m_receiveEnable;

  /**
   * Pointer to the DLL management
   */
  Ptr<GhnPlcDllManagement> m_ncDllManagement;

  /**
   * Pointer to the DLL APC sublayer
   */
  Ptr<GhnPlcDllApc> m_ncDllApc;

  /**
   * avoid DLL stack and channel letting the broadcast messages originated above DLL
   * be delivered to all terminals immediately, which saves the simulation time
   * and guarantees the successful initialization of the layers above
   */
  VitualBroadcastCallback m_virtualBroadcast;

  /**
   * The trace source fired when packets coming into the "top" of the device
   * at the L3/L2 transition are dropped before being queued for transmission.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_ethTxDropTrace;

  /**
   * The trace source fired when packets come into the "top" of the device
   * at the L3/L2 transition, before being queued for transmission.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_ethTxTrace;

};
}
} // namespace ns3

#endif /* GHN_PLC_NET_DEVICE_H_ */
