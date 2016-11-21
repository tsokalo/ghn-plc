/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 TUD
 *
 *  Created on: 26.06.2013
 *      Author: Stanislav Mudriievskyi <stanislav.mudriievskyi@tu-dresden.de>
 */

#ifndef GHN_PLC_PHY_PCS_H_
#define GHN_PLC_PHY_PCS_H_

#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/uan-address.h"
#include "ns3/traced-callback.h"
#include "ns3/nstime.h"

#include "ghn-plc-phy-header.h"
#include "ghn-plc-phy-pma.h"
#include "ghn-plc-phy-management.h"

namespace ns3 {
namespace ghn {
class GhnPlcPhyManagement;
class GhnPlcPhyPma;

class GhnPlcPhyPcs : public Object
{
public:
  static TypeId GetTypeId (void);
  GhnPlcPhyPcs ();
  virtual ~GhnPlcPhyPcs ();

  void SetPhyManagement (Ptr<GhnPlcPhyManagement> ghnPhyManagement);
  Ptr<GhnPlcPhyManagement> GetPhyManagement (void);
  void SetPhyPma (Ptr<GhnPlcPhyPma> ghnPhyPmd);
  Ptr<GhnPlcPhyPma> GetPhyPma (void);

  void SetCurrentTs (uint8_t currentTs);
  uint8_t GetCurrentTs (void);

  bool StartTx (GhnPlcPhyFrameType frameType, Ptr<const Packet> mpdu);
  void ReceiveSuccess (Ptr<const Packet> txPhyFrame, uint16_t msgId);
  void ReceiveFailure ();

  typedef Callback<bool, GhnPlcPhyFrameType, Ptr<Packet>, const UanAddress &, const UanAddress &> MpduForwardUpCallback;
  void SetMpduForwardUpCallback (MpduForwardUpCallback cb);

  uint32_t
  GetDataAmount (Time txTime, uint8_t sourceId, uint8_t destinationId);

private:
  Ptr<GhnPlcPhyPma> m_ghnPhyPma;
  Ptr<GhnPlcPhyManagement> m_ghnPhyManagement;

  uint8_t m_currentTs;

  Ptr<Packet> m_rxPacket;

  TracedCallback<uint32_t> m_phyRcvFailureLog;
  TracedCallback<uint32_t> m_phyRcvSuccessLog;

protected:
  MpduForwardUpCallback m_forwardUp;
};
}
} // namespace ns3

#endif /* GHN_PLC_PHY_PCS_H_ */
