/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 TUD
 *
 *  Created on: 25.08.2015
 *      Author: Stanislav Mudriievskyi <stanislav.mudriievskyi@tu-dresden.de>
 */

#include "ns3/log.h"
#include "ns3/simulator.h"
#include <ns3/object-factory.h>

#include "ghn-plc-dll-management.h"

NS_LOG_COMPONENT_DEFINE ("GhnPlcDllManagement");

namespace ns3
{
namespace ghn {
NS_OBJECT_ENSURE_REGISTERED (GhnPlcDllManagement);

TypeId
GhnPlcDllManagement::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::GhnPlcDllManagement") .SetParent<Object> () .AddConstructor<GhnPlcDllManagement> ();
  return tid;
}

GhnPlcDllManagement::GhnPlcDllManagement ()
{
  NS_LOG_FUNCTION (this);

  m_nNodes = 1;
  m_rho = 0;
}

GhnPlcDllManagement::~GhnPlcDllManagement ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
GhnPlcDllManagement::SetDllApc (Ptr<GhnPlcDllApc> ncDllApc)
{
  m_ncDllApc = ncDllApc;
}

Ptr<GhnPlcDllApc>
GhnPlcDllManagement::GetDllApc (void)
{
  return m_ncDllApc;
}

void
GhnPlcDllManagement::SetDllLlc (Ptr<GhnPlcDllLlc> ncDllLlc)
{
  m_ncDllLlc = ncDllLlc;

  m_ncDllMacCsma->SetDllLlc (ncDllLlc);
  m_ncDllMacCsma->SetLpduForwardUpCallback (MakeCallback (&GhnPlcDllLlc::Receive, m_ncDllLlc));

  m_ncDllLlc->SetDllMac (m_ncDllMacCsma);
  m_ncDllLlc->SetTriggerSendCallback (MakeCallback (&GhnPlcDllMac::TriggerSend, m_ncDllMacCsma));
}

Ptr<GhnPlcDllLlc>
GhnPlcDllManagement::GetDllLlc (void)
{
  return m_ncDllLlc;
}

void
GhnPlcDllManagement::SetDllMac (Ptr<GhnPlcDllMacCsma> ncDllMac)
{
  m_ncDllMacCsma = ncDllMac;
}

Ptr<GhnPlcDllMacCsma>
GhnPlcDllManagement::GetDllMac (void)
{
  return m_ncDllMacCsma;
}
void
GhnPlcDllManagement::CreateDllMac ()
{
  ObjectFactory macFactory;
  macFactory.SetTypeId (m_macTid);
  m_ncDllMacCsma = macFactory.Create<GhnPlcDllMacCsma> ();
  if (m_macTid == GhnPlcDllMacCsmaCd::GetTypeId ())
    {
      m_ncDllMacCsma->SetBackoffValueCallback (MakeCallback (&GhnPlcDllMacCsmaCd::GetBackoffSlots, (StaticCast<GhnPlcDllMacCsmaCd,
              GhnPlcDllMacCsma> (m_ncDllMacCsma))));
    }
  else if (m_macTid == GhnPlcDllMacCsma::GetTypeId ())
    {
      m_ncDllMacCsma->SetBackoffValueCallback (MakeCallback (&GhnPlcDllMacCsma::GetBackoffSlots, m_ncDllMacCsma));
    }
  m_ncDllMacCsma->SetDllManagement (this);
  m_ncDllMacCsma->SetMpduForwardDownCallback (MakeCallback (&GhnPlcPhyPcs::StartTx, m_ghnPhyPcs));
  m_ghnPhyPcs->SetMpduForwardUpCallback (MakeCallback (&GhnPlcDllMac::Receive, m_ncDllMacCsma));
}
void
GhnPlcDllManagement::SetPhyManagement (Ptr<GhnPlcPhyManagement> phyManagement)
{
  m_phyManagement = phyManagement;
  m_ncDllMacCsma->SetPhyManagement (phyManagement);
}

Ptr<GhnPlcPhyManagement>
GhnPlcDllManagement::GetPhyManagement (void)
{
  return m_phyManagement;
}

void
GhnPlcDllManagement::SetPhyPcs (Ptr<GhnPlcPhyPcs> ghnPhyPcs)
{
  m_ghnPhyPcs = ghnPhyPcs;
}

Ptr<GhnPlcPhyPcs>
GhnPlcDllManagement::GetPhyPcs (void)
{
  return m_ghnPhyPcs;
}

void
GhnPlcDllManagement::SetAddress (UanAddress address)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_address = address;
}

UanAddress
GhnPlcDllManagement::GetAddress (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_address;
}

UanAddress
GhnPlcDllManagement::GetBroadcast (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return UanAddress::GetBroadcast ();
}

void
GhnPlcDllManagement::SetNNodes (uint8_t nNodes)
{
  m_nNodes = nNodes;
  NewMacCycle ();

}

uint8_t
GhnPlcDllManagement::GetNNodes (void)
{
  return m_nNodes;
}
void
GhnPlcDllManagement::SetTxPsd (Ptr<SpectrumValue> txPsd)
{
  //  NS_LOG_FUNCTION(this << *txPsd);

  m_setTxPsd (txPsd);
}

void
GhnPlcDllManagement::NewMacCycle (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("New MAC cycle: " << Simulator::Now ().GetNanoSeconds () << " ns");

  m_ncDllMacCsma->SetMacCycleBegin (Simulator::Now ());
  m_ncDllMacCsma->CheckStart ();

  m_rho = (float) GetPhyManagement ()->GetRxTime ().GetMilliSeconds () / (float) GDOTHN_MAC_CYCLE;

  m_phyManagement->SetRxTime (NanoSeconds (0));
  Simulator::Schedule (MilliSeconds (GDOTHN_MAC_CYCLE), &GhnPlcDllManagement::NewMacCycle, this);
}
}
} // namespace ns3
