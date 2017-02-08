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
#ifndef GHN_PLC_UDP_CLIENT_HELPER_H
#define GHN_PLC_UDP_CLIENT_HELPER_H

#include <stdint.h>
#include <string.h>
#include "ns3/application-container.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/ipv4-address.h"

#include "ns3/ghn-plc-udp-client.h"
#include "ns3/ghn-plc-greedy-udp-client.h"

namespace ns3 {
namespace ghn {
/**
 * \ingroup udpclientserver
 * \brief Create a client application which sends UDP packets carrying
 *  a 32bit sequence number and a 64 bit time stamp.
 *
 */
class GhnPlcUdpClientHelper
{

public:
  /**
   * Create GhnPlcUdpClientHelper which will make life easier for people trying
   * to set up simulations with udp-client-server.
   *
   */
  GhnPlcUdpClientHelper ();

  /**
   *  Create GhnPlcUdpClientHelper which will make life easier for people trying
   * to set up simulations with udp-client-server.
   *
   * \param ip The IPv4 address of the remote UDP server
   * \param port The port number of the remote UDP server
   */

  GhnPlcUdpClientHelper (Ipv4Address ip, uint16_t port, bool useGreedy = false);
  /**
   *  Create GhnPlcUdpClientHelper which will make life easier for people trying
   * to set up simulations with udp-client-server.
   *
   * \param ip The IPv6 address of the remote UDP server
   * \param port The port number of the remote UDP server
   */

  GhnPlcUdpClientHelper (Ipv6Address ip, uint16_t port, bool useGreedy = false);
  /**
   *  Create GhnPlcUdpClientHelper which will make life easier for people trying
   * to set up simulations with udp-client-server.
   *
   * \param ip The IP address of the remote UDP server
   * \param port The port number of the remote UDP server
   */

  GhnPlcUdpClientHelper (Address ip, uint16_t port, bool useGreedy = false);

  /**
   * Record an attribute to be set in each Application after it is is created.
   *
   * \param name the name of the attribute to set
   * \param value the value of the attribute to set
   */
  void SetAttribute (std::string name, const AttributeValue &value);

  /**
     * \param c the nodes
     *
     * Create one UDP client application on each of the input nodes
     *
     * \returns the applications created, one application per input node.
     */
  ApplicationContainer Install (NodeContainer c);

  Ptr<Application> Install (Ptr<Node> c);

  void
  SetResDirectory (std::string resDir)
  {
    m_resDir = resDir;
  }

private:
  ObjectFactory m_factory; //!< Object factory.

  std::string m_resDir;
  bool m_useGreedy;
};
}
} // namespace ns3

#endif /* UDP_CLIENT_SERVER_H */
