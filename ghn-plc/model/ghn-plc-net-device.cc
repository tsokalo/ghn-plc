/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 TUD
 *
 *  Created on: 21.02.2013
 *      Author: Stanislav Mudriievskyi <stanislav.mudriievskyi@tu-dresden.de>
 */
#include "ns3/log.h"
#include "ns3/abort.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/llc-snap-header.h"
#include "ns3/pointer.h"

#include "ns3/plc-full-duplex-ofdm-phy.h"

#include "ghn-plc-net-device.h"

NS_LOG_COMPONENT_DEFINE ("GhnPlcNetDevice");

namespace ns3
{
namespace ghn {
NS_OBJECT_ENSURE_REGISTERED (GhnPlcNetDevice);

TypeId
GhnPlcNetDevice::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::GhnPlcNetDevice") .SetParent<PLC_NetDevice> ()

  .AddConstructor<GhnPlcNetDevice> ()

  .AddAttribute ("SendEnable", "Enable or disable the transmitter section of the device.", BooleanValue (true),
          MakeBooleanAccessor (&GhnPlcNetDevice::m_sendEnable), MakeBooleanChecker ())

  .AddAttribute ("ReceiveEnable", "Enable or disable the receiver section of the device.", BooleanValue (true),
          MakeBooleanAccessor (&GhnPlcNetDevice::m_receiveEnable), MakeBooleanChecker ())

  .AddAttribute ("GhnPlcDllManagement", "The GhnPlcDllManagement attached to this device.", PointerValue (), MakePointerAccessor (
          &GhnPlcNetDevice::SetDllManagement, &GhnPlcNetDevice::GetDllManagement), MakePointerChecker<GhnPlcDllManagement> ())

  .AddTraceSource ("EthTxDrop", "Trace source indicating a packet has been "
    "dropped by the net device before transmission", MakeTraceSourceAccessor (&GhnPlcNetDevice::m_ethTxDropTrace),
          "ns3::Packet::TracedCallback")

  .AddTraceSource ("EthTx", "Trace source indicating a packet has "
    "arrived for transmission by this net device", MakeTraceSourceAccessor (&GhnPlcNetDevice::m_ethTxTrace),
          "ns3::Packet::TracedCallback");
  return tid;
}

GhnPlcNetDevice::GhnPlcNetDevice ()
{
  NS_LOG_FUNCTION (this);
}

GhnPlcNetDevice::~GhnPlcNetDevice ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
GhnPlcNetDevice::SetDllApc (Ptr<GhnPlcDllApc> ncDllApc)
{
  m_ncDllApc = ncDllApc;
}

Ptr<GhnPlcDllApc>
GhnPlcNetDevice::GetDllApc (void)
{
  return m_ncDllApc;
}

void
GhnPlcNetDevice::SetDllManagement (Ptr<GhnPlcDllManagement> ncDllManagement)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_ncDllManagement = ncDllManagement;
}

Ptr<GhnPlcDllManagement>
GhnPlcNetDevice::GetDllManagement (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_ncDllManagement;
}

void
GhnPlcNetDevice::SetAddress (Address address)
{
  NS_LOG_FUNCTION (address);
  m_ncDllManagement->SetAddress (UanAddress::ConvertFrom (address));
}

Address
GhnPlcNetDevice::GetAddress (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_ncDllManagement->GetAddress ();
}

Address
GhnPlcNetDevice::GetBroadcast (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_ncDllManagement->GetBroadcast ();
}

void
GhnPlcNetDevice::SetSendEnable (bool sendEnable)
{
  NS_LOG_FUNCTION (sendEnable);
  m_sendEnable = sendEnable;
}

bool
GhnPlcNetDevice::IsSendEnabled (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_sendEnable;
}
void
GhnPlcNetDevice::SetReceiveEnable (bool receiveEnable)
{
  NS_LOG_FUNCTION (receiveEnable);
  m_receiveEnable = receiveEnable;
}

bool
GhnPlcNetDevice::IsReceiveEnabled (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_receiveEnable;
}

bool
GhnPlcNetDevice::Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (packet << dest << protocolNumber);
  return SendFrom (packet, m_ncDllManagement->GetAddress (), dest, protocolNumber);
}

bool
GhnPlcNetDevice::SendFrom (Ptr<Packet> packet, const Address& src, const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (this << src << dest << packet->GetSize() << protocolNumber);
  NS_LOG_LOGIC ("UID is " << packet->GetUid ());
  NS_LOG_LOGIC ("PLC_NetDevice::SendFrom: Packet size without header: " << packet->GetSize ());

  NS_ASSERT (IsLinkUp ());

  //
  // Only transmit if send side of net device is enabled
  //
  if (IsSendEnabled () == false)
    {
      m_ethTxDropTrace (packet);
      return false;
    }

  m_ethTxTrace (packet);

  NS_LOG_LOGIC ("GhnPlcNetDevice::SendFrom: Packet size: " << packet->GetSize ());

  LlcSnapHeader llcHdr;
  llcHdr.SetType (protocolNumber);
  packet->AddHeader (llcHdr);

  return m_ncDllApc->SendFrom (packet, UanAddress::ConvertFrom (src), UanAddress::ConvertFrom (dest));
}

bool
GhnPlcNetDevice::Receive (Ptr<Packet> packet, const UanAddress& source, const UanAddress& dest)
{
  NS_LOG_FUNCTION (this << source << dest << packet->GetSize());
  LlcSnapHeader llc;
  packet->RemoveHeader (llc);

  m_receive_cb (this, packet, llc.GetType (), source);
  return true;
}

bool
GhnPlcNetDevice::ConfigComplete (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_configComplete;
}

void
GhnPlcNetDevice::CompleteConfig (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  if (m_spectrum_model == 0 || m_noiseFloor == 0 || m_txPsd == 0 || m_node == 0 || m_plc_node == 0 || ConfigComplete ())
    {
      return;
    }

  if (m_phy->GetInstanceTypeId ().IsChildOf (PLC_HalfDuplexOfdmPhy::GetTypeId ()))
    {
      NS_LOG_LOGIC ("PHY is child of Half Duplex PHY");
      Ptr<PLC_HalfDuplexOfdmPhy> phy = StaticCast<PLC_HalfDuplexOfdmPhy, PLC_Phy> (m_phy);

      m_outlet = CreateObject<PLC_Outlet> (m_plc_node, m_shuntImpedance);
      phy->CreateInterfaces (m_outlet, m_txPsd, m_rxImpedance, m_txImpedance);
      phy->GetTxInterface ()->AggregateObject (m_node);
      phy->GetRxInterface ()->AggregateObject (m_node);
      phy->SetNoiseFloor (m_noiseFloor);
    }
  else if (m_phy->GetInstanceTypeId ().IsChildOf (PLC_FullDuplexOfdmPhy::GetTypeId ()))
    {
      NS_LOG_LOGIC ("PHY is child of Full Duplex PHY");
      Ptr<PLC_FullDuplexOfdmPhy> phy = StaticCast<PLC_FullDuplexOfdmPhy, PLC_Phy> (m_phy);

      m_outlet = CreateObject<PLC_Outlet> (m_plc_node, m_shuntImpedance);
      phy->CreateInterfaces (m_outlet, m_txPsd, m_rxImpedance, m_txImpedance);
      phy->GetTxInterface ()->AggregateObject (m_node);
      phy->GetRxInterface ()->AggregateObject (m_node);
      phy->SetNoiseFloor (m_noiseFloor);
    }
  else
    {
      NS_ABORT_MSG ("Incompatible PHY!");
    }

  m_configComplete = true;

}

bool
GhnPlcNetDevice::SupportsSendFrom (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return false;
}

void
GhnPlcNetDevice::NotifyPhyReceptionFailure ()
{

}
}
} // namespace ns3
