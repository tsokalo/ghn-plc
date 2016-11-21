/*
 * GhnPlcCrcTrailer.h
 *
 *  Created on: Jul 14, 2015
 *      Author: tsokalo
 */

#ifndef GHN_PLC_CRCTRAILER_H_
#define GHN_PLC_CRCTRAILER_H_

#include <ns3/trailer.h>

using namespace std;

namespace ns3
{
namespace ghn {
class GhnPlcCrcTrailer : public Trailer
{
public:
  GhnPlcCrcTrailer ();
  virtual
  ~GhnPlcCrcTrailer ();

  uint32_t
  GetTrailerSize () const;

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
  SetCrc (uint32_t crc);
  uint32_t
  GetCrc (void);
};
}
}
#endif /* GHN_PLC_CRCTRAILER_H_ */
