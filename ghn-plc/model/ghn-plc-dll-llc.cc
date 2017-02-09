/*
 * GhnPlcDllLlc.cpp
 *
 *  Created on: Jul 14, 2015
 *      Author: tsokalo
 */

#include <ns3/log.h>
#include "ns3/random-variable-stream.h"
#include "ns3/ghn-plc-lpdu-header.h"
#include <ns3/object-factory.h>

#include "ghn-plc-dll-llc.h"
#include "ghn-plc-llc-coded-flow.h"
#include "ghn-plc-lpdu-header.h"

NS_LOG_COMPONENT_DEFINE("GhnPlcDllLlc");

namespace ns3
{
namespace ghn
{
NS_OBJECT_ENSURE_REGISTERED(GhnPlcDllLlc);

GhnPlcDllLlc::GhnPlcDllLlc ()
{
  m_flowIdCounter = 0;
  m_allowCooperation = false;
}

GhnPlcDllLlc::~GhnPlcDllLlc ()
{

}

TypeId
GhnPlcDllLlc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::GhnPlcDllLlc").SetParent<Object> ()

  .AddTraceSource ("LlcNoUseDroppedLog", "Received data by LLC but dropped because I don't belong to the main route",
          MakeTraceSourceAccessor (&GhnPlcDllLlc::m_llcNoUseDroppedLogTrace), "ns3::LlcNoUseDroppedLog::TracedCallback")

  .AddTraceSource ("LlcRcvLog", "Log about all received LPDUs on LLC (dropped/fail CRC/normal)",
          MakeTraceSourceAccessor (&GhnPlcDllLlc::m_llcRcvLogTrace), "ns3::LlcRcvLog::TracedCallback");
  return tid;
}

void
GhnPlcDllLlc::AllowCooperation (bool v)
{
  m_allowCooperation = v;
  m_flowMap.set_allow_cooperation (true);
}

void
GhnPlcDllLlc::SetDllMac (Ptr<GhnPlcDllMacCsma> dllMac)
{
  NS_LOG_FUNCTION(this);
  m_dllMac = dllMac;
}

Ptr<GhnPlcDllMacCsma>
GhnPlcDllLlc::GetDllMac (void)
{
  return m_dllMac;
}

void
GhnPlcDllLlc::SetDllApc (Ptr<GhnPlcDllApc> dllApc)
{
  m_dllApc = dllApc;
}

Ptr<GhnPlcDllApc>
GhnPlcDllLlc::GetDllApc (void)
{
  return m_dllApc;
}

void
GhnPlcDllLlc::SetApduForwardUpCallback (ApduForwardUpCallback cb)
{
  m_forwardUp = cb;
}
void
GhnPlcDllLlc::SetTriggerSendCallback (TriggerSendCallback cb)
{
  m_triggerSend = cb;
}
void
GhnPlcDllLlc::SetCreateFlowCallback (CreateFlowCallback cb)
{
  m_createFlow = cb;
}

bool
GhnPlcDllLlc::SendFrom (Ptr<Packet> packet, const UanAddress& source, const UanAddress& dest, int16_t ttl)
{
  NS_LOG_FUNCTION(this << packet->GetSize());

  auto dll = m_dllMac->GetDllManagement ();

  //
  // in this implementation there can be only one flow between a given source and a destination
  //
  ConnId connId = ConnId (source, dest);

  //
  // treat packets differently depending on their size
  //
  connId.flowId = (m_allowCooperation && packet->GetSize() < 500) ? MANAGMENT_CONN_ID : connId.flowId;

  MatchFlow (connId);

  if (!m_flowMap.is_in (connId)) CreateFlow (connId);
  return m_flowMap.at (connId).SendFrom (packet, connId, ttl);
}
GroupEncAckInfo
GhnPlcDllLlc::Receive (GhnBuffer buffer, ConnId connId)
{
  NS_LOG_FUNCTION(this);

  if (m_aggr.empty ()) CreateLogger ();

  auto dll = m_dllMac->GetDllManagement ();

  if (connId.src == dll->GetAddress ()) return GroupEncAckInfo (false);

  auto foward_packet_to_flow = [&](GhnBuffer buffer, ConnId connId)
    {
      NS_ASSERT (connId.IsFlowIdValid ());
      if (!m_flowMap.is_in (connId)) CreateFlow (connId);
      return m_flowMap.at (connId).Receive (buffer, connId);
    };
  //
  // if packet is broadcast
  //
  if (connId.dst == dll->GetBroadcast ())
    {
      return foward_packet_to_flow (buffer, connId);
    }

  SaveRcvLog (buffer, connId);
  //
  // check if I am next on the route
  //
  if (!m_allowCooperation)
    {
      if (!dll->GetRoutingTable ()->IsNextOnRoute (connId.src, connId.dst, dll->GetAddress ()))
        {
          NS_LOG_DEBUG("Drop the packets");
          for (auto packet : buffer)
            m_llcNoUseDroppedLogTrace (connId.dst.GetAsInt (), connId.src.GetAsInt (), dll->GetAddress ().GetAsInt (),
                    packet->GetSize ());
          return GroupEncAckInfo (false);
        }
    }

  //
  // if packet is destined to us
  //
  if (connId.dst == dll->GetAddress ())
    {
      return foward_packet_to_flow (buffer, connId);
    }
  //
  // if packet is not destined to us; needed for repeating
  //

  else
    {
      return foward_packet_to_flow (buffer, connId);
    }

  NS_ASSERT(0);
}
void
GhnPlcDllLlc::ReceiveAck (GroupEncAckInfo info, ConnId connId)
{
  NS_LOG_FUNCTION(this << connId);

  auto dll = m_dllMac->GetDllManagement ();

  NS_ASSERT(connId.dst != dll->GetBroadcast ());
  NS_ASSERT(connId.IsFlowIdValid ());
  //
  // if packet is destined to us
  //
  if (connId.dst != dll->GetAddress () && !m_allowCooperation)
    {
      NS_LOG_DEBUG("This ACK is not targeted to us");
      return;
    }
  connId.SwapSrcDst ();

  NS_ASSERT_MSG(m_flowMap.is_in (connId), "We do not have a corresponding flow fo this ACK");
  m_flowMap.at (connId).ReceiveAck (info, connId);
}
SendTuple
GhnPlcDllLlc::SendDown ()
{
  NS_LOG_FUNCTION_NOARGS ();

  for (uint32_t i = 0; i < m_flowMap.size (); i++)
    {
      auto f = m_flowMap.at (i).second;
      if (!f.IsQueueEmpty ()) return f.SendDown ();
    }
  NS_ASSERT(0);
}
bool
GhnPlcDllLlc::IsQueueEmpty ()
{
  NS_LOG_FUNCTION_NOARGS ();

  for (uint32_t i = 0; i < m_flowMap.size (); i++)
    {
      auto f = m_flowMap.at (i).second;
      if (!f.IsQueueEmpty ()) return false;
    }

  return true;
}

void
GhnPlcDllLlc::CreateFlow (ConnId &connId)
{
  if (!connId.IsFlowIdValid ()) connId.flowId = ++m_flowIdCounter;

  NS_LOG_DEBUG("Creating a flow " << connId);

  m_flowMap.add (connId, m_createFlow (connId, m_dllMac, m_dllApc, this));
}
void
GhnPlcDllLlc::MatchFlow (ConnId &connId)
{
  if (connId.IsFlowIdValid ()) return;
  if (!m_flowMap.is_in (connId)) return;

  auto c = m_flowMap.match (connId);

  NS_LOG_INFO("Connection " << connId << " matches " << c);

  //
  // In this implementation there can be only one flow between a selected source and destination
  //
  connId.flowId = c.flowId;
}
void
GhnPlcDllLlc::CreateLogger ()
{
  m_aggr.push_back (
          CreateObject<FileAggregator> (
                  m_resDir + "llc_dropped_nouse_" + std::to_string (m_dllMac->GetDllManagement ()->GetAddress ().GetAsInt ())
                          + ".txt", FileAggregator::FORMATTED));
  auto aggr = *(m_aggr.end () - 1);
  aggr->Set4dFormat ("%.0f\t%.0f\t%.0f\t%.0f");
  aggr->Enable ();
  TraceConnect ("LlcNoUseDroppedLog", "LlcNoUseDroppedLogContext", MakeCallback (&FileAggregator::Write4d, aggr));

  m_aggr.push_back (
          CreateObject<FileAggregator> (
                  m_resDir + "llc_rcv_all_" + std::to_string (m_dllMac->GetDllManagement ()->GetAddress ().GetAsInt ())
                          + ".txt", FileAggregator::FORMATTED));
  aggr = *(m_aggr.end () - 1);
  aggr->Set7dFormat ("%.0f\t%.0f\t%.0f\t%.0f\t%.0f\t%.0f\t%.0f");
  aggr->Enable ();
  TraceConnect ("LlcRcvLog", "LlcRcvLogContext", MakeCallback (&FileAggregator::Write7d, aggr));
}
void
GhnPlcDllLlc::SaveRcvLog (GhnBuffer buffer, ConnId connId)
{
  auto dll = m_dllMac->GetDllManagement ();
  UanAddress my = dll->GetAddress ();
  auto rt = dll->GetRoutingTable ();

  double orig_cost = rt->GetRouteCost (connId.src, connId.dst);
  double my_cost = rt->GetRouteCost (my, connId.dst);
  NS_LOG_DEBUG(
          "Src: " << connId.src << "\tDst: " << connId.dst << "\tMy: " << my << "\tOrig. cost: " << orig_cost << "\tMy cost: " << my_cost);
  //  bool behind = (my_cost > orig_cost);
  //
  //  if(behind)return;

  GhnPlcLpduHeader header;
  UanAddress next = rt->GetNextHopAddress (connId.src, connId.dst);
  bool drop = !rt->IsNextOnRoute (connId.src, connId.dst, my);
  double next_cost = rt->GetRouteCost (next, connId.dst);
  bool before_next_hop = (next_cost < my_cost);
  bool i_am_next_hop = (next == my);

  uint16_t mpduSeqNum = m_dllMac->GetPhyManagement ()->GetMcAckSlotsNumber ();

  for (auto packet : buffer)
    {
      if (!m_dllMac->GetPhyManagement ()->IsBlockSuccess ()) continue;
      packet->PeekHeader (header);
      m_llcRcvLogTrace (mpduSeqNum, connId.src.GetAsInt (), my.GetAsInt (), drop, before_next_hop, i_am_next_hop,
              header.GetSsn ());
    };;
}

}
}
