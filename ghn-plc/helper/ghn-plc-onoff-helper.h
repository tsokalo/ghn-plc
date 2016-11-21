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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#ifndef GHN_PLC_ONOFF_HELPER_H
#define GHN_PLC_ONOFF_HELPER_H

#include <stdint.h>
#include <string>

#include "ns3/object-factory.h"
#include "ns3/address.h"
#include "ns3/attribute.h"
#include "ns3/net-device.h"
#include "ns3/node-container.h"
#include "ns3/application-container.h"

#include "ns3/ghn-plc-onoff-app.h"

namespace ns3 {
namespace ghn {

/**
 * \ingroup onoff
 * \brief A helper to make it easier to instantiate an ns3::GhnPlcOnOffApp
 * on a set of nodes.
 */
class GhnPlcOnOffHelper
{
public:

  GhnPlcOnOffHelper (std::string protocol, Address address);

  void SetAttribute (std::string name, const AttributeValue &value);

  void SetConstantRate (DataRate dataRate, uint32_t packetSize = 512);

  ApplicationContainer Install (NodeContainer c) const;
  ApplicationContainer Install (Ptr<Node> node) const;
  ApplicationContainer Install (std::string nodeName) const;


  int64_t AssignStreams (NodeContainer c, int64_t stream);

  void
  SetResDirectory (std::string resDir)
  {
    m_resDir = resDir;
  }

private:

  Ptr<Application> InstallPriv (Ptr<Node> node) const;

  ObjectFactory m_factory;
  std::string m_resDir;
};
}
} // namespace ns3

#endif /* GHN_PLC_ONOFF_HELPER_H */

