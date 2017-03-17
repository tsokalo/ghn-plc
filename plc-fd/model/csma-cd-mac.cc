/*
 * CsmaCdMac.cpp
 *
 *  Created on: 08.09.2016
 *      Author: tsokalo
 */
#include "ns3/type-id.h"
#include "ns3/plc-mac.h"
#include "ns3/log.h"
#include "ns3/random-variable-stream.h"
#include "ns3/simulator.h"
#include "csma-cd-mac.h"
NS_LOG_COMPONENT_DEFINE ("CsmaCdMac");

namespace ns3
{
TypeId
CsmaCdMac::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CsmaCdMac") .SetParent<PLC_ArqMac> () .AddConstructor<CsmaCdMac> ();
  return tid;
}

CsmaCdMac::CsmaCdMac ()
{

}

CsmaCdMac::~CsmaCdMac ()
{

}

void
CsmaCdMac::CollisionDetection ()
{
  NS_LOG_FUNCTION (this);

  NS_LOG_DEBUG ("Collision detected");
  if (m_ackPacket != 0 && m_txQueue->IsEmpty())
    {
      StartCsmaCa ();
    }
  else
    {
      NS_LOG_LOGIC ("Do not contend for next transmission since we have not contended before");
    }
}
uint64_t
CsmaCdMac::GetBackoffSlots ()
{
  NS_LOG_FUNCTION (this);

  Ptr<UniformRandomVariable> uniformVar = CreateObject<UniformRandomVariable> ();
  uint64_t backoffPeriod;

  backoffPeriod = (uint64_t) uniformVar->GetValue (m_macMinBE, m_macMaxBE); //num backoff periods
  NS_LOG_LOGIC ("CSMA/CA backoff period /slots: " << backoffPeriod);

  return backoffPeriod;
}

}
