/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 TUD
 *
 *  Created on: 25.08.2015
 *      Author: Stanislav Mudriievskyi <stanislav.mudriievskyi@tu-dresden.de>
 */
#include <array>
#include <chrono>

#include "ns3/log.h"

#include "ghn-plc-phy-management.h"

NS_LOG_COMPONENT_DEFINE("GhnPlcPhyManagement");

namespace ns3
{
namespace ghn
{

NS_OBJECT_ENSURE_REGISTERED (GhnPlcPhyManagement);

TypeId
GhnPlcPhyManagement::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::GhnPlcPhyManagement").SetParent<Object> ().AddConstructor<GhnPlcPhyManagement> ();
  return tid;
}

GhnPlcPhyManagement::GhnPlcPhyManagement () :
        m_gen (m_rd ())
{
  NS_LOG_FUNCTION_NOARGS ();

  SetTxMulticastIndication (0);
  SetTxHeaderSegmentationIndication (0);
  SetTxExtendedHeaderIndication (0);
  SetTxFecBlockSize (FEC_BLOCK_SIZE_120);
  SetTxPayloadFecRate (FEC_RATE_1_2);
  SetTxRepetitionsNumber (ENCODING_REPETITIONS_1);
  m_rxTime = NanoSeconds (0);
  m_rxAckReceived = false;
}

GhnPlcPhyManagement::~GhnPlcPhyManagement ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
GhnPlcPhyManagement::SetPhyPcs (Ptr<GhnPlcPhyPcs> ghnPhyPcs)
{
  m_ghnPhyPcs = ghnPhyPcs;
}

Ptr<GhnPlcPhyPcs>
GhnPlcPhyManagement::GetPhyPcs (void)
{
  return m_ghnPhyPcs;
}

void
GhnPlcPhyManagement::SetPhyPma (Ptr<GhnPlcPhyPma> ghnPhyPma)
{
  m_ghnPhyPma = ghnPhyPma;
}

Ptr<GhnPlcPhyPma>
GhnPlcPhyManagement::GetPhyPma (void)
{
  return m_ghnPhyPma;
}

void
GhnPlcPhyManagement::SetTxPacketSize (uint32_t txPacketSize)
{
  m_txPacketSize = txPacketSize;
}
uint32_t
GhnPlcPhyManagement::GetTxPacketSize (void)
{
  return m_txPacketSize;
}

void
GhnPlcPhyManagement::SetTxMulticastIndication (uint8_t txMulticastIndication)
{
  m_txMulticastIndication = txMulticastIndication;
}

uint8_t
GhnPlcPhyManagement::GetTxMulticastIndication (void)
{
  return m_txMulticastIndication;
}

void
GhnPlcPhyManagement::SetTxHeaderSegmentationIndication (uint8_t txHeaderSegmentationIndication)
{
  m_txHeaderSegmentationIndication = txHeaderSegmentationIndication;
}

uint8_t
GhnPlcPhyManagement::GetTxHeaderSegmentationIndication (void)
{
  return m_txHeaderSegmentationIndication;
}

void
GhnPlcPhyManagement::SetTxExtendedHeaderIndication (uint8_t txExtendedHeaderIndication)
{
  m_txExtendedHeaderIndication = txExtendedHeaderIndication;
}

uint8_t
GhnPlcPhyManagement::GetTxExtendedHeaderIndication (void)
{
  return m_txExtendedHeaderIndication;
}

void
GhnPlcPhyManagement::SetTxFecBlockSize (FecBlockSizeType txFecBlockSize)
{
  m_txFecBlockSize = txFecBlockSize;
}

FecBlockSizeType
GhnPlcPhyManagement::GetTxFecBlockSize (void)
{
  return m_txFecBlockSize;
}

void
GhnPlcPhyManagement::SetTxPayloadFecRate (FecRateType txPayloadFecRate)
{
  m_txPayloadFecRate = txPayloadFecRate;
}

FecRateType
GhnPlcPhyManagement::GetTxPayloadFecRate (void)
{
  return m_txPayloadFecRate;
}

void
GhnPlcPhyManagement::SetTxRepetitionsNumber (EncodingRepetitionsNumberType txRepetitionsNumber)
{
  m_txRepetitionsNumber = txRepetitionsNumber;
}

EncodingRepetitionsNumberType
GhnPlcPhyManagement::GetTxRepetitionsNumber (void)
{
  return m_txRepetitionsNumber;
}

void
GhnPlcPhyManagement::SetTxFrameDuration (uint16_t txFrameDuration)
{
  m_txFrameDuration = txFrameDuration;
}

uint16_t
GhnPlcPhyManagement::GetTxFrameDuration (void)
{
  return m_txFrameDuration;
}

void
GhnPlcPhyManagement::SetTxReplyRequired (uint8_t replyRequired)
{
  m_txReplyRequired = replyRequired;
}

uint8_t
GhnPlcPhyManagement::GetTxReplyRequired (void)
{
  return m_txReplyRequired;
}

void
GhnPlcPhyManagement::SetRxFrameDuration (uint16_t rxFrameDuration)
{
  m_rxFrameDuration = rxFrameDuration;
}

uint16_t
GhnPlcPhyManagement::GetRxFrameDuration (void)
{
  return m_rxFrameDuration;
}

void
GhnPlcPhyManagement::SetRxTime (Time rxTime)
{
  m_rxTime = rxTime;
}
Time
GhnPlcPhyManagement::GetRxTime (void)
{
  return m_rxTime;
}

void
GhnPlcPhyManagement::SetRxReplyRequired (uint8_t replyRequired)
{
  m_rxReplyRequired = replyRequired;
}

uint8_t
GhnPlcPhyManagement::GetRxReplyRequired (void)
{
  return m_rxReplyRequired;
}

void
GhnPlcPhyManagement::SetTxDomainId (uint8_t txDomainId)
{
  m_txDomainId = txDomainId;
}

uint8_t
GhnPlcPhyManagement::GetTxDomainId (void)
{
  return m_txDomainId;
}

void
GhnPlcPhyManagement::SetTxSourceId (uint8_t txSourceId)
{
  m_txSourceId = txSourceId;
}

uint8_t
GhnPlcPhyManagement::GetTxSourceId (void)
{
  return m_txSourceId;
}

void
GhnPlcPhyManagement::SetTxDestinationId (uint8_t txDestinationId)
{
  m_txDestinationId = txDestinationId;
}

uint8_t
GhnPlcPhyManagement::GetTxDestinationId (void)
{
  return m_txDestinationId;
}

void
GhnPlcPhyManagement::SetTxCtmgData16 (uint16_t txCtmgData16)
{
  m_txCtmgData16 = txCtmgData16;
}

uint16_t
GhnPlcPhyManagement::GetTxCtmgData16 (void)
{
  return m_txCtmgData16;
}

void
GhnPlcPhyManagement::SetRxAckReceived (bool ackReceived)
{
  m_rxAckReceived = ackReceived;
}

bool
GhnPlcPhyManagement::GetRxAckReceived (void)
{
  return m_rxAckReceived;
}

void
GhnPlcPhyManagement::SetTxConnectionIdentifier (uint8_t txConnectionIdentifier)
{
  m_txConnectionIdentifier = txConnectionIdentifier;
}

uint8_t
GhnPlcPhyManagement::GetTxConnectionIdentifier (void)
{
  return m_txConnectionIdentifier;
}

void
GhnPlcPhyManagement::SetRxConnectionIdentifier (uint8_t rxConnectionIdentifier)
{
  m_rxConnectionIdentifier = rxConnectionIdentifier;
}

uint8_t
GhnPlcPhyManagement::GetRxConnectionIdentifier (void)
{
  return m_rxConnectionIdentifier;
}

void
GhnPlcPhyManagement::SetRxDomainId (uint8_t rxDomainId)
{
  m_rxDomainId = rxDomainId;
}

uint8_t
GhnPlcPhyManagement::GetRxDomainId (void)
{
  return m_rxDomainId;
}

void
GhnPlcPhyManagement::SetRxCtmgData16 (uint16_t rxCtmgData16)
{
  m_rxCtmgData16 = rxCtmgData16;
}

uint16_t
GhnPlcPhyManagement::GetRxCtmgData16 (void)
{
  return m_rxCtmgData16;
}

uint16_t
GhnPlcPhyManagement::GetTxFecBlockSizeFromHeader (void)
{
  switch (m_txFecBlockSize)
    {
  case FEC_BLOCK_SIZE_120:
    return 120 * 8;
    break;
  case FEC_BLOCK_SIZE_540:
    return 540 * 8;
    break;
    }
  NS_ASSERT_MSG (0, "Wrong FEC block size!");
  return 0;
}

uint8_t
GhnPlcPhyManagement::GetTxRepetitionsNumberFromHeader (void)
{
  if (m_txRepetitionsNumber < 5)
    return (uint8_t) m_txRepetitionsNumber;
  else if (m_txRepetitionsNumber == ENCODING_REPETITIONS_6)
    return 6;
  else
    return 8;
}

Time
GhnPlcPhyManagement::GetTxTime (uint32_t mpduLength, uint8_t sourceId, uint8_t destinationId)
{
  uint32_t payloadUncodedBits = mpduLength * 8;
  uint16_t fecBlockSize = GetTxFecBlockSizeFromHeader ();
  uint32_t kFecPayloadBlocks = payloadUncodedBits / fecBlockSize;
  uint8_t repetitionsNumber = GetTxRepetitionsNumberFromHeader ();
  uint32_t payloadEncodedBits = m_ghnPhyPma->GetPayloadEncodedBits (fecBlockSize, kFecPayloadBlocks, m_txPayloadFecRate,
          repetitionsNumber);
  uint32_t payloadSymbols = ceil ((double) payloadEncodedBits / m_ghnPhyPma->GetBitsPerSymbol ());
  return Time::FromInteger (
          (payloadSymbols + m_txHeaderSegmentationIndication + 1) * PLC_Phy::GetSymbolDuration ().GetInteger ()
                  + PLC_Preamble::GetDuration ().GetInteger (), Time::GetResolution ());
}

uint32_t
GhnPlcPhyManagement::GetDataAmount (Time txTime, uint8_t sourceId, uint8_t destinationId)
{
  uint32_t payloadSymbols = (txTime.GetInteger ()
          - (m_txHeaderSegmentationIndication + 1) * (PLC_Phy::GetHeaderSymbolDuration ()).GetInteger ()
          - PLC_Preamble::GetDuration ().GetInteger ()) / (PLC_Phy::GetSymbolDuration ()).GetInteger ();
  uint32_t payloadEncodedBits = payloadSymbols * m_ghnPhyPma->GetBitsPerSymbol ();
  uint8_t repetitionsNumber = GetTxRepetitionsNumberFromHeader ();
  uint32_t payloadUncodedBits = payloadEncodedBits / repetitionsNumber;
  switch (m_txPayloadFecRate)
    {
  case FEC_RATE_1_2:
    payloadUncodedBits = payloadUncodedBits * 1 / 2;
    break;
  case FEC_RATE_2_3:
    payloadUncodedBits = payloadUncodedBits * 2 / 3;
    break;
  case FEC_RATE_5_6:
    payloadUncodedBits = payloadUncodedBits * 5 / 6;
    break;
  case FEC_RATE_16_18:
    payloadUncodedBits = payloadUncodedBits * 16 / 18;
    break;
  case FEC_RATE_16_21:
    payloadUncodedBits = payloadUncodedBits * 16 / 21;
    break;
  case FEC_RATE_20_21:
    payloadUncodedBits = payloadUncodedBits * 20 / 21;
    break;
    }
  return payloadUncodedBits;
}

uint32_t
GhnPlcPhyManagement::GetDatarate (uint8_t sourceId, uint8_t destinationId)
{
  return (double) m_ghnPhyPma->GetBitsPerSymbol () / (PLC_Phy::GetSymbolDuration ()).GetSeconds ();
}

bool
GhnPlcPhyManagement::IsBlockSuccess ()
{
  NS_ASSERT (!m_frameSizeCallback.IsNull () && !m_gatheredInfBitsCallback.IsNull ());
  std::array<double, 2> block_size =
    { 120, 540 };
  uint32_t bs = block_size.at (m_txFecBlockSize) * 8;

  std::array<double, 7> fec_rate =
    { 1, 1.0 / 2.0, 2.0 / 3.0, 5.0 / 6.0, 16.0 / 18.0, 16.0 / 21.0, 20.0 / 21.0 };
  double fc = fec_rate.at (m_txPayloadFecRate);

  uint32_t frame_size = m_frameSizeCallback ();
  uint32_t gathered_bits = m_gatheredInfBitsCallback ();
  uint32_t sent_bits = (double) frame_size / fc;
  double ber = (gathered_bits > sent_bits) ? 0 : (double) (sent_bits - gathered_bits) / (double) sent_bits;
  std::binomial_distribution<uint32_t> m_binomDistr (bs / fc, 1 - ber);
  uint32_t gathered_bits_rand = m_binomDistr (m_gen);


  NS_LOG_UNCOND(
          "Frame size: " << frame_size << ", FEC rate: " << fc << ", Sent bits: " << sent_bits << ", Gathered bits: " << gathered_bits);
  NS_LOG_UNCOND(
          "BER: " << ber << ", BS: " << bs << ", Gathered rand bits: " << gathered_bits_rand << ", ret: " << (gathered_bits_rand >= bs));

  return (gathered_bits_rand >= bs);
}
}
} // namespace ns3
