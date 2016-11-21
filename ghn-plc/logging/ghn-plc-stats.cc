/*
 * GhnPlcStats.cpp
 *
 *  Created on: Jun 15, 2016
 *      Author: tsokalo
 */

#include "ns3/log.h"

#include "ghn-plc-stats.h"

namespace ns3
{
namespace ghn {
NS_LOG_COMPONENT_DEFINE("GhnPlcStats");
NS_OBJECT_ENSURE_REGISTERED(GhnPlcStats);

TypeId
GhnPlcStats::GetTypeId (void)
  {
    static TypeId tid = ns3::TypeId ("ns3::GhnPlcStats") .SetParent<Object> ();
    return tid;
  }
GhnPlcStats::GhnPlcStats ()
  {

  }

GhnPlcStats::~GhnPlcStats ()
  {

  }
}
}
