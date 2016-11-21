/*
 * GhnPlcCrcTrailer.cpp
 *
 *  Created on: Jul 14, 2015
 *      Author: tsokalo
 */
#include <ns3/log.h>

#include "ghn-plc-crc-trailer.h"

NS_LOG_COMPONENT_DEFINE ("GhnPlcCrcTrailer");

namespace ns3
{
namespace ghn {
NS_OBJECT_ENSURE_REGISTERED ( GhnPlcCrcTrailer);
TypeId
GhnPlcCrcTrailer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::GhnPlcCrcTrailer") .SetParent<Trailer> () .AddConstructor<GhnPlcCrcTrailer> ();
  return tid;
}
TypeId
GhnPlcCrcTrailer::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

GhnPlcCrcTrailer::GhnPlcCrcTrailer ()
{

}
GhnPlcCrcTrailer::~GhnPlcCrcTrailer ()
{

}

uint32_t
GhnPlcCrcTrailer::GetTrailerSize () const
{

}


void
GhnPlcCrcTrailer::Print (std::ostream &os) const
{

}

uint32_t
GhnPlcCrcTrailer::GetSerializedSize (void) const
{

}
void
GhnPlcCrcTrailer::Serialize (Buffer::Iterator start) const
{

}
uint32_t
GhnPlcCrcTrailer::Deserialize (Buffer::Iterator start)
{

}

void
GhnPlcCrcTrailer::SetCrc (uint32_t crc)
{

}
uint32_t
GhnPlcCrcTrailer::GetCrc (void)
{

}
}
}
