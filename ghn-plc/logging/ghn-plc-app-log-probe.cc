/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Bucknell University
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
 * Authors: L. Felipe Perrone (perrone@bucknell.edu)
 *          Tiago G. Rodrigues (tgr002@bucknell.edu)
 *
 * Modified by: Mitch Watrous (watrous@u.washington.edu)
 *
 */

#include "ns3/object.h"
#include "ns3/log.h"
#include "ns3/names.h"
#include "ns3/config.h"
#include "ns3/trace-source-accessor.h"

#include "ghn-plc-app-log-probe.h"

namespace ns3
{
namespace ghn {
NS_LOG_COMPONENT_DEFINE ("GhnPlcAppLogProbe");

NS_OBJECT_ENSURE_REGISTERED (GhnPlcAppLogProbe);

TypeId
GhnPlcAppLogProbe::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::GhnPlcAppLogProbe") .SetParent<Probe> ()

  .SetGroupName ("Stats")

  .AddConstructor<GhnPlcAppLogProbe> ()

  .AddTraceSource ("Output", "The AppLog valued probe output", MakeTraceSourceAccessor (&GhnPlcAppLogProbe::m_output),
          "ns3::TracedValue::AppLogCallback");
  return tid;
}

GhnPlcAppLogProbe::GhnPlcAppLogProbe ()
{
  NS_LOG_FUNCTION (this);
}

GhnPlcAppLogProbe::~GhnPlcAppLogProbe ()
{
  NS_LOG_FUNCTION (this);
}


bool
GhnPlcAppLogProbe::ConnectByObject (std::string traceSource, Ptr<Object> obj)
{
  NS_LOG_FUNCTION (this << traceSource << obj);
  NS_LOG_DEBUG ("Name of trace source (if any) in names database: " << Names::FindPath (obj));
  bool connected = obj->TraceConnectWithoutContext (traceSource, MakeCallback (&ns3::GhnPlcAppLogProbe::TraceSink, this));
  return connected;
}

void
GhnPlcAppLogProbe::ConnectByPath (std::string path)
{
  NS_LOG_FUNCTION (this << path);
  NS_LOG_DEBUG ("Name of trace source to search for in config database: " << path);
  Config::ConnectWithoutContext (path, MakeCallback (&ns3::GhnPlcAppLogProbe::TraceSink, this));
}

void
GhnPlcAppLogProbe::TraceSink (AppLog oldData, AppLog newData)
{
  if (IsEnabled ())
    {
      m_output = newData;
    }
  NS_LOG_DEBUG ("Get out of trace");
}
}
} // namespace ns3
