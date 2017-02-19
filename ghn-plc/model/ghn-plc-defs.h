/*
 * nc-defs.h
 *
 *  Created on: May 25, 2016
 *      Author: tsokalo
 */

#ifndef GHN_PLC_DEFS_H_
#define GHN_PLC_DEFS_H_

#include <utility>
#include <string.h>
#include <map>
#include <memory>
#include <stdint.h>
#include <tuple>
#include <functional>

#include <ns3/ghn-plc-header.h>
#include "ns3/uan-address.h"
#include <ns3/object.h>
#include "ns3/nstime.h"

namespace ns3
{
namespace ghn
{
enum TopologyType
{
  LINE_TOPOLOGY_TYPE, INHOME_TOPOLOGY_TYPE, STAR_TOPOLOGY_TYPE
};

/**
 * In case if the type "USING_BODY_LOG_INSERTION_TYPE" is selected, the log data will rewrite the user-specified space in the incoming packets
 */
enum LogInsertionType
{
  ADDING_HEADER_LOG_INSERTION_TYPE, USING_BODY_LOG_INSERTION_TYPE,
};

typedef uint32_t NcSeqNum;

struct LogFormat
{
  NcSeqNum seqNum;
  Time stamp;
};

struct FileLogData
{
  std::string fileName;
  std::string format;
  std::string typeId;
  std::string path;
  std::string probeTraceSource;
};

struct AppLog
{
  uint32_t numLost;
  Time delay;
  uint32_t pktSize;
};

struct ConnId
{
  ConnId ()
  {
    src = 0;
    dst = 0;
    this->flowId = 0;
  }
  ConnId (const UanAddress& source, const UanAddress& dest, int16_t flowId = -1)
  {
    src = source;
    dst = dest;
    this->flowId = flowId;
  }
  ConnId (const ConnId& arg)
  {
    src = arg.src;
    dst = arg.dst;
    flowId = arg.flowId;
  }
  ConnId&
  operator= (ConnId arg)
  {
    src = arg.src;
    dst = arg.dst;
    flowId = arg.flowId;
    return *this;
  }
  bool
  operator < (const ConnId& l) const
  {
    std::cout << *this << " <> " << l << std::endl;

    if (flowId == -1 || l.flowId == -1)
      {
        return !(l.src == src && l.dst == dst);
      }
    else
      {
        return !(l.src == src && l.dst == dst && l.flowId == flowId);
      }
  }
  //  friend bool
  //  operator< (const ConnId& l, const ConnId& r)
  //  {
  //    if (r.flowId == -1 || l.flowId == -1)
  //      {
  //        std::cout << l << " <> " << r << " " << l.src.GetAsInt () * 256 + l.dst.GetAsInt () << " " << r.src.GetAsInt () * 256
  //                + r.dst.GetAsInt () << std::endl;
  //        return (l.src.GetAsInt () * 256 + l.dst.GetAsInt () < r.src.GetAsInt () * 256 + r.dst.GetAsInt ());
  //      }
  //    else
  //      {
  //        return (l.flowId * 256 * 256 + l.src.GetAsInt () * 256 + l.dst.GetAsInt () < r.flowId * 256 * 256 + r.src.GetAsInt ()
  //                * 256 + r.dst.GetAsInt ());
  //      }
  //  }
  //  friend bool
  //  operator< (const ConnId& l, const ConnId& r)
  //  {
  //    std::cout << l.first << " <> " << r << std::endl;
  //
  //    if (r.flowId == -1 || l.flowId == -1)
  //      {
  //        return (l.src == r.src && l.dst == r.dst);
  //      }
  //    else
  //      {
  //        return (l.src == r.src && l.dst == r.dst && l.flowId == r.flowId);
  //      }
  //  }

  friend bool
  operator== (const ConnId& l, const ConnId& r)
  {
    return (r.flowId == l.flowId && r.src == l.src);
  }
  friend std::ostream&
  operator<< (std::ostream& os, const ConnId& connId)
  {
    os << "<" << connId.src << "," << connId.dst << "," << connId.flowId << ">";
    return os;
  }
  bool
  IsFlowIdValid ()
  {
    return (flowId != -1);
  }
  void
  MakeInvalid ()
  {
    flowId = -1;
  }
  void
  SwapSrcDst ()
  {
    UanAddress temp = dst;
    dst = src;
    src = temp;
  }

  UanAddress src;
  UanAddress dst;
  int16_t flowId;
};

class SendTuple
{
public:

  SendTuple ()
  {

  }

  SendTuple (GhnBuffer b, ConnId c, UanAddress nextHopNode) :
    connId (c)
  {
    b.swap (this->b);
    this->nextHopNode = nextHopNode;
  }

  SendTuple&
  operator= (SendTuple other)
  {
    if (this != &other)
      {
        other.get_buffer ().swap (b);
        connId = other.connId;
        nextHopNode = other.nextHopNode;
      }
    return *this;
  }

  GhnBuffer
  get_buffer ()
  {
    return b;
  }
  ConnId
  get_conn_id ()
  {
    return connId;
  }
  UanAddress GetNextHopAddress()
  {
    return nextHopNode;
  }
private:

  GhnBuffer b;
  ConnId connId;
  UanAddress nextHopNode;
};
}
}
#endif /* GHN_PLC_DEFS_H_ */
