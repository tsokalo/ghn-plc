/*
 * GhnPlcLpduHeader.h
 *
 *  Created on: Jun 26, 2012
 *      Author: tsokalo
 */

#ifndef GHN_PLC_LPDU_HEADER_H_
#define GHN_PLC_LPDU_HEADER_H_

#include <vector>

#include "ns3/mac48-address.h"
#include "ns3/packet.h"

using namespace std;

namespace ns3
{
namespace ghn {
enum Ghn_Interpacket_field_names
{
  SSN, // Segment sequence number Clause 8.1.3.2.1.1
  LFBO, // LLC frame boundary offset Clause 8.1.3.2.1.2
  VSF, // Valid segment flag Clause 8.1.3.2.1.3
  MQF, // Management queue flag Clause 8.1.3.2.1.4
  OPSF
// Oldest pending segment flag Clause 8.1.3.2.1.5
};
enum OpsfValue
{
  NOT_OLDEST_SEGMENT_PRESENT = 0, OLDEST_SEGMENT_PRESENT = 1
};
enum VsfValue
{
  VALID_VSF_VALUE = 0, INVALID_VSF_VALUE = 1
};

struct GhnPlcInterpacketFeild
{
  unsigned int size;
  unsigned int content;
};

class GhnPlcLpduHeader : public Header
{
public:

  GhnPlcLpduHeader ();
  virtual
  ~GhnPlcLpduHeader ();

  void
  SetSsn (unsigned int ssn);
  unsigned int
  GetSsn ();

  void
  SetLfbo (unsigned int lfbo);
  unsigned int
  GetLfbo ();

  void
  SetMqf (unsigned int mqf);
  unsigned int
  GetMqf ();

  void
  SetVsf (VsfValue vsf);
  VsfValue
  GetVsf ();

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

private:
  vector<GhnPlcInterpacketFeild> lpdu_header_field;
  void
  defaultHeader ();
};
}
}
#endif /* Ghn_LPDU_HEADER_H_ */
