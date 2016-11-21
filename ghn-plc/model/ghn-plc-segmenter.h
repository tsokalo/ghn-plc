/*
 * GhnPlcSegmenter.h
 *
 *  Created on: Jul 14, 2015
 *      Author: tsokalo
 */

#ifndef GHN_PLC_SEGMENTER_H_
#define GHN_PLC_SEGMENTER_H_

#include <ns3/ptr.h>
#include <ns3/packet.h>

#include "ghn-plc-header.h"

namespace ns3
{
namespace ghn {

class GhnPlcSegmenter
{
public:
  GhnPlcSegmenter (uint16_t segSize);
  virtual
  ~GhnPlcSegmenter ();

  SegGhnBuffer
  SegmentData (GhnBuffer buffer);
  GhnBuffer
  DesegmentData (SegGhnBuffer buffer);
  SegGhnBuffer
  GetNotDesegmented (SegGhnBuffer buffer);

  uint64_t GetPaddingAmount();

private:

  uint16_t m_segSize;
  uint64_t m_paddingAmount;

  uint64_t m_partialDesegm;

};
}
}
#endif /* GHN_PLC_SEGMENTER_H_ */
