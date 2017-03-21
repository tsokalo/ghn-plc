/*
 * GhnPlcCutLog.h
 *
 *  Created on: May 30, 2016
 *      Author: tsokalo
 */

#ifndef GHN_PLC_CUTLOG_H_
#define GHN_PLC_CUTLOG_H_

#include <stdint.h>
#include <iostream>
#include <memory>
#include <map>

#include "ns3/object.h"
#include "ns3/callback.h"
#include "ns3/ptr.h"
#include "ns3/nstime.h"
#include "ns3/packet.h"
#include "ns3/tag.h"
#include "ns3/tag-buffer.h"
#include "ns3/traced-callback.h"
#include "ns3/address.h"
#include "ns3/stats-module.h"

#include "ns3/ghn-plc-defs.h"
#include "ns3/ghn-plc-logger.h"

#include "ssn.h"

namespace ns3
{
namespace ghn
{

class TimeProbe;

typedef Callback<NcSeqNum, Ptr<Packet> > GetNcSeqNumCallback;
typedef Callback<NcSeqNum, Time> GetTimeStampCallback;
typedef std::shared_ptr<GhnPlcLogger> LoggerPtr;

class GhnPlcCutLog : public Object
{
public:

  static TypeId
  GetTypeId (void);

  GhnPlcCutLog ();
  virtual
  ~GhnPlcCutLog ();

  void
  SetType (LogInsertionType type)
  {
    m_type = type;
  }

  void
  AddLogData (Ptr<Packet> pkt, uint16_t nodeId);
  void
  ReadLogData (Ptr<Packet> pkt, uint16_t nodeId);
  void
  SetExternalNcSeqNumCallback (GetNcSeqNumCallback externalNcSeqNum)
  {
    m_externalNcSeqNum = externalNcSeqNum;
  }
  void
  SetExternalNcSeqNumCallback (GetTimeStampCallback externalTimeStamp)
  {
    m_externalTimeStamp = externalTimeStamp;
  }
  void
  SetResDirectory (std::string resDir)
  {
    m_resDir = resDir;
  }
  void
  SetLogId (std::string logId)
  {
    m_logId = logId;
  }

private:

  void
  CreateLogger (uint16_t nodeId);

  LogInsertionType m_type;

  /**
   * You can provide your own functions that will extract the sequence number and the time stamp
   * It can be useful if e.g. you already write this data anywhere in the packet headers and you either
   * don't have a place in the packet to write it again or want to save the computational power
   */
  GetNcSeqNumCallback m_externalNcSeqNum;
  GetTimeStampCallback m_externalTimeStamp;

  std::map<uint16_t, ncr::symb_ssn_t> m_seqNum;
  LoggerPtr m_logger;

  TracedValue<uint32_t> m_seqNumTrace;
  TracedValue<Time> m_delayTrace;
  TracedValue<uint32_t> m_pktSizeTrace;// unit [bytes]

  TracedCallback<double, double, double, double, double, double> m_appLogTrace;
  Ptr<FileAggregator> m_aggr;
  std::string m_resDir;
  std::string m_logId;
  Time m_lastRcvd;

};

//------------------------------------------------------
class AppHeader : public Header
{
public:

  static TypeId
  GetTypeId (void);
  virtual TypeId
  GetInstanceTypeId (void) const;
  virtual void
  Print (std::ostream &os) const;

  virtual uint32_t
  GetSerializedSize (void) const;
  virtual void
  Serialize (Buffer::Iterator start) const;
  virtual uint32_t
  Deserialize (Buffer::Iterator start);

  // these are our accessors to our tag structure
  void
  SetTimestamp (Time time);
  Time
  GetTimestamp (void) const;
  void
  SetNcSeqNum (NcSeqNum seqNum);
  NcSeqNum
  GetNcSeqNum (void) const;
  void
  SetNodeId (uint16_t nodeId);
  uint16_t
  GetNodeId (void) const;

private:
  Time m_timestamp;
  NcSeqNum m_seqNum;
  uint16_t m_nodeId;

  // end class LogTag
};
}
}
#endif /* GHN_PLC_CUTLOG_H_ */
