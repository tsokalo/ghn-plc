/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 TUD
 *
 *  Created on: 26.06.2013
 *      Author: Stanislav Mudriievskyi <stanislav.mudriievskyi@tu-dresden.de>
 */

#ifndef GHN_PLC_PHY_PMA_H_
#define GHN_PLC_PHY_PMA_H_

#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/callback.h"
#include "ns3/plc-defs.h"

#include "ghn-plc-phy-header.h"
#include "ghn-plc-phy-pcs.h"
#include "ghn-plc-phy-management.h"

#define PERMANENTLY_MASKED_SUBCARRIERS 75
#define BAND_80_100_MHZ_SUBCARRIERS 819

namespace ns3 {
namespace ghn {
class GhnPlcPhyManagement;
class GhnPlcPhyPcs;

typedef Callback<bool, Ptr<Packet>, GhnPlcPhyFrameType, uint8_t, uint32_t > SendCallback;
typedef Callback<ModulationAndCodingScheme> GetPmcScheme;

class GhnPlcPhyPma : public Object
{
public:
  static TypeId GetTypeId (void);
  GhnPlcPhyPma ();
  virtual ~GhnPlcPhyPma ();

  void SetPhyManagement (Ptr<GhnPlcPhyManagement> ghnPhyManagement);
  Ptr<GhnPlcPhyManagement> GetPhyManagement (void);
  void SetPhyPcs (Ptr<GhnPlcPhyPcs> ghnPhyPcs);
  Ptr<GhnPlcPhyPcs> GetPhyPcs (void);

  bool StartTx (Ptr<Packet> txPhyFrame, GhnPlcPhyFrameType frameType);

  uint16_t GetBitsPerSymbol ();
  void SetBatId (uint8_t batId);
  uint8_t GetBatId (void);

  //concatenation of FEC codewords is not implemented here
  uint32_t GetPayloadEncodedBits (uint16_t blockSize, uint16_t blocksNumber, FecRateType fecRate, uint8_t repetitionsNumber);

  void SetBandPlanType(BandPlanType bp){m_bandPlan = bp;}
  void SetSendCallback(SendCallback cb){m_sendCallback = cb;}
  void SetGetPmcScheme(GetPmcScheme cb){m_getPmcScheme = cb;}

private:
  uint8_t GetFecConcatenationFactorHFromHeader (uint8_t fecConcatenationFactor);
  uint8_t GetFecConcatenationFactorZFromHeader (uint8_t fecConcatenationFactor);

  uint32_t m_payloadSymbols;
  uint16_t m_fecBlockSize;
  uint32_t m_kFecPayloadBlocks;
  FecRateType m_payloadFecRate;
  uint32_t m_payloadEncodedBits;
  uint32_t m_payloadUncodedBits;
  uint8_t m_repetitionsNumber;

  Ptr<GhnPlcPhyManagement> m_ghnPhyManagement;
  Ptr<GhnPlcPhyPcs> m_ghnPhyPcs;

  uint8_t m_fecConcatenationFactorH;
  uint8_t m_fecConcatenationFactorZ;
  uint8_t m_batId;
  ModulationType m_runtimeBats[24][4096];
  BandPlanType m_bandPlan;

  SendCallback m_sendCallback;
  GetPmcScheme m_getPmcScheme;
};
}
} // namespace ns3

#endif /* GHN_PLC_PHY_PMA_H_ */
