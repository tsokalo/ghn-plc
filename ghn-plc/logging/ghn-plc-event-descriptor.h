/*
 * event-descriptor.h
 *
 *  Created on: Jun 13, 2016
 *      Author: tsokalo
 */

#ifndef GHN_PLC_EVENT_DESCRIPTOR_H_
#define GHN_PLC_EVENT_DESCRIPTOR_H_

#include <stdint.h>
#include <vector>
#include <map>

#include <ns3/ghn-plc-defs.h>

namespace ns3
{
namespace ghn {
//
//typedef uint16_t Sid;
//typedef uint16_t OrigId;
//typedef NcSeqNum LastSn;
//typedef NcSeqNum Sn;
//enum EventOrig
//{
//  DATA_PACKET_EVENT_ORIG, FEEDBACK_EVENT_ORIG, RETRANSMISSION_EVENT_ORIG
//};
//struct LlcLog
//{
//  uint16_t arqBufferSize;
//  // ...
//};
//
//typedef std::map<Sid, std::map<OrigId, std::map<EventOrig, std::map<LastSn, std::map<Sn, LlcLog> > > > > LogMap;
//
//enum MatchType
//{
//  RECEIVE_RECEIVE_MATCH_TYPE, RECEIVE_SEND_MATCH_TYPE,
//};
//
//struct EventDescriptor
//{
//  EventDescriptor ()
//  {
//    valid = false;
//  }
//  EventDescriptor (uint16_t si, uint16_t oi, EventOrig o, NcSeqNum l, NcSeqNum s)
//  {
//    sndId = si;
//    sn = s;
//    origNodeId = oi;
//    lastSn = l;
//    orig = o;
//    valid = true;
//  }
//  EventDescriptor (const EventDescriptor arg)
//  {
//    sndId = arg.sndId;
//    sn = arg.sn;
//    origNodeId = arg.origNodeId;
//    lastSn = arg.lastSn;
//    orig = arg.orig;
//    valid = arg.valid;
//  }
//  EventDescriptor (const EventDescriptor arg, bool v)
//  {
//    sndId = arg.sndId;
//    sn = arg.sn;
//    origNodeId = arg.origNodeId;
//    lastSn = arg.lastSn;
//    orig = arg.orig;
//    valid = v;
//  }
//
//  EventDescriptor&
//  operator= (EventDescriptor arg)
//  {
//    sndId = arg.sndId;
//    sn = arg.sn;
//    origNodeId = arg.origNodeId;
//    lastSn = arg.lastSn;
//    orig = arg.orig;
//    valid = arg.valid;
//    return *this;
//  }
//  friend bool
//  operator>= (const EventDescriptor& l, const EventDescriptor& r)
//  {
//    return (l.sn >= r.sn);
//  }
//
//  uint16_t sndId;
//  NcSeqNum sn;
//  uint16_t origNodeId;
//  NcSeqNum lastSn;
//  EventOrig orig;
//  bool valid;
//};
//
//struct MatchFunctor
//{
//  MatchFunctor (EventOrig o, MatchType m)
//  {
//    orig = o;
//    matchType = m;
//  }
//  bool
//  Match (const EventDescriptor& l, const EventDescriptor& r)
//  {
//    if (l.orig != orig || r.orig != orig) return false;
//
//    switch (matchType)
//      {
//    case RECEIVE_RECEIVE_MATCH_TYPE:
//      {
//        if (l.sndId != r.sndId) return false;
//        if (l.sndId != l.origNodeId) return false;
//        if (r.sndId != r.origNodeId) return false;
//        return true;
//      }
//    case RECEIVE_SEND_MATCH_TYPE:
//      {
//        if (l.sndId != r.sndId) return false;
//        return true;
//      }
//      }
//  }
//
//  EventOrig orig;
//  MatchType matchType;
//};
//
//struct EventMatcher
//{
//  EventMatcher (EventOrig o, MatchType m)
//  {
//    orig = o;
//    matchType = m;
//  }
//  bool
//  ValidateList (LogMap m)
//  {
//    EventDescriptor p;
//    for (auto m1 : m)
//    for(auto m2 : m1)
//    for(auto m3 : m2)
//    for(auto m4 : m3)
//    for(auto m5 : m4)
//      {
//        EventDescriptor c(m1.first, m2.first, m3.first, m4.first, m5.first);
//        if(!p.valid)continue;
//        if(p >= c)return false;
//        p = c;
//      };;
//  }
//
//  EventDescriptor
//  FindMatch (LogMap m, EventDescriptor s)
//  {
//    auto it1 = m.find (s.sndId);
//    if (it1 == m.end ()) return EventDescriptor ();
//    auto it2 = it1->find (s.origNodeId);
//    if (it2 == it1->end ()) return EventDescriptor ();
//    auto it3 = it2->find (s.orig);
//    if (it3 == it2->end ()) return EventDescriptor ();
//    auto it4 = it3->find (s.lastSn);
//    if (it4 == it3->end ()) return EventDescriptor ();
//
//    MatchFunctor f (orig, matchType);
//    EventDescriptor p;
//    for(auto e : l)
//      {
//        if(f.Match(e, s))
//          {
//            if(e >= s)
//              {
//                return EventDescriptor(e, true);
//              }
//            p = e;
//          }
//      };;
//    return EventDescriptor ();
//  }
//
//  EventOrig orig;
//  MatchType matchType;
//};
}
}

#endif /* GHN_PLC_EVENT_DESCRIPTOR_H_ */
