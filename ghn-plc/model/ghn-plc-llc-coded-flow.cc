/*
 * GhnPlcLlcCodedFlow.cc
 *
 *  Created on: Jul 26, 2016
 *      Author: tsokalo
 */

#include "ns3/log.h"

#include "ghn-plc-llc-coded-flow.h"

NS_LOG_COMPONENT_DEFINE ("FictiveGhnPlcLlcCodedFlow");

namespace ns3
{
namespace ghn {
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NS_OBJECT_ENSURE_REGISTERED ( GhnPlcLlcCodedFlow);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TypeId
GhnPlcLlcCodedFlow::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::GhnPlcLlcCodedFlow") .SetParent<GhnPlcLlcFlow> () .AddConstructor<GhnPlcLlcCodedFlow>();
  return tid;
}

GhnPlcLlcCodedFlow::GhnPlcLlcCodedFlow ()
{

}


GhnPlcLlcCodedFlow::~GhnPlcLlcCodedFlow ()
{

}

}
}
