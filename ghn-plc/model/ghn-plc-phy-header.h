/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 TUD
 *
 *  Created on: 26.06.2013
 *      Author: Stanislav Mudriievskyi <stanislav.mudriievskyi@tu-dresden.de>
 */

#ifndef GHN_PLC_PHY_HEADER_H_
#define GHN_PLC_PHY_HEADER_H_

#include "ns3/header.h"

#define GDOTHN_PREAMBLE_N1 7
#define GDOTHN_PREAMBLE_N2 2
#define GDOTHN_PREAMBLE_K1 8
#define GDOTHN_PREAMBLE_K2 8

#define GDOTHN_PHY_HEADER 21

namespace ns3 {
namespace ghn {

enum GhnPlcPhyFrameType
{
  PHY_FRAME_MAP_RMAP    = 0,
  PHY_FRAME_MSG         = 1,
  PHY_FRAME_ACK         = 2,
  PHY_FRAME_RTS         = 3,
  PHY_FRAME_CTS         = 4,
  PHY_FRAME_CTMG        = 5,
  PHY_FRAME_PROBE       = 6,
  PHY_FRAME_ACKRQ       = 7,
  PHY_FRAME_BMSG        = 8,
  PHY_FRAME_BACK        = 9,
  PHY_FRAME_ACTMG       = 10,
  PHY_FRAME_FTE         = 15,
};

enum FecBlockSizeType
{
  FEC_BLOCK_SIZE_120 = 0,
  FEC_BLOCK_SIZE_540 = 1,
};

enum FecRateType
{
  FEC_RATE_1_2          = 1,
  FEC_RATE_2_3          = 2,
  FEC_RATE_5_6          = 3,
  FEC_RATE_16_18        = 4,
  FEC_RATE_20_21        = 5,
};

enum EncodingRepetitionsNumberType
{
  ENCODING_REPETITIONS_1        = 1,
  ENCODING_REPETITIONS_2        = 2,
  ENCODING_REPETITIONS_3        = 3,
  ENCODING_REPETITIONS_4        = 4,
  ENCODING_REPETITIONS_6        = 5,
  ENCODING_REPETITIONS_8        = 6,
};

//enum ModulationType
//{
//        NOMOD   = 0,
//        BPSK    = 1,
//        QAM4    = 2,
//        QAM8    = 3,
//        QAM16   = 4,
//        QAM32   = 5,
//        QAM64   = 6,
//        QAM128  = 7,
//        QAM256  = 8,
//        QAM512  = 9,
//        QAM1024 = 10,
//        QAM2048 = 11,
//        QAM4096 = 12,
//};

enum BatType
{
        BAT_TYPE0   = 0,
        BAT_TYPE1   = 1,
        BAT_TYPE2   = 2,
        BAT_TYPE3   = 3,
        BAT_RUNTIME = 8,
};

enum AckType
{
        ACK_TYPE_NO_ACK         = 0,
        ACK_TYPE_IMMEDIATE_ACK  = 1,
        ACK_TYPE_DELAYED_ACK    = 2,
        ACK_TYPE_RESERVED       = 3,
};

enum BandPlanType
{
  GDOTHN_BANDPLAN_25MHZ         = 0,
  GDOTHN_BANDPLAN_50MHZ         = 1,
  GDOTHN_BANDPLAN_100MHZ        = 2,
};

/**
 *      \brief PHY headers for G.hn
 *
 *      The PHY header has a core part and may have an extended part. The core part has common and variable fields.
 *      Common part until the frame type specific field (FTSF) is described with the GhnPlcPhyFrameHeaderCoreCommon. Variable
 *      part of the header is described with corresponding headers for different header types. The last part which is
 *      header check sequence (HCS) and belongs to the common part is implemented as a small separate header class
 *      GhnPlcPhyFrameHeaderCommonHcs.
 */

class GhnPlcPhyFrameHeaderCoreCommon : public Header
{
public:
  static TypeId GetTypeId (void);

  GhnPlcPhyFrameHeaderCoreCommon (void);
  ~GhnPlcPhyFrameHeaderCoreCommon (void);

  void SetFrameType (GhnPlcPhyFrameType frameType) { m_frameType = (uint8_t) frameType; }
  GhnPlcPhyFrameType GetFrameType (void) { return (GhnPlcPhyFrameType) m_frameType; }
  void SetDomainId (uint8_t domainId) { m_domainId = domainId; }
  uint8_t GetDomainId (void) { return m_domainId; }
  void SetSourceId (uint8_t sourceId) { m_sourceId = sourceId; }
  uint8_t GetSourceId (void) { return m_sourceId; }
  void SetDestinationId (uint8_t destinationId) { m_destinationId = destinationId; }
  uint8_t GetDestinationId (void) { return m_destinationId; }
  void SetMulticastIndication (uint8_t multicastIndication) { m_multicastIndication = multicastIndication; }
  uint8_t GetMulticastIndication (void) { return m_multicastIndication; }
  void SetDurationIndication (uint8_t durationIndication) { m_durationIndication = durationIndication; }
  uint8_t GetDurationIndication (void) { return m_durationIndication; }
  void SetExtendedHeaderIndication (uint8_t extendedHeaderIndication) { m_extendedHeaderIndication = extendedHeaderIndication; }
  uint8_t GetExtendedHeaderIndication (void) { return m_extendedHeaderIndication; }
  void SetHeaderSegmentationIndication (uint8_t headerSegmentationIndication) { m_headerSegmentationIndication = headerSegmentationIndication; }
  uint8_t GetHeaderSegmentationIndication (void) { return m_headerSegmentationIndication; }

  virtual uint32_t GetSerializedSize (void) const;
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;

private:
  uint8_t m_frameType;
  uint8_t m_domainId;
  uint8_t m_sourceId;
  uint8_t m_destinationId;
  uint8_t m_multicastIndication;
  uint8_t m_durationIndication;
  uint8_t m_extendedHeaderIndication;
  uint8_t m_headerSegmentationIndication;
};


class GhnPlcPhyFrameHeaderCoreVariableMapRmap : public Header
{
public:
  static TypeId GetTypeId (void);

  GhnPlcPhyFrameHeaderCoreVariableMapRmap (void);
  ~GhnPlcPhyFrameHeaderCoreVariableMapRmap (void);

  void SetMapFrameDuration (uint16_t mapFrameDuration) { m_mapFrameDuration = mapFrameDuration; }
  uint16_t GetMapFrameDuration (void) { return m_mapFrameDuration; }
  void SetNetworkTimeReference (uint32_t networkTimeReference) { m_networkTimeReference = networkTimeReference; }
  uint32_t GetNetworkTimeReference (void) { return m_networkTimeReference; }
  void SetMacCycleStartTime (uint32_t macCycleStartTime) { m_macCycleStartTime = macCycleStartTime; }
  uint32_t GetMacCycleStartTime (void) { return m_macCycleStartTime; }
  void SetRcmSectionSize (uint16_t rcmSectionSize) { m_rcmSectionSize = rcmSectionSize; }
  uint16_t GetRcmSectionSize (void) { return m_rcmSectionSize; }
  void SetScramblerInitialization (uint16_t scramblerInitialization) { m_scramblerInitialization = scramblerInitialization; }
  uint16_t GetScramblerInitialization (void) { return m_scramblerInitialization; }
  void SetFecBlockSize (FecBlockSizeType fecBlockSize) { m_fecBlockSize = (uint8_t) fecBlockSize; }
  FecBlockSizeType GetFecBlockSize (void) { return (FecBlockSizeType) m_fecBlockSize; }
  void SetRepetitionsNumber (EncodingRepetitionsNumberType repetitionsNumber) { m_repetitionsNumber = (uint8_t) repetitionsNumber; }
  EncodingRepetitionsNumberType GetRepetitionsNumber (void) { return (EncodingRepetitionsNumberType) m_repetitionsNumber; }
  void SetFecConcatenationFactor (uint8_t fecConcatenationFactor) { m_fecConcatenationFactor = fecConcatenationFactor; }
  uint8_t GetFecConcatenationFactor (void) { return m_fecConcatenationFactor; }
  void SetBandplan (uint8_t bandplan) { m_bandplan = bandplan; }
  uint8_t GetBandplan (void) { return m_bandplan; }
  void SetMapType (uint8_t mapType) { m_mapType = mapType; }
  uint8_t GetMapType (void) { return m_mapType; }
  void SetRmapIndication (uint8_t rmapIndication) { m_rmapIndication = rmapIndication; }
  uint8_t GetRmapIndication (void) { return m_rmapIndication; }
  void SetHopsNumber (uint8_t hopsNumber) { m_hopsNumber = hopsNumber; }
  uint8_t GetHopsNumber (void) { return m_hopsNumber; }


  virtual uint32_t GetSerializedSize (void) const;
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;

private:
  uint16_t m_mapFrameDuration;
  uint32_t m_networkTimeReference;
  uint32_t m_macCycleStartTime;
  uint16_t m_rcmSectionSize;
  uint16_t m_scramblerInitialization;
  uint8_t m_fecBlockSize;
  uint8_t m_repetitionsNumber;
  uint8_t m_fecConcatenationFactor;
  uint8_t m_bandplan;
  uint8_t m_mapType;
  uint8_t m_rmapIndication;
  uint8_t m_hopsNumber;
};


class GhnPlcPhyFrameHeaderCoreVariableMsg : public Header
{
public:
  static TypeId GetTypeId (void);

  GhnPlcPhyFrameHeaderCoreVariableMsg (void);
  ~GhnPlcPhyFrameHeaderCoreVariableMsg (void);

  void SetMsgFrameDuration (uint16_t msgFrameDuration) { m_msgFrameDuration = msgFrameDuration; }
  uint16_t GetMsgFrameDuration (void) { return m_msgFrameDuration; }
  void SetFecBlockSize (FecBlockSizeType fecBlockSize) { m_fecBlockSize = (uint8_t) fecBlockSize; }
  FecBlockSizeType GetFecBlockSize (void) { return (FecBlockSizeType) m_fecBlockSize; }
  void SetFecCodingRate (FecRateType fecCodingRate) { m_fecCodingRate = (uint8_t) fecCodingRate; }
  FecRateType GetFecCodingRate (void) { return (FecRateType) m_fecCodingRate; }
  void SetRepetitionsNumber (EncodingRepetitionsNumberType repetitionsNumber) { m_repetitionsNumber = (uint8_t) repetitionsNumber; }
  EncodingRepetitionsNumberType GetRepetitionsNumber (void) { return (EncodingRepetitionsNumberType) m_repetitionsNumber; }
  void SetFecConcatenationFactor (uint8_t fecConcatenationFactor) { m_fecConcatenationFactor = fecConcatenationFactor; }
  uint8_t GetFecConcatenationFactor (void) { return m_fecConcatenationFactor; }
  void SetScramblerInitialization (uint8_t scramblerInitialization) { m_scramblerInitialization = scramblerInitialization; }
  uint8_t GetScramblerInitialization (void) { return m_scramblerInitialization; }
  void SetMasterDetected (uint8_t masterDetected) { m_masterDetected = masterDetected; }
  uint8_t GetMasterDetected (void) { return m_masterDetected; }
  void SetBatIdentifier (uint8_t batIdentifier) { m_batIdentifier = batIdentifier; }
  uint8_t GetBatIdentifier (void) { return m_batIdentifier; }
  void SetBandplanIdentifier (uint8_t bandplanIdentifier) { m_bandplanIdentifier = bandplanIdentifier; }
  uint8_t GetBandplanIdentifier (void) { return m_bandplanIdentifier; }
  void SetGuardIntervalIdentifier (uint8_t guardIntervalIdentifier) { m_guardIntervalIdentifier = guardIntervalIdentifier; }
  uint8_t GetGuardIntervalIdentifier (void) { return m_guardIntervalIdentifier; }
  void SetPsdCeiling (uint8_t psdCeiling) { m_psdCeiling = psdCeiling; }
  uint8_t GetPsdCeiling (void) { return m_psdCeiling; }
  void SetConnectionIdentifier (uint8_t connectionIdentifier) { m_connectionIdentifier = connectionIdentifier; }
  uint8_t GetConnectionIdentifier (void) { return m_connectionIdentifier; }
  void SetReplyRequired (uint8_t replyRequired) { m_replyRequired = replyRequired; }
  uint8_t GetReplyRequired (void) { return m_replyRequired; }
  void SetBurstFrameCount (uint8_t burstFrameCount) { m_burstFrameCount = burstFrameCount; }
  uint8_t GetBurstFrameCount (void) { return m_burstFrameCount; }
  void SetBurstEndFlag (uint8_t burstEndFlag) { m_burstEndFlag = burstEndFlag; }
  uint8_t GetBurstEndFlag (void) { return m_burstEndFlag; }
  void SetAifgIndication (uint8_t aifgIndication) { m_aifgIndication = aifgIndication; }
  uint8_t GetAifgIndication (void) { return m_aifgIndication; }
  void SetAceSymbolsNumber (uint8_t aceSymbolsNumber) { m_aceSymbolsNumber = aceSymbolsNumber; }
  uint8_t GetAceSymbolsNumber (void) { return m_aceSymbolsNumber; }
  void SetConnectionManagement (uint8_t connectionManagement) { m_connectionManagement = connectionManagement; }
  uint8_t GetConnectionManagement (void) { return m_connectionManagement; }
  void SetBrurqStartSsn (uint16_t brurqStartSsn) { m_brurqStartSsn = brurqStartSsn; }
  uint16_t GetBrurqStartSsn (void) { return m_brurqStartSsn; }
  void SetCurrentTs (uint8_t currentTs) { m_currentTs = currentTs; }
  uint8_t GetCurrentTs (void) { return m_currentTs; }
  void SetRequestBidirectionalTransmission (uint8_t requestBidirectionalTransmission) { m_requestBidirectionalTransmission = requestBidirectionalTransmission; }
  uint8_t GetRequestBidirectionalTransmission (void) { return m_requestBidirectionalTransmission; }
  void SetMcAckSlotsNumber (uint8_t mcAckSlotsNumber) { m_mcAckSlotsNumber = mcAckSlotsNumber; }
  uint8_t GetMcAckSlotsNumber (void) { return m_mcAckSlotsNumber; }

  virtual uint32_t GetSerializedSize (void) const;
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;

private:
  uint16_t m_msgFrameDuration;
  uint8_t m_fecBlockSize;
  uint8_t m_fecCodingRate;
  uint8_t m_repetitionsNumber;
  uint8_t m_fecConcatenationFactor;
  uint8_t m_scramblerInitialization;
  uint8_t m_masterDetected;
  uint8_t m_batIdentifier;
  uint8_t m_bandplanIdentifier;
  uint8_t m_guardIntervalIdentifier;
  uint8_t m_psdCeiling;
  uint8_t m_connectionIdentifier;
  uint8_t m_replyRequired;
  uint8_t m_burstFrameCount;
  uint8_t m_burstEndFlag;
  uint8_t m_aifgIndication;
  uint8_t m_aceSymbolsNumber;
  uint8_t m_connectionManagement;
  uint16_t m_brurqStartSsn;
  uint8_t m_currentTs;
  uint8_t m_requestBidirectionalTransmission;
  uint8_t m_mcAckSlotsNumber;
};


class GhnPlcPhyFrameHeaderCoreVariableAck : public Header
{
public:
  static TypeId GetTypeId (void);

  GhnPlcPhyFrameHeaderCoreVariableAck (void);
  ~GhnPlcPhyFrameHeaderCoreVariableAck (void);

  void SetFlowControlConnectionFlag (uint8_t flowControlConnectionFlag) { m_flowControlConnectionFlag = flowControlConnectionFlag; }
  uint8_t GetFlowControlConnectionFlag (void) { return m_flowControlConnectionFlag; }
  void SetFlowControlType (uint8_t flowControlType) { m_flowControlType = flowControlType; }
  uint8_t GetFlowControlType (void) { return m_flowControlType; }
  void SetFlowControl (uint8_t flowControl) { m_flowControl = flowControl; }
  uint8_t GetFlowControl (void) { return m_flowControl; }
  void SetDataRxResetFlag (uint8_t dataRxResetFlag) { m_dataRxResetFlag = dataRxResetFlag; }
  uint8_t GetDataRxResetFlag (void) { return m_dataRxResetFlag; }
  void SetManagementRxResetFlag (uint8_t managementRxResetFlag) { m_managementRxResetFlag = managementRxResetFlag; }
  uint8_t GetManagementRxResetFlag (void) { return m_managementRxResetFlag; }
  void SetBadBurstIndication (uint8_t badBurstIndication) { m_badBurstIndication = badBurstIndication; }
  uint8_t GetBadBurstIndication (void) { return m_badBurstIndication; }
  void SetRequestBidirectionalTransmission (uint8_t requestBidirectionalTransmission) { m_requestBidirectionalTransmission = requestBidirectionalTransmission; }
  uint8_t GetRequestBidirectionalTransmission (void) { return m_requestBidirectionalTransmission; }
  void SetAckChannelEstimationReceiverWindowSize (uint8_t ackChannelEstimationReceiverWindowSize) { m_ackChannelEstimationReceiverWindowSize = ackChannelEstimationReceiverWindowSize; }
  uint8_t GetAckChannelEstimationReceiverWindowSize (void) { return m_ackChannelEstimationReceiverWindowSize; }
  void SetAcknowledgementDateAndMcAckDescriptor32 (uint32_t acknowledgementDateAndMcAckDescriptor32) { m_acknowledgementDateAndMcAckDescriptor32 = acknowledgementDateAndMcAckDescriptor32; }
  uint32_t GetAcknowledgementDateAndMcAckDescriptor32 (void) { return m_acknowledgementDateAndMcAckDescriptor32; }
  void SetAcknowledgementDateAndMcAckDescriptor64 (uint64_t acknowledgementDateAndMcAckDescriptor64) { m_acknowledgementDateAndMcAckDescriptor64 = acknowledgementDateAndMcAckDescriptor64; }
  uint64_t GetAcknowledgementDateAndMcAckDescriptor64 (void) { return m_acknowledgementDateAndMcAckDescriptor64; }

  virtual uint32_t GetSerializedSize (void) const;
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;

private:
  uint8_t m_flowControlConnectionFlag;
  uint8_t m_flowControlType;
  uint8_t m_flowControl;
  uint8_t m_dataRxResetFlag;
  uint8_t m_managementRxResetFlag;
  uint8_t m_badBurstIndication;
  uint8_t m_requestBidirectionalTransmission;
  uint8_t m_ackChannelEstimationReceiverWindowSize;
  //the following field is 96 bytes long and is
  //implemented as 64- and 32-bit variables
  uint32_t m_acknowledgementDateAndMcAckDescriptor32;
  uint64_t m_acknowledgementDateAndMcAckDescriptor64;
};


class GhnPlcPhyFrameHeaderCoreVariableRts : public Header
{
public:
  static TypeId GetTypeId (void);

  GhnPlcPhyFrameHeaderCoreVariableRts (void);
  ~GhnPlcPhyFrameHeaderCoreVariableRts (void);

  void SetRtsFrameDuration (uint16_t rtsFrameDuration) { m_rtsFrameDuration = rtsFrameDuration; }
  uint16_t GetRtsFrameDuration (void) { return m_rtsFrameDuration; }
  void SetCtsProxyId (uint8_t ctsProxyId) { m_ctsProxyId = ctsProxyId; }
  uint8_t GetCtsProxyId (void) { return m_ctsProxyId; }
  void SetCurrentTs (uint8_t currentTs) { m_currentTs = currentTs; }
  uint8_t GetCurrentTs (void) { return m_currentTs; }

  virtual uint32_t GetSerializedSize (void) const;
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;

private:
  uint16_t m_rtsFrameDuration;
  uint8_t m_ctsProxyId;
  uint8_t m_currentTs;
};


class GhnPlcPhyFrameHeaderCoreVariableCts : public Header
{
public:
  static TypeId GetTypeId (void);

  GhnPlcPhyFrameHeaderCoreVariableCts (void);
  ~GhnPlcPhyFrameHeaderCoreVariableCts (void);

  void SetCtsFrameDuration (uint16_t ctsFrameDuration) { m_ctsFrameDuration = ctsFrameDuration; }
  uint16_t GetCtsFrameDuration (void) { return m_ctsFrameDuration; }

  virtual uint32_t GetSerializedSize (void) const;
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;

private:
  uint16_t m_ctsFrameDuration;
};


class GhnPlcPhyFrameHeaderCoreVariableCtmg : public Header
{
public:
  static TypeId GetTypeId (void);

  GhnPlcPhyFrameHeaderCoreVariableCtmg (void);
  ~GhnPlcPhyFrameHeaderCoreVariableCtmg (void);

  void SetImmediateAcknowledgmentRequired (uint8_t immediateAcknowledgmentRequired) { m_immediateAcknowledgmentRequired = immediateAcknowledgmentRequired; }
  uint8_t GetImmediateAcknowledgmentRequired (void) { return m_immediateAcknowledgmentRequired; }
  void SetCurrentTs (uint8_t currentTs) { m_currentTs = currentTs; }
  uint8_t GetCurrentTs (void) { return m_currentTs; }
  void SetCtmgData64 (uint64_t ctmgData64) { m_ctmgData64 = ctmgData64; }
  uint64_t GetCtmgData64 (void) { return m_ctmgData64; }
  void SetCtmgData32 (uint32_t ctmgData32) { m_ctmgData32 = ctmgData32; }
  uint32_t GetCtmgData32 (void) { return m_ctmgData32; }
  void SetCtmgData16 (uint16_t ctmgData16) { m_ctmgData16 = ctmgData16; }
  uint16_t GetCtmgData16 (void) { return m_ctmgData16; }

  virtual uint32_t GetSerializedSize (void) const;
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;

private:
  uint8_t m_immediateAcknowledgmentRequired;
  uint8_t m_currentTs;
  //the following field is 112 bits long and is
  //implemented as 64-, 32-, and 16-bit variables
  uint64_t m_ctmgData64;
  uint32_t m_ctmgData32;
  uint16_t m_ctmgData16;
};


class GhnPlcPhyFrameHeaderCoreVariableProbe : public Header
{
public:
  static TypeId GetTypeId (void);

  GhnPlcPhyFrameHeaderCoreVariableProbe (void);
  ~GhnPlcPhyFrameHeaderCoreVariableProbe (void);

  void SetProbeFrameDuration (uint16_t probeFrameDuration) { m_probeFrameDuration = probeFrameDuration; }
  uint16_t GetProbeFrameDuration (void) { return m_probeFrameDuration; }
  void SetProbeFrameType (uint8_t probeFrameType) { m_probeFrameType = probeFrameType; }
  uint8_t GetProbeFrameType (void) { return m_probeFrameType; }
  void SetProbeSymbols (uint8_t probeSymbols) { m_probeSymbols = probeSymbols; }
  uint8_t GetProbeSymbols (void) { return m_probeSymbols; }
  void SetPsdCeiling (uint8_t psdCeiling) { m_psdCeiling = psdCeiling; }
  uint8_t GetPsdCeiling (void) { return m_psdCeiling; }
  void SetProbeGuardInterval (uint8_t probeGuardInterval) { m_probeGuardInterval = probeGuardInterval; }
  uint8_t GetProbeGuardInterval (void) { return m_probeGuardInterval; }
  void SetCurrentTs (uint8_t currentTs) { m_currentTs = currentTs; }
  uint8_t GetCurrentTs (void) { return m_currentTs; }
  void SetProbeFrameTypeSpecificField64 (uint64_t probeFrameTypeSpecificField64) { m_probeFrameTypeSpecificField64 = probeFrameTypeSpecificField64; }
  uint64_t GetProbeFrameTypeSpecificField64 (void) { return m_probeFrameTypeSpecificField64; }
  void SetProbeFrameTypeSpecificField8 (uint8_t probeFrameTypeSpecificField8) { m_probeFrameTypeSpecificField8 = probeFrameTypeSpecificField8; }
  uint8_t GetProbeFrameTypeSpecificField8 (void) { return m_probeFrameTypeSpecificField8; }

  virtual uint32_t GetSerializedSize (void) const;
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;

private:
  uint16_t m_probeFrameDuration;
  uint8_t m_probeFrameType;
  uint8_t m_probeSymbols;
  uint8_t m_psdCeiling;
  uint8_t m_probeGuardInterval;
  uint8_t m_currentTs;
  //the following field is 72 bytes long and is
  //implemented as 64- and 8-bit variables
  uint64_t m_probeFrameTypeSpecificField64;
  uint8_t m_probeFrameTypeSpecificField8;
};


class GhnPlcPhyFrameHeaderCoreVariableAckrq : public Header
{
public:
  static TypeId GetTypeId (void);

  GhnPlcPhyFrameHeaderCoreVariableAckrq (void);
  ~GhnPlcPhyFrameHeaderCoreVariableAckrq (void);

  void SetRequestedRxWindow (uint8_t requestedRxWindow) { m_requestedRxWindow = requestedRxWindow; }
  uint8_t GetRequestedRxWindow (void) { return m_requestedRxWindow; }
  void SetConnectionIdentifier (uint8_t connectionIdentifier) { m_connectionIdentifier = connectionIdentifier; }
  uint8_t GetConnectionIdentifier (void) { return m_connectionIdentifier; }
  void SetCurrentTs (uint8_t currentTs) { m_currentTs = currentTs; }
  uint8_t GetCurrentTs (void) { return m_currentTs; }

  virtual uint32_t GetSerializedSize (void) const;
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;

private:
  uint8_t m_requestedRxWindow;
  uint8_t m_connectionIdentifier;
  uint8_t m_currentTs;
};


class GhnPlcPhyFrameHeaderCoreVariableBmsg : public Header
{
public:
  static TypeId GetTypeId (void);

  GhnPlcPhyFrameHeaderCoreVariableBmsg (void);
  ~GhnPlcPhyFrameHeaderCoreVariableBmsg (void);

  void SetBmsgFrameDuration (uint16_t bmsgFrameDuration) { m_bmsgFrameDuration = bmsgFrameDuration; }
  uint16_t GetBmsgFrameDuration (void) { return m_bmsgFrameDuration; }
  void SetFecBlockSize (FecBlockSizeType fecBlockSize) { m_fecBlockSize = (uint8_t) fecBlockSize; }
  FecBlockSizeType GetFecBlockSize (void) { return (FecBlockSizeType) m_fecBlockSize; }
  void SetFecCodingRate (uint8_t fecCodingRate) { m_fecCodingRate = fecCodingRate; }
  uint8_t GetFecCodingRate (void) { return m_fecCodingRate; }
  void SetRepetitionsNumber (EncodingRepetitionsNumberType repetitionsNumber) { m_repetitionsNumber = (uint8_t) repetitionsNumber; }
  EncodingRepetitionsNumberType GetRepetitionsNumber (void) { return (EncodingRepetitionsNumberType) m_repetitionsNumber; }
  void SetFecConcatenationFactor (uint8_t fecConcatenationFactor) { m_fecConcatenationFactor = fecConcatenationFactor; }
  uint8_t GetFecConcatenationFactor (void) { return m_fecConcatenationFactor; }
  void SetScramblerInitialization (uint8_t scramblerInitialization) { m_scramblerInitialization = scramblerInitialization; }
  uint8_t GetScramblerInitialization (void) { return m_scramblerInitialization; }
  void SetMasterDetected (uint8_t masterDetected) { m_masterDetected = masterDetected; }
  uint8_t GetMasterDetected (void) { return m_masterDetected; }
  void SetBatIdentifier (uint8_t batIdentifier) { m_batIdentifier = batIdentifier; }
  uint8_t GetBatIdentifier (void) { return m_batIdentifier; }
  void SetBandplanIdentifier (uint8_t bandplanIdentifier) { m_bandplanIdentifier = bandplanIdentifier; }
  uint8_t GetBandplanIdentifier (void) { return m_bandplanIdentifier; }
  void SetGuardIntervalIdentifier (uint8_t guardIntervalIdentifier) { m_guardIntervalIdentifier = guardIntervalIdentifier; }
  uint8_t GetGuardIntervalIdentifier (void) { return m_guardIntervalIdentifier; }
  void SetPsdCeiling (uint8_t psdCeiling) { m_psdCeiling = psdCeiling; }
  uint8_t GetPsdCeiling (void) { return m_psdCeiling; }
  void SetConnectionIdentifier (uint8_t connectionIdentifier) { m_connectionIdentifier = connectionIdentifier; }
  uint8_t GetConnectionIdentifier (void) { return m_connectionIdentifier; }
  void SetReplyRequired (uint8_t replyRequired) { m_replyRequired = replyRequired; }
  uint8_t GetReplyRequired (void) { return m_replyRequired; }
  void SetBurstFrameCount (uint8_t burstFrameCount) { m_burstFrameCount = burstFrameCount; }
  uint8_t GetBurstFrameCount (void) { return m_burstFrameCount; }
  void SetBurstEndFlag (uint8_t burstEndFlag) { m_burstEndFlag = burstEndFlag; }
  uint8_t GetBurstEndFlag (void) { return m_burstEndFlag; }
  void SetAifgIndication (uint8_t aifgIndication) { m_aifgIndication = aifgIndication; }
  uint8_t GetAifgIndication (void) { return m_aifgIndication; }
  void SetAceSymbolsNumber (uint8_t aceSymbolsNumber) { m_aceSymbolsNumber = aceSymbolsNumber; }
  uint8_t GetAceSymbolsNumber (void) { return m_aceSymbolsNumber; }
  void SetConnectionManagement (uint8_t connectionManagement) { m_connectionManagement = connectionManagement; }
  uint8_t GetConnectionManagement (void) { return m_connectionManagement; }
  void SetBrurqStartSsn (uint16_t brurqStartSsn) { m_brurqStartSsn = brurqStartSsn; }
  uint16_t GetBrurqStartSsn (void) { return m_brurqStartSsn; }
  void SetCurrentTs (uint8_t currentTs) { m_currentTs = currentTs; }
  uint8_t GetCurrentTs (void) { return m_currentTs; }
  void SetBidirectionalTransmissionGrantLength (uint16_t bidirectionalTransmissionGrantLength) { m_bidirectionalTransmissionGrantLength = bidirectionalTransmissionGrantLength; }
  uint16_t GetBidirectionalTransmissionGrantLength (void) { return m_bidirectionalTransmissionGrantLength; }
  void SetBidirectionalTransmissionEndFlag (uint16_t bidirectionalTransmissionEndFlag) { m_bidirectionalTransmissionEndFlag = bidirectionalTransmissionEndFlag; }
  uint16_t GetBidirectionalTransmissionEndFlag (void) { return m_bidirectionalTransmissionEndFlag; }
  void SetAckChannelEstimationControl (uint8_t ackChannelEstimationControl) { m_ackChannelEstimationControl = ackChannelEstimationControl; }
  uint8_t GetAckChannelEstimationControl (void) { return m_ackChannelEstimationControl; }

  virtual uint32_t GetSerializedSize (void) const;
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;

private:
  uint16_t m_bmsgFrameDuration;
  uint8_t m_fecBlockSize;
  uint8_t m_fecCodingRate;
  uint8_t m_repetitionsNumber;
  uint8_t m_fecConcatenationFactor;
  uint8_t m_scramblerInitialization;
  uint8_t m_masterDetected;
  uint8_t m_batIdentifier;
  uint8_t m_bandplanIdentifier;
  uint8_t m_guardIntervalIdentifier;
  uint8_t m_psdCeiling;
  uint8_t m_connectionIdentifier;
  uint8_t m_replyRequired;
  uint8_t m_burstFrameCount;
  uint8_t m_burstEndFlag;
  uint8_t m_aifgIndication;
  uint8_t m_aceSymbolsNumber;
  uint8_t m_connectionManagement;
  uint16_t m_brurqStartSsn;
  uint8_t m_currentTs;
  uint16_t m_bidirectionalTransmissionGrantLength;
  uint16_t m_bidirectionalTransmissionEndFlag;
  uint8_t m_ackChannelEstimationControl;
};


class GhnPlcPhyFrameHeaderCoreVariableBack : public Header
{
public:
  static TypeId GetTypeId (void);

  GhnPlcPhyFrameHeaderCoreVariableBack (void);
  ~GhnPlcPhyFrameHeaderCoreVariableBack (void);

  void SetBackFrameDuration (uint16_t backFrameDuration) { m_backFrameDuration = backFrameDuration; }
  uint16_t GetBackFrameDuration (void) { return m_backFrameDuration; }
  void SetFecBlockSize (FecBlockSizeType fecBlockSize) { m_fecBlockSize = (uint8_t) fecBlockSize; }
  FecBlockSizeType GetFecBlockSize (void) { return (FecBlockSizeType) m_fecBlockSize; }
  void SetFecCodingRate (uint8_t fecCodingRate) { m_fecCodingRate = fecCodingRate; }
  uint8_t GetFecCodingRate (void) { return m_fecCodingRate; }
  void SetRepetitionsNumber (EncodingRepetitionsNumberType repetitionsNumber) { m_repetitionsNumber = (uint8_t) repetitionsNumber; }
  EncodingRepetitionsNumberType GetRepetitionsNumber (void) { return (EncodingRepetitionsNumberType) m_repetitionsNumber; }
  void SetFecConcatenationFactor (uint8_t fecConcatenationFactor) { m_fecConcatenationFactor = fecConcatenationFactor; }
  uint8_t GetFecConcatenationFactor (void) { return m_fecConcatenationFactor; }
  void SetScramblerInitialization (uint8_t scramblerInitialization) { m_scramblerInitialization = scramblerInitialization; }
  uint8_t GetScramblerInitialization (void) { return m_scramblerInitialization; }
  void SetMasterDetected (uint8_t masterDetected) { m_masterDetected = masterDetected; }
  uint8_t GetMasterDetected (void) { return m_masterDetected; }
  void SetBatIdentifier (uint8_t batIdentifier) { m_batIdentifier = batIdentifier; }
  uint8_t GetBatIdentifier (void) { return m_batIdentifier; }
  void SetBandplanIdentifier (uint8_t bandplanIdentifier) { m_bandplanIdentifier = bandplanIdentifier; }
  uint8_t GetBandplanIdentifier (void) { return m_bandplanIdentifier; }
  void SetGuardIntervalIdentifier (uint8_t guardIntervalIdentifier) { m_guardIntervalIdentifier = guardIntervalIdentifier; }
  uint8_t GetGuardIntervalIdentifier (void) { return m_guardIntervalIdentifier; }
  void SetPsdCeiling (uint8_t psdCeiling) { m_psdCeiling = psdCeiling; }
  uint8_t GetPsdCeiling (void) { return m_psdCeiling; }
  void SetConnectionIdentifier (uint8_t connectionIdentifier) { m_connectionIdentifier = connectionIdentifier; }
  uint8_t GetConnectionIdentifier (void) { return m_connectionIdentifier; }
  void SetReplyRequired (uint8_t replyRequired) { m_replyRequired = replyRequired; }
  uint8_t GetReplyRequired (void) { return m_replyRequired; }
  void SetBurstFrameCount (uint8_t burstFrameCount) { m_burstFrameCount = burstFrameCount; }
  uint8_t GetBurstFrameCount (void) { return m_burstFrameCount; }
  void SetBurstEndFlag (uint8_t burstEndFlag) { m_burstEndFlag = burstEndFlag; }
  uint8_t GetBurstEndFlag (void) { return m_burstEndFlag; }
  void SetAifgIndication (uint8_t aifgIndication) { m_aifgIndication = aifgIndication; }
  uint8_t GetAifgIndication (void) { return m_aifgIndication; }
  void SetAceSymbolsNumber (uint8_t aceSymbolsNumber) { m_aceSymbolsNumber = aceSymbolsNumber; }
  uint8_t GetAceSymbolsNumber (void) { return m_aceSymbolsNumber; }
  void SetConnectionManagement (uint8_t connectionManagement) { m_connectionManagement = connectionManagement; }
  uint8_t GetConnectionManagement (void) { return m_connectionManagement; }
  uint16_t GetBidirectionalTransmissionRequestLength (void) { return m_bidirectionalTransmissionRequestLength; }
  void SetAckChannelEstimationControl (uint8_t ackChannelEstimationControl) { m_ackChannelEstimationControl = ackChannelEstimationControl; }
  uint8_t GetAckChannelEstimationControl (void) { return m_ackChannelEstimationControl; }

  virtual uint32_t GetSerializedSize (void) const;
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;

private:
  uint16_t m_backFrameDuration;
  uint8_t m_fecBlockSize;
  uint8_t m_fecCodingRate;
  uint8_t m_repetitionsNumber;
  uint8_t m_fecConcatenationFactor;
  uint8_t m_scramblerInitialization;
  uint8_t m_masterDetected;
  uint8_t m_batIdentifier;
  uint8_t m_bandplanIdentifier;
  uint8_t m_guardIntervalIdentifier;
  uint8_t m_psdCeiling;
  uint8_t m_connectionIdentifier;
  uint8_t m_replyRequired;
  uint8_t m_burstFrameCount;
  uint8_t m_burstEndFlag;
  uint8_t m_aifgIndication;
  uint8_t m_aceSymbolsNumber;
  uint8_t m_connectionManagement;
  uint8_t m_bidirectionalTransmissionRequestLength;
  uint8_t m_ackChannelEstimationControl;
};


class GhnPlcPhyFrameHeaderCoreVariableActmg : public Header
{
public:
  static TypeId GetTypeId (void);

  GhnPlcPhyFrameHeaderCoreVariableActmg (void);
  ~GhnPlcPhyFrameHeaderCoreVariableActmg (void);

  void SetCtmgAcknowledgment (uint16_t ctmgAcknowledgment) { m_ctmgAcknowledgment = ctmgAcknowledgment; }
  uint16_t GetCtmgAcknowledgment (void) { return m_ctmgAcknowledgment; }

  virtual uint32_t GetSerializedSize (void) const;
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;

private:
  uint8_t m_ctmgAcknowledgment;
};


class GhnPlcPhyFrameHeaderCoreVariableFte : public Header
{
public:
  static TypeId GetTypeId (void);

  GhnPlcPhyFrameHeaderCoreVariableFte (void);
  ~GhnPlcPhyFrameHeaderCoreVariableFte (void);

  void SetFteFrameDuration (uint16_t fteFrameDuration) { m_fteFrameDuration = fteFrameDuration; }
  uint16_t GetFteFrameDuration (void) { return m_fteFrameDuration; }
  void SetCurrentTs (uint8_t currentTs) { m_currentTs = currentTs; }
  uint8_t GetCurrentTs (void) { return m_currentTs; }
  void SetEftSpecificFields64 (uint64_t eftSpecificFields64) { m_eftSpecificFields64 = eftSpecificFields64; }
  uint64_t GetEftSpecificFields64 (void) { return m_eftSpecificFields64; }
  void SetEftSpecificFields32 (uint32_t eftSpecificFields32) { m_eftSpecificFields32 = eftSpecificFields32; }
  uint32_t GetEftSpecificFields32 (void) { return m_eftSpecificFields32; }

  virtual uint32_t GetSerializedSize (void) const;
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;

private:
  uint16_t m_fteFrameDuration;
  uint8_t m_currentTs;
  //the following field is 96 bytes long and is
  //implemented as 64- and 32-bit variables
  uint64_t m_eftSpecificFields64;
  uint32_t m_eftSpecificFields32;
};


class GhnPlcPhyFrameHeaderCoreCommonHcs : public Header
{
public:
  static TypeId GetTypeId (void);

  GhnPlcPhyFrameHeaderCoreCommonHcs (void);
  ~GhnPlcPhyFrameHeaderCoreCommonHcs (void);

  void SetHeaderCheckSequence (uint16_t headerCheckSequence) { m_headerCheckSequence = headerCheckSequence; }
  uint16_t GetHeaderCheckSequence (void) { return m_headerCheckSequence; }

  virtual uint32_t GetSerializedSize (void) const;
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;

private:
  uint16_t m_headerCheckSequence;
};

struct GhnPlcOfdmParameters
{
    uint16_t m_subcarriersNumber;
    double m_subcarrierSpacing;
    uint16_t m_windowSize;
    uint16_t m_headerGuardInterval;
    uint16_t m_payloadDefaultGuardInterval;
    uint8_t m_sampleDuration;
    double m_fUs;
};

const GhnPlcOfdmParameters m_ofdmParameters[3] = {{1024, 24.4140625, 128, 256, 256, 40, 12.5},
                                                  {2048, 24.4140625, 256, 512, 512, 20, 25.0},
                                                  {4096, 24.4140625, 512, 1024, 1024, 10, 50.0}};



}
} // namespace ns3

#endif /* GHN_PLC_PHY_HEADER_H_ */
