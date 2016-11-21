/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 TUD
 *
 *  Created on: 25.06.2013
 *      Author: Stanislav Mudriievskyi <stanislav.mudriievskyi@tu-dresden.de>
 */

#ifndef GDOTHN_PHY_H_
#define GDOTHN_PHY_H_

#include "ns3/plc-phy.h"

#include "ghn-plc-phy-header.h"
#include "ghn-plc-phy-pma.h"
#include "ghn-plc-link-performance-model.h"

namespace ns3 {
namespace ghn {

class GhnPlcPhyPma;

class GhnPlcPhyPmdHalfD : public PLC_InformationRatePhy
{
public:
  static TypeId GetTypeId (void);
  GhnPlcPhyPmdHalfD ();
  virtual ~GhnPlcPhyPmdHalfD ();
  void SetBandplan (BandPlanType bandplan);
  BandPlanType GetBandplan (void);
  bool Send (Ptr<Packet> txPhyFrame, GhnPlcPhyFrameType frameType, uint8_t headerSymbols, uint32_t payloadSymbols);
  void SetPhyPma (Ptr<GhnPlcPhyPma> gdothnPhyPma);
  Ptr<GhnPlcPhyPma> GetPhyPma (void);
  virtual void EndRxHeader(uint32_t txId, Ptr<const SpectrumValue> rxPsd, Ptr<const PLC_TrxMetaInfo> metaInfo);
  double GetGatheredInformationBits(){return m_information_rate_model->GetGatheredInformationBits();}

  void EndRxPayload(Ptr<const PLC_TrxMetaInfo> metaInfo);
  uint32_t GetFrameSize(){return m_incommingFrameSize;}

protected:
  virtual bool DoStartTx (Ptr<const Packet> p);
  void PreparePhyFrame (Ptr<PLC_TrxMetaInfo> metaInfo);
  virtual void StartReception (uint32_t txId, Ptr<const SpectrumValue> rxPsd, Time duration, Ptr<const PLC_TrxMetaInfo> metaInfo);
  Ptr<GhnPlcLinkPerformanceModel> m_gdothnLinkPerformanceModel;

private:
  BandPlanType m_bandplan;
  Ptr<GhnPlcPhyPma> m_gdothnPhyPma;
  GhnPlcPhyFrameHeaderCoreCommon m_rxPhyHeaderCoreCommon;
  uint32_t m_incommingFrameSize;
};
}
} // namespace ns3

#endif /* GDOTHN_PHY_H_ */
