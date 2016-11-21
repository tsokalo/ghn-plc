/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 TUD
 *
 *  Created on: 25.06.2013
 *      Author: Stanislav Mudriievskyi <stanislav.mudriievskyi@tu-dresden.de>
 */

#ifndef GHN_PLC_PHY_PMD_H_
#define GHN_PLC_PHY_PMD_H_

#include "ns3/plc-inf-rate-fd-phy.h"

#include "ghn-plc-phy-header.h"
#include "ghn-plc-phy-pma.h"

namespace ns3 {
namespace ghn {

class GhnPlcPhyPma;

class GhnPlcPhyPmdFullD : public PLC_InfRateFDPhy
{
public:
  static TypeId GetTypeId (void);
  GhnPlcPhyPmdFullD ();
  virtual ~GhnPlcPhyPmdFullD ();
  void SetBandplan (BandPlanType bandplan);
  BandPlanType GetBandplan (void);
  bool Send (Ptr<Packet> txPhyFrame, GhnPlcPhyFrameType frameType, uint8_t headerSymbols, uint32_t payloadSymbols);
  void SetPhyPma (Ptr<GhnPlcPhyPma> ghnPhyPma);
  Ptr<GhnPlcPhyPma> GetPhyPma (void);
  virtual void EndRxHeader(uint32_t txId, Ptr<const SpectrumValue> rxPsd, Ptr<const PLC_TrxMetaInfo> metaInfo);
  double GetGatheredInformationBits(){return m_information_rate_model->GetGatheredInformationBits();}

  void EndRxPayload(Ptr<const PLC_TrxMetaInfo> metaInfo);
  uint32_t GetFrameSize(){return m_incommingFrameSize;}

protected:
//  virtual bool DoStartTx (Ptr<const Packet> p);
//  void PreparePhyFrame (Ptr<PLC_TrxMetaInfo> metaInfo);
  virtual void StartReception (uint32_t txId, Ptr<const SpectrumValue> rxPsd, Time duration, Ptr<const PLC_TrxMetaInfo> metaInfo);

private:
  BandPlanType m_bandplan;
  Ptr<GhnPlcPhyPma> m_ghnPhyPma;
  GhnPlcPhyFrameHeaderCoreCommon m_rxPhyHeaderCoreCommon;
  uint32_t m_incommingFrameSize;
};
}
} // namespace ns3

#endif /* GHN_PLC_PHY_PMD_H_ */
