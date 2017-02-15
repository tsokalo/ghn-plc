/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 TUD
 *
 *  Created on: 25.08.2015
 *      Author: Stanislav Mudriievskyi <stanislav.mudriievskyi@tu-dresden.de>
 */

#ifndef GHN_PLC_DLL_LLC_FLOW_H_
#define GHN_PLC_DLL_LLC_FLOW_H_

#include "ns3/object.h"
#include "ns3/stats-module.h"

#include "ghn-plc-ack-info.h"
#include "ghn-plc-segmenter.h"
#include "ghn-plc-defs.h"
#include "ghn-plc-dll-mac.h"
#include "ghn-plc-dll-apc.h"

namespace ns3
{
namespace ghn {
class GhnPlcDllMac;
class GhnPlcDllApc;

class GhnPlcLlcFlow : public Object
{
  typedef Callback<void,uint32_t> GenCallback;

public:
  static TypeId
  GetTypeId (void);
  GhnPlcLlcFlow ();
  virtual
  ~GhnPlcLlcFlow ();

  void
  SetConnId(ConnId connId);
  bool
  SendFrom (Ptr<Packet> packet, ConnId connId, int16_t ttl = -1);
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

  void
  SetDllMac (Ptr<GhnPlcDllMacCsma> ncDllMac);
  Ptr<GhnPlcDllMacCsma>
  GetDllMac (void);
  void
  SetDllApc (Ptr<GhnPlcDllApc> ncDllApc);
  Ptr<GhnPlcDllApc>
  GetDllApc (void);
  void
  SetDllLlc (Ptr<GhnPlcDllLlc> ncDllLlc);
  Ptr<GhnPlcDllLlc>
  GetDllLlc (void);
  void
  SetResDirectory (std::string resDir)
  {
    m_resDir = resDir;
  }
  void SetGenCallback(GenCallback cb)
  {
    m_genCallback = cb;
  }

protected:

  bool
  Enqueue (Ptr<Packet> packet, ConnId connId);
  std::deque<SegmentState>
  CheckCrc (GhnBuffer &buffer, ConnId connId);
  std::deque<Ssn>
  RemoveLpduHeaders (GhnBuffer &buffer, std::deque<SegmentState> &state, SegGhnBuffer &segmentBuffer);
  void
  UpdateRcvdSegments (SegGhnBuffer segmentBuffer);
  void
  AddNotDesegmented (std::deque<Ssn> &ssns, SegGhnBuffer &segmentBuffer);
  void
  AddJustAcknowledged (std::deque<Ssn> &ssns, SegGhnBuffer &segmentBuffer);
  GhnBuffer
  ConvertSegsToLlcFrames (SegGhnBuffer &segmentBuffer);
  void
  SaveNotFullyDesegmented (SegGhnBuffer &segmentBuffer);
  void
  ProcessDeseqmented (GhnBuffer buffer, ConnId connId);
  bool
  IsNewBcLlcFrame (NcSeqNum current);
  void
  RemoveCrc (Ptr<Packet> packet);

  GroupEncAckInfo
  ReceiveForUs (GhnBuffer buffer, ConnId connId);
  GroupEncAckInfo
  ReceiveBroadcast (GhnBuffer buffer, ConnId connId);
  GroupEncAckInfo
  ReceiveNotForUs (GhnBuffer buffer, ConnId connId);

  Ptr<Packet>
  ConvertApduToLlcFrame (Ptr<Packet> apdu, ConnId connId, int16_t ttl);
  void
  CreateLogger ();

  //
  // only for TX
  //
  GhnBuffer m_frameBuffer;
  SegGhnBuffer m_nonindexedSegs;
  GhnBuffer m_indexedSegs;

  //
  // only for RX
  //
  SegGhnBuffer m_segmentBuffer;

  GhnPlcTxAckInfo *m_txArq;
  GhnPlcRxAckInfo *m_rxArq;

  GhnPlcSegmenter *m_rxSegmenter;
  GhnPlcSegmenter *m_txSegmenter;

  uint16_t m_blockSize;
  VirtSsn m_winConfSize;
  bool m_containPartDesegm;
  NcSeqNum m_llcFrameSeqNum;
  NcSeqNum m_rxBcSeqNum;
  ConnId m_connId;

  Ptr<GhnPlcDllMacCsma> m_dllMac;
  Ptr<GhnPlcDllApc> m_dllApc;
  Ptr<GhnPlcDllLlc> m_dllLlc;

  GenCallback m_genCallback;

  std::vector<Ptr<FileAggregator> > m_aggr;
  //
  // <destination ID> <source ID> <received from ID> <LLC frame index>
  //
  TracedCallback<double, double, double, double> m_llcRcvLogTrace;
  //
  // <destination ID> <source ID> <my ID> <LLC frame index>
  //
  TracedCallback<double, double, double, double> m_llcRelayedLogTrace;
  //
  // <destination ID> <received from ID> <my ID> <LLC frame index>
  //
  TracedCallback<double, double, double, double> m_llcTtlDroppedLogTrace;

  std::string m_resDir;

};

}
} // namespace ns3

#endif /* GHN_PLC_DLL_LLC_FLOW_H_ */
