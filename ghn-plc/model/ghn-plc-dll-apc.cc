/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016 TUD
 *
 *  Created on: 31.05.2016
 *      Author: Ievgenii Tsokalo
 */

#include "ghn-plc-dll-apc.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("GhnPlcDllApc");

namespace ns3 {
namespace ghn {
NS_OBJECT_ENSURE_REGISTERED (GhnPlcDllApc);

TypeId
GhnPlcDllApc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::GhnPlcDllApc")
    .SetParent<Object> ()
    .AddConstructor<GhnPlcDllApc> ()
 ;
  return tid;
}

GhnPlcDllApc::GhnPlcDllApc ()
{
  NS_LOG_FUNCTION (this);
}

GhnPlcDllApc::~GhnPlcDllApc()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
GhnPlcDllApc::SetDllLlc (Ptr<GhnPlcDllLlc> ncDllLlc)
{
  m_dllLlc = ncDllLlc;
}

Ptr<GhnPlcDllLlc>
GhnPlcDllApc::GetDllLlc (void)
{
  return m_dllLlc;
}

void
GhnPlcDllApc::SetNetDevice (Ptr<GhnPlcNetDevice> ghnNetDevice)
{
  m_ghnNetDevice = ghnNetDevice;
}

Ptr<GhnPlcNetDevice>
GhnPlcDllApc::GetNetDevice (void)
{
  return m_ghnNetDevice;
}

bool
GhnPlcDllApc::SendFrom (Ptr<Packet> packet, const UanAddress& source, const UanAddress& dest)
{
  NS_LOG_FUNCTION (this << packet->GetSize());
  return m_forwardDown (packet, source, dest, -1);
}

bool
GhnPlcDllApc::Receive (Ptr<Packet> packet, const UanAddress& source, const UanAddress& dest)
{
  NS_LOG_FUNCTION (this << packet->GetSize());
  NS_LOG_LOGIC("Receive packet with size " << packet->GetSize() << ", src " << source << ", dst " << dest);
  return m_forwardUp (packet, source, dest);
}

void
GhnPlcDllApc::SetApduForwardDownCallback (ApduForwardDownCallback cb)
{
  m_forwardDown = cb;
}
void
GhnPlcDllApc::SetAdpForwardUpCallback (AdpForwardUpCallback cb)
{
  m_forwardUp = cb;
}
}


} // namespace ns3
