/*
 * GhnPlcAckInfo.cpp
 *
 *  Created on: Jun 29, 2015
 *      Author: tsokalo
 */
#include "ns3/simulator.h"
#include "ns3/log.h"

#include "ghn-plc-utilities.h"
#include "ghn-plc-ack-info.h"

NS_LOG_COMPONENT_DEFINE("GhnPlcAckInfo");

namespace ns3
{
namespace ghn {

GhnPlcAckInfo::GhnPlcAckInfo (uint8_t lssN, uint16_t blockSize, VirtSsn winConfSize, Time blockLifeTime) :
  m_twoPowerLssN (1 << lssN), m_blockSize (blockSize), m_blockLifeTime (blockLifeTime)
{
  DefaultAll (blockSize);
  m_winConfSize = (winConfSize > m_winSize) ? m_winSize : winConfSize;
  NS_ASSERT(m_twoPowerLssN >= m_winConfSize);
}
GhnPlcAckInfo::~GhnPlcAckInfo ()
{
  m_segs.clear ();
  m_lastContAck.clear ();
}
std::pair<Ssn, Ssn>
GhnPlcAckInfo::GetUsedSsnRange ()
{
  std::pair<Ssn, Ssn> range;
  range.first = m_rWinStart;
  range.second = RotateSsnFwrd (m_rWinStart, m_winConfSize);
  return range;
}
std::deque<Ssn>
GhnPlcAckInfo::GetAckSsns ()
{
  return m_lastContAck;
}
Ssn
GhnPlcAckInfo::GetWinStart ()
{
  return m_rWinStart;
}
bool
GhnPlcAckInfo::IsInRange (Ssn startSsn, Ssn endSsn, Ssn requestSsn)
{
  if (endSsn > startSsn)
    {
      return (requestSsn >= startSsn && requestSsn <= endSsn) ? true : false;
    }
  else
    {
      return (requestSsn > endSsn && requestSsn < startSsn) ? false : true;
    }
}
void
GhnPlcAckInfo::DefaultAll (uint16_t blockSize)
{
  if (blockSize != 0) m_winSize = GHN_ACK_MAX_WINDOW_SIZE_DATA (blockSize);
  m_winStart = 0;
  m_rWinStart = 0;
}
void
GhnPlcAckInfo::StartBlockLife (std::deque<Ssn> segsIndex)
{

}
void
GhnPlcAckInfo::DiscardSegs (std::deque<Ssn> segsIndex)
{

}

Ssn
GhnPlcAckInfo::RotateSsnFwrd (Ssn toRotate, Ssn howFar)
{
  return RotateVarFwrd (toRotate, howFar, m_twoPowerLssN);
}
Ssn
GhnPlcAckInfo::RotateSsnBck (Ssn toRotate, Ssn howFar)
{
  return RotateVarBck (toRotate, howFar, m_twoPowerLssN);
}

Ssn
GhnPlcAckInfo::VirtToRealSsn (VirtSsn ssn)
{
  if (ssn >= m_winStart)
    {
      return RotateSsnFwrd (m_rWinStart, ssn - m_winStart);
    }
  else
    {
      return RotateSsnFwrd (m_rWinStart, m_winConfSize + ssn - m_winStart);
    }
}
VirtSsn
GhnPlcAckInfo::RealToVirtSsn (Ssn ssn)
{
  NS_ASSERT(IsInRange(ssn));

  Ssn rCurr = m_rWinStart;
  Ssn curr = m_winStart;
  while (rCurr != ssn)
    {
      rCurr = RotateSsnFwrd (rCurr, 1);
      curr = RotateVarFwrd (curr, 1, m_winConfSize);
    }
  return curr;
}
bool
GhnPlcAckInfo::IsInRange (Ssn segindex)
{
  if (m_rWinStart + m_winConfSize < m_twoPowerLssN)
    {
      if (segindex >= m_rWinStart && segindex < m_rWinStart + m_winConfSize) return true;
    }
  else
    {
      if (RotateSsnFwrd (segindex, m_twoPowerLssN - m_rWinStart) < m_winConfSize) return true;
    }
  return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
GhnPlcTxAckInfo::GhnPlcTxAckInfo (uint8_t lssN, uint16_t blockSize, VirtSsn winConfSize, Time blockLifeTime) :
  GhnPlcAckInfo (lssN, blockSize, winConfSize, blockLifeTime)
{
  GhnPlcTxAckInfo::DefaultAll (blockSize);
  m_recieverWinConfSize = m_winConfSize;// it can be though changed
}
GhnPlcTxAckInfo::~GhnPlcTxAckInfo ()
{

}
VirtSsn
GhnPlcTxAckInfo::GetFreeTxBufferSize ()
{
  VirtSsn freeBuffer = (m_nextSsn > m_winStart) ? (m_winConfSize - m_nextSsn + m_winStart) : (m_winStart - m_nextSsn);
  if (m_nextSsn == m_winStart && (m_segs.at (m_nextSsn) == DONE_SEGMENT_STATE || m_segs.at (m_nextSsn)
          == DISCARDED_SEGMENT_STATE || m_segs.at (m_nextSsn) == NOT_SENT_SEGMENT_STATE)) freeBuffer = m_winConfSize;

  return freeBuffer;
}
Ssn
GhnPlcTxAckInfo::GetNextSsn ()
{
  VirtSsn freeBuffer = GetFreeTxBufferSize ();
  NS_LOG_DEBUG("freeBuffer: " << freeBuffer << ", m_nextSsn: " << m_nextSsn << ", m_winStart: " << m_winStart);
  if (freeBuffer > 0)
    {
      NS_LOG_DEBUG("Give SSN: " << VirtToRealSsn(m_nextSsn));
      Ssn toRet = m_nextSsn;
      UpdateNextSsn ();
      m_segs.at (toRet) = NOT_SENT_SEGMENT_STATE;
      return VirtToRealSsn (toRet);
    }
  else
    {
      NS_LOG_DEBUG("No more packets are permitted to be sent");
      return SSN_UNDEF;
    }
}

void
GhnPlcTxAckInfo::MarkSentSegs (std::deque<Ssn> segsIndex)
{
  VirtSsn bufSize = segsIndex.size ();
  for (VirtSsn i = 0; i < bufSize; i++)
    {
      VirtSsn virtSsn = RealToVirtSsn (segsIndex.at (i));//after start
      NS_ASSERT(IsInRange(segsIndex.at (i)));
      NS_LOG_DEBUG("Win start: " << m_winStart << ", m_rWinStart:" << m_rWinStart << ", real ssn: " << segsIndex.at (i) << ", virtual ssn: " << virtSsn << ": set to WAIT_ACK_SEGMENT_STATE");

      m_segs.at (virtSsn) = WAIT_ACK_SEGMENT_STATE;
    }
  StartBlockLife (segsIndex);
}
std::deque<Ssn>
GhnPlcTxAckInfo::GetMarkedForSend ()
{
  std::deque<Ssn> ssns;
  VirtSsn currSsn = m_winStart;
  VirtSsn lastSsn = m_nextSsn;
  do
    {
      //NS_LOG_DEBUG("currSsn: " << currSsn << ", lastSsn: " << lastSsn << ", m_segs.at (currSsn): " << m_segs.at (currSsn));
      if (m_segs.at (currSsn) == WAIT_ACK_SEGMENT_STATE)
        {
          ssns.push_back (VirtToRealSsn (currSsn));
        }
      currSsn = RotateVarFwrd (currSsn, 1, m_winConfSize);
    }
  while (currSsn != lastSsn);

  NS_LOG_DEBUG("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!Number of SSNs marked for sending: " << ssns.size());
  return ssns;
}

uint16_t
GhnPlcTxAckInfo::MarkAckSegs (GroupEncAckInfo ackInfo, AckCompressType compType)
{
  if (compType != NO_ACK_COMPRESS && compType != GROUP_ACK_COMPRESS) return 0;

  uint16_t numAcks = 0;

  uint16_t detNum = ackInfo.details.size ();

  NS_LOG_DEBUG("Number of ack groups: " << detNum << ", group size: " << ackInfo.groupSize
          << ", real win start on receiver: " << ackInfo.winStart
          << ", real win start on transmitter: " << m_rWinStart);

  m_lastContAck.clear ();
  while (m_rWinStart != ackInfo.winStart)
    {
    NS_LOG_DEBUG("Segment with SSN: " << m_rWinStart << " and virt SSN: " << m_winStart << " belongs to continuously acknowledged");
      if (m_segs.at (m_winStart) != DONE_SEGMENT_STATE) numAcks++;
      m_segs.at (m_winStart) = DONE_SEGMENT_STATE;
      m_lastContAck.push_back (m_rWinStart);
      m_winStart = RotateVarFwrd (m_winStart, 1, m_winConfSize);
      m_rWinStart = RotateSsnFwrd (m_rWinStart, 1);
    }
  VirtSsn virtSsn = m_winStart;

  while (!ackInfo.details.empty ())
    {
      if (*ackInfo.details.begin ())
        {
        NS_LOG_DEBUG("Segment with real SSN: " << VirtToRealSsn(virtSsn) << " and virt SSN: " << virtSsn << " is DONE");
          if (m_segs.at (virtSsn) != DONE_SEGMENT_STATE) numAcks++;
          m_segs.at (virtSsn) = DONE_SEGMENT_STATE;
        }
      else
        {
        NS_LOG_DEBUG("Segment with real SSN: " << VirtToRealSsn(virtSsn) << " and virt SSN: " << virtSsn << " WAITS FOR ACK");
          m_segs.at (virtSsn) = WAIT_ACK_SEGMENT_STATE;
        }
      ackInfo.details.pop_front ();
      virtSsn = RotateVarFwrd (virtSsn, 1, m_winConfSize);

    }

  NS_LOG_DEBUG("Tx: New win start: " << m_winStart << ", new real win start: " << m_rWinStart);

  UpdateWinConf ();

  return numAcks;
}

bool
GhnPlcTxAckInfo::IsInRange (Ssn segindex)
{
  return GhnPlcAckInfo::IsInRange (segindex);
}
void
GhnPlcTxAckInfo::Reset ()
{
   m_lastContAck.clear ();
   while (m_segs.at (m_winStart) != DONE_SEGMENT_STATE)
     {
       NS_LOG_DEBUG("Segment with SSN: " << m_rWinStart << " and virt SSN: " << m_winStart << " belongs to continuously acknowledged");
       m_segs.at (m_winStart) = DONE_SEGMENT_STATE;
//       m_lastContAck.push_back (m_rWinStart);
//       m_winStart = RotateVarFwrd (m_winStart, 1, m_winConfSize);
//       m_rWinStart = RotateSsnFwrd (m_rWinStart, 1);
     }

   NS_LOG_DEBUG("Tx: New win start: " << m_winStart << ", new real win start: " << m_rWinStart);

   UpdateWinConf ();
}
void
GhnPlcTxAckInfo::DefaultAll (uint16_t blockSize)
{
  GhnPlcAckInfo::DefaultAll (blockSize);
  m_nextSsn = 0;
  m_segs.clear ();
  m_segs.resize (m_winConfSize, NOT_SENT_SEGMENT_STATE);
  m_winConf = m_winConfSize;
  NS_LOG_DEBUG("Tx: m_winConfSize: " << m_winConfSize);
}

void
GhnPlcTxAckInfo::UpdateWinStart ()
{
  VirtSsn origWinStart = m_winStart;
  while (m_segs.at (m_winStart) == DONE_SEGMENT_STATE || m_segs.at (m_winStart) == DISCARDED_SEGMENT_STATE)
    {
      m_winStart = RotateVarFwrd (m_winStart, 1, m_winConfSize);
      m_rWinStart = RotateSsnFwrd (m_rWinStart, 1);
      NS_LOG_DEBUG("m_winStart: " << m_winStart << ", m_rWinStart: " << m_rWinStart);
      if (origWinStart == m_winStart) break;
    }

  NS_LOG_DEBUG("Tx: New win start: " << m_winStart << ", new real win start: " << m_rWinStart);
}
void
GhnPlcTxAckInfo::UpdateNextSsn ()
{
  //  do
  //    {
  m_nextSsn = RotateVarFwrd (m_nextSsn, 1, m_winConfSize);
  //      NS_LOG_DEBUG("m_winStart: " << m_winStart << ", m_nextSsn: " << m_nextSsn << ", m_segs.at (m_nextSsn): " << m_segs.at (m_nextSsn));
  //      if (m_segs.at (m_nextSsn) != WAIT_ACK_SEGMENT_STATE)
  //        {
  //          break;
  //        }
  //    }
  //  while (m_nextSsn != m_winStart);
}
void
GhnPlcTxAckInfo::UpdateWinConf ()
{
  m_winConf = 0;

  VirtSsn beforeWinStart = RotateVarBck (m_winStart, 1, m_winConfSize);

  VirtSsn nextSsn = m_nextSsn;

  while (nextSsn != beforeWinStart)
    {
      nextSsn = RotateVarFwrd (nextSsn, 1, m_winConfSize);
      //      NS_LOG_DEBUG("m_winStart: " << m_winStart << ", segIndex: " << segIndex << ", m_segs.at (segIndex): " << m_segs.at (segIndex));
      if (m_segs.at (nextSsn) == DISCARDED_SEGMENT_STATE || m_segs.at (nextSsn) == DONE_SEGMENT_STATE)
        {
          m_winConf++;
        }
    }
}
void
GhnPlcTxAckInfo::UpdateWin ()
{
  m_win = (m_winStart < m_nextSsn) ? (m_nextSsn - m_winStart) : (m_winConfSize + m_nextSsn - m_winStart);

  NS_LOG_DEBUG("New m_win: " << m_win << ", m_winStart: " << m_winStart << ", m_nextSsn: " << m_nextSsn);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
GhnPlcRxAckInfo::GhnPlcRxAckInfo (uint8_t lssN, uint16_t blockSize, VirtSsn winConfSize, AckCompressType compType, Time blockLifeTime) :
  GhnPlcAckInfo (lssN, blockSize, winConfSize, blockLifeTime)
{
  GhnPlcRxAckInfo::DefaultAll (blockSize);
  m_ackInfo.details.clear ();
  m_ackInfo.groupSize = 0;
  m_ackInfo.winStart = 0;
  m_ackCompressType = compType;
  m_transmitterWinConfSize = m_winConfSize;
  NS_LOG_DEBUG("Rx: m_winConfSize: " << m_winConfSize);
}
GhnPlcRxAckInfo::~GhnPlcRxAckInfo ()
{
  m_ackInfo.details.clear ();
}

void
GhnPlcRxAckInfo::MarkRcvSegs (std::deque<Ssn> segsIndex, std::deque<SegmentState> segState)
{
  NS_ASSERT(segsIndex.size() == segState.size());

  VirtSsn bufSize = segState.size ();

  for (Ssn i = 0; i < bufSize; i++)
    {
      if (!IsInRange (segsIndex.at (i)))
        {
          NS_LOG_DEBUG("realSsn: " << segsIndex.at (i) << " is not in range");
          continue;
        }
      VirtSsn virtSsn = RealToVirtSsn (segsIndex.at (i));

      NS_ASSERT(segState.at (i) == DONE_SEGMENT_STATE || segState.at (i) == WAIT_RETRANSMISSION_SEG_STATE);

      if (segState.at (i) == DONE_SEGMENT_STATE)
        {
          NS_LOG_DEBUG("Win start: " << m_winStart << ", real ssn: " << segsIndex.at (i) << ", virtual ssn: " << virtSsn << ": set to DONE_SEGMENT_STATE");
          m_segs.at (virtSsn) = DONE_SEGMENT_STATE;
        }
      else
        {
        NS_LOG_DEBUG("Win start: " << m_winStart << ", real ssn: " << segsIndex.at (i) << ", virtual ssn: " << virtSsn << ": set to WAIT_RETRANSMISSION_SEG_STATE");
          m_segs.at (virtSsn) = WAIT_RETRANSMISSION_SEG_STATE;
        }

    }
  SaveAck (segsIndex.at (segsIndex.size () - 1));
  UpdateWinStart (segsIndex.at (0));

}

void
GhnPlcRxAckInfo::GetAck (GroupEncAckInfo &ackInfo)
{
  ackInfo.groupSize = m_ackInfo.groupSize;
  ackInfo.winStart = m_ackInfo.winStart;
  ackInfo.details.swap (m_ackInfo.details);
  m_ackInfo.details.clear ();
}
void
GhnPlcRxAckInfo::Reset ()
{
  m_winStart = 0;
  m_rWinStart = 0;
  m_segs.clear ();
  m_segs.resize (m_winConfSize, NOT_SENT_SEGMENT_STATE);
}

void
GhnPlcRxAckInfo::DefaultAll (uint16_t blockSize)
{
  GhnPlcAckInfo::DefaultAll (blockSize);
  m_segs.clear ();
  m_segs.resize (m_winConfSize, NOT_SENT_SEGMENT_STATE);
}

void
GhnPlcRxAckInfo::UpdateWinStart (Ssn lastAck)
{
  Ssn lastSsn = RotateSsnFwrd (lastAck, m_transmitterWinConfSize);
  //  lastSsn = RotateSsnFwrd (lastSsn, m_transmitterWinConfSize);

  NS_LOG_DEBUG("lastSsn: " << lastSsn << ", lastAck: " << lastAck);

  while (m_rWinStart != lastSsn)
    {
      NS_LOG_DEBUG("At m_winStart: " << m_winStart << ", m_rWinStart: " << m_rWinStart << ", m_segs.at (m_winStart): " << m_segs.at (m_winStart));
      if (m_segs.at (m_winStart) == WAIT_RETRANSMISSION_SEG_STATE || m_segs.at (m_winStart) == NOT_SENT_SEGMENT_STATE
              || m_rWinStart == lastSsn)
        {
          break;
        }
      m_segs.at (m_winStart) = NOT_SENT_SEGMENT_STATE;
      m_rWinStart = RotateSsnFwrd (m_rWinStart, 1);
      m_winStart = RotateVarFwrd (m_winStart, 1, m_winConfSize);
    }
  NS_LOG_DEBUG("Rx: New win start: " << m_winStart << ", new real win start: " << m_rWinStart);
}

void
GhnPlcRxAckInfo::CreateGroupAckInfo (Ssn sizeAck)
{
  //  NS_LOG_DEBUG("Use group compressed ACK");
  //  m_ackInfo.winStart = VirtToRealSsn (m_winStart);
  //  uint8_t mingroupsize = 2, maxgroupsize = 9, optgroupsize = 2;
  //  uint16_t encLoss = m_winConfSize, corrSeg = 0;
  //
  //  //
  //  // find optimal group size
  //  //
  //  for (uint8_t g = mingroupsize; g <= maxgroupsize; g++)
  //    {
  //      uint16_t tempEncLoss = 0;
  //      for (VirtSsn i = 0; i < sizeAck;)
  //        {
  //          for (VirtSsn j = 0; j < g; j++)
  //            {
  //              if (m_segs.at (RotateVarFwrd (m_winStart, i, m_winConfSize))) corrSeg++;
  //              if (++i == sizeAck) break;
  //            }
  //          //
  //          // if there is at least one error segment
  //          //
  //          if (g != corrSeg) tempEncLoss += corrSeg;
  //          corrSeg = 0;
  //        }
  //      //
  //      // harder criteria is the encLoss
  //      // but if encLoss is equal to tempEncLoss it is better to increase the group size, which decreases the ACK overhead
  //      //
  //      if (tempEncLoss <= encLoss)
  //        {
  //          optgroupsize = g;
  //          encLoss = tempEncLoss;
  //        }
  //    }
  //  m_ackInfo.groupSize = optgroupsize;
  //
  //  //
  //  // write details corresponding to the selected group size
  //  //
  //  for (VirtSsn i = 0; i < sizeAck;)
  //    {
  //      for (VirtSsn j = 0; j < m_ackInfo.groupSize; j++)
  //        {
  //          if (m_segs.at (RotateVarFwrd (m_winStart, i, m_winConfSize)) == DONE_SEGMENT_STATE) corrSeg++;
  //          if (++i == sizeAck) break;
  //        }
  //      //
  //      // if there is at least one error segment
  //      //
  //      if (m_ackInfo.groupSize != corrSeg)
  //        m_ackInfo.details.push_back (false);
  //      else
  //        m_ackInfo.details.push_back (true);
  //      corrSeg = 0;
  //    }
  //
  //  NS_ASSERT(m_ackInfo.details.size() > 0);
  //
  //  NS_ASSERT(m_ackInfo.details.size() == 1 || (m_ackInfo.details.size() > 1 && (!m_ackInfo.details.at(0))));
  //
  //  //
  //  // adapt to g.hn notation
  //  //GetFreeTxBufferSize
  //  m_ackInfo.groupSize -= 2;

}
void
GhnPlcRxAckInfo::CreateBitMapAckInfo (Ssn sizeAck)
{
  NS_LOG_DEBUG("Use uncompressed ACK");

  m_ackInfo.groupSize = 1;

  //
  // find first bad segment
  //
  VirtSsn firstBadSeg = 0, corrIndex = 0;
  m_lastContAck.clear ();
  for (VirtSsn i = 0; i < m_winConfSize; i++)//sizeAck
    {
      firstBadSeg = RotateVarFwrd (m_winStart, i, m_winConfSize);
      NS_LOG_DEBUG ("virtual ssn: " << firstBadSeg << ", corrIndex: " << corrIndex << ", m_segs.at (firstBadSeg): " << m_segs.at (firstBadSeg));
      if (m_segs.at (firstBadSeg) != DONE_SEGMENT_STATE) break;
      m_lastContAck.push_back (VirtToRealSsn (firstBadSeg));
      NS_LOG_DEBUG ("Adding SSN: " << VirtToRealSsn(firstBadSeg) << " to continuously acknowledged SSNs");
      corrIndex = i;
    }
  if (!m_lastContAck.empty ()) corrIndex++;

  m_ackInfo.winStart = RotateSsnFwrd (m_rWinStart, corrIndex);
  //
  // compose the ack
  //
  m_ackInfo.details.clear ();
  for (VirtSsn i = 0; i < m_winConfSize - corrIndex; i++)//sizeAck
    {
      if (m_segs.at (RotateVarFwrd (m_winStart, i + corrIndex, m_winConfSize)) == WAIT_RETRANSMISSION_SEG_STATE)
        {
          NS_LOG_DEBUG("m_ackInfo details at " << i << " is " << 0);
          m_ackInfo.details.push_back (false);
        }
      else if (m_segs.at (RotateVarFwrd (m_winStart, i + corrIndex, m_winConfSize)) == DONE_SEGMENT_STATE)
        {
          NS_LOG_DEBUG("m_ackInfo details at " << i << " is " << 1);
          m_ackInfo.details.push_back (true);
        }
      else
        {
          NS_LOG_DEBUG("Have reached the segment with NOT_SENT_SEGMENT_STATE state");
          break;
        }
    }
  if (m_ackInfo.details.empty ())
    {
      NS_LOG_DEBUG("All received segments are positively acknowledged");
    }
  else
    {
      NS_LOG_DEBUG ("First bad segment: " << firstBadSeg);
      NS_ASSERT(!m_ackInfo.details.at(0)); // the first should be bad!
    }
}
void
GhnPlcRxAckInfo::SaveAck (Ssn sizeAck)
{
  switch (m_ackCompressType)
    {
  case NO_ACK_COMPRESS:
    {
      CreateBitMapAckInfo (sizeAck);
      break;
    }
  case GROUP_ACK_COMPRESS:
    {
      CreateGroupAckInfo (sizeAck);
      break;
    }
  default:
    {
      CreateBitMapAckInfo (sizeAck);
      break;
    }
    }
}
}
}
