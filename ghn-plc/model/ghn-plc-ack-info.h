
/*
 * GhnPlcAckInfo.h
 *
 *  Created on: Jun 29, 2015
 *      Author: tsokalo
 */

#ifndef GHN_PLC_ACKINFO_H_
#define GHN_PLC_ACKINFO_H_

#include <stdint.h>
#include <deque>

#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/callback.h"
#include "ns3/traced-callback.h"
#include "ns3/nstime.h"

#include "ghn-plc-header.h"

namespace ns3
{
namespace ghn {
/*
 * By default the rx and tx window sizes are equal to the maximal ack window size
 * conf window realizes tradeoff between latency and throughput. By default rx and tx conf window sizes are equal
 */
class GhnPlcAckInfo
{
public:

  GhnPlcAckInfo (uint8_t lssN, uint16_t blockSize, VirtSsn winConfSize, Time blockLifeTime);
  virtual
  ~GhnPlcAckInfo ();

  /*
   * including the first and not including the second
   */
  std::pair<Ssn, Ssn>
  GetUsedSsnRange ();
  //
  // call GetAckSsns after MarkAckSegs for transmitter and after MarkRcvSegs for receiver
  //
  std::deque<Ssn>
  GetAckSsns ();
  Ssn
  GetWinStart ();
  bool
  IsInRange (Ssn startSsn, Ssn endSsn, Ssn requestSsn);

protected:

  void
  DefaultAll (uint16_t blockSize);
  void
  StartBlockLife (std::deque<Ssn> segsIndex);
  void
  DiscardSegs (std::deque<Ssn> segsIndex);
  Ssn
  RotateSsnFwrd (Ssn toRotate, Ssn howFar);
  Ssn
  RotateSsnBck (Ssn toRotate, Ssn howFar);
  Ssn
  VirtToRealSsn (VirtSsn ssn);
  VirtSsn
  RealToVirtSsn (Ssn ssn);
  bool
  IsInRange (Ssn segindex);

  VirtSsn m_winStart;
  Ssn m_rWinStart;
  VirtSsn m_winSize;
  /*
   * flow dependent
   */
  VirtSsn m_winConfSize;
  std::deque<SegmentState> m_segs;
  Ssn m_twoPowerLssN;
  uint16_t m_blockSize;
  std::deque<Ssn> m_lastContAck;

  /*
   * flow dependent
   */
  Time m_blockLifeTime;
};

class GhnPlcTxAckInfo : public GhnPlcAckInfo
{
public:

  GhnPlcTxAckInfo (uint8_t lssN, uint16_t blockSize, VirtSsn winConfSize, Time blockLifeTime);
  virtual
  ~GhnPlcTxAckInfo ();

  virtual VirtSsn
  GetFreeTxBufferSize ();
  Ssn
  GetNextSsn ();
  virtual void
  MarkSentSegs (std::deque<Ssn> segsIndex);
  std::deque<Ssn>
  GetMarkedForSend ();
  virtual uint16_t
  MarkAckSegs (GroupEncAckInfo ackInfo, AckCompressType compType);

  bool
  IsInRange (Ssn segindex);

  virtual CodingUnit
  GetCodingUnit ()
  {
    CodingUnit codingUnit;
    return codingUnit;
  }
  void Reset();

protected:

  void
  DefaultAll (uint16_t blockSize = 0);
  void
  UpdateWinStart ();
  void
  UpdateNextSsn ();
  void
  UpdateWinConf ();
  void
  UpdateWin ();

  VirtSsn m_nextSsn;
  //
  // meaning of the winConf is changed in comparison to the G.hn standard
  // here it represents the number of segments, which transmitter is permitted to send more
  // and is equal to the number of done or discarded segments in range from nextSsn+1 till winConfSize
  //
  VirtSsn m_winConf;
  VirtSsn m_win;
  VirtSsn m_recieverWinConfSize;
};
class GhnPlcRxAckInfo : public GhnPlcAckInfo
{
public:
  GhnPlcRxAckInfo (uint8_t lssN, uint16_t blockSize, VirtSsn winConfSize, AckCompressType compType, Time blockLifeTime);
  virtual
  ~GhnPlcRxAckInfo ();

  void
  MarkRcvSegs (std::deque<Ssn> segsIndex, std::deque<SegmentState> segState);
  virtual void
  GetAck (GroupEncAckInfo &ackInfo);

  virtual void
  MarkRcvSegs (CodingUnit codingUnit)
  {
  }
  void Reset();

protected:

  void
  DefaultAll (uint16_t blockSize = 0);
  void
  UpdateWinStart (Ssn lastAck);
  void
  CreateGroupAckInfo (Ssn sizeAck);
  void
  CreateBitMapAckInfo (Ssn sizeAck);
  void
  SaveAck (Ssn sizeAck);

  GroupEncAckInfo m_ackInfo;
  AckCompressType m_ackCompressType;
  Ssn m_lastRcvSsn;
  VirtSsn m_transmitterWinConfSize;
};
}
}
#endif /* GHN_PLC_ACKINFO_H_ */

