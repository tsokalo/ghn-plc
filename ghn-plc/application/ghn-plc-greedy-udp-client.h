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
 *
 */

#ifndef GHN_PLC_GREEDY_UDP_CLIENT_H
#define GHN_PLC_GREEDY_UDP_CLIENT_H

#include <string.h>

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/socket.h"
#include "ns3/address.h"
#include "ns3/ipv4-address.h"
#include "ns3/data-rate.h"
#include "ns3/traced-callback.h"

#include "ns3/ghn-plc-cut-log.h"

namespace ns3
{
namespace ghn {

/**
 * \ingroup udpclientserver
 * \class GhnPlcGreedyUdpClient
 * \brief A Udp client. Sends UDP packet carrying sequence number and time stamp
 *  in their payloads
 *
 */
class GhnPlcGreedyUdpClient : public Application
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId
  GetTypeId (void);

  GhnPlcGreedyUdpClient ();

  virtual
  ~GhnPlcGreedyUdpClient ();

  /**
   * \brief set the remote address and port
   * \param ip remote IPv4 address
   * \param port remote port
   */
  void
  SetRemote (Ipv4Address ip, uint16_t port);
  /**
   * \brief set the remote address and port
   * \param ip remote IPv6 address
   * \param port remote port
   */
  void
  SetRemote (Ipv6Address ip, uint16_t port);
  /**
   * \brief set the remote address and port
   * \param ip remote IP address
   * \param port remote port
   */
  void
  SetRemote (Address ip, uint16_t port);

  void
  SetResDirectory (std::string resDir)
  {
    m_resDir = resDir;
  }
  void
  SendBatch(uint32_t numBytes);

protected:
  virtual void
  DoDispose (void);

private:

  virtual void
  StartApplication (void);
  virtual void
  StopApplication (void);

  uint32_t m_size; //!< Size of the sent packet (including the SeqTsHeader)

  uint32_t m_sent; //!< Counter for sent packets
  Ptr<Socket> m_socket; //!< Socket
  Address m_peerAddress; //!< Remote peer address
  uint16_t m_peerPort; //!< Remote peer port
  EventId m_sendEvent; //!< Event to send the next packet

  Ptr<GhnPlcCutLog> m_cutLog;
  std::string m_resDir;

};
}
} // namespace ns3

#endif /* GHN_PLC_GREEDY_UDP_CLIENT_H */
