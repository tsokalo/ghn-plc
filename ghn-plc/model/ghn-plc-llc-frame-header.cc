/*
 * GhnPlcLlcFrameHeader.cpp
 *
 *  Created on: Jun 26, 2012
 *      Author: tsokalo
 */
#include <iomanip>
#include <iostream>

#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/header.h"
#include "ns3/address-utils.h"

#include "ghn-plc-llc-frame-header.h"
#include "ghn-plc-utilities.h"

NS_LOG_COMPONENT_DEFINE ("GhnPlcLlcFrameHeader");

//in bytes
#define FRAME_HEADER_SIZE 9

namespace ns3
{
namespace ghn {
NS_OBJECT_ENSURE_REGISTERED (GhnPlcLlcFrameHeader);

TypeId
GhnPlcLlcFrameHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::GhnPlcLlcFrameHeader") .SetParent<Header> () .AddConstructor<GhnPlcLlcFrameHeader> ();
  return tid;
}
TypeId
GhnPlcLlcFrameHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

GhnPlcLlcFrameHeader::GhnPlcLlcFrameHeader ()
{
  m_frameFields.resize (TSMP + 1);

  DefaultHeader ();
}

GhnPlcLlcFrameHeader::~GhnPlcLlcFrameHeader ()
{
  m_frameFields.resize (0);
}

uint32_t
GhnPlcLlcFrameHeader::GetHeaderSize () const
{
  return FRAME_HEADER_SIZE;
}

void
GhnPlcLlcFrameHeader::Print (std::ostream &os) const
{
  os << endl;
  for (unsigned int i = 0; i < m_frameFields.size (); i++)
    os << "GhnLlcFrame header field " << i << " :: " << m_frameFields.at (i).content << endl;
  os << endl;
}
void
GhnPlcLlcFrameHeader::DefaultHeader ()
{
  // all in bits

  m_frameFields.at (GhnLlcLayerFT).size = 3;
  m_frameFields.at (GhnLlcLayerFT).content = 0;

  m_frameFields.at (TSMPI).size = 1;
  m_frameFields.at (TSMPI).content = 0;

  m_frameFields.at (CCMPI).size = 1;
  m_frameFields.at (CCMPI).content = 0;

  m_frameFields.at (LPRI).size = 3;
  m_frameFields.at (LPRI).content = 0;

  m_frameFields.at (FLEN).size = 14;
  m_frameFields.at (FLEN).content = 0;

  m_frameFields.at (OriginatingNode).size = 8;
  m_frameFields.at (OriginatingNode).content = 0;

  m_frameFields.at (BRCTI).size = 1;
  m_frameFields.at (BRCTI).content = 0;

  m_frameFields.at (TTL).size = 6;
  m_frameFields.at (TTL).content = 0;

  m_frameFields.at (TSMP).size = 32;
  m_frameFields.at (TSMP).content = 0;
}

uint32_t
GhnPlcLlcFrameHeader::GetSerializedSize (void) const
{
  return GetHeaderSize ();
}
void
GhnPlcLlcFrameHeader::Serialize (Buffer::Iterator start) const
{
  uint8_t buffer[FRAME_HEADER_SIZE];
  memset (buffer, 0, FRAME_HEADER_SIZE);

  buffer[0] = CopyBits (0, 3, m_frameFields.at (GhnLlcLayerFT).content, 0, buffer[0]);
  buffer[0] = CopyBits (0, 1, m_frameFields.at (TSMPI).content, 3, buffer[0]);
  buffer[0] = CopyBits (0, 1, m_frameFields.at (CCMPI).content, 4, buffer[0]);
  buffer[0] = CopyBits (0, 3, m_frameFields.at (LPRI).content, 5, buffer[0]);

  buffer[1] = CopyBits (0, 8, m_frameFields.at (FLEN).content, 0, buffer[1]);
  buffer[2] = CopyBits (8, 14, m_frameFields.at (FLEN).content, 0, buffer[2]);

  buffer[3] = CopyBits (0, 8, m_frameFields.at (OriginatingNode).content, 0, buffer[3]);

  buffer[4] = CopyBits (0, 1, m_frameFields.at (BRCTI).content, 0, buffer[4]);

  buffer[4] = CopyBits (0, 6, m_frameFields.at (TTL).content, 2, buffer[4]);

  buffer[5] = CopyBits (0, 8, m_frameFields.at (TSMP).content, 0, buffer[5]);
  buffer[6] = CopyBits (8, 16, m_frameFields.at (TSMP).content, 0, buffer[6]);
  buffer[7] = CopyBits (16, 24, m_frameFields.at (TSMP).content, 0, buffer[7]);
  buffer[8] = CopyBits (24, 32, m_frameFields.at (TSMP).content, 0, buffer[8]);

  for (int j = 0; j < FRAME_HEADER_SIZE; j++)
    start.WriteU8 (buffer[j]);
}
uint32_t
GhnPlcLlcFrameHeader::Deserialize (Buffer::Iterator start)
{
  uint8_t buffer[FRAME_HEADER_SIZE];
  memset (buffer, 0, FRAME_HEADER_SIZE);

  for (int j = 0; j < FRAME_HEADER_SIZE; j++)
    buffer[j] = start.ReadU8 ();

  DefaultHeader ();

  m_frameFields.at (GhnLlcLayerFT).content = CopyBits (0, 3, buffer[0], 0, m_frameFields.at (GhnLlcLayerFT).content);
  m_frameFields.at (TSMPI).content = CopyBits (3, 4, buffer[0], 0, m_frameFields.at (TSMPI).content);
  m_frameFields.at (CCMPI).content = CopyBits (4, 5, buffer[0], 0, m_frameFields.at (CCMPI).content);
  m_frameFields.at (LPRI).content = CopyBits (5, 8, buffer[0], 0, m_frameFields.at (LPRI).content);

  m_frameFields.at (FLEN).content = CopyBits (0, 8, buffer[1], 0, m_frameFields.at (FLEN).content);
  m_frameFields.at (FLEN).content = CopyBits (0, 8, buffer[2], 8, m_frameFields.at (FLEN).content);

  m_frameFields.at (OriginatingNode).content = CopyBits (0, 8, buffer[3], 0, m_frameFields.at (OriginatingNode).content);

  m_frameFields.at (BRCTI).content = CopyBits (0, 1, buffer[4], 0, m_frameFields.at (BRCTI).content);

  m_frameFields.at (TTL).content = CopyBits (2, 8, buffer[4], 0, m_frameFields.at (TTL).content);

  m_frameFields.at (TSMP).content = CopyBits (0, 8, buffer[5], 0, m_frameFields.at (TSMP).content);
  m_frameFields.at (TSMP).content = CopyBits (0, 8, buffer[6], 8, m_frameFields.at (TSMP).content);
  m_frameFields.at (TSMP).content = CopyBits (0, 8, buffer[7], 16, m_frameFields.at (TSMP).content);
  m_frameFields.at (TSMP).content = CopyBits (0, 8, buffer[8], 24, m_frameFields.at (TSMP).content);

  return GetSerializedSize ();
}
void
GhnPlcLlcFrameHeader::Initilize (vector<FrameField> fields)
{
  for (unsigned int i = 0; i < m_frameFields.size (); i++)
    m_frameFields.at (i).content = fields.at (i).content;
}
vector<FrameField>
GhnPlcLlcFrameHeader::getFields ()
{
  return m_frameFields;
}
uint64_t
GhnPlcLlcFrameHeader::GetOrigNodeId ()
{
  return m_frameFields.at (OriginatingNode).content;
}
uint64_t
GhnPlcLlcFrameHeader::GetPriority ()
{
  return m_frameFields.at (LPRI).content;
}
void
GhnPlcLlcFrameHeader::SetBroadcastIndicator (uint8_t val)
{
  m_frameFields.at (BRCTI).content = val;
}
uint8_t
GhnPlcLlcFrameHeader::GetBroadcastIndicator ()
{
  return m_frameFields.at (BRCTI).content;
}
void
GhnPlcLlcFrameHeader::SetOrigNodeId (uint64_t p)
{
  m_frameFields.at (OriginatingNode).content = p;
}
void
GhnPlcLlcFrameHeader::SetPriority (uint64_t p)
{
  m_frameFields.at (LPRI).content = p;
}
void
GhnPlcLlcFrameHeader::SetFrameSize (uint64_t p)
{
  m_frameFields.at (FLEN).content = p;
}
uint64_t
GhnPlcLlcFrameHeader::GetFrameSize ()
{
  return m_frameFields.at (FLEN).content;
}
void
GhnPlcLlcFrameHeader::SetFrameType (LlcFrameType llc_frame_type)
{
  m_frameFields.at (GhnLlcLayerFT).content = llc_frame_type;
}
LlcFrameType
GhnPlcLlcFrameHeader::GetFrameType ()
{
  return LlcFrameType (m_frameFields.at (GhnLlcLayerFT).content);
}
void
GhnPlcLlcFrameHeader::SetTtl (unsigned int val)
{
  m_frameFields.at (TTL).content = val;
}
unsigned int
GhnPlcLlcFrameHeader::GetTtl ()
{
  return m_frameFields.at (TTL).content;
}
void
GhnPlcLlcFrameHeader::SetTsmp (uint32_t val)
{
  m_frameFields.at (TSMP).content = val;
}
uint32_t
GhnPlcLlcFrameHeader::GetTsmp ()
{
  return m_frameFields.at (TSMP).content;
}
void
GhnPlcLlcFrameHeader::SetTsmpI (uint8_t val)
{
  m_frameFields.at (TSMPI).content = val;
}
uint8_t
GhnPlcLlcFrameHeader::GetTsmpI ()
{
  return m_frameFields.at (TSMPI).content;
}
uint32_t
GhnPlcLlcFrameHeader::GetMaxTsmp ()
{
  return ((1 << m_frameFields.at (TSMP).size) - 1);
}
}
} // namespace ns3
