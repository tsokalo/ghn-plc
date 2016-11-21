/*
 * GhnPlcLpduHeader.cpp
 *
 *  Created on: Jun 26, 2012
 *      Author: tsokalo
 */

#include <iomanip>
#include <iostream>
#include <sstream>
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/header.h"
#include "ns3/address-utils.h"

#include "ghn-plc-lpdu-header.h"
#include "ghn-plc-header.h"
#include "ghn-plc-utilities.h"

NS_LOG_COMPONENT_DEFINE ("GhnPlcLpduHeader");

namespace ns3
{
namespace ghn {
void
printBitsInByte (char byte);
NS_OBJECT_ENSURE_REGISTERED (GhnPlcLpduHeader);

TypeId
GhnPlcLpduHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::GhnPlcLpduHeader") .SetParent<Header> () .AddConstructor<GhnPlcLpduHeader> ();
  return tid;
}
TypeId
GhnPlcLpduHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

GhnPlcLpduHeader::GhnPlcLpduHeader ()
{
  lpdu_header_field.resize (OPSF + 1);
  defaultHeader ();
}

GhnPlcLpduHeader::~GhnPlcLpduHeader ()
{
  lpdu_header_field.resize (0);
}
void
GhnPlcLpduHeader::defaultHeader ()
{
  //all sizes in bits
  lpdu_header_field.at (SSN).size = 16;
  lpdu_header_field.at (SSN).content = 0;

  lpdu_header_field.at (LFBO).size = 10;
  lpdu_header_field.at (LFBO).content = 0;

  lpdu_header_field.at (VSF).size = 1;
  lpdu_header_field.at (VSF).content = 0;

  lpdu_header_field.at (MQF).size = 1;
  lpdu_header_field.at (MQF).content = 0;

  lpdu_header_field.at (OPSF).size = 1;
  lpdu_header_field.at (OPSF).content = 0;
}
void
GhnPlcLpduHeader::SetSsn (unsigned int ssn)
{
  lpdu_header_field.at (SSN).content = ssn;
}
unsigned int
GhnPlcLpduHeader::GetSsn ()
{
  return lpdu_header_field.at (SSN).content;
}
void
GhnPlcLpduHeader::SetLfbo (unsigned int lfbo)
{
  if (lfbo > (unsigned int) ((1 << lpdu_header_field.at (LFBO).size) - 1))
    {
      lfbo = ((1 << lpdu_header_field.at (LFBO).size) - 1);
      //		NS_LOG_ERROR("Setting LFBO to: " << lfbo);
    }
  lpdu_header_field.at (LFBO).content = lfbo;
}
unsigned int
GhnPlcLpduHeader::GetLfbo ()
{
  return lpdu_header_field.at (LFBO).content;
}
void
GhnPlcLpduHeader::SetMqf (unsigned int mqf)
{
  lpdu_header_field.at (MQF).content = mqf;
}
unsigned int
GhnPlcLpduHeader::GetMqf ()
{
  return lpdu_header_field.at (MQF).content;
}
void
GhnPlcLpduHeader::SetVsf (VsfValue vsf)
{
  lpdu_header_field.at (VSF).content = vsf;
}
VsfValue
GhnPlcLpduHeader::GetVsf ()
{
  return VsfValue(lpdu_header_field.at (VSF).content);
}
uint32_t
GhnPlcLpduHeader::GetHeaderSize () const
{
  return GHN_LPDU_HEADER_SIZE;
}
void
GhnPlcLpduHeader::Print (std::ostream &os) const
{
  os << endl;
  for (unsigned int i = 0; i < lpdu_header_field.size (); i++)
    os << "GhnLpdu header field " << i << " :: " << lpdu_header_field.at (i).content << endl;
}
uint32_t
GhnPlcLpduHeader::GetSerializedSize (void) const
{
  return GetHeaderSize ();
}

void
GhnPlcLpduHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  uint8_t buffer[GHN_LPDU_HEADER_SIZE];
  memset (buffer, 0, GHN_LPDU_HEADER_SIZE);

  buffer[0] = CopyBits (0, 8, lpdu_header_field.at (SSN).content, 0, buffer[0]);
  buffer[1] = CopyBits (8, 16, lpdu_header_field.at (SSN).content, 0, buffer[1]);

  buffer[2] = CopyBits (0, 8, lpdu_header_field.at (LFBO).content, 0, buffer[2]);
  buffer[3] = CopyBits (8, 10, lpdu_header_field.at (LFBO).content, 0, buffer[3]);

  buffer[3] = CopyBits (0, 1, lpdu_header_field.at (VSF).content, 2, buffer[3]);

  buffer[3] = CopyBits (0, 1, lpdu_header_field.at (MQF).content, 3, buffer[3]);

  buffer[3] = CopyBits (0, 1, lpdu_header_field.at (OPSF).content, 4, buffer[3]);

  for (int j = 0; j < GHN_LPDU_HEADER_SIZE; j++)
    i.WriteU8 (buffer[j]);
}

uint32_t
GhnPlcLpduHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  uint8_t buffer[GHN_LPDU_HEADER_SIZE];
  memset (buffer, 0, GHN_LPDU_HEADER_SIZE);

  for (int j = 0; j < GHN_LPDU_HEADER_SIZE; j++)
    buffer[j] = i.ReadU8 ();

  defaultHeader ();

  lpdu_header_field.at (SSN).content = CopyBits (0, 8, buffer[0], 0, lpdu_header_field.at (SSN).content);
  lpdu_header_field.at (SSN).content = CopyBits (0, 8, buffer[1], 8, lpdu_header_field.at (SSN).content);

  lpdu_header_field.at (LFBO).content = CopyBits (0, 8, buffer[2], 0, lpdu_header_field.at (LFBO).content);
  lpdu_header_field.at (LFBO).content = CopyBits (0, 2, buffer[3], 8, lpdu_header_field.at (LFBO).content);

  if (buffer[3] & (1 << 2)) lpdu_header_field.at (VSF).content = 1;

  if (buffer[3] & (1 << 3)) lpdu_header_field.at (MQF).content = 1;

  if (buffer[3] & (1 << 4)) lpdu_header_field.at (OPSF).content = 1;

  return GetSerializedSize ();
}

void
printBitsInByte (char byte)
{
  std::stringstream ss;

  ss << "0b";
  for (int i = 0; i < 8; i++)
    if (byte & (1 << i))
      ss << "1";
    else
      ss << "0";
  NS_LOG_UNCOND(ss.str());
}
}
} // namespace ns3
