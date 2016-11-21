/*
 * GhnPlcCutLog.cpp
 *
 *  Created on: May 30, 2016
 *      Author: tsokalo
 */

#include <iostream>
#include <string>

#include "ns3/log.h"
#include "ns3/uinteger.h"
#include "ns3/enum.h"
#include "ns3/simulator.h"

#include "ghn-plc-cut-log.h"

namespace ns3
{
namespace ghn {
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

  LogTag logtag;
  uint32_t pkt_size = pkt->GetSize ();

  logtag.SetTimestamp (Simulator::Now ());
  logtag.SetNcSeqNum (m_seqNum[nodeId]);
  logtag.SetNodeId (nodeId);

  pkt->AddByteTag (logtag);

  NS_LOG_LOGIC("Added time stamp: " << logtag.GetTimestamp() << ", sequence number: " << logtag.GetNcSeqNum()
          << ", node ID: " << logtag.GetNodeId()
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

  LogTag logtag;
  if (pkt->FindFirstMatchingByteTag (logtag))
    {
      uint16_t sid = logtag.GetNodeId ();
      Time latency = Simulator::Now () - logtag.GetTimestamp ();
      NcSeqNum lostPkts = 0;
      if (m_seqNum[sid] + 1 < logtag.GetNcSeqNum ())
        {
          lostPkts = logtag.GetNcSeqNum () - (m_seqNum[sid] + 1);
        }
      else if (m_seqNum[sid] + 1 > logtag.GetNcSeqNum ())
        {
          lostPkts = logtag.GetNcSeqNum () - 1 + (((uint64_t) 1 << sizeof(NcSeqNum) * 8) - m_seqNum[sid]);
        }
      if (lostPkts > ((uint64_t) 1 << (sizeof(NcSeqNum) - 1) * 8))
        {
          NS_LOG_LOGIC("Received a packet out of order");
          return;
        }

      NS_LOG_LOGIC("Now: " << Simulator::Now () << ", stamp: " << logtag.GetTimestamp ()
              << ", latency: " << latency << ", seq num: " << m_seqNum[sid] << ", number of lost packets: " << lostPkts
              << ", packet size: " << pkt->GetSize());

      m_seqNum[sid] = logtag.GetNcSeqNum ();

      Time iat = Simulator::Now() - m_lastRcvd;
      m_lastRcvd = Simulator::Now ();

      m_appLogTrace (sid, m_seqNum[sid], lostPkts, latency.GetMicroSeconds (), iat.GetMicroSeconds(), pkt->GetSize ());
    }
  else
    {
      NS_LOG_UNCOND("No log Tag is found!!!!!!!!!!!!!!!!!!!!!");
    }
}
void
GhnPlcCutLog::CreateLogger (uint16_t nodeId)
{
  m_aggr = CreateObject<FileAggregator> (m_resDir + "app_data_" + std::to_string (nodeId) + m_logId + ".txt", FileAggregator::FORMATTED);
  m_aggr->Set6dFormat ("%.0f\t%.0f\t%.0f\t%.0f\t%.0f\t%.0f");
  m_aggr->Enable ();
  this->TraceConnect ("AppLog", "AppLogContext", MakeCallback (&FileAggregator::Write6d, m_aggr));
}
//----------------------------------------------------------------------
//----------------------------------------------------------------------
//------------------------ LogTag
//----------------------------------------------------------------------
TypeId
LogTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("LogTag") .SetParent<Tag> () .AddConstructor<LogTag> ()

  .AddAttribute ("Timestamp", "Some momentous point in time!", EmptyAttributeValue (),
          MakeTimeAccessor (&LogTag::GetTimestamp), MakeTimeChecker ())

  .AddAttribute ("NcSeqNum", "Sequence number", UintegerValue (0), MakeUintegerAccessor (&LogTag::GetNcSeqNum),
          MakeUintegerChecker<NcSeqNum> ())

  .AddAttribute ("NodeId", "Node ID", UintegerValue (0), MakeUintegerAccessor (&LogTag::GetNodeId), MakeUintegerChecker<
          uint16_t> ());
  return tid;
}
TypeId
LogTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
LogTag::GetSerializedSize (void) const
{
  return 8 + sizeof(NcSeqNum) + sizeof(uint16_t);
}
void
LogTag::Serialize (TagBuffer i) const
{
  int64_t t = m_timestamp.GetNanoSeconds ();
  i.Write ((const uint8_t *) &t, 8);
  i.Write ((const uint8_t *) &m_seqNum, sizeof(NcSeqNum));
  i.Write ((const uint8_t *) &m_nodeId, sizeof(uint16_t));
}
void
LogTag::Deserialize (TagBuffer i)
{
  int64_t t;
  i.Read ((uint8_t *) &t, 8);
  m_timestamp = NanoSeconds (t);
  i.Read ((uint8_t *) &m_seqNum, sizeof(NcSeqNum));
  i.Read ((uint8_t *) &m_nodeId, sizeof(uint16_t));
}

void
LogTag::SetTimestamp (Time time)
{
  m_timestamp = time;
}
Time
LogTag::GetTimestamp (void) const
{
  return m_timestamp;
}

void
LogTag::Print (std::ostream &os) const
{
  os << "t=" << m_timestamp << " s=" << m_seqNum << " n=" << m_nodeId;
}

void
LogTag::SetNcSeqNum (NcSeqNum seqNum)
{
  m_seqNum = seqNum;
}
NcSeqNum
LogTag::GetNcSeqNum (void) const
{
  return m_seqNum;
}

void
LogTag::SetNodeId (uint16_t nodeId)
{
  m_nodeId = nodeId;
}
uint16_t
LogTag::GetNodeId (void) const
{
  return m_nodeId;
}
}
}
