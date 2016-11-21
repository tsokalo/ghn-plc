/*
 * NcRoutingRules.cpp
 *
 *  Created on: 02.10.2016
 *      Author: tsokalo
 */

#include "ns3/log.h"

#include "ghn-plc-routing-rules.h"

NS_LOG_COMPONENT_DEFINE ("NcRoutingRules");

namespace ns3
{
namespace ghn {
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NS_OBJECT_ENSURE_REGISTERED (NcRoutingRules);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TypeId
NcRoutingRules::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NcRoutingRules") .SetParent<Object> () .AddConstructor<NcRoutingRules>();
  return tid;
}

NcRoutingRules::NcRoutingRules ()
{

}

NcRoutingRules::~NcRoutingRules ()
{

}
}
}
