/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007,2008,2009 INRIA, UDCAST
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
 * Author: Amine Ismail <amine.ismail@sophia.inria.fr>
 *                      <amine.ismail@udcast.com>
 */
#include <cstdlib>
#include <cstdio>

#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"

#include "ghn-plc-udp-client.h"

namespace ns3
{
namespace ghn {
NS_LOG_COMPONENT_DEFINE ("GhnPlcUdpClient");

NS_OBJECT_ENSURE_REGISTERED (GhnPlcUdpClient);

TypeId
GhnPlcUdpClient::GetTypeId (void)
{
  static TypeId
          tid =
                  TypeId ("ns3::GhnPlcUdpClient") .SetParent<Application> () .SetGroupName ("Applications") .AddConstructor<
                          GhnPlcUdpClient> ()

                  .AddAttribute ("RemoteAddress", "The destination Address of the outbound packets", AddressValue (),
                          MakeAddressAccessor (&GhnPlcUdpClient::m_peerAddress), MakeAddressChecker ())

                  .AddAttribute ("RemotePort", "The destination port of the outbound packets", UintegerValue (100),
                          MakeUintegerAccessor (&GhnPlcUdpClient::m_peerPort), MakeUintegerChecker<uint16_t> ())

                  .AddAttribute (
                          "PacketSize",
                          "Size of packets generated. The minimum packet size is 12 bytes which is the size of the header carrying the sequence number and the time stamp.",
                          UintegerValue (1024), MakeUintegerAccessor (&GhnPlcUdpClient::m_size),
                          MakeUintegerChecker<uint32_t> (12, 1500))

                  .AddAttribute ("DataRate", "The data rate in on state.", DataRateValue (DataRate ("500kb/s")),
                          MakeDataRateAccessor (&GhnPlcUdpClient::m_cbrRate), MakeDataRateChecker ());
  return tid;
}

GhnPlcUdpClient::GhnPlcUdpClient ()
{
  NS_LOG_FUNCTION (this);
  m_sent = 0;
  m_socket = 0;
  m_sendEvent = EventId ();
  m_cutLog = CreateObject<GhnPlcCutLog> ();
}

GhnPlcUdpClient::~GhnPlcUdpClient ()
{
  NS_LOG_FUNCTION (this);
}

void
GhnPlcUdpClient::SetRemote (Ipv4Address ip, uint16_t port)
{
  NS_LOG_FUNCTION (this << ip << port);
  m_peerAddress = Address (ip);
  m_peerPort = port;
}

void
GhnPlcUdpClient::SetRemote (Ipv6Address ip, uint16_t port)
{
  NS_LOG_FUNCTION (this << ip << port);
  m_peerAddress = Address (ip);
  m_peerPort = port;
}

void
GhnPlcUdpClient::SetRemote (Address ip, uint16_t port)
{
  NS_LOG_FUNCTION (this << ip << port);
  m_peerAddress = ip;
  m_peerPort = port;
}

void
GhnPlcUdpClient::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void
GhnPlcUdpClient::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_UNCOND("Connecting to " << Ipv4Address::IsMatchingType(m_peerAddress) << " port " << m_peerPort);
  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      if (Ipv4Address::IsMatchingType (m_peerAddress) == true)
        {
          m_socket->Bind ();
          m_socket->Connect (InetSocketAddress (Ipv4Address::ConvertFrom (m_peerAddress), m_peerPort));
        }
      else if (Ipv6Address::IsMatchingType (m_peerAddress) == true)
        {
          m_socket->Bind6 ();
          m_socket->Connect (Inet6SocketAddress (Ipv6Address::ConvertFrom (m_peerAddress), m_peerPort));
        }
    }

  uint32_t bits = m_size * 8;
  NS_LOG_LOGIC ("bits = " << bits);
  m_interval = Seconds (bits / static_cast<double> (m_cbrRate.GetBitRate ()));
  NS_LOG_UNCOND ("packet size: " << m_size << ", bit rate: " << m_cbrRate.GetBitRate () << ", m_interval = " << m_interval);

  m_cutLog->SetResDirectory (m_resDir);

  m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
  m_sendEvent = Simulator::Schedule (Seconds (0.0), &GhnPlcUdpClient::Send, this);
}

void
GhnPlcUdpClient::StopApplication (void)
{
  NS_LOG_FUNCTION (this);
  Simulator::Cancel (m_sendEvent);
}

void
GhnPlcUdpClient::Send (void)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_sendEvent.IsExpired ());
  Ptr<Packet> p = Create<Packet> (m_size);
  m_cutLog->AddLogData (p, this->GetNode ()->GetId ());

  std::stringstream peerAddressStringStream;
  if (Ipv4Address::IsMatchingType (m_peerAddress))
    {
      peerAddressStringStream << Ipv4Address::ConvertFrom (m_peerAddress);
    }
  else if (Ipv6Address::IsMatchingType (m_peerAddress))
    {
      peerAddressStringStream << Ipv6Address::ConvertFrom (m_peerAddress);
    }

  if ((m_socket->Send (p)) >= 0)
    {
      ++m_sent;
      NS_LOG_INFO ("TraceDelay TX " << m_size << " bytes to "
              << peerAddressStringStream.str () << " Uid: "
              << p->GetUid () << " Time: "
              << (Simulator::Now ()).GetSeconds ());

    }
  else
    {
      NS_LOG_INFO ("Error while sending " << m_size << " bytes to "
              << peerAddressStringStream.str ());
    }

  m_sendEvent = Simulator::Schedule (m_interval, &GhnPlcUdpClient::Send, this);

}
}
} // Namespace ns3
