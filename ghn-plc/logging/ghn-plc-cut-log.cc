/*
 * GhnPlcCutLog.cpp
 *
 *  Created on: May 30, 2016
 *      Author: tsokalo
 */

#include <iostream>
#include <string>
#include <assert.h>

#include "ns3/log.h"
#include "ns3/uinteger.h"
#include "ns3/enum.h"
#include "ns3/simulator.h"

#include "ghn-plc-cut-log.h"

namespace ns3
{
namespace ghn
{
NS_LOG_COMPONENT_DEFINE ("GhnPlcCutLog");

NS_OBJECT_ENSURE_REGISTERED ( GhnPlcCutLog);

TypeId
GhnPlcCutLog::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::GhnPlcCutLog") .SetParent<Object> ()

  .SetGroupName ("Stats")

  .AddConstructor<GhnPlcCutLog> ()

  //  .AddTraceSource ("Delay", "Time difference", MakeTraceSourceAccessor (&GhnPlcCutLog::m_delayTrace), "mytracename")
          //
          //  .AddTraceSource ("NcSeqNum", "Sequence number", MakeTraceSourceAccessor (&GhnPlcCutLog::m_seqNumTrace),
          //          "ns3::TracedValue::Uint32Callback")
          //
          //  .AddTraceSource ("PktSize", "Packet size", MakeTraceSourceAccessor (&GhnPlcCutLog::m_pktSizeTrace),
          //          "ns3::TracedValue::Uint32Callback")

          .AddTraceSource ("AppLog", "Application layer log data", MakeTraceSourceAccessor (&GhnPlcCutLog::m_appLogTrace),
                  "ns3::AppLog::TracedCallback");

  return tid;
}

GhnPlcCutLog::GhnPlcCutLog () :
  m_type (USING_BODY_LOG_INSERTION_TYPE), m_delayTrace (Seconds (0.0)), m_pktSizeTrace (0)
{
  m_lastRcvd = Seconds (0);
}

GhnPlcCutLog::~GhnPlcCutLog ()
{

}

void
GhnPlcCutLog::AddLogData (Ptr<Packet> pkt, uint16_t nodeId)
{
  if (!m_aggr) m_seqNum[nodeId]++;
  if (!m_aggr) CreateLogger (nodeId);

  AppHeader appHeader;
  uint32_t pkt_size = pkt->GetSize ();

  appHeader.SetTimestamp (Simulator::Now ());
  appHeader.SetNcSeqNum (m_seqNum[nodeId]);
  appHeader.SetNodeId (nodeId);

  assert (pkt->GetSize () > appHeader.GetSerializedSize ());
  pkt->RemoveAtEnd (appHeader.GetSerializedSize ());
  pkt->AddHeader (appHeader);

  NS_LOG_LOGIC("Added time stamp: " << appHeader.GetTimestamp() << ", sequence number: " << appHeader.GetNcSeqNum()
          << ", node ID: " << appHeader.GetNodeId()
          << ", packet size before: " << pkt_size << ", packet size after: " << pkt->GetSize());

  m_seqNum[nodeId]++;
}
void
GhnPlcCutLog::ReadLogData (Ptr<Packet> pkt, uint16_t nodeId)
{
  if (!m_aggr)
    {
      CreateLogger (nodeId);
    }

  NS_LOG_LOGIC("Receive packet with size " << pkt->GetSize());

  AppHeader appHeader;
  pkt->RemoveHeader (appHeader);
  pkt->AddPaddingAtEnd(appHeader.GetSerializedSize());

  uint16_t sid = appHeader.GetNodeId ();
  Time latency = Simulator::Now () - appHeader.GetTimestamp ();
  NcSeqNum lostPkts = 0;
  if (m_seqNum[sid] + 1 < appHeader.GetNcSeqNum ())
    {
      lostPkts = appHeader.GetNcSeqNum () - (m_seqNum[sid] + 1);
    }
  else if (m_seqNum[sid] + 1 > appHeader.GetNcSeqNum ())
    {
      lostPkts = appHeader.GetNcSeqNum () - 1 + (((uint64_t) 1 << sizeof(NcSeqNum) * 8) - m_seqNum[sid]);
    }
  if (lostPkts > ((uint64_t) 1 << (sizeof(NcSeqNum) - 1) * 8))
    {
      NS_LOG_LOGIC("Received a packet out of order");
      return;
    }

  NS_LOG_LOGIC("Now: " << Simulator::Now () << ", stamp: " << appHeader.GetTimestamp ()
          << ", latency: " << latency << ", seq num: " << m_seqNum[sid] << ", number of lost packets: " << lostPkts
          << ", packet size: " << pkt->GetSize());

  m_seqNum[sid] = appHeader.GetNcSeqNum ();

  Time iat = Simulator::Now () - m_lastRcvd;
  m_lastRcvd = Simulator::Now ();

  m_appLogTrace (sid, m_seqNum[sid], lostPkts, latency.GetMicroSeconds (), iat.GetMicroSeconds (), pkt->GetSize ());
}
void
GhnPlcCutLog::CreateLogger (uint16_t nodeId)
{
  m_aggr = CreateObject<FileAggregator> (m_resDir + "app_data_" + std::to_string (nodeId) + m_logId + ".txt",
          FileAggregator::FORMATTED);
  m_aggr->Set6dFormat ("%.0f\t%.0f\t%.0f\t%.0f\t%.0f\t%.0f");
  m_aggr->Enable ();
  this->TraceConnect ("AppLog", "AppLogContext", MakeCallback (&FileAggregator::Write6d, m_aggr));
}
//----------------------------------------------------------------------
//----------------------------------------------------------------------
//------------------------ AppHeader
//----------------------------------------------------------------------
TypeId
AppHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("AppHeader") .SetParent<Header> () .AddConstructor<AppHeader> ()

  .AddAttribute ("Timestamp", "Some momentous point in time!", EmptyAttributeValue (), MakeTimeAccessor (
          &AppHeader::GetTimestamp), MakeTimeChecker ())

  .AddAttribute ("NcSeqNum", "Sequence number", UintegerValue (0), MakeUintegerAccessor (&AppHeader::GetNcSeqNum),
          MakeUintegerChecker<NcSeqNum> ())

  .AddAttribute ("NodeId", "Node ID", UintegerValue (0), MakeUintegerAccessor (&AppHeader::GetNodeId), MakeUintegerChecker<
          uint16_t> ());
  return tid;
}
TypeId
AppHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
AppHeader::GetSerializedSize (void) const
{
  return 8 + sizeof(NcSeqNum) + sizeof(uint16_t);
}

void
AppHeader::Serialize (Buffer::Iterator i) const
{
  int64_t t = m_timestamp.GetNanoSeconds ();
  i.Write ((const uint8_t *) &t, 8);
  i.Write ((const uint8_t *) &m_seqNum, sizeof(NcSeqNum));
  i.Write ((const uint8_t *) &m_nodeId, sizeof(uint16_t));
}
uint32_t
AppHeader::Deserialize (Buffer::Iterator i)
{
  int64_t t;
  i.Read ((uint8_t *) &t, 8);
  m_timestamp = NanoSeconds (t);
  i.Read ((uint8_t *) &m_seqNum, sizeof(NcSeqNum));
  i.Read ((uint8_t *) &m_nodeId, sizeof(uint16_t));
}

void
AppHeader::SetTimestamp (Time time)
{
  m_timestamp = time;
}
Time
AppHeader::GetTimestamp (void) const
{
  return m_timestamp;
}

void
AppHeader::Print (std::ostream &os) const
{
  os << "t=" << m_timestamp << " s=" << m_seqNum << " n=" << m_nodeId;
}

void
AppHeader::SetNcSeqNum (NcSeqNum seqNum)
{
  m_seqNum = seqNum;
}
NcSeqNum
AppHeader::GetNcSeqNum (void) const
{
  return m_seqNum;
}

void
AppHeader::SetNodeId (uint16_t nodeId)
{
  m_nodeId = nodeId;
}
uint16_t
AppHeader::GetNodeId (void) const
{
  return m_nodeId;
}
}
}
