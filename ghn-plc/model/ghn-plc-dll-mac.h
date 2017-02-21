/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 TUD
 *
 *  Created on: 25.08.2015
 *      Author: Stanislav Mudriievskyi <stanislav.mudriievskyi@tu-dresden.de>
 */

#ifndef GHN_PLC_DLL_MAC_H_
#define GHN_PLC_DLL_MAC_H_

#include <iostream>

#include "ns3/object.h"
#include "ns3/uan-address.h"
#include "ns3/backoff.h"
#include "ns3/random-variable-stream.h"
#include "ns3/plc-phy.h"

#include "ghn-plc-header.h"
#include "ghn-plc-defs.h"
#include "ghn-plc-dll-llc.h"
#include "ghn-plc-phy-management.h"
#include "ghn-plc-dll-management.h"
#include "ghn-plc-dll-mac-backoff.h"

namespace ns3
{
namespace ghn {
std::ostream&
operator<< (std::ostream& os, GhnPlcCsmaNodeState state);

typedef Callback<uint64_t> BackoffValueCallback;
typedef Callback<void, ModulationAndCodingScheme> SetMcsCallback;
typedef Callback<void, Ptr<SpectrumValue> > SetTxPsdCallback;
typedef Callback<void, uint64_t> TimeCallback;

class GhnPlcPhyManagement;
class GhnPlcDllManagement;
class GhnPlcDllLlc;

class GhnPlcDllMac : public Object
{
public:
  static TypeId
  GetTypeId (void);
  GhnPlcDllMac ();
  virtual
  ~GhnPlcDllMac ();

  void
  AllowCooperation (bool v = true);
  void
  SetImmediateFeedback (bool v = true);

  void
  SetDllLlc (Ptr<GhnPlcDllLlc> ncDllLlc);
  Ptr<GhnPlcDllLlc>
  GetDllLlc (void);
  void
  SetDllManagement (Ptr<GhnPlcDllManagement> ncDllManagement);
  Ptr<GhnPlcDllManagement>
  GetDllManagement (void);
  void
  SetPhyManagement (Ptr<GhnPlcPhyManagement> ghnPhyManagement);
  Ptr<GhnPlcPhyManagement>
  GetPhyManagement (void);

  /**
   * \param packet packet sent from DLL MAC down to PHY PCS
   * \param source source mac address
   * \param dest mac address of the destination (already resolved)
   *
   *  Called from net device to send packet into DLL APC
   *  with the specified source and destination Addresses.
   *
   * \return whether the Send operation succeeded
   */
  bool
  StartTx (void);
  virtual bool
  DoStartTx (void) = 0;
  void
  NotifyTransmissionEnd (void);
  virtual void
  DoNotifyTransmissionEnd (void) = 0;
  virtual void
  EndTx (void) = 0;
  virtual void
  DoNotifyReceiptionEnd (Time time) = 0;
  virtual void
  TriggerSend () = 0;
  bool
  SendAck (ConnId connId);
  virtual void
  DoCancelEvents (void) = 0;

  bool
  Receive (GhnPlcPhyFrameType frameType, Ptr<Packet> packet, const UanAddress& source, const UanAddress& dest);
  virtual bool
          DoReceive (GhnPlcPhyFrameType frameType, Ptr<Packet> packet, const UanAddress& source, const UanAddress& dest,
                  uint8_t flowId) = 0;

  typedef Callback<bool, GhnPlcPhyFrameType, Ptr<const Packet> > MpduForwardDownCallback;
  typedef Callback<GroupEncAckInfo, GhnBuffer, ConnId> LpduForwardUpCallback;

  void
  SetMpduForwardDownCallback (MpduForwardDownCallback cb);
  void
  SetLpduForwardUpCallback (LpduForwardUpCallback cb);

  void
  SetPriorityQueue (Ptr<Queue> queue);
  Ptr<Queue>
  GetPriorityQueue (void) const;

  void
  SetMacCycleBegin (Time macBegin);


  void SetSetMcsCallback(SetMcsCallback cb){m_setMcsCallback = cb;}
  void SetSetTxPsdCallback(SetTxPsdCallback cb){m_setTxPsdCallback = cb;}
  void CreateLogger ();
  void
  SetResDirectory (std::string resDir)
  {
    m_resDir = resDir;
  }
  void SetTimeCallback(TimeCallback cb)
  {
    m_setTimeCallback = cb;
  }

protected:

  Ptr<Packet>
  AssembleMpdu (GhnBuffer buffer);
  GhnBuffer
  DisassembleMpdu (Ptr<Packet> mpdu);
  virtual void
  ConfigurePhy (SendTuple st) = 0;
  void
  SetState (GhnPlcCsmaNodeState s);
  GhnPlcCsmaNodeState
  GetState ()
  {
    return m_nodeState;
  }

  uint16_t m_blockSize;

  Ptr<GhnPlcDllManagement> m_dllMan;
  Ptr<GhnPlcDllLlc> m_ncDllLlc;
  Ptr<GhnPlcPhyManagement> m_phyMan;

  MpduForwardDownCallback m_forwardDown;
  LpduForwardUpCallback m_forwardUp;

  bool m_txAllowed;

  Time m_macBegin;
  bool m_askedForAck;
  Ptr<Packet> m_transPacket;
  bool m_sentAck;

  GhnPlcCsmaNodeState m_nodeState;

  bool m_allowCooperation;
  bool m_immediateFeedback;

  SetMcsCallback m_setMcsCallback;
  SetTxPsdCallback m_setTxPsdCallback;

  std::vector<Ptr<FileAggregator> > m_aggr;
  //
  // <double time in us> <destination ID> <source ID> <flow ID> <bytes in MPDU>
  //
  TracedCallback<double, double, double, double, double> m_mpduBytes;

  std::string m_resDir;

  TimeCallback m_setTimeCallback;
};
/*
 * CSMA with Collision Avoidance
 */
class GhnPlcDllMacCsma : public GhnPlcDllMac
{
public:
  static TypeId
  GetTypeId (void);
  GhnPlcDllMacCsma ();
  virtual
  ~GhnPlcDllMacCsma ();

  typedef Callback<void> GhnPlcCcaRequestCallback; // This method informs MAC whether the channel is idle or busy
  typedef Callback<void> GhnPlcCcaCancelCallback; // This method cancels previous CCA request

  void
  SetCcaRequestCallback (GhnPlcCcaRequestCallback c);
  void
  SetCcaCancelCallback (GhnPlcCcaCancelCallback c);
  void
  CcaRequest (void);
  void
  CcaConfirm (PLC_PhyCcaResult status);

  virtual void
  DoNotifyTransmissionEnd (void);
  virtual void
  DoNotifyTransmissionFailure (void);
  virtual void
  EndTx (void);
  virtual void
  EndTxFailure (void);
  void
  CheckStart ()
  {
    if (m_nodeState == READY) EndTx ();
  }
  virtual void
  DoNotifyReceiptionEnd (Time time);
  virtual void
  DoCancelEvents (void);

  virtual bool
          DoReceive (GhnPlcPhyFrameType frameType, Ptr<Packet> packet, const UanAddress& source, const UanAddress& dest,
                  uint8_t flowId);
  virtual bool
  DoStartTx (void);
  void
  TriggerSend ();

  void
  StopCsma (void);

  void
  SetBackoffValueCallback (BackoffValueCallback cb)
  {
    m_backoffCallback = cb;
  }

  void
  SetBackoffSlotDuration (Time v)
  {
    m_backoffSlotDuration = v;
  }
  uint64_t
  GetBackoffSlots ();

  void
  SetMinCw (uint16_t v)
  {
    m_minCw = v;
  }
  void
  SetMaxCw (uint16_t v)
  {
    m_maxCw = v;
  }

protected:

  void
  StartBackoff (void);
  void
  ConfigurePhy (SendTuple st);

  GhnPlcCcaRequestCallback m_ccaRequest;
  GhnPlcCcaCancelCallback m_ccaCancel;
  //  Backoff m_backoff;
  Ptr<GhnPlcMacBackoff> m_backoff;
  Time m_lastBackoffTime;
  EventId m_lastBackoffEvent;
  EventId m_lastEndTxEvent;
  /*
   * if sending: ID of current node
   * if receiving: ID of sending node
   */
  uint8_t m_lastTxSourceId;
  /*
   * ID of final destination
   */
  uint8_t m_destinationId;
  uint8_t m_lastTxReplyRequired;
  uint8_t m_lastTxMulticastIndication;
  uint16_t m_mpduSeqNum;

  //tsokalo
  BackoffValueCallback m_backoffCallback;
  Time m_backoffSlotDuration;
  SendTuple m_sendTuple;

  uint16_t m_minCw;
  uint16_t m_maxCw;
  Ptr<UniformRandomVariable> m_uniformVar;

};
/*
 * CSMA with Collision Detection
 */
class GhnPlcDllMacCsmaCd : public GhnPlcDllMacCsma
{
public:

  static TypeId
  GetTypeId (void);

  GhnPlcDllMacCsmaCd ();
  virtual
  ~GhnPlcDllMacCsmaCd ();

  void
  CollisionDetection ();

  uint64_t
  GetBackoffSlots ();


private:



};
}
} // namespace ns3

#endif /* GHN_PLC_DLL_MAC_H_ */
