/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 TUD
 *
 *  Created on: 01.12.2015
 *      Author: Stanislav Mudriievskyi <stanislav.mudriievskyi@tu-dresden.de>
 */

#ifndef GHN_PLC_MAC_BACKOFF_H
#define GHN_PLC_MAC_BACKOFF_H

#include <stdint.h>
#include "ns3/nstime.h"
#include "ns3/random-variable-stream.h"
#include "ns3/object.h"

namespace ns3 {
namespace ghn {

  /**
   * \brief The backoff class is used for calculating backoff times
   * when many net devices can write to the same channel
   *
   */

#define INTEGRATION_TIMES 10
#define INTEGRATION_NODES 200

class GhnPlcMacBackoff : public Object {
public:
  /**
   * Minimum number of backoff slots (when multiplied by m_slotTime, determines minimum backoff time)
   */
  uint32_t m_minSlots;

  /**
   * Maximum number of backoff slots (when multiplied by m_slotTime, determines maximum backoff time)
   */
  uint32_t m_maxSlots;

  /**
   * Length of one slot. A slot time, it usually the packet transmission time, if the packet size is fixed.
   */
  Time m_slotTime;

  double m_v;

  double m_Cw;

  uint32_t m_N;

  double m_vOld;

  double m_vEst;

  //for polynomial backoff
  bool m_polynomial;

  uint32_t m_nCollisions;

  uint32_t m_minCwPolynomial;

  double m_beta;

  uint32_t m_maxNCollisions;

  //for new algorithm
  uint32_t m_Integration[500];
  uint32_t m_curInt;
  uint32_t m_IntegrationNodes[500];
  uint32_t m_curIntNodes;


  GhnPlcMacBackoff (void);
  ~GhnPlcMacBackoff (void);
  GhnPlcMacBackoff (Time slotTime, uint32_t minSlots, uint32_t maxSlots);

  static TypeId GetTypeId (void);

  /**
   * \return The amount of time that the net device should wait before
   * trying to retransmit the packet
   */
  Time GetBackoffTime();

  void RecalculateCw (Time backoffTime);

  uint32_t m_integrationTimesNumber;
  uint32_t m_integrationNodesNumber;

//  void SetCw (uint32_t cW);

private:
  Ptr<UniformRandomVariable> m_rng;
};
}
} // namespace ns3

#endif /* GHN_PLC_MAC_BACKOFF_H */
