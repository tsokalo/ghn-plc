/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
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
 * Author: Mohamed Amine Ismail <amine.ismail@sophia.inria.fr>
 */

#include "ns3/udp-trace-client.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"

#include "ghn-plc-udp-client-helper.h"

#include "ns3/ghn-plc-udp-client.h"
#include "ns3/ghn-plc-greedy-udp-client.h"

namespace ns3
{
namespace ghn
{
GhnPlcUdpClientHelper::GhnPlcUdpClientHelper ()
{
}

GhnPlcUdpClientHelper::GhnPlcUdpClientHelper (Address address, uint16_t port, bool useGreedy) :
        m_useGreedy (useGreedy)
{
  m_factory.SetTypeId (useGreedy ? GhnPlcGreedyUdpClient::GetTypeId () : GhnPlcUdpClient::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
  SetAttribute ("RemotePort", UintegerValue (port));
}

GhnPlcUdpClientHelper::GhnPlcUdpClientHelper (Ipv4Address address, uint16_t port, bool useGreedy) :
        m_useGreedy (useGreedy)
{
  m_factory.SetTypeId (useGreedy ? GhnPlcGreedyUdpClient::GetTypeId () : GhnPlcUdpClient::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (Address (address)));
  SetAttribute ("RemotePort", UintegerValue (port));
}

GhnPlcUdpClientHelper::GhnPlcUdpClientHelper (Ipv6Address address, uint16_t port, bool useGreedy) :
        m_useGreedy (useGreedy)
{
  m_factory.SetTypeId (useGreedy ? GhnPlcGreedyUdpClient::GetTypeId () : GhnPlcUdpClient::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (Address (address)));
  SetAttribute ("RemotePort", UintegerValue (port));
}

void
GhnPlcUdpClientHelper::SetAttribute (std::string name, const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
GhnPlcUdpClientHelper::Install (NodeContainer c)
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Node> node = *i;
      apps.Add (Install(node));
    }
  return apps;
}
Ptr<Application>
GhnPlcUdpClientHelper::Install (Ptr<Node> node)
{
  if (!m_useGreedy)
    {
      Ptr<GhnPlcUdpClient> client = m_factory.Create<GhnPlcUdpClient> ();
      client->SetResDirectory (m_resDir);
      node->AddApplication (client);
      return client;
    }
  else
    {
      Ptr<GhnPlcGreedyUdpClient> client = m_factory.Create<GhnPlcGreedyUdpClient> ();
      client->SetResDirectory (m_resDir);
      node->AddApplication (client);
      return client;
    }
}
}
} // namespace ns3
