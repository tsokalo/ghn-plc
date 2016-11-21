/*
 * GhnConvLayer.h
 *
 *  Created on: Jun 26, 2012
 *      Author: tsokalo
 */

#ifndef GHN_PLC_LLC_FRAME_HEADER_H
#define GHN_PLC_LLC_FRAME_HEADER_H

#include <vector>

#include "ns3/mac48-address.h"
#include "ns3/packet.h"

using namespace std;

#define UNDEF_PRIORITY  0

namespace ns3
{
namespace ghn {
enum FrameFieldName
{
  GhnLlcLayerFT, // LLC frame type Clause 8.1.3.1.1.1
  TSMPI, // Time stamp present indication Clause 8.1.3.1.1.2
  CCMPI, // CCMP header present indication Clause 8.1.3.1.1.3
  LPRI, // User priority of the LLC frame Clause 8.1.3.1.1.4
  FLEN, // LLC frame body length in bytes Clause 8.1.3.1.1.5
  //MCSTI, // is specified by G.9961 from 12.2011 but not used
  Reserved, // Reserved by ITU-T (Note)
  OriginatingNode, // DEVICE_ID of the node that created the LLC frame Clause 8.1.3.1.1.6
  //DestinationNode, // is specified by G.9961 from 12.2011 but practically not needed
  BRCTI, // Broadcast indicator Clause 8.1.3.1.1.7
  TTL, // Time to live Clause 8.1.3.1.1.8
  TSMP
//Time stamp. This field is included in the header only when TSMPI is set to one Clause 8.1.3.1.1.9
};

enum LlcFrameType
{
  PADDING_FRAME_TYPE, LCDU_FRAME_TYPE, APDU_FRAME_TYPE, NULL_FRAME_TYPE, NOTSEL_FRAME_TYPE
};
enum TsmpInclusion
{
  TSMP_NOT_INCLUDED = 0, TSMP_INCLUDED = 1
};
enum BroadcastIndicator
{
  BROADCAST_INDICATION_OFF = 0, BROADCAST_INDICATION_ON = 1
};
enum CcmpHeaderPresence
{
  CCMP_HEADER_NOT_PRESENT = 0, CCMP_HEADER_PRESENT = 1
};
struct FrameField
{
  int size;
  uint64_t content;
};

class GhnPlcLlcFrameHeader : public Header
{
public:

  GhnPlcLlcFrameHeader ();
  ~GhnPlcLlcFrameHeader ();

  uint32_t
  GetHeaderSize () const;
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

  void
  Initilize (vector<FrameField> fields);
  vector<FrameField>
  getFields ();

  void
  SetOrigNodeId (uint64_t);
  uint64_t
  GetOrigNodeId ();

  void
  SetPriority (uint64_t);
  uint64_t
  GetPriority ();

  void
  SetBroadcastIndicator (uint8_t);
  uint8_t
  GetBroadcastIndicator ();
  /*
   * total Ghn_LPDUBlock length in bytes, just without header
   */
  void
  SetFrameSize (uint64_t);
  uint64_t
  GetFrameSize ();

  void
  SetFrameType (LlcFrameType llc_frame_type);
  LlcFrameType
  GetFrameType ();

  void
  SetTtl (unsigned int val);
  unsigned int
  GetTtl ();

  void
  SetTsmp (uint32_t val);
  uint32_t
  GetTsmp ();
  uint32_t
  GetMaxTsmp ();

  void
  SetTsmpI (uint8_t);
  uint8_t
  GetTsmpI ();

private:
  vector<FrameField> m_frameFields;

  void
  DefaultHeader ();

};
}
}
#endif /* GHN_PLC_LLC_FRAME_HEADER_H */
