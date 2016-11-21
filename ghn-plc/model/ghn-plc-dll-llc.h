/*
 * GhnPlcDllLlc.h
 *
 *  Created on: Jul 14, 2015
 *      Author: tsokalo
 */

#ifndef GHN_PLC_DLL_LLC_H_
#define GHN_PLC_DLL_LLC_H_

#include <memory>
#include <iostream>

#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/callback.h"
#include "ns3/packet.h"
#include "ns3/traced-callback.h"
#include "ns3/queue.h"
#include "ns3/object-factory.h"
#include "ns3/nstime.h"
#include "ns3/mac48-address.h"
#include "ns3/callback.h"
#include "ns3/random-variable-stream.h"
#include "ns3/stats-module.h"

#include "ns3/ghn-plc-header.h"
#include "ghn-plc-defs.h"
#include "ghn-plc-dll-mac.h"
#include "ghn-plc-dll-apc.h"

namespace ns3
{
namespace ghn {
class GhnPlcDllMac;
class GhnPlcDllApc;

class FlowInterface
{
  typedef Callback<bool, Ptr<Packet> , ConnId, int16_t> SendFromCallback;
  typedef Callback<GroupEncAckInfo, GhnBuffer, ConnId> ReceiveCallback;
  typedef Callback<void, GroupEncAckInfo, ConnId> ReceiveAckCallback;
  typedef Callback<bool> IsQueueEmptyCallback;
  typedef Callback<SendTuple> SendDownCallback;

public:

  FlowInterface (SendFromCallback sendFrom, ReceiveCallback receive, ReceiveAckCallback receiveAck,
          IsQueueEmptyCallback isQueueEmpty, SendDownCallback sendDown)
  {
    m_sendFrom = sendFrom;
    m_receive = receive;
    m_receiveAck = receiveAck;
    m_isQueueEmpty = isQueueEmpty;
    m_sendDown = sendDown;
  }
  ~FlowInterface ()
  {
  }
  FlowInterface&
  operator= (const FlowInterface& other) // copy assignment
  {
    if (this != &other)
      { // self-assignment check expected
        m_sendFrom = other.m_sendFrom;
        m_receive = other.m_receive;
        m_receiveAck = other.m_receiveAck;
        m_isQueueEmpty = other.m_isQueueEmpty;
        m_sendDown = other.m_sendDown;
      }
    return *this;
  }
  bool
  SendFrom (Ptr<Packet> packet, ConnId connId, int16_t ttl)
  {
    return m_sendFrom (packet, connId, ttl);
  }
  GroupEncAckInfo
  Receive (GhnBuffer buffer, ConnId connId)
  {
    return m_receive (buffer, connId);
  }
  void
  ReceiveAck (GroupEncAckInfo info, ConnId connId)
  {
    m_receiveAck (info, connId);
  }
  bool
  IsQueueEmpty ()
  {
    return m_isQueueEmpty ();
  }
  SendTuple
  SendDown ()
  {
    return m_sendDown ();
  }

protected:

  SendFromCallback m_sendFrom;
  ReceiveCallback m_receive;
  ReceiveAckCallback m_receiveAck;
  IsQueueEmptyCallback m_isQueueEmpty;
  SendDownCallback m_sendDown;
};

typedef std::pair<ConnId, FlowInterface> Flow;

class FlowMap
{
public:

  FlowMap ()
  {
    m_allowCooperation = false;
  }
  void
  set_allow_cooperation (bool v)
  {
    m_allowCooperation = v;
  }
  void
  add (ConnId connId, FlowInterface f)
  {
    m_flows.push_back (Flow (connId, f));
  }

  bool
  is_in (ConnId connId)
  {
    return (find (connId) != m_flows.size ());
  }

  FlowInterface
  at (ConnId connId)
  {
    auto f = find (connId);
    assert(f != m_flows.size ());
    return m_flows.at (f).second;
  }
  Flow
  at (uint32_t i)
  {
    return m_flows.at (i);
  }

  ConnId
  match (ConnId connId)
  {
    auto f = find (connId);
    assert(f != m_flows.size ());
    return m_flows.at (f).first;
  }

  uint32_t
  size ()
  {
    return m_flows.size ();
  }

private:

  uint32_t
  find (ConnId connId)
  {
    for (auto i = 0; i < m_flows.size (); i++)
      {
        auto l = m_flows.at (i).first;

        if (!m_allowCooperation)
          {
            if (connId.flowId == -1 || l.flowId == -1)
              {
                if (connId.src == l.src && connId.dst == l.dst) return i;
              }
            else
              {
                if (connId.src == l.src && connId.dst == l.dst && connId.flowId == l.flowId) return i;
              }
          }
        else
          {
            if (connId.flowId == -1 || l.flowId == -1)
              {
                if (connId.dst == l.dst) return i;
              }
            else
              {
                if (connId.dst == l.dst && connId.flowId == l.flowId) return i;
              }
          }
      }
    return m_flows.size ();
  }

  std::vector<Flow> m_flows;

  bool m_allowCooperation;

};

class GhnPlcDllLlc : public Object
{

public:
  GhnPlcDllLlc ();
  virtual
  ~GhnPlcDllLlc ();

  static TypeId
  GetTypeId (void);

  void
  AllowCooperation (bool v = true);

  void
  SetDllMac (Ptr<GhnPlcDllMacCsma> dllMac);
  Ptr<GhnPlcDllMacCsma>
  GetDllMac (void);
  void
  SetDllApc (Ptr<GhnPlcDllApc> dllApc);
  Ptr<GhnPlcDllApc>
  GetDllApc (void);

  bool
  SendFrom (Ptr<Packet> packet, const UanAddress& source, const UanAddress& dest, int16_t ttl = -1);
  GroupEncAckInfo
  Receive (GhnBuffer buffer, ConnId connId);
  void
  ReceiveAck (GroupEncAckInfo info, ConnId connId);
  //
  // SendDown will be called with the callback function on MAC
  //
  SendTuple
  SendDown ();

  bool
  IsQueueEmpty ();

  typedef Callback<bool, Ptr<Packet> , const UanAddress &, const UanAddress &> ApduForwardUpCallback;
  typedef Callback<void> TriggerSendCallback;
  typedef Callback<FlowInterface, ConnId, Ptr<GhnPlcDllMacCsma> , Ptr<GhnPlcDllApc> , Ptr<GhnPlcDllLlc> > CreateFlowCallback;

  void
  SetApduForwardUpCallback (ApduForwardUpCallback cb);
  void
  SetTriggerSendCallback (TriggerSendCallback cb);
  void
  SetCreateFlowCallback (CreateFlowCallback cb);

  void
  SetResDirectory (std::string resDir)
  {
    m_resDir = resDir;
  }

protected:

  void
  CreateFlow (ConnId &connId);
  void
  MatchFlow (ConnId &connId);
  void
  CreateLogger ();
  void
  SaveRcvLog (GhnBuffer buffer, ConnId connId);

  FlowMap m_flowMap;
  CreateFlowCallback m_createFlow;

  bool m_allowCooperation;

  Ptr<GhnPlcDllMacCsma> m_dllMac;
  Ptr<GhnPlcDllApc> m_dllApc;

  ApduForwardUpCallback m_forwardUp;
  TriggerSendCallback m_triggerSend;
  uint16_t m_flowIdCounter;

  std::vector<Ptr<FileAggregator> > m_aggr;
  //
  // <destination ID> <received from ID> <my ID> <packet size>
  //
  TracedCallback<double, double, double, double> m_llcNoUseDroppedLogTrace;
  //
  // <MPDU seq num> <received from ID> <my ID> <if drop> <if before next hop> <if I am next hop> <sequence number>
  //
  TracedCallback<double, double, double, double, double, double, double> m_llcRcvLogTrace;
  std::string m_resDir;
};

}
}
#endif /* GHN_PLC_DLL_LLC_H_ */
