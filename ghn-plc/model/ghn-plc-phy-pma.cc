/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 TUD
 *
 *  Created on: 26.06.2013
 *      Author: Stanislav Mudriievskyi <stanislav.mudriievskyi@tu-dresden.de>
 */

#include "ns3/log.h"

#include "ghn-plc-phy-pma.h"

NS_LOG_COMPONENT_DEFINE ("GhnPlcPhyPma");

namespace ns3
{
namespace ghn {
NS_OBJECT_ENSURE_REGISTERED (GhnPlcPhyPma);

TypeId
GhnPlcPhyPma::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::GhnPlcPhyPma") .SetParent<Object> () .AddConstructor<GhnPlcPhyPma> ();
  return tid;
}

GhnPlcPhyPma::GhnPlcPhyPma ()
{
  NS_LOG_FUNCTION (this);
  for (uint8_t i = 0; i < 24; i++)
    for (uint16_t j = 0; j < 4096; j++)
      m_runtimeBats[i][j] = NOMOD;
  SetBatId (BAT_TYPE0);
}

GhnPlcPhyPma::~GhnPlcPhyPma ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
GhnPlcPhyPma::SetPhyManagement (Ptr<GhnPlcPhyManagement> ghnPhyManagement)
{
  m_ghnPhyManagement = ghnPhyManagement;
}

Ptr<GhnPlcPhyManagement>
GhnPlcPhyPma::GetPhyManagement (void)
{
  return m_ghnPhyManagement;
}

void
GhnPlcPhyPma::SetPhyPcs (Ptr<GhnPlcPhyPcs> ghnPhyPcs)
{
  m_ghnPhyPcs = ghnPhyPcs;
}

Ptr<GhnPlcPhyPcs>
GhnPlcPhyPma::GetPhyPcs (void)
{
  return m_ghnPhyPcs;
}

bool
GhnPlcPhyPma::StartTx (Ptr<Packet> txPhyFrame, GhnPlcPhyFrameType frameType)
{
  NS_LOG_FUNCTION_NOARGS ();
  NS_ASSERT(!m_sendCallback.IsNull());

  uint8_t ehi = m_ghnPhyManagement->GetTxExtendedHeaderIndication ();

  uint8_t headerSymbols = (1 + ehi) * (1 + m_ghnPhyManagement->GetTxHeaderSegmentationIndication ());

  switch (frameType)
    {
  case PHY_FRAME_MAP_RMAP:
    m_fecBlockSize = m_ghnPhyManagement->GetTxFecBlockSizeFromHeader ();
    m_payloadUncodedBits = m_ghnPhyManagement->GetTxPacketSize () * 8;
    m_kFecPayloadBlocks = m_payloadUncodedBits / m_fecBlockSize;
    m_payloadFecRate = FEC_RATE_1_2;
    m_repetitionsNumber = m_ghnPhyManagement->GetTxRepetitionsNumberFromHeader ();
    //    m_fecConcatenationFactorH = GetFecConcatenationFactorHFromHeader (header201.GetFecConcatenationFactor ());
    //    m_fecConcatenationFactorZ = GetFecConcatenationFactorZFromHeader (header201.GetFecConcatenationFactor ());
    m_payloadEncodedBits = GetPayloadEncodedBits (m_fecBlockSize, m_kFecPayloadBlocks, m_payloadFecRate, m_repetitionsNumber);
    m_payloadSymbols = ceil ((double) m_payloadEncodedBits / GetBitsPerSymbol ());
    NS_LOG_LOGIC ("MAP_RMAP Payload symbols: " << m_payloadSymbols);
    break;
  case PHY_FRAME_MSG:
    m_fecBlockSize = m_ghnPhyManagement->GetTxFecBlockSizeFromHeader ();
    m_payloadUncodedBits = m_ghnPhyManagement->GetTxPacketSize () * 8;
    m_kFecPayloadBlocks = m_payloadUncodedBits / m_fecBlockSize;
    m_payloadFecRate = m_ghnPhyManagement->GetTxPayloadFecRate ();
    m_repetitionsNumber = m_ghnPhyManagement->GetTxRepetitionsNumberFromHeader ();
    //    m_fecConcatenationFactorH = GetFecConcatenationFactorHFromHeader (header202.GetFecConcatenationFactor ());
    //    m_fecConcatenationFactorZ = GetFecConcatenationFactorZFromHeader (header202.GetFecConcatenationFactor ());
    m_payloadEncodedBits = GetPayloadEncodedBits (m_fecBlockSize, m_kFecPayloadBlocks, m_payloadFecRate, m_repetitionsNumber);
    m_payloadSymbols = ceil ((double) m_payloadEncodedBits / GetBitsPerSymbol ());
    NS_LOG_LOGIC ("MSG Payload symbols: " << m_payloadSymbols);
    break;
  case PHY_FRAME_ACK:
  case PHY_FRAME_RTS:
  case PHY_FRAME_CTS:
  case PHY_FRAME_CTMG:
  case PHY_FRAME_ACKRQ:
  case PHY_FRAME_ACTMG:
  case PHY_FRAME_FTE:
    m_payloadSymbols = 0;
    break;
  case PHY_FRAME_PROBE:
    //dot not use GetPayloadEncodedBytes() for this frame!!!
    break;
  case PHY_FRAME_BMSG:
    break;
  case PHY_FRAME_BACK:
    break;
    }

  return m_sendCallback (txPhyFrame, frameType, headerSymbols, m_payloadSymbols);
}

uint32_t
GhnPlcPhyPma::GetPayloadEncodedBits (uint16_t blockSize, uint16_t blocksNumber, FecRateType fecRate, uint8_t repetitionsNumber)
{
  uint32_t encodedBits;
  uint16_t fecOutputBlockSize = blockSize * 2 / 1; //default FEC rate
  uint16_t bitsPerSymbol = GetBitsPerSymbol ();

  switch (fecRate)
    {
  case FEC_RATE_1_2:
    fecOutputBlockSize = blockSize * 2 / 1;
    break;
  case FEC_RATE_2_3:
    fecOutputBlockSize = blockSize * 3 / 2;
    break;
  case FEC_RATE_5_6:
    fecOutputBlockSize = blockSize * 6 / 5;
    break;
  case FEC_RATE_16_18:
    fecOutputBlockSize = blockSize * 18 / 16;
    break;
  case FEC_RATE_20_21:
    fecOutputBlockSize = blockSize * 21 / 20;
    break;
    }

  uint32_t bitsPerSection;
  uint16_t sectionsNumber;

  if ((uint32_t) (floor ((double) bitsPerSymbol / repetitionsNumber)) % 4 == 0)
    bitsPerSection = (floor ((double) bitsPerSymbol / repetitionsNumber)) - 1;
  else
    bitsPerSection = (floor ((double) bitsPerSymbol / repetitionsNumber));

  sectionsNumber = ceil ((double) fecOutputBlockSize / bitsPerSection);

  NS_LOG_DEBUG ("Repetition number: " << (uint32_t)repetitionsNumber << ", FEC rate: " << fecRate << ", FEC output block size: " << fecOutputBlockSize
          << ", number of blocks: " << blocksNumber << ", number of sections: " << sectionsNumber << ", bits per section: " << bitsPerSection);

  if (repetitionsNumber == 1)
    encodedBits = fecOutputBlockSize * blocksNumber;
  else
    encodedBits = sectionsNumber * bitsPerSection * repetitionsNumber * blocksNumber;

  NS_LOG_LOGIC ("Encoded bits: " << encodedBits);

  return encodedBits;
}

uint16_t
GhnPlcPhyPma::GetBitsPerSymbol ()
{
  NS_ASSERT(!m_getPmcScheme.IsNull());
  uint16_t bitsPerSymbol;
  uint16_t subcarriers;
  if (m_bandPlan == GDOTHN_BANDPLAN_100MHZ)
    subcarriers = m_ofdmParameters[m_bandPlan].m_subcarriersNumber - PERMANENTLY_MASKED_SUBCARRIERS
            - BAND_80_100_MHZ_SUBCARRIERS;
  else
    subcarriers = m_ofdmParameters[m_bandPlan].m_subcarriersNumber - PERMANENTLY_MASKED_SUBCARRIERS;

  //n-bit per subcarrier loading is used here as default. To be extended and implemented with BAT.
  auto mcs = m_getPmcScheme();
  bitsPerSymbol = (mcs.use_bat) ? mcs.bat.get_total_bits () : subcarriers * ((uint16_t) mcs.mt);
  return bitsPerSymbol;
}

void
GhnPlcPhyPma::SetBatId (uint8_t batId)
{
  m_batId = batId;
}
uint8_t
GhnPlcPhyPma::GetBatId (void)
{
  return m_batId;
}

uint8_t
GhnPlcPhyPma::GetFecConcatenationFactorHFromHeader (uint8_t fecConcatenationFactor)
{
  switch (fecConcatenationFactor)
    {
  case 0:
    return 1;
    break;
  case 2:
  case 3:
    return 2;
    break;
  case 4:
  case 5:
  case 6:
  case 7:
    return 4;
    break;
    }
  NS_ASSERT_MSG(1, "Wrong FEC concatenation factor!");
  return 0;
}

uint8_t
GhnPlcPhyPma::GetFecConcatenationFactorZFromHeader (uint8_t fecConcatenationFactor)
{
  switch (fecConcatenationFactor)
    {
  case 0:
  case 2:
  case 4:
    return 0;
    break;
  case 3:
  case 5:
    return 1;
    break;
  case 6:
    return 2;
    break;
  case 7:
    return 3;
    break;
    }
  NS_ASSERT_MSG(1, "Wrong FEC concatenation factor!");
  return 0;
}
}
} // namespace ns3
