/*
 * GhnPlcSegmenter.cpp
 *
 *  Created on: Jul 14, 2015
 *      Author: tsokalo
 */


#include "ns3/log.h"

#include "ghn-plc-segmenter.h"
#include "ghn-plc-llc-frame-header.h"

NS_LOG_COMPONENT_DEFINE("GhnPlcSegmenter");

namespace ns3
{
namespace ghn {
GhnPlcSegmenter::GhnPlcSegmenter (uint16_t segSize) :
  m_segSize (segSize)
{
  m_paddingAmount = 0;
  m_partialDesegm = NO_PARTIAL_DESEGEMENTED;
}

GhnPlcSegmenter::~GhnPlcSegmenter ()
{

}

SegGhnBuffer
GhnPlcSegmenter::SegmentData (GhnBuffer buffer)
{
  SegGhnBuffer segBuffer;
  if (buffer.empty ()) return segBuffer;

  Ptr<Packet> bigpack = Create<Packet> ();
  GhnPlcLlcFrameHeader header;
  std::vector<uint32_t> pktSizes;
  for (uint32_t bufI = 0; bufI < buffer.size (); bufI++)
    {
      Ptr<Packet> c = buffer.at (bufI)->Copy ();
//      NS_LOG_UNCOND("1st: " << buffer.at (bufI)->GetSize() << " " << bigpack->GetSize()<< " " << buffer.size() << " " << bufI << " " << c->GetUid());
      bigpack->AddAtEnd (c);
//      NS_LOG_UNCOND("1st: end");
      pktSizes.push_back (bigpack->GetSize ());

      uint16_t paddingSize = m_segSize - bigpack->GetSize () % m_segSize;
      paddingSize = (paddingSize == m_segSize) ? 0 : paddingSize;

      NS_LOG_DEBUG("Packet " << bufI << " has size " << buffer.at (bufI)->GetSize() << ", possible padding: " << paddingSize << ", segment size: " << m_segSize);

      if (paddingSize < header.GetSerializedSize () && paddingSize > 0)
        {
          NS_LOG_DEBUG("Insert small zero padding: " << paddingSize << " " << bigpack->GetSize());
//          NS_LOG_UNCOND("2nd: " << paddingSize << " " << bigpack->GetSize());
          bigpack->AddAtEnd (Create<Packet> (paddingSize));
//          NS_LOG_UNCOND("2nd: end");
          pktSizes.push_back (bigpack->GetSize ());
          m_paddingAmount += paddingSize;
        }
      else if (bufI == buffer.size () - 1 && paddingSize != 0)
        {
          NS_LOG_DEBUG("Last packet. Insert padding frame with size (with header): " << paddingSize);
          Ptr<Packet> paddingFrame = Create<Packet> (paddingSize - header.GetSerializedSize ());
          header.SetFrameType (PADDING_FRAME_TYPE);
          header.SetFrameSize (paddingSize - header.GetSerializedSize ());
          header.SetOrigNodeId (0);
          header.SetTsmpI (TSMP_NOT_INCLUDED);
          header.SetTsmp (0);
          header.SetPriority (UNDEF_PRIORITY);
          header.SetBroadcastIndicator (BROADCAST_INDICATION_OFF);
          header.SetTtl (32);
          paddingFrame->AddHeader (header);
//          NS_LOG_UNCOND("3rd: " << paddingFrame->GetSize() << " " << bigpack->GetSize());
          bigpack->AddAtEnd (paddingFrame);
//          NS_LOG_UNCOND("3rd: end");
          pktSizes.push_back (bigpack->GetSize ());
          m_paddingAmount += paddingSize;
        }
      else
        {
          NS_LOG_DEBUG("Packet " << bufI << ", possible padding: " << paddingSize << ", no padding should be added");
        }
    }

  uint32_t bufI = 0;
  while (bigpack->GetSize () != 0)
    {
      GhnSeg seg;
      seg.validSeg = true;
      seg.ssn = 0;

      //      NS_LOG_DEBUG("Size of all packets till " << bufI << " (inclusive): " << pktSizes.at(bufI) << ", size of " << segBuffer.size() << " segments: " << (segBuffer.size() + 1) * m_segSize);
      if (pktSizes.at (bufI) <= (segBuffer.size () + 1) * m_segSize)
        {
          NS_ASSERT(bufI < pktSizes.size());
          NS_ASSERT(pktSizes.at(bufI) >= segBuffer.size() * m_segSize);

          seg.posLlcFrame = pktSizes.at (bufI) - segBuffer.size () * m_segSize;
          seg.posLlcFrame = (seg.posLlcFrame == m_segSize) ? 0 : seg.posLlcFrame;

          while (pktSizes.at (bufI) <= (segBuffer.size () + 1) * m_segSize)
            {
              if (bufI++ == pktSizes.size () - 1)
                {
                  NS_ASSERT (pktSizes.at (bufI - 1) == (segBuffer.size () + 1) * m_segSize);
                  NS_LOG_DEBUG("Perfect size match.");
                  break;
                }
              NS_LOG_DEBUG("Segment " << segBuffer.size () << " contains more then one packet piece");
            }
        }
      else
        {
          seg.posLlcFrame = 0;
        }
      NS_ASSERT(bigpack->GetSize() >= m_segSize);
      seg.pkt = bigpack->CreateFragment (0, m_segSize);
      bigpack->RemoveAtStart (m_segSize);
      segBuffer.push_back (seg);
      NS_LOG_DEBUG("Packet " << bufI << " is included in segment " << segBuffer.size() - 1);
      NS_LOG_DEBUG("Remaining bigpack->GetSize (): " << bigpack->GetSize () << ", seg.posLlcFrame: " << seg.posLlcFrame);
      NS_ASSERT_MSG(bigpack->GetSize() == 0 || bigpack->GetSize() >= m_segSize, "bigpack->GetSize(): " << bigpack->GetSize());
    }
  return segBuffer;
}
GhnBuffer
GhnPlcSegmenter::DesegmentData (SegGhnBuffer segBuffer)
{
  NS_LOG_DEBUG("Size of buffer to be desegmented: " << segBuffer.size());
  GhnBuffer buffer;
  if (segBuffer.empty ()) return buffer;

  m_partialDesegm = 0;

  Ptr<Packet> pkt = Create<Packet> ();
  GhnPlcLlcFrameHeader frameHeader;
  bool previousInvalid = false;
  for (uint32_t bufI = 0; bufI < segBuffer.size (); bufI++)
    {
      //
      // Ignore invalid segments
      //
      if (!segBuffer.at (bufI).validSeg)
        {
          NS_LOG_DEBUG("Have got an invalid segment: " << bufI);
          previousInvalid = true;
          continue;
        }
      NS_ASSERT_MSG(segBuffer.at (bufI).posLlcFrame <= segBuffer.at(bufI).pkt->GetSize(), "Packet size: " << segBuffer.at(bufI).pkt->GetSize());

      //
      // if segment contains the data of only one LLC frame
      // and possibly additionally zero padding
      //
      if (segBuffer.at (bufI).posLlcFrame == 0)
        {
          if (previousInvalid)
            {
              //
              // if previousInvalid we look for the beginning of the next frame
              //
              NS_LOG_DEBUG("We ignore this segment because we still look for next frame start after the last segment failure");
              continue;
            }
          NS_LOG_DEBUG("Add complete segment " << bufI << " with SSN " << segBuffer.at(bufI).ssn << " at end of packet with size " << buffer.size());
          if (pkt->GetSize () == 0)
            {
              m_partialDesegm = bufI;
            }
          pkt->AddAtEnd (segBuffer.at (bufI).pkt->Copy ());
          NS_ASSERT(pkt->GetSize() >= frameHeader.GetSerializedSize());
          pkt->PeekHeader (frameHeader);
          if (pkt->GetSize () == frameHeader.GetFrameSize () + frameHeader.GetSerializedSize ())
            {
              NS_LOG_DEBUG("The packet end coincides with the end of the frame");

              buffer.push_back (pkt->Copy ());
              m_partialDesegm = NO_PARTIAL_DESEGEMENTED;
              NS_LOG_DEBUG("Pushing packet with size " << pkt->GetSize ());
              pkt->RemoveAtEnd (pkt->GetSize ());
              NS_LOG_DEBUG("This fucking single line!!!");
//              NS_LOG_DEBUG("After pushing packet with size " << pkt->GetSize ());
            }

        }
      //
      // if the segment contains the data of at least two LLC frames
      // or it contains padding frame
      // and possibly additionally zero padding
      //
      else
        {
          //
          // process the first part of the segment
          //
          if (!previousInvalid)
            {
              NS_LOG_DEBUG("Add first " << segBuffer.at (bufI).posLlcFrame << " bytes of segment " << bufI << " with SSN " << segBuffer.at(bufI).ssn << " at end of packet " << buffer.size());
              pkt->AddAtEnd (segBuffer.at (bufI).pkt->CreateFragment (0, segBuffer.at (bufI).posLlcFrame));
              buffer.push_back (pkt->Copy ());
              m_partialDesegm = NO_PARTIAL_DESEGEMENTED;

              NS_LOG_DEBUG("Pushing packet with size " << pkt->GetSize ());
              pkt->RemoveAtEnd (pkt->GetSize ());
            }
          else
            {
              //
              // if previousInvalid we look for beginning of the next packet
              //
              pkt->RemoveAtEnd (pkt->GetSize ());
              NS_LOG_DEBUG("We found the next frame start after the last segment failure");
            }
          //
          // process the second part of the segment
          //
          NS_ASSERT_MSG(segBuffer.at (bufI).pkt->GetSize () > segBuffer.at (bufI).posLlcFrame, "segBuffer.at (bufI).posLlcFrame: " << segBuffer.at (bufI).posLlcFrame);
          uint32_t pktRest = segBuffer.at (bufI).pkt->GetSize () - segBuffer.at (bufI).posLlcFrame;
          if (pktRest < frameHeader.GetSerializedSize ())
            {
              NS_LOG_DEBUG("Ignore small zero padding: " << pktRest);
            }
          else
            {
              NS_LOG_DEBUG("Add the rest of bytes " << pktRest << " of segment " << bufI << " with SSN " << segBuffer.at(bufI).ssn << " at end of packet " << buffer.size());
              if (pkt->GetSize () == 0)
                {
                  m_partialDesegm = bufI;
                }
              pkt->AddAtEnd (segBuffer.at (bufI).pkt->CreateFragment (segBuffer.at (bufI).posLlcFrame, pktRest));

              pkt->PeekHeader (frameHeader);
              while (pkt->GetSize () >= frameHeader.GetSerializedSize () + frameHeader.GetFrameSize ())
                {
                  NS_LOG_DEBUG("Current packet payload size: " << pkt->GetSize () - frameHeader.GetSerializedSize () << ", next LLC frame payload size: " << frameHeader.GetFrameSize ());

                  if (frameHeader.GetFrameType () == PADDING_FRAME_TYPE)
                    {
                      NS_LOG_DEBUG("Got padding LLC frame. Ignore the rest of the segment");
                      //
                      // TODO: check the defaulting of m_partialDesegm
                      //
//                      m_partialDesegm = NO_PARTIAL_DESEGEMENTED;
                      pkt->RemoveAtEnd (pkt->GetSize ());
                      break;
                    }

                  NS_LOG_DEBUG("Splitting the current packet");
                  buffer.push_back (pkt->CreateFragment (0, frameHeader.GetFrameSize () + frameHeader.GetSerializedSize ()));
                  NS_LOG_DEBUG("Pushing packet with size " << buffer.at(buffer.size() - 1)->GetSize ());
                  pkt->RemoveAtStart (frameHeader.GetFrameSize () + frameHeader.GetSerializedSize ());

                  if (pkt->GetSize () < frameHeader.GetSerializedSize ())
                    {
                      m_partialDesegm = NO_PARTIAL_DESEGEMENTED;
                      NS_LOG_DEBUG("Ignore small zero padding: " << pkt->GetSize ());
                      pkt->RemoveAtEnd (pkt->GetSize ());
                      break;
                    }

                  pkt->PeekHeader (frameHeader);
                }
            }
          previousInvalid = false;
        }
    }

  NS_LOG_DEBUG("Partially decomposed segment " << m_partialDesegm << " (undefined value: " << NO_PARTIAL_DESEGEMENTED << ")");

  uint16_t headerSize = frameHeader.GetSerializedSize ();
  for (uint32_t bufI = 0; bufI < buffer.size ();)
    {
      if (buffer.at (bufI)->GetSize () < headerSize)
        {
          NS_LOG_DEBUG("Packet segments are lost.." << ", buffer.at (bufI)->GetSize (): " << buffer.at (bufI)->GetSize ());
          buffer.erase (buffer.begin () + bufI, buffer.begin () + bufI + 1);
        }
      buffer.at (bufI)->PeekHeader (frameHeader);
      if (headerSize + frameHeader.GetFrameSize () != buffer.at (bufI)->GetSize ())
        {
          NS_LOG_DEBUG("Packet segments are lost.." << " headerSize + header.GetFrameSize (): " << headerSize + frameHeader.GetFrameSize ()
                  << ", buffer.at (bufI)->GetSize (): " << buffer.at (bufI)->GetSize ());
          buffer.erase (buffer.begin () + bufI, buffer.begin () + bufI + 1);
        }
      else
        {
          NS_LOG_DEBUG("Retrieving packet " << bufI << " with size " << buffer.at(bufI)->GetSize());
          bufI++;
        }
    }
  return buffer;
}
SegGhnBuffer
GhnPlcSegmenter::GetNotDesegmented (SegGhnBuffer buffer)
{
  SegGhnBuffer notUsed;
  if (buffer.empty ()) return notUsed;

  if (m_partialDesegm == NO_PARTIAL_DESEGEMENTED) return notUsed;
  NS_ASSERT(m_partialDesegm < buffer.size());
  for (uint32_t bufI = m_partialDesegm; bufI < buffer.size (); bufI++)
    {
      NS_LOG_DEBUG("Segment with SSN " << buffer.at (bufI).ssn << " may contain not full packet. We save it for the next desegmentation"
              << ", size: " << buffer.at (bufI).pkt->GetSize());
      //      NS_ASSERT(buffer.at (bufI).validSeg);
      if (!buffer.at (bufI).validSeg) continue;
      notUsed.push_back (buffer.at (bufI));
    }

  //  for (uint32_t bufI = 0; bufI < buffer.size (); bufI++)
  //    {
  //      NS_LOG_DEBUG("Segment with SSN " << buffer.at (buffer.size () - 1 - bufI).ssn << " may contain not full packet. We save it for the next desegmentation");
  //      NS_ASSERT(buffer.at (buffer.size () - 1 - bufI).validSeg);
  //      notUsed.push_front (buffer.at (buffer.size () - 1 - bufI));
  //      if (buffer.at (buffer.size () - 1 - bufI).posLlcFrame != 0) break;
  //    }

  if (notUsed.empty ()) return notUsed;
  //
  // in the first not-desegmented we erase all packets (if any) before the last packet part in the segment
  // because this packets are already desegmented
  //
  //  NS_ASSERT((*notUsed.begin ()).posLlcFrame > 0);
  NS_LOG_DEBUG("Erasing already desegmented packets from the segment with SSN " << (*notUsed.begin ()).ssn);
  NS_LOG_DEBUG("Cut off previous packet at start: " << (*notUsed.begin ()).posLlcFrame);
  uint32_t origSize = (*notUsed.begin ()).pkt->GetSize ();
  if ((*notUsed.begin ()).posLlcFrame > 0) (*notUsed.begin ()).pkt->RemoveAtStart ((*notUsed.begin ()).posLlcFrame);
  GhnPlcLlcFrameHeader frameHeader;
  while ((*notUsed.begin ()).pkt->GetSize () >= frameHeader.GetSerializedSize ())
    {
      (*notUsed.begin ()).pkt->PeekHeader (frameHeader);
      NS_LOG_DEBUG("Current remaining packet size: " << (*notUsed.begin ()).pkt->GetSize ()
              << ", next frame size (with header): " << frameHeader.GetSerializedSize () + frameHeader.GetFrameSize ());
      if ((*notUsed.begin ()).pkt->GetSize () > frameHeader.GetSerializedSize () + frameHeader.GetFrameSize ())
        {
          NS_LOG_DEBUG("Removing at start of the segment with SSN " << (*notUsed.begin ()).ssn
                  << " bytes: " << frameHeader.GetSerializedSize () + frameHeader.GetFrameSize ());
          (*notUsed.begin ()).pkt->RemoveAtStart (frameHeader.GetSerializedSize () + frameHeader.GetFrameSize ());
        }
      else
        {
          break;
        }
    }
  NS_LOG_DEBUG("Padding empty bytes " << origSize - (*notUsed.begin ()).pkt->GetSize ());
  (*notUsed.begin ()).posLlcFrame = origSize - (*notUsed.begin ()).pkt->GetSize ();
  Ptr<Packet> zeroPart = Create<Packet> (origSize - (*notUsed.begin ()).pkt->GetSize ());
  zeroPart->AddAtEnd ((*notUsed.begin ()).pkt);
  (*notUsed.begin ()).pkt = zeroPart;

  return notUsed;
}
uint64_t
GhnPlcSegmenter::GetPaddingAmount ()
{
  return m_paddingAmount;
}
}
}
