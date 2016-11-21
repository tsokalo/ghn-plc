/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 University of British Columbia, Vancouver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Alexander Schloegl <alexander.schloegl@gmx.de>
 */

#include <ns3/log.h>
#include <ns3/llc-snap-header.h>
#include <ns3/simulator.h>
#include "plc-net-device.h"
#include "ns3/abort.h"
#include "ns3/node.h"
#include "ns3/plc-mac.h"
#include "ns3/plc-phy.h"
#include "ns3/spectrum-channel.h"
#include "ns3/pointer.h"
#include "plc-interface.h"
#include "ns3/ethernet-header.h"
#include "ns3/ethernet-trailer.h"
#include "ns3/plc-full-duplex-ofdm-phy.h"


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("PLC_NetDevice");

////////////////////////////////////// PLC_Netdevice ////////////////////////////////////////////////

TypeId
PLC_NetDevice::GetTypeId (void)
{
	static TypeId tid = TypeId ("ns3::PLC_NetDevice")
	.SetParent<NetDevice> ()
	.AddConstructor<PLC_NetDevice> ()
	.AddAttribute ("Channel", "The channel attached to this device",
				   PointerValue (),
				   MakePointerAccessor (&PLC_NetDevice::DoGetChannel),
				   MakePointerChecker<SpectrumChannel> ())
	;
	return tid;
}

PLC_NetDevice::PLC_NetDevice ()
{
	NS_LOG_FUNCTION (this);
	// Default Modulation and Coding Scheme
	m_mcs = ModulationAndCodingScheme (BPSK, CODING_RATE_1_2, 0);
	m_linkUp = true;
	m_mtu = 1024;
	m_configComplete = false;
}

PLC_NetDevice::~PLC_NetDevice ()
{
	NS_LOG_FUNCTION (this);
}

void
PLC_NetDevice::DoDispose (void)
{
	NS_LOG_FUNCTION (this);

	m_node = 0;
	m_noiseFloor = 0;
	m_txPsd = 0;
	m_plc_node = 0;
	m_rxImpedance = 0;
	m_txImpedance = 0;
	// chain up.
	NetDevice::DoDispose ();
}

void
PLC_NetDevice::DoStart (void)
{
	NS_LOG_FUNCTION (this);
	NetDevice::DoInitialize ();
}

void
PLC_NetDevice::SetPlcNode(Ptr<PLC_Node> plc_node)
{
	NS_LOG_FUNCTION(this);
	m_plc_node = plc_node;
//	m_plc_node->AggregateObject(this);
	CompleteConfig ();
}

void
PLC_NetDevice::SetSpectrumModel(Ptr<const SpectrumModel> sm)
{
	NS_LOG_FUNCTION(this);
	this->m_spectrum_model = sm;
	CompleteConfig ();
}

void
PLC_NetDevice::SetNoiseFloor(Ptr<const SpectrumValue> psd)
{
	NS_LOG_FUNCTION(this << psd);
	m_noiseFloor = psd;
	CompleteConfig();
}

void
PLC_NetDevice::SetTxPowerSpectralDensity(Ptr<SpectrumValue> txPsd)
{
	NS_LOG_FUNCTION(this << txPsd);
	m_txPsd = txPsd;
	CompleteConfig();
}

void
PLC_NetDevice::SetShuntImpedance(Ptr<PLC_Impedance> shuntImpedance)
{
	NS_LOG_FUNCTION(this << shuntImpedance);
	m_shuntImpedance = shuntImpedance;

}

void
PLC_NetDevice::SetRxImpedance(Ptr<PLC_Impedance> rxImpedance)
{
	NS_LOG_FUNCTION(this << rxImpedance);
	m_rxImpedance = rxImpedance;

	if (GetPhy())
	{
		if (GetPhy()->GetInstanceTypeId().IsChildOf(PLC_HalfDuplexOfdmPhy::GetTypeId()))
		{
			Ptr<PLC_HalfDuplexOfdmPhy> phy = StaticCast<PLC_HalfDuplexOfdmPhy, PLC_Phy> (GetPhy());
			phy->SetRxImpedance(rxImpedance);
		}
                if (GetPhy()->GetInstanceTypeId().IsChildOf(PLC_FullDuplexOfdmPhy::GetTypeId()))
                {
                        Ptr<PLC_FullDuplexOfdmPhy> phy = StaticCast<PLC_FullDuplexOfdmPhy, PLC_Phy> (GetPhy());
                        phy->SetRxImpedance(rxImpedance);
                }
	}
}

void
PLC_NetDevice::SetTxImpedance(Ptr<PLC_Impedance> txImpedance)
{
	NS_LOG_FUNCTION(this << txImpedance);
	m_txImpedance = txImpedance;

	if (GetPhy())
	{
		if (GetPhy()->GetInstanceTypeId().IsChildOf(PLC_HalfDuplexOfdmPhy::GetTypeId()))
		{
			Ptr<PLC_HalfDuplexOfdmPhy> phy = StaticCast<PLC_HalfDuplexOfdmPhy, PLC_Phy> (GetPhy());
			phy->SetTxImpedance(txImpedance);
		}
                if (GetPhy()->GetInstanceTypeId().IsChildOf(PLC_FullDuplexOfdmPhy::GetTypeId()))
                {
                        Ptr<PLC_FullDuplexOfdmPhy> phy = StaticCast<PLC_FullDuplexOfdmPhy, PLC_Phy> (GetPhy());
                        phy->SetTxImpedance(txImpedance);
                }
	}
}

Ptr<const SpectrumModel>
PLC_NetDevice::GetSpectrumModel(void)
{
	NS_LOG_FUNCTION (this);
	return m_spectrum_model;
}
//mudriievskyi
void
PLC_NetDevice::Receive (Ptr<Packet> packet, Mac48Address from, Mac48Address to)
{
//	NS_LOG_FUNCTION (this);
//	NS_LOG_LOGIC(*packet);
//
//	LlcSnapHeader llc;
//	packet->RemoveHeader (llc);
//
//	if (!m_receive_cb.IsNull())
//	{
//		m_receive_cb(this, packet, llc.GetType (), from);
//	}
  NS_LOG_FUNCTION (packet << from);
  NS_LOG_LOGIC ("UID is " << packet->GetUid ());
  NS_LOG_LOGIC ("PLC_NetDevice::SendFrom: Packet size with trailer: " << packet->GetSize ());


  //
  // Trace sinks will expect complete packets, not packets without some of the
  // headers.
  //
  Ptr<Packet> originalPacket = packet->Copy ();

  EthernetTrailer trailer;
  packet->RemoveTrailer (trailer);

  NS_LOG_LOGIC ("PLC_NetDevice::SendFrom: Packet size without trailer: " << packet->GetSize ());

  if (Node::ChecksumEnabled ())
    {
      trailer.EnableFcs (true);
    }

  bool crcGood = trailer.CheckFcs (packet);
  if (!crcGood)
    {
      NS_LOG_INFO ("CRC error on Packet " << packet);
      return;
    }

  EthernetHeader header (false);
  packet->RemoveHeader (header);

  NS_LOG_LOGIC ("PLC_NetDevice::SendFrom: Packet size without eth header: " << packet->GetSize ());


  NS_LOG_LOGIC ("Pkt source is " << header.GetSource ());
  NS_LOG_LOGIC ("Pkt destination is " << header.GetDestination ());

  uint16_t protocol;
  //
  // If the length/type is less than 1500, it corresponds to a length
  // interpretation packet.  In this case, it is an 802.3 packet and
  // will also have an 802.2 LLC header.  If greater than 1500, we
  // find the protocol number (Ethernet type) directly.
  //
  if (header.GetLengthType () <= 1500)
    {
      NS_LOG_LOGIC ("netdevice p->GetSize () = " << packet->GetSize ());
      NS_LOG_LOGIC ("netdevice header.GetLengthType () = " << header.GetLengthType ());
      NS_ASSERT (packet->GetSize () >= header.GetLengthType ());
      uint32_t padlen = packet->GetSize () - header.GetLengthType ();
      NS_ASSERT (padlen <= 503);
      if (padlen > 0)
        {
          packet->RemoveAtEnd (padlen);
        }

      NS_LOG_LOGIC ("PLC_NetDevice::SendFrom: Packet size withoout padding header: " << packet->GetSize ());


      LlcSnapHeader llc;
      packet->RemoveHeader (llc);

      NS_LOG_LOGIC ("PLC_NetDevice::SendFrom: Packet size without llc header: " << packet->GetSize ());

      protocol = llc.GetType ();
    }
  else
    {
      protocol = header.GetLengthType ();
    }

  //
  // Classify the packet based on its destination.
  //
  PacketType packetType;

  if (header.GetDestination ().IsBroadcast ())
    {
      packetType = PACKET_BROADCAST;
    }
  else if (header.GetDestination ().IsGroup ())
    {
      packetType = PACKET_MULTICAST;
    }
  else if (header.GetDestination () == m_mac->GetAddress ())
    {
      packetType = PACKET_HOST;
    }
  else
    {
      packetType = PACKET_OTHERHOST;
    }

  //
  // If this packet is not destined for some other host, it must be for us
  // as either a broadcast, multicast or unicast.  We need to hit the mac
  // packet received trace hook and forward the packet up the stack.
  //
  if (packetType != PACKET_OTHERHOST)
    {
      m_receive_cb (this, packet, protocol, header.GetSource ());
    }

}

Ptr<Channel> PLC_NetDevice::GetChannel (void) const
{
	return DoGetChannel();
}

Ptr<Channel>
PLC_NetDevice::DoGetChannel (void) const
{
	NS_LOG_FUNCTION(this);
	NS_ASSERT_MSG(m_plc_node, "The device is not bound to a PLC_Node which defines the Channel");
	return m_plc_node->GetChannel();
}

void
PLC_NetDevice::SetIfIndex (const uint32_t index)
{
	NS_LOG_FUNCTION (this << index);
	m_ifIndex = index;
}

uint32_t
PLC_NetDevice::GetIfIndex (void) const
{
	NS_LOG_FUNCTION (this);
	return m_ifIndex;
}

void
PLC_NetDevice::LinkUp (void)
{
	NS_LOG_FUNCTION (this);
	m_linkUp = true;

	if (GetPhy () != NULL)
	{
		GetPhy ()->Initialize ();
	}

	m_linkChanges ();
}
void
PLC_NetDevice::LinkDown (void)
{
	NS_LOG_FUNCTION (this);
	m_linkUp = false;
	m_linkChanges ();
}

bool
PLC_NetDevice::SetMtu (const uint16_t mtu)
{
  NS_LOG_FUNCTION (this);
  m_mtu = mtu;
  return true;
}

uint16_t
PLC_NetDevice::GetMtu (void) const
{
  NS_LOG_FUNCTION (this);
  return m_mtu;
}

bool
PLC_NetDevice::IsLinkUp (void) const
{
	NS_LOG_FUNCTION (this);
	return m_linkUp;
}

void
PLC_NetDevice::AddLinkChangeCallback (Callback<void> callback)
{
	NS_LOG_FUNCTION (this);
	m_linkChanges.ConnectWithoutContext (callback);
}

bool
PLC_NetDevice::IsBroadcast (void) const
{
	NS_LOG_FUNCTION (this);
	return true;
}

Address
PLC_NetDevice::GetBroadcast (void) const
{
	NS_LOG_FUNCTION (this);
	NS_ASSERT_MSG(m_mac, "MAC not set!");
	return m_mac->GetBroadcastAddress();
}

bool
PLC_NetDevice::IsMulticast (void) const
{
	NS_LOG_FUNCTION (this);
	return false;
}

Address
PLC_NetDevice::GetMulticast (Ipv4Address multicastGroup) const
{
	NS_LOG_FUNCTION (this);
	return m_mac->GetMulticastAddress ();
}

Address
PLC_NetDevice::GetMulticast (Ipv6Address addr) const
{
	NS_LOG_FUNCTION (this);
	return m_mac->GetMulticastAddress ();
}

bool
PLC_NetDevice::IsBridge (void) const
{
	NS_LOG_FUNCTION (this);
	return false;
}

bool
PLC_NetDevice::IsPointToPoint (void) const
{
	NS_LOG_FUNCTION (this);
	return false;
}

Ptr<Node>
PLC_NetDevice::GetNode (void) const
{
	NS_LOG_FUNCTION (this);
	return m_node;
}

void
PLC_NetDevice::SetNode (Ptr<Node> node)
{
	NS_LOG_FUNCTION (this);
	m_node = node;
	CompleteConfig ();
}

void
PLC_NetDevice::SetPhy (Ptr<PLC_Phy> phy)
{
	NS_LOG_FUNCTION (this << phy);
	m_phy = phy;
}

Ptr<PLC_Phy>
PLC_NetDevice::GetPhy (void)
{
	NS_LOG_FUNCTION (this);
	NS_ASSERT_MSG(m_phy, "PHY not set!");
	return m_phy;
}

void
PLC_NetDevice::SetMac (Ptr<PLC_Mac> mac)
{
	NS_LOG_FUNCTION (this << mac);
	m_mac = mac;
}

Ptr<PLC_Mac>
PLC_NetDevice::GetMac (void)
{
	NS_LOG_FUNCTION (this);
	NS_ASSERT_MSG(m_mac, "MAC not set!");
	return m_mac;
}

bool
PLC_NetDevice::NeedsArp (void) const
{
	NS_LOG_FUNCTION (this);
	return true;
}

void
PLC_NetDevice::SetReceiveCallback (ReceiveCallback cb)
{
	NS_LOG_FUNCTION (this);
	m_receive_cb = cb;
}

void
PLC_NetDevice::SetPromiscReceiveCallback (PromiscReceiveCallback cb)
{
	NS_LOG_FUNCTION (this);
	m_promiscuous_receive_cb = cb;
}

bool
PLC_NetDevice::SupportsSendFrom (void) const
{
	NS_LOG_FUNCTION_NOARGS ();
	return true;
}

PLC_ChannelTransferImpl *
PLC_NetDevice::GetChannelTransferImpl(Ptr<PLC_NetDevice> dev)
{
	NS_ASSERT(ConfigComplete() && dev->ConfigComplete());
	return m_phy->GetChannelTransferImpl(PeekPointer(dev->GetPhy()));
}

void
PLC_NetDevice::SetAddress (Address address)
{
	NS_LOG_FUNCTION (this);
	m_mac->SetAddress (Mac48Address::ConvertFrom (address));
}

Address
PLC_NetDevice::GetAddress (void) const
{
	NS_LOG_FUNCTION (this);
	return m_mac->GetAddress ();
}

bool
PLC_NetDevice::ConfigComplete(void)
{
	NS_LOG_FUNCTION (this);
	return m_configComplete;
}

void
PLC_NetDevice::CompleteConfig (void)
{
	NS_LOG_FUNCTION (this);
	if (m_spectrum_model == 0
	  || m_noiseFloor == 0
	  || m_txPsd == 0
	  || m_node == 0
	  || m_plc_node == 0
	  || ConfigComplete())
	{
	  return;
	}

	if (m_phy == 0)
	{
		m_phy = CreateObject<PLC_InformationRatePhy> ();
	}

	if (m_mac == 0)
	{
		m_mac = CreateObject<PLC_ArqMac> ();
	}

	m_outlet = CreateObject<PLC_Outlet> (m_plc_node, m_shuntImpedance);
	if(m_phy->GetInstanceTypeId().IsChildOf(PLC_HalfDuplexOfdmPhy::GetTypeId()))
	{
            Ptr<PLC_HalfDuplexOfdmPhy> phy = StaticCast<PLC_HalfDuplexOfdmPhy, PLC_Phy> (m_phy);

            phy->CreateInterfaces(m_outlet, m_txPsd, m_rxImpedance, m_txImpedance);
            phy->GetTxInterface()->AggregateObject(m_node);
            phy->GetRxInterface()->AggregateObject(m_node);
            phy->SetNoiseFloor(m_noiseFloor);
	}
	else if(m_phy->GetInstanceTypeId().IsChildOf(PLC_FullDuplexOfdmPhy::GetTypeId()))
	  {
              Ptr<PLC_FullDuplexOfdmPhy> phy = StaticCast<PLC_FullDuplexOfdmPhy, PLC_Phy> (m_phy);

              phy->CreateInterfaces(m_outlet, m_txPsd, m_rxImpedance, m_txImpedance);
              phy->GetTxInterface()->AggregateObject(m_node);
              phy->GetRxInterface()->AggregateObject(m_node);
              phy->SetNoiseFloor(m_noiseFloor);
	  }
	m_mac->SetPhy(m_phy);
	m_mac->SetMacDataCallback(MakeCallback(&PLC_NetDevice::Receive, this));

	//
	// no implementation of Full Duplex with HARQ MAC
	//
	if 	(
		m_mac->GetInstanceTypeId() == PLC_HarqMac::GetTypeId() &&
		(
		 m_phy->GetInstanceTypeId() == PLC_InformationRatePhy::GetTypeId() ||
		 m_phy->GetInstanceTypeId().IsChildOf(PLC_InformationRatePhy::GetTypeId()))
		)
	{
		Ptr<PLC_HarqMac> harq_mac = StaticCast<PLC_HarqMac, PLC_Mac> (m_mac);
		Ptr<PLC_InformationRatePhy> ir_phy = StaticCast<PLC_InformationRatePhy, PLC_Phy> (m_phy);

//		ir_phy->SetPayloadReceptionFailedCallback(MakeCallback(&PLC_HarqMac::SendNegativeAcknowledgement, m_mac));
	}

	m_configComplete = true;
}
//mudriievskyi
bool
PLC_NetDevice::Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
//	NS_LOG_FUNCTION (this);
//	NS_ASSERT(m_configComplete);
//
//	if (IsLinkUp () == false)
//	{
//		NS_LOG_INFO ("Link is down, cannot send packet.");
//		return false;
//	}
//
//	LlcSnapHeader llc;
//	llc.SetType (protocolNumber);
//	packet->AddHeader (llc);
//
//	return m_mac->Send(packet, Mac48Address::ConvertFrom (dest));

  //mudriievskyi
  NS_LOG_FUNCTION (packet << dest << protocolNumber);
  return SendFrom (packet, m_mac->GetAddress (), dest, protocolNumber);
  //<-mudriievskyi
}

bool
PLC_NetDevice::SendFrom (Ptr<Packet> packet, const Address& src, const Address& dest, uint16_t protocolNumber)
{
//	NS_LOG_FUNCTION (this);
//	NS_ASSERT(m_configComplete);
//
//	if (IsLinkUp () == false)
//	{
//		NS_LOG_INFO ("Link is down, cannot send packet.");
//		return false;
//	}
//
//	LlcSnapHeader llc;
//	llc.SetType (protocolNumber);
//	packet->AddHeader (llc);
//
//	return m_mac->SendFrom(packet, Mac48Address::ConvertFrom (source), Mac48Address::ConvertFrom (dest));

  NS_LOG_FUNCTION (packet << src << dest << protocolNumber);
  NS_LOG_LOGIC ("packet =" << packet);
  NS_LOG_LOGIC ("UID is " << packet->GetUid () << ")");
  NS_LOG_LOGIC ("PLC_NetDevice::SendFrom: Packet size without header: " << packet->GetSize ());

  NS_ASSERT (IsLinkUp ());

  Mac48Address destination = Mac48Address::ConvertFrom (dest);
  Mac48Address source = Mac48Address::ConvertFrom (src);
  NS_LOG_LOGIC ("Encapsulating packet as LLC (length interpretation)");

  LlcSnapHeader llc;
  EthernetTrailer trailer;
  EthernetHeader header (false);
  header.SetSource (source);
  header.SetDestination (destination);

  uint16_t lengthType = 0;
  llc.SetType (protocolNumber);
  packet->AddHeader (llc);

  NS_LOG_LOGIC ("PLC_NetDevice::SendFrom: Packet size with llc header: " << packet->GetSize ());

  //
  // This corresponds to the length interpretation of the lengthType
  // field but with an LLC/SNAP header added to the payload as in
  // IEEE 802.2
  //
  lengthType = packet->GetSize ();

  //
  // All Ethernet frames must carry a minimum payload of 46 bytes.  The
  // LLC SNAP header counts as part of this payload.  We need to padd out
  // if we don't have enough bytes.  These must be real bytes since they
  // will be written to pcap files and compared in regression trace files.
  //
  if (packet->GetSize () < 503)
    {
      uint8_t buffer[503];
      memset (buffer, 0, 503);
      Ptr<Packet> padd = Create<Packet> (buffer, 503 - packet->GetSize ());
      packet->AddAtEnd (padd);
    }

  NS_LOG_LOGIC ("header.SetLengthType (" << lengthType << ")");
  NS_LOG_LOGIC ("PLC_NetDevice::SendFrom: Packet size with padding: " << packet->GetSize ());
  header.SetLengthType (lengthType);
  packet->AddHeader (header);
  NS_LOG_LOGIC ("PLC_NetDevice::SendFrom: Packet size with eth header: " << packet->GetSize ());


  if (Node::ChecksumEnabled ())
    {
      trailer.EnableFcs (true);
    }
  trailer.CalcFcs (packet);
  packet->AddTrailer (trailer);

  NS_LOG_LOGIC ("PLC_NetDevice::SendFrom: Packet size with trailer: " << packet->GetSize ());


  NS_ASSERT_MSG (packet->GetSize () <= GetMtu (),
                 "CsmaNetDevice::AddHeader(): 802.3 Length/Type field with LLC/SNAP: "
                 "length interpretation must not exceed device frame size minus overhead. Packet size: "
          << packet->GetSize () << ", MTU: " << GetMtu() );


  return m_mac->SendFrom(packet, Mac48Address::ConvertFrom (source), Mac48Address::ConvertFrom (dest));}

}
