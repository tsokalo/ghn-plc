/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 TUD
 *
 *  Created on: 26.06.2013
 *      Author: Stanislav Mudriievskyi <stanislav.mudriievskyi@tu-dresden.de>
 */

#include "ghn-plc-phy-header.h"

namespace ns3 {
namespace ghn {
NS_OBJECT_ENSURE_REGISTERED(GhnPlcPhyFrameHeaderCoreCommon);

TypeId
GhnPlcPhyFrameHeaderCoreCommon::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::GhnPlcPhyFrameHeaderCoreCommon")
    .SetParent<Header> ()
    .AddConstructor<GhnPlcPhyFrameHeaderCoreCommon> ();
  return tid;
}

GhnPlcPhyFrameHeaderCoreCommon::GhnPlcPhyFrameHeaderCoreCommon(void)
{
}

GhnPlcPhyFrameHeaderCoreCommon::~GhnPlcPhyFrameHeaderCoreCommon(void)
{
}

uint32_t
GhnPlcPhyFrameHeaderCoreCommon::GetSerializedSize (void) const
{
  return 4;
}

TypeId
GhnPlcPhyFrameHeaderCoreCommon::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
GhnPlcPhyFrameHeaderCoreCommon::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteU8 ((m_domainId << 4) | m_frameType);
  i.WriteU8 (m_sourceId);
  i.WriteU8 (m_destinationId);
  i.WriteU8 ((m_headerSegmentationIndication << 3) |
             (m_extendedHeaderIndication << 2) |
             (m_durationIndication << 1) |
              m_multicastIndication);
}

uint32_t
GhnPlcPhyFrameHeaderCoreCommon::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  uint8_t temp;
  temp = i.ReadU8 ();
  m_frameType = 0b00001111 & temp;
  m_domainId = temp >> 4;
  m_sourceId = i.ReadU8 ();
  m_destinationId = i.ReadU8 ();
  temp = i.ReadU8 ();
  m_multicastIndication = 0b00000001 & temp;
  m_durationIndication = 0b00000001 & (temp >> 1);
  m_extendedHeaderIndication = 0b00000001 & (temp >> 2);
  m_headerSegmentationIndication = 0b00000001 & (temp >> 3);
  return i.GetDistanceFrom(start);
}

void
GhnPlcPhyFrameHeaderCoreCommon::Print (std::ostream &os) const
{
  os << std::endl;
  os << "Frame type = " << m_frameType << std::endl;
  os << ", domain ID = " << m_domainId << std::endl;
  os << ", source ID = " << m_sourceId << std::endl;
  os << ", destination ID = " << m_destinationId << std::endl;
  os << ", multicast indication = " << m_multicastIndication << std::endl;
  os << ", duration indication = " << m_durationIndication << std::endl;
  os << ", extended header indication = " << m_extendedHeaderIndication << std::endl;
  os << ", header segmentation indication = " << m_headerSegmentationIndication << std::endl;
  os << std::endl;
}


NS_OBJECT_ENSURE_REGISTERED(GhnPlcPhyFrameHeaderCoreVariableMapRmap);

TypeId
GhnPlcPhyFrameHeaderCoreVariableMapRmap::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::GhnPlcPhyFrameHeaderCoreVariableMapRmap")
    .SetParent<Header> ()
    .AddConstructor<GhnPlcPhyFrameHeaderCoreVariableMapRmap> ();
  return tid;
}

GhnPlcPhyFrameHeaderCoreVariableMapRmap::GhnPlcPhyFrameHeaderCoreVariableMapRmap(void)
{
}

GhnPlcPhyFrameHeaderCoreVariableMapRmap::~GhnPlcPhyFrameHeaderCoreVariableMapRmap(void)
{
}

uint32_t
GhnPlcPhyFrameHeaderCoreVariableMapRmap::GetSerializedSize (void) const
{
  return 15;
}

TypeId
GhnPlcPhyFrameHeaderCoreVariableMapRmap::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
GhnPlcPhyFrameHeaderCoreVariableMapRmap::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteU16 (m_mapFrameDuration);
  i.WriteU32 (m_networkTimeReference);
  i.WriteU32 (m_macCycleStartTime);
  i.WriteU16 ((m_scramblerInitialization << 12) | m_rcmSectionSize);
  i.WriteU8 ((m_fecConcatenationFactor << 5) |
             (m_repetitionsNumber << 2) |
              m_fecBlockSize);
  i.WriteU8 ((m_rmapIndication << 4) |
             (m_mapType << 3) |
             m_bandplan);
  i.WriteU8 (m_hopsNumber);
}

uint32_t
GhnPlcPhyFrameHeaderCoreVariableMapRmap::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  m_mapFrameDuration = i.ReadU16 ();
  m_networkTimeReference = i.ReadU32 ();
  m_macCycleStartTime = i.ReadU32 ();
  uint16_t temp16;
  temp16 = i.ReadU16 ();
  m_rcmSectionSize = 0b0000111111111111 & temp16;
  m_scramblerInitialization = temp16 >> 12;
  uint8_t temp;
  temp = i.ReadU8 ();
  m_fecBlockSize = 0b00000011 & temp;
  m_repetitionsNumber = 0b00000111 & (temp >> 2);
  m_fecConcatenationFactor = 0b00000111 & (temp >> 5);
  temp = i.ReadU8 ();
  m_bandplan = 0b00000111 & temp;
  m_mapType = 0b00000001 & (temp >> 3);
  m_rmapIndication = 0b00000001 & (temp >> 4);
  m_hopsNumber = i.ReadU8 ();
  return i.GetDistanceFrom (start);
}

void
GhnPlcPhyFrameHeaderCoreVariableMapRmap::Print (std::ostream &os) const
{
  os << std::endl;
  os << "Duration for MAP frame = " << m_mapFrameDuration << std::endl;
  os << ", network time reference = " << m_networkTimeReference << std::endl;
  os << ", MAC cycle start time = " << m_macCycleStartTime << std::endl;
  os << ", RCM section size = " << m_rcmSectionSize << std::endl;
  os << ", scrambler initialization = " << m_scramblerInitialization << std::endl;
  os << ", block size of FEC codeword for MAP frame payload = " << m_fecBlockSize << std::endl;
  os << ", number of repetitions for encoding payload = " << m_repetitionsNumber << std::endl;
  os << ", FEC concatenation factor = " << m_fecConcatenationFactor << std::endl;
  os << ", bandplan = " << m_bandplan << std::endl;
  os << ", MAP type = " << m_mapType << std::endl;
  os << ", RMAP indication = " << m_rmapIndication << std::endl;
  os << ", number of hops from domain master = " << m_hopsNumber << std::endl;
  os << std::endl;
}


NS_OBJECT_ENSURE_REGISTERED(GhnPlcPhyFrameHeaderCoreVariableMsg);

TypeId
GhnPlcPhyFrameHeaderCoreVariableMsg::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::GhnPlcPhyFrameHeaderCoreVariableMsg")
    .SetParent<Header> ()
    .AddConstructor<GhnPlcPhyFrameHeaderCoreVariableMsg> ();
  return tid;
}

GhnPlcPhyFrameHeaderCoreVariableMsg::GhnPlcPhyFrameHeaderCoreVariableMsg(void)
{
}

GhnPlcPhyFrameHeaderCoreVariableMsg::~GhnPlcPhyFrameHeaderCoreVariableMsg(void)
{
}

uint32_t
GhnPlcPhyFrameHeaderCoreVariableMsg::GetSerializedSize (void) const
{
  return 15;
}

TypeId
GhnPlcPhyFrameHeaderCoreVariableMsg::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
GhnPlcPhyFrameHeaderCoreVariableMsg::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteU16 (m_msgFrameDuration);
  i.WriteU8 ((m_repetitionsNumber << 5) |
             (m_fecCodingRate << 2) |
              m_fecBlockSize);
  i.WriteU8 ((m_masterDetected << 7) |
             (m_scramblerInitialization << 3) |
              m_fecConcatenationFactor);
  i.WriteU8 ((m_bandplanIdentifier << 5) | m_batIdentifier);
  i.WriteU8 ((m_psdCeiling << 3) | m_guardIntervalIdentifier);
  i.WriteU8 (m_connectionIdentifier);
  i.WriteU8 ((m_aifgIndication << 5) |
             (m_burstEndFlag << 4) |
             (m_burstFrameCount << 2) |
              m_replyRequired);
  i.WriteU8 ((m_connectionManagement << 3) | m_aceSymbolsNumber);
  i.WriteU16 (m_brurqStartSsn);
  i.WriteU8 ((m_requestBidirectionalTransmission << 7) | m_currentTs);
  i.WriteU8 (m_mcAckSlotsNumber);
  i.WriteU16 (0);
}

uint32_t
GhnPlcPhyFrameHeaderCoreVariableMsg::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  m_msgFrameDuration = i.ReadU16 ();
  uint8_t temp;
  temp = i.ReadU8 ();
  m_fecBlockSize = 0b00000011 & temp;
  m_fecCodingRate = 0b00000111 & (temp >> 2);
  m_repetitionsNumber = 0b00000111 & (temp >> 5);
  temp = i.ReadU8 ();
  m_fecConcatenationFactor = 0b00000111 & temp;
  m_scramblerInitialization = 0b00001111 & (temp >> 3);
  m_masterDetected = 0b00000001 & (temp >> 7);
  temp = i.ReadU8 ();
  m_batIdentifier = 0b00011111 & temp;
  m_bandplanIdentifier = 0b00000111 & (temp >> 5);
  temp = i.ReadU8 ();
  m_guardIntervalIdentifier = 0b00000111 & temp;
  m_psdCeiling = 0b00011111 & (temp >> 3);
  m_connectionIdentifier = i.ReadU8 ();
  temp = i.ReadU8 ();
  m_replyRequired = 0b00000011 & temp;
  m_burstFrameCount = 0b00000011 & (temp >> 2);
  m_burstEndFlag = 0b00000001 & (temp >> 4);
  m_aifgIndication = 0b00000001 & (temp >> 5);
  temp = i.ReadU8 ();
  m_aceSymbolsNumber = 0b00000111 & temp;
  m_connectionManagement = 0b00001111 & (temp >> 3);
  m_brurqStartSsn = i.ReadU16 ();
  temp = i.ReadU8 ();
  m_currentTs = 0b01111111 & temp;
  m_requestBidirectionalTransmission = 0b00000001 & (temp >> 7);
  m_mcAckSlotsNumber = i.ReadU8 ();
  i.ReadU16 ();
  return i.GetDistanceFrom (start);
}

void
GhnPlcPhyFrameHeaderCoreVariableMsg::Print (std::ostream &os) const
{
  os << std::endl;
  os << "Duration for MSG frame = " << m_msgFrameDuration << std::endl;
  os << ", block size of FEC codeword for MSG frame payload = " << m_fecBlockSize << std::endl;
  os << ", FEC coding rate for MSG frame payload = " << m_fecCodingRate << std::endl;
  os << ", number of repetitions udes for encoding the MSG frame payload = " << m_repetitionsNumber << std::endl;
  os << ", FEC concatenation factor = " << m_fecConcatenationFactor << std::endl;
  os << ", scrambler initialization = " << m_scramblerInitialization << std::endl;
  os << ", master is detected = " << m_masterDetected << std::endl;
  os << ", bit allocation table identifier = " << m_batIdentifier << std::endl;
  os << ", bandplan identifier/sub-carrier grouping identifier = " << m_bandplanIdentifier << std::endl;
  os << ", guard interval identifier = " << m_guardIntervalIdentifier << std::endl;
  os << ", actual PSD ceiling of MSG frame = " << m_psdCeiling << std::endl;
  os << ", connection identifier = " << m_connectionIdentifier << std::endl;
  os << ", reply required = " << m_replyRequired << std::endl;
  os << ", burst frame count = " << m_burstFrameCount << std::endl;
  os << ", burst end flag = " << m_burstEndFlag << std::endl;
  os << ", AIFG indication = " << m_aifgIndication << std::endl;
  os << ", number of ACE symbols = " << m_aceSymbolsNumber << std::endl;
  os << ", connection management = " << m_connectionManagement << std::endl;
  os << ", bandwidth reservation update request / start segment sequence number = " << m_brurqStartSsn << std::endl;
  os << ", current TS = " << m_currentTs << std::endl;
  os << ", request for bidirectional transmission = " << m_requestBidirectionalTransmission << std::endl;
  os << ", number of Mc-ACK slots = " << m_mcAckSlotsNumber << std::endl;
  os << std::endl;
}


NS_OBJECT_ENSURE_REGISTERED(GhnPlcPhyFrameHeaderCoreVariableAck);

TypeId
GhnPlcPhyFrameHeaderCoreVariableAck::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::GhnPlcPhyFrameHeaderCoreVariableAck")
    .SetParent<Header> ()
    .AddConstructor<GhnPlcPhyFrameHeaderCoreVariableAck> ();
  return tid;
}

GhnPlcPhyFrameHeaderCoreVariableAck::GhnPlcPhyFrameHeaderCoreVariableAck(void)
{
}

GhnPlcPhyFrameHeaderCoreVariableAck::~GhnPlcPhyFrameHeaderCoreVariableAck(void)
{
}

uint32_t
GhnPlcPhyFrameHeaderCoreVariableAck::GetSerializedSize (void) const
{
  return 15;
}

TypeId
GhnPlcPhyFrameHeaderCoreVariableAck::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
GhnPlcPhyFrameHeaderCoreVariableAck::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteU8 ((m_flowControl << 2) |
             (m_flowControlType << 1) |
              m_flowControlConnectionFlag);
  i.WriteU8 ((m_requestBidirectionalTransmission << 3) |
             (m_badBurstIndication << 2) |
             (m_managementRxResetFlag << 1) |
              m_dataRxResetFlag);
  i.WriteU8 (m_ackChannelEstimationReceiverWindowSize);
  i.WriteU32 (m_acknowledgementDateAndMcAckDescriptor32);
  i.WriteU64 (m_acknowledgementDateAndMcAckDescriptor64);
}

uint32_t
GhnPlcPhyFrameHeaderCoreVariableAck::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  uint8_t temp;
  temp = i.ReadU8 ();
  m_flowControlConnectionFlag = 0b00000001 & temp;
  m_flowControlType = 0b00000001 & (temp >> 1);
  m_flowControl = 0b00011111 & (temp >> 2);
  temp = i.ReadU8 ();
  m_dataRxResetFlag = 0b00000001 & temp;
  m_managementRxResetFlag = 0b00000001 & (temp >> 1);
  m_badBurstIndication = 0b00000001 & (temp >> 2);
  m_requestBidirectionalTransmission = 0b00000001 & (temp >> 3);
  m_ackChannelEstimationReceiverWindowSize = i.ReadU8 ();
  m_acknowledgementDateAndMcAckDescriptor32 = i.ReadU32 ();
  m_acknowledgementDateAndMcAckDescriptor64 = i.ReadU64 ();
  return i.GetDistanceFrom (start);
}

void
GhnPlcPhyFrameHeaderCoreVariableAck::Print (std::ostream &os) const
{
  os << std::endl;
  os << "Flow control connection flag = " << m_flowControlConnectionFlag << std::endl;
  os << ", flow control type = " << m_flowControlType << std::endl;
  os << ", flow control = " << m_flowControl << std::endl;
  os << ", data RX reset flag = " << m_dataRxResetFlag << std::endl;
  os << ", management RX reset flag = " << m_managementRxResetFlag << std::endl;
  os << ", bad burst indication = " << m_badBurstIndication << std::endl;
  os << ", request for bidirectional transmission = " << m_requestBidirectionalTransmission << std::endl;
  os << ", ACK channel estimation control/receiver window size for the connection = " << m_ackChannelEstimationReceiverWindowSize << std::endl;
  os << ", acknowledgement data and Mc-ACK descriptor 32-bit part = " << m_acknowledgementDateAndMcAckDescriptor32 << std::endl;
  os << ", acknowledgement data and Mc-ACK descriptor 64-bit part = " << m_acknowledgementDateAndMcAckDescriptor64 << std::endl;
  os << std::endl;
}


NS_OBJECT_ENSURE_REGISTERED(GhnPlcPhyFrameHeaderCoreVariableRts);

TypeId
GhnPlcPhyFrameHeaderCoreVariableRts::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::GhnPlcPhyFrameHeaderCoreVariableRts")
    .SetParent<Header> ()
    .AddConstructor<GhnPlcPhyFrameHeaderCoreVariableRts> ();
  return tid;
}

GhnPlcPhyFrameHeaderCoreVariableRts::GhnPlcPhyFrameHeaderCoreVariableRts(void)
{
}

GhnPlcPhyFrameHeaderCoreVariableRts::~GhnPlcPhyFrameHeaderCoreVariableRts(void)
{
}

uint32_t
GhnPlcPhyFrameHeaderCoreVariableRts::GetSerializedSize (void) const
{
  return 15;
}

TypeId
GhnPlcPhyFrameHeaderCoreVariableRts::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
GhnPlcPhyFrameHeaderCoreVariableRts::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteU16 (m_rtsFrameDuration);
  i.WriteU8 (m_ctsProxyId);
  i.WriteU8 (m_currentTs);
  i.WriteU64 (0);
  i.WriteU16 (0);
  i.WriteU8 (0);
}

uint32_t
GhnPlcPhyFrameHeaderCoreVariableRts::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  m_rtsFrameDuration = i.ReadU16 ();
  m_ctsProxyId = i.ReadU8 ();
  m_currentTs = i.ReadU8 ();
  i.ReadU64 ();
  i.ReadU16 ();
  i.ReadU8 ();
  return i.GetDistanceFrom (start);
}

void
GhnPlcPhyFrameHeaderCoreVariableRts::Print (std::ostream &os) const
{
  os << "Duration for RTS frame = " << m_rtsFrameDuration;
  os << ", CTS proxy ID = " << m_ctsProxyId;
  os << ", current TS = " << m_currentTs;
}


NS_OBJECT_ENSURE_REGISTERED(GhnPlcPhyFrameHeaderCoreVariableCts);

TypeId
GhnPlcPhyFrameHeaderCoreVariableCts::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::GhnPlcPhyFrameHeaderCoreVariableCts")
    .SetParent<Header> ()
    .AddConstructor<GhnPlcPhyFrameHeaderCoreVariableCts> ();
  return tid;
}

GhnPlcPhyFrameHeaderCoreVariableCts::GhnPlcPhyFrameHeaderCoreVariableCts(void)
{
}

GhnPlcPhyFrameHeaderCoreVariableCts::~GhnPlcPhyFrameHeaderCoreVariableCts(void)
{
}

uint32_t
GhnPlcPhyFrameHeaderCoreVariableCts::GetSerializedSize (void) const
{
  return 15;
}

TypeId
GhnPlcPhyFrameHeaderCoreVariableCts::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
GhnPlcPhyFrameHeaderCoreVariableCts::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteU16 (m_ctsFrameDuration);
  i.WriteU64 (0);
  i.WriteU32 (0);
  i.WriteU8 (0);
}

uint32_t
GhnPlcPhyFrameHeaderCoreVariableCts::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  m_ctsFrameDuration = i.ReadU16 ();
  i.ReadU64 ();
  i.ReadU32 ();
  i.ReadU8 ();
  return i.GetDistanceFrom (start);
}

void
GhnPlcPhyFrameHeaderCoreVariableCts::Print (std::ostream &os) const
{
  os << "Duration for CTS frame = " << m_ctsFrameDuration;
}


NS_OBJECT_ENSURE_REGISTERED(GhnPlcPhyFrameHeaderCoreVariableCtmg);

TypeId
GhnPlcPhyFrameHeaderCoreVariableCtmg::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::GhnPlcPhyFrameHeaderCoreVariableCtmg")
    .SetParent<Header> ()
    .AddConstructor<GhnPlcPhyFrameHeaderCoreVariableCtmg> ();
  return tid;
}

GhnPlcPhyFrameHeaderCoreVariableCtmg::GhnPlcPhyFrameHeaderCoreVariableCtmg(void)
{
}

GhnPlcPhyFrameHeaderCoreVariableCtmg::~GhnPlcPhyFrameHeaderCoreVariableCtmg(void)
{
}

uint32_t
GhnPlcPhyFrameHeaderCoreVariableCtmg::GetSerializedSize (void) const
{
  return 15;
}

TypeId
GhnPlcPhyFrameHeaderCoreVariableCtmg::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
GhnPlcPhyFrameHeaderCoreVariableCtmg::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteU8 ((m_currentTs << 1) | m_immediateAcknowledgmentRequired);
  i.WriteU64 (m_ctmgData64);
  i.WriteU32 (m_ctmgData32);
  i.WriteU16 (m_ctmgData16);
}

uint32_t
GhnPlcPhyFrameHeaderCoreVariableCtmg::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  uint8_t temp;
  temp = i.ReadU8 ();
  m_immediateAcknowledgmentRequired = 0b00000001 & temp;
  m_currentTs = 0b01111111 & (temp >> 1);
  m_ctmgData64 = i.ReadU64 ();
  m_ctmgData32 = i.ReadU32 ();
  m_ctmgData16 = i.ReadU16 ();
  return i.GetDistanceFrom (start);
}

void
GhnPlcPhyFrameHeaderCoreVariableCtmg::Print (std::ostream &os) const
{
  os << "Immediate acknowledgment required = " << m_immediateAcknowledgmentRequired;
  os << ", current TS = " << m_currentTs;
  os << ", CTMG data 64 = " << m_ctmgData64;
  os << ", CTMG data 32 = " << m_ctmgData32;
  os << ", CTMG data 16 = " << m_ctmgData16;
}


NS_OBJECT_ENSURE_REGISTERED(GhnPlcPhyFrameHeaderCoreVariableProbe);

TypeId
GhnPlcPhyFrameHeaderCoreVariableProbe::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::GhnPlcPhyFrameHeaderCoreVariableProbe")
    .SetParent<Header> ()
    .AddConstructor<GhnPlcPhyFrameHeaderCoreVariableProbe> ();
  return tid;
}

GhnPlcPhyFrameHeaderCoreVariableProbe::GhnPlcPhyFrameHeaderCoreVariableProbe(void)
{
}

GhnPlcPhyFrameHeaderCoreVariableProbe::~GhnPlcPhyFrameHeaderCoreVariableProbe(void)
{
}

uint32_t
GhnPlcPhyFrameHeaderCoreVariableProbe::GetSerializedSize (void) const
{
  return 15;
}

TypeId
GhnPlcPhyFrameHeaderCoreVariableProbe::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
GhnPlcPhyFrameHeaderCoreVariableProbe::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteU16 (m_probeFrameDuration);
  i.WriteU8 ((m_probeSymbols << 4) | m_probeFrameType);
  i.WriteU8 ((m_probeGuardInterval << 5) | m_psdCeiling);
  i.WriteU8 (m_currentTs);
  i.WriteU8 (0);
  i.WriteU64 (m_probeFrameTypeSpecificField64);
  i.WriteU8 (m_probeFrameTypeSpecificField8);
}

uint32_t
GhnPlcPhyFrameHeaderCoreVariableProbe::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  m_probeFrameDuration = i.ReadU16 ();
  uint8_t temp;
  temp = i.ReadU8 ();
  m_probeFrameType = 0b00001111 & temp;
  m_probeSymbols = 0b00001111 & (temp >> 4);
  temp = i.ReadU8 ();
  m_psdCeiling = 0b00011111 & temp;
  m_probeGuardInterval = 0b00000111 & (temp >> 4);
  m_currentTs = i.ReadU8 ();
  i.ReadU8 ();
  m_probeFrameTypeSpecificField64 = i.ReadU64 ();
  m_probeFrameTypeSpecificField8 = i.ReadU8 ();
  return i.GetDistanceFrom (start);
}

void
GhnPlcPhyFrameHeaderCoreVariableProbe::Print (std::ostream &os) const
{
  os << std::endl;
  os << "Duration for PROBE frame = " << m_probeFrameDuration << std::endl;
  os << ", PROBE frame type = " << m_probeFrameType << std::endl;
  os << ", probe symbols = " << m_probeSymbols << std::endl;
  os << ", actual PSD ceiling of PROBE frame = " << m_psdCeiling << std::endl;
  os << ", PROBE guard interval = " << m_probeGuardInterval << std::endl;
  os << ", current TS = " << m_currentTs << std::endl;
  os << ", PROBE frame type specific field 64 = " << m_probeFrameTypeSpecificField64 << std::endl;
  os << ", PROBE frame type specific field 8 = " << m_probeFrameTypeSpecificField8 << std::endl;
  os << std::endl;
}


NS_OBJECT_ENSURE_REGISTERED(GhnPlcPhyFrameHeaderCoreVariableAckrq);

TypeId
GhnPlcPhyFrameHeaderCoreVariableAckrq::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::GhnPlcPhyFrameHeaderCoreVariableAckrq")
    .SetParent<Header> ()
    .AddConstructor<GhnPlcPhyFrameHeaderCoreVariableAckrq> ();
  return tid;
}

GhnPlcPhyFrameHeaderCoreVariableAckrq::GhnPlcPhyFrameHeaderCoreVariableAckrq(void)
{
}

GhnPlcPhyFrameHeaderCoreVariableAckrq::~GhnPlcPhyFrameHeaderCoreVariableAckrq(void)
{
}

uint32_t
GhnPlcPhyFrameHeaderCoreVariableAckrq::GetSerializedSize (void) const
{
  return 15;
}

TypeId
GhnPlcPhyFrameHeaderCoreVariableAckrq::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
GhnPlcPhyFrameHeaderCoreVariableAckrq::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteU8 (m_requestedRxWindow);
  i.WriteU8 (m_connectionIdentifier);
  i.WriteU8 (m_currentTs);
  i.WriteU64 (0);
  i.WriteU32 (0);
}

uint32_t
GhnPlcPhyFrameHeaderCoreVariableAckrq::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  m_requestedRxWindow = i.ReadU8 ();
  m_connectionIdentifier = i.ReadU8 ();
  m_currentTs = i.ReadU8 ();
  i.ReadU64 ();
  i.ReadU32 ();
  return i.GetDistanceFrom (start);
}

void
GhnPlcPhyFrameHeaderCoreVariableAckrq::Print (std::ostream &os) const
{
  os << std::endl;
  os << "Requested RX window = " << m_requestedRxWindow << std::endl;
  os << ", connection identifier = " << m_connectionIdentifier << std::endl;
  os << ", current TS = " << m_currentTs << std::endl;
  os << std::endl;
}


NS_OBJECT_ENSURE_REGISTERED(GhnPlcPhyFrameHeaderCoreVariableBmsg);

TypeId
GhnPlcPhyFrameHeaderCoreVariableBmsg::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::GhnPlcPhyFrameHeaderCoreVariableBmsg")
    .SetParent<Header> ()
    .AddConstructor<GhnPlcPhyFrameHeaderCoreVariableBmsg> ();
  return tid;
}

GhnPlcPhyFrameHeaderCoreVariableBmsg::GhnPlcPhyFrameHeaderCoreVariableBmsg(void)
{
}

GhnPlcPhyFrameHeaderCoreVariableBmsg::~GhnPlcPhyFrameHeaderCoreVariableBmsg(void)
{
}

uint32_t
GhnPlcPhyFrameHeaderCoreVariableBmsg::GetSerializedSize (void) const
{
  return 15;
}

TypeId
GhnPlcPhyFrameHeaderCoreVariableBmsg::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
GhnPlcPhyFrameHeaderCoreVariableBmsg::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteU16 (m_bmsgFrameDuration);
  i.WriteU8 ((m_repetitionsNumber << 5) |
             (m_fecCodingRate << 2) |
              m_fecBlockSize);
  i.WriteU8 ((m_masterDetected << 7) |
             (m_scramblerInitialization << 3) |
              m_fecConcatenationFactor);
  i.WriteU8 ((m_bandplanIdentifier << 5) | m_batIdentifier);
  i.WriteU8 ((m_psdCeiling << 3) | m_guardIntervalIdentifier);
  i.WriteU8 (m_connectionIdentifier);
  i.WriteU8 ((m_aifgIndication << 5) |
             (m_burstEndFlag << 4) |
             (m_burstFrameCount << 2) |
              m_replyRequired);
  i.WriteU8 ((m_connectionManagement << 3) | m_aceSymbolsNumber);
  i.WriteU16 (m_brurqStartSsn);
  i.WriteU8 (m_currentTs);
  i.WriteU16 ((m_bidirectionalTransmissionEndFlag << 9) |
              (m_bidirectionalTransmissionGrantLength << 1));
  i.WriteU8 (m_ackChannelEstimationControl);
}

uint32_t
GhnPlcPhyFrameHeaderCoreVariableBmsg::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  m_bmsgFrameDuration = i.ReadU16 ();
  uint8_t temp;
  temp = i.ReadU8 ();
  m_fecBlockSize = 0b00000011 & temp;
  m_fecCodingRate = 0b00000111 & (temp >> 2);
  m_repetitionsNumber = 0b00000111 & (temp >> 5);
  temp = i.ReadU8 ();
  m_fecConcatenationFactor = 0b00000111 & temp;
  m_scramblerInitialization = 0b00001111 & (temp >> 3);
  m_masterDetected = 0b00000001 & (temp >> 7);
  temp = i.ReadU8 ();
  m_batIdentifier = 0b00011111 & temp;
  m_bandplanIdentifier = 0b00000111 & (temp >> 5);
  temp = i.ReadU8 ();
  m_guardIntervalIdentifier = 0b00000111 & temp;
  m_psdCeiling = 0b00011111 & (temp >> 3);
  m_connectionIdentifier = i.ReadU8 ();
  temp = i.ReadU8 ();
  m_replyRequired = 0b00000011 & temp;
  m_burstFrameCount = 0b00000011 & (temp >> 2);
  m_burstEndFlag = 0b00000001 & (temp >> 4);
  m_aifgIndication = 0b00000001 & (temp >> 5);
  temp = i.ReadU8 ();
  m_aceSymbolsNumber = 0b00000111 & temp;
  m_connectionManagement = 0b00001111 & (temp >> 3);
  m_brurqStartSsn = i.ReadU16 ();
  temp = i.ReadU8 ();
  m_currentTs = 0b01111111 & temp;
  uint16_t temp16;
  temp16 = i.ReadU16 ();
  m_bidirectionalTransmissionGrantLength = 0b0000000011111111 & (temp16 >> 1);
  m_bidirectionalTransmissionEndFlag = 0b0000000000000001 & (temp16 >> 9);
  m_ackChannelEstimationControl = i.ReadU8 ();
  return i.GetDistanceFrom (start);
}

void
GhnPlcPhyFrameHeaderCoreVariableBmsg::Print (std::ostream &os) const
{
  os << std::endl;
  os << "Duration for BMSG frame = " << m_bmsgFrameDuration << std::endl;
  os << ", block size of FEC codeword for BMSG frame payload = " << m_fecBlockSize << std::endl;
  os << ", FEC coding rate for BMSG frame payload = " << m_fecCodingRate << std::endl;
  os << ", number of repetitions udes for encoding the BMSG frame payload = " << m_repetitionsNumber << std::endl;
  os << ", FEC concatenation factor = " << m_fecConcatenationFactor << std::endl;
  os << ", scrambler initialization = " << m_scramblerInitialization << std::endl;
  os << ", master is detected = " << m_masterDetected << std::endl;
  os << ", bit allocation table identifier = " << m_batIdentifier << std::endl;
  os << ", bandplan identifier/sub-carrier grouping identifier = " << m_bandplanIdentifier << std::endl;
  os << ", guard interval identifier = " << m_guardIntervalIdentifier << std::endl;
  os << ", actual PSD ceiling of BMSG frame = " << m_psdCeiling << std::endl;
  os << ", connection identifier = " << m_connectionIdentifier << std::endl;
  os << ", reply required = " << m_replyRequired << std::endl;
  os << ", burst frame count = " << m_burstFrameCount << std::endl;
  os << ", burst end flag = " << m_burstEndFlag << std::endl;
  os << ", AIFG indication = " << m_aifgIndication << std::endl;
  os << ", number of ACE symbols = " << m_aceSymbolsNumber << std::endl;
  os << ", connection management = " << m_connectionManagement << std::endl;
  os << ", bandwidth reservation update request / start segment sequence number = " << m_brurqStartSsn << std::endl;
  os << ", current TS = " << m_currentTs << std::endl;
  os << ", bidirectional transmission grant length = " << m_bidirectionalTransmissionGrantLength << std::endl;
  os << ", bidirectional transmission end flag = " << m_bidirectionalTransmissionEndFlag << std::endl;
  os << ", ACK channel estimation control = " << m_ackChannelEstimationControl << std::endl;
  os << std::endl;
}


NS_OBJECT_ENSURE_REGISTERED(GhnPlcPhyFrameHeaderCoreVariableBack);

TypeId
GhnPlcPhyFrameHeaderCoreVariableBack::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::GhnPlcPhyFrameHeaderCoreVariableBack")
    .SetParent<Header> ()
    .AddConstructor<GhnPlcPhyFrameHeaderCoreVariableBack> ();
  return tid;
}

GhnPlcPhyFrameHeaderCoreVariableBack::GhnPlcPhyFrameHeaderCoreVariableBack(void)
{
}

GhnPlcPhyFrameHeaderCoreVariableBack::~GhnPlcPhyFrameHeaderCoreVariableBack(void)
{
}

uint32_t
GhnPlcPhyFrameHeaderCoreVariableBack::GetSerializedSize (void) const
{
  return 15;
}

TypeId
GhnPlcPhyFrameHeaderCoreVariableBack::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
GhnPlcPhyFrameHeaderCoreVariableBack::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteU16 (m_backFrameDuration);
  i.WriteU8 ((m_repetitionsNumber << 5) |
             (m_fecCodingRate << 2) |
              m_fecBlockSize);
  i.WriteU8 ((m_masterDetected << 7) |
             (m_scramblerInitialization << 3) |
              m_fecConcatenationFactor);
  i.WriteU8 ((m_bandplanIdentifier << 5) | m_batIdentifier);
  i.WriteU8 ((m_psdCeiling << 3) | m_guardIntervalIdentifier);
  i.WriteU8 (m_connectionIdentifier);
  i.WriteU8 ((m_aifgIndication << 5) |
             (m_burstEndFlag << 4) |
             (m_burstFrameCount << 2) |
              m_replyRequired);
  i.WriteU8 ((m_connectionManagement << 3) | m_aceSymbolsNumber);
  i.WriteU8 (m_bidirectionalTransmissionRequestLength);
  i.WriteU8 (m_ackChannelEstimationControl);
  i.WriteU32 (0);
}

uint32_t
GhnPlcPhyFrameHeaderCoreVariableBack::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  m_backFrameDuration = i.ReadU16 ();
  uint8_t temp;
  temp = i.ReadU8 ();
  m_fecBlockSize = 0b00000011 & temp;
  m_fecCodingRate = 0b00000111 & (temp >> 2);
  m_repetitionsNumber = 0b00000111 & (temp >> 5);
  temp = i.ReadU8 ();
  m_fecConcatenationFactor = 0b00000111 & temp;
  m_scramblerInitialization = 0b00001111 & (temp >> 3);
  m_masterDetected = 0b00000001 & (temp >> 7);
  temp = i.ReadU8 ();
  m_batIdentifier = 0b00011111 & temp;
  m_bandplanIdentifier = 0b00000111 & (temp >> 5);
  temp = i.ReadU8 ();
  m_guardIntervalIdentifier = 0b00000111 & temp;
  m_psdCeiling = 0b00011111 & (temp >> 3);
  m_connectionIdentifier = i.ReadU8 ();
  temp = i.ReadU8 ();
  m_replyRequired = 0b00000011 & temp;
  m_burstFrameCount = 0b00000011 & (temp >> 2);
  m_burstEndFlag = 0b00000001 & (temp >> 4);
  m_aifgIndication = 0b00000001 & (temp >> 5);
  temp = i.ReadU8 ();
  m_aceSymbolsNumber = 0b00000111 & temp;
  m_connectionManagement = 0b00001111 & (temp >> 3);
  m_bidirectionalTransmissionRequestLength = i.ReadU8 ();
  m_ackChannelEstimationControl = i.ReadU8 ();
  i.ReadU32 ();
  return i.GetDistanceFrom (start);
}

void
GhnPlcPhyFrameHeaderCoreVariableBack::Print (std::ostream &os) const
{
  os << std::endl;
  os << "Duration for BACK frame = " << m_backFrameDuration << std::endl;
  os << ", block size of FEC codeword for BACK frame payload = " << m_fecBlockSize << std::endl;
  os << ", FEC coding rate for BACK frame payload = " << m_fecCodingRate << std::endl;
  os << ", number of repetitions udes for encoding the BACK frame payload = " << m_repetitionsNumber << std::endl;
  os << ", FEC concatenation factor = " << m_fecConcatenationFactor << std::endl;
  os << ", scrambler initialization = " << m_scramblerInitialization << std::endl;
  os << ", master is detected = " << m_masterDetected << std::endl;
  os << ", bit allocation table identifier = " << m_batIdentifier << std::endl;
  os << ", bandplan identifier/sub-carrier grouping identifier = " << m_bandplanIdentifier << std::endl;
  os << ", guard interval identifier = " << m_guardIntervalIdentifier << std::endl;
  os << ", actual PSD ceiling of BACK frame = " << m_psdCeiling << std::endl;
  os << ", connection identifier = " << m_connectionIdentifier << std::endl;
  os << ", reply required = " << m_replyRequired << std::endl;
  os << ", burst frame count = " << m_burstFrameCount << std::endl;
  os << ", burst end flag = " << m_burstEndFlag << std::endl;
  os << ", AIFG indication = " << m_aifgIndication << std::endl;
  os << ", number of ACE symbols = " << m_aceSymbolsNumber << std::endl;
  os << ", connection management = " << m_connectionManagement << std::endl;
  os << ", bidirectional transmission request length = " << m_bidirectionalTransmissionRequestLength << std::endl;
  os << ", ACK channel estimation control = " << m_ackChannelEstimationControl << std::endl;
  os << std::endl;
}


NS_OBJECT_ENSURE_REGISTERED(GhnPlcPhyFrameHeaderCoreVariableActmg);

TypeId
GhnPlcPhyFrameHeaderCoreVariableActmg::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::GhnPlcPhyFrameHeaderCoreVariableActmg")
    .SetParent<Header> ()
    .AddConstructor<GhnPlcPhyFrameHeaderCoreVariableActmg> ();
  return tid;
}

GhnPlcPhyFrameHeaderCoreVariableActmg::GhnPlcPhyFrameHeaderCoreVariableActmg(void)
{
}

GhnPlcPhyFrameHeaderCoreVariableActmg::~GhnPlcPhyFrameHeaderCoreVariableActmg(void)
{
}

uint32_t
GhnPlcPhyFrameHeaderCoreVariableActmg::GetSerializedSize (void) const
{
  return 15;
}

TypeId
GhnPlcPhyFrameHeaderCoreVariableActmg::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
GhnPlcPhyFrameHeaderCoreVariableActmg::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteU8 (m_ctmgAcknowledgment);
  i.WriteU64 (0);
  i.WriteU32 (0);
  i.WriteU16 (0);
}

uint32_t
GhnPlcPhyFrameHeaderCoreVariableActmg::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  m_ctmgAcknowledgment = i.ReadU8 ();
  i.ReadU64 ();
  i.ReadU32 ();
  i.ReadU16 ();
  return i.GetDistanceFrom (start);
}

void
GhnPlcPhyFrameHeaderCoreVariableActmg::Print (std::ostream &os) const
{
  os << "CTMG acknowledgment = " << m_ctmgAcknowledgment;
}


NS_OBJECT_ENSURE_REGISTERED(GhnPlcPhyFrameHeaderCoreVariableFte);

TypeId
GhnPlcPhyFrameHeaderCoreVariableFte::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::GhnPlcPhyFrameHeaderCoreVariableFte")
    .SetParent<Header> ()
    .AddConstructor<GhnPlcPhyFrameHeaderCoreVariableFte> ();
  return tid;
}

GhnPlcPhyFrameHeaderCoreVariableFte::GhnPlcPhyFrameHeaderCoreVariableFte(void)
{
}

GhnPlcPhyFrameHeaderCoreVariableFte::~GhnPlcPhyFrameHeaderCoreVariableFte(void)
{
}

uint32_t
GhnPlcPhyFrameHeaderCoreVariableFte::GetSerializedSize (void) const
{
  return 15;
}

TypeId
GhnPlcPhyFrameHeaderCoreVariableFte::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
GhnPlcPhyFrameHeaderCoreVariableFte::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteU16 (m_fteFrameDuration);
  i.WriteU8 (m_currentTs);
  i.WriteU64 (m_eftSpecificFields64);
  i.WriteU32 (m_eftSpecificFields32);
}

uint32_t
GhnPlcPhyFrameHeaderCoreVariableFte::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  m_fteFrameDuration = i.ReadU16 ();
  m_currentTs = i.ReadU8 ();
  m_eftSpecificFields64 = i.ReadU64 ();
  m_eftSpecificFields32 = i.ReadU32 ();
  return i.GetDistanceFrom (start);
}

void
GhnPlcPhyFrameHeaderCoreVariableFte::Print (std::ostream &os) const
{
  os << std::endl;
  os << "Duration for FTE frame = " << m_eftSpecificFields32 << std::endl;
  os << ", current TS = " << m_currentTs << std::endl;
  os << ", EFT specific fields 64 = " << m_eftSpecificFields64 << std::endl;
  os << ", EFT specific fields 32 = " << m_eftSpecificFields64 << std::endl;
  os << std::endl;
}


NS_OBJECT_ENSURE_REGISTERED(GhnPlcPhyFrameHeaderCoreCommonHcs);

TypeId
GhnPlcPhyFrameHeaderCoreCommonHcs::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::GhnPlcPhyFrameHeaderCoreCommonHcs")
    .SetParent<Header> ()
    .AddConstructor<GhnPlcPhyFrameHeaderCoreCommonHcs> ();
  return tid;
}

GhnPlcPhyFrameHeaderCoreCommonHcs::GhnPlcPhyFrameHeaderCoreCommonHcs(void)
{
}

GhnPlcPhyFrameHeaderCoreCommonHcs::~GhnPlcPhyFrameHeaderCoreCommonHcs(void)
{
}

uint32_t
GhnPlcPhyFrameHeaderCoreCommonHcs::GetSerializedSize (void) const
{
  return 2;
}

TypeId
GhnPlcPhyFrameHeaderCoreCommonHcs::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
GhnPlcPhyFrameHeaderCoreCommonHcs::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteU16 (m_headerCheckSequence);
}

uint32_t
GhnPlcPhyFrameHeaderCoreCommonHcs::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  m_headerCheckSequence = i.ReadU16 ();
  return i.GetDistanceFrom (start);
}

void
GhnPlcPhyFrameHeaderCoreCommonHcs::Print (std::ostream &os) const
{
  os << std::endl;
  os << "Header check sequence = " << m_headerCheckSequence << std::endl;
  os << std::endl;
}

}
} // namespace ns3
