/*
 * GhnPlcRoutingTable.cpp
 *
 *  Created on: Jun 2, 2016
 *      Author: tsokalo
 */

#include <sstream>

#include "ns3/log.h"
#include <ns3/config.h>
#include <ns3/plc-undirected-dfs.h>
#include <ns3/object-vector.h>
#include "ns3/simulator.h"

#include "ghn-plc-routing-table.h"
#include "ghn-plc-net-device.h"
#include "ghn-plc-utilities.h"

using namespace boost;

namespace ns3
{
namespace ghn {
NS_LOG_COMPONENT_DEFINE ("GhnPlcRoutingTable");
NS_OBJECT_ENSURE_REGISTERED ( GhnPlcRoutingTable);

TypeId
GhnPlcRoutingTable::GetTypeId (void)
{
  static TypeId tid = ns3::TypeId ("ns3::GhnPlcRoutingTable") .SetParent<Object> () .AddConstructor<GhnPlcRoutingTable> ();
  return tid;
}

GhnPlcRoutingTable::GhnPlcRoutingTable ()
{
  m_disconnectedLinkCost = std::numeric_limits<double>::max ();
  m_disconnectedNumHops = UanAddress::GetBroadcast ().GetAsInt ();
}

GhnPlcRoutingTable::~GhnPlcRoutingTable ()
{

}

UanAddress
GhnPlcRoutingTable::GetNextHopAddress (uint32_t from_id, uint32_t to_id)
{
  NS_LOG_FUNCTION(this);
  if (to_id == UanAddress::GetBroadcast ().GetAsInt ()) return to_id;
  return m_route[m_indexByIdMap[to_id]][m_indexByIdMap[from_id]];
}
UanAddress
GhnPlcRoutingTable::GetNextHopAddress (UanAddress from, UanAddress to)
{
  if (to == UanAddress::GetBroadcast ()) return to;
  return GetNextHopAddress (m_idByAddressMap[from], m_idByAddressMap[to]);
}
UanAddress
GhnPlcRoutingTable::GetNextHopAddress (Ptr<PLC_Node> from, Ptr<PLC_Node> to)
{
  return GetNextHopAddress (from->GetVertexId (), to->GetVertexId ());
}
bool
GhnPlcRoutingTable::IsOnRoute (UanAddress from, UanAddress to, UanAddress my)
{
  NS_LOG_FUNCTION(this);

  if (!DoesRouteExist (my, to)) return false;

  uint32_t from_id = m_idByAddressMap[from];
  uint32_t to_id = m_idByAddressMap[to];
  uint32_t my_id = m_idByAddressMap[my];

  do
    {
      uint32_t next_id = m_route[m_indexByIdMap[to_id]][m_indexByIdMap[from_id]];
      if (next_id == -1) return false;
      if (next_id == my_id) return true;
      from_id = next_id;
    }
  while (from_id != to_id);

  return false;
}
bool
GhnPlcRoutingTable::DoesRouteExist (UanAddress from, UanAddress to)
{
  NS_LOG_FUNCTION(this);

  return (GetNumHops (from, to) != m_disconnectedNumHops);
}
bool
GhnPlcRoutingTable::IsNextOnRoute (UanAddress from, UanAddress to, UanAddress my)
{
  //  std::cout << "From " << from << " to " << to << " is " << GetNextHopAddress (from, to) << " and my is " << my << std::endl;
  return (GetNextHopAddress (from, to) == my);
}
uint32_t
GhnPlcRoutingTable::GetNumHops (UanAddress from, UanAddress to)
{
  return m_numHops.at (from.GetAsInt ()).at (to.GetAsInt ());
}
uint32_t
GhnPlcRoutingTable::GetMaxNumHops (UanAddress from)
{
  return m_maxNumHops.at (from.GetAsInt ());
}
double
GhnPlcRoutingTable::GetCost (uint32_t from_id, uint32_t to_id)
{
  return m_cost[m_indexByIdMap[to_id]][m_indexByIdMap[from_id]];
}

double
GhnPlcRoutingTable::GetCost (UanAddress from, UanAddress to)
{
  return GetCost (m_idByAddressMap[from], m_idByAddressMap[to]);
}

double
GhnPlcRoutingTable::GetCost (Ptr<PLC_Node> from, Ptr<PLC_Node> to)
{
  return GetCost (from->GetVertexId (), to->GetVertexId ());
}
void
GhnPlcRoutingTable::AddNode (Ptr<PLC_Node> node)
{
  NS_LOG_FUNCTION(this << node);
  this->m_nodes.push_back (node);
}
double
GhnPlcRoutingTable::GetRouteCost (UanAddress src, UanAddress dst)
{
  return m_routeCost.at (src.GetAsInt ()).at (dst.GetAsInt ());
}
double
GhnPlcRoutingTable::CalcRouteCost (UanAddress from, UanAddress to)
{
  NS_LOG_FUNCTION(this);

  if (!DoesRouteExist (from, to)) return -1;

  uint32_t from_id = m_idByAddressMap[from];
  uint32_t to_id = m_idByAddressMap[to];

  double hop_counter = 0;
  double bps = 0;
  do
    {
      uint32_t next_id = m_route[m_indexByIdMap[to_id]][m_indexByIdMap[from_id]];
      bps += CalcCost (from_id, next_id);
      hop_counter++;
      NS_ASSERT (next_id != -1);
      from_id = next_id;
    }
  while (from_id != to_id);

  return bps;
}
void
GhnPlcRoutingTable::PrintRoutingTable ()
{
  std::cout << "Routing table:" << std::endl;
  graph_traits<graph_t>::vertex_iterator vi, vend;
  for (uint32_t j = 0; j < m_nodes.size (); j++)
    {
      for (uint32_t i = 0; i < m_nodes.size (); i++)
        {
          if (i == j) continue;
          UanAddress from = LookUp (i);
          UanAddress to = LookUp (j);
          std::cout << "From " << m_nodes.at (j)->GetVertexId () << " to " << m_nodes.at (i)->GetVertexId () << " next is "
                  << m_nodes.at (m_route[i][j])->GetVertexId () << " with cost " << m_cost[j][i] << " num hops " << GetNumHops (
                  from, to) << " max num hops " << GetMaxNumHops (from) << std::endl;
        }
    }
}

UanAddress
GhnPlcRoutingTable::LookUp (uint32_t id)
{
  for (uint32_t i = 0; i < m_channel->GetNDevices (); i++)
    {
      if (m_channel->GetDevice (i)->GetObject<GhnPlcNetDevice> ()->GetPlcNode ()->GetVertexId () == id)
        {
          return m_channel->GetDevice (i)->GetObject<GhnPlcNetDevice> ()->GetDllManagement ()->GetAddress ();
        }
    }
  NS_ASSERT(0);
}

Ptr<SpectrumValue>
GhnPlcRoutingTable::GetTxPsd (uint32_t vertex_id)
{
  for (uint32_t i = 0; i < m_channel->GetNDevices (); i++)
    {
      if (m_channel->GetDevice (i)->GetObject<GhnPlcNetDevice> ()->GetPlcNode ()->GetVertexId () == vertex_id)
        {
          return m_channel->GetDevice (i)->GetObject<GhnPlcNetDevice> ()->GetTxPowerSpectralDensity ();
        }
    }
  NS_ASSERT(0);
}

double
GhnPlcRoutingTable::CalcCost (Ptr<PLC_Node> node1, Ptr<PLC_Node> node2)
{
  NS_LOG_FUNCTION(this);
  NS_ASSERT(m_bitLoadingTable);

  uint32_t src_id = node1->GetVertexId ();
  uint32_t dst_id = node2->GetVertexId ();

  return CalcCost (src_id, dst_id);
}
double
GhnPlcRoutingTable::CalcCost (uint32_t src_id, uint32_t dst_id)
{
  double cost = (m_bitLoadingTable->GetNumEffBits (src_id, dst_id) == 0) ? m_disconnectedLinkCost : 1
          / m_bitLoadingTable->GetNumEffBits (src_id, dst_id);

  NS_LOG_INFO("Link cost from Node " << src_id << " to " << dst_id << ": " << cost);

  return cost;
}
void
GhnPlcRoutingTable::CalcNumHops ()
{
  uint8_t max_address = UanAddress::GetBroadcast ().GetAsInt ();
  m_maxNumHops.resize (max_address, 0);
  m_numHops.resize (max_address, std::vector<uint32_t> (max_address, 0));
  for (uint32_t i = 0; i < m_channel->GetNDevices (); i++)
    {
      uint32_t max_num_hops = 0;
      auto from_address = m_channel->GetDevice (i)->GetObject<GhnPlcNetDevice> ()->GetDllManagement ()->GetAddress ();

      for (uint32_t j = 0; j < m_channel->GetNDevices (); j++)
        {
          if (i == j)
            {
              continue;
            }
          auto to_address = m_channel->GetDevice (j)->GetObject<GhnPlcNetDevice> ()->GetDllManagement ()->GetAddress ();

          /////////////////////////////////////////////////
          uint32_t num_hops = 0;
          auto from_id = m_channel->GetDevice (i)->GetObject<GhnPlcNetDevice> ()->GetPlcNode ()->GetVertexId ();
          auto to_id = m_channel->GetDevice (j)->GetObject<GhnPlcNetDevice> ()->GetPlcNode ()->GetVertexId ();

          do
            {
              uint32_t next_id = m_route[m_indexByIdMap[to_id]][m_indexByIdMap[from_id]];
              if (next_id == -1) break;
              NS_LOG_DEBUG("From " << from_id << " to " << to_id << " next is " << next_id);
              from_id = next_id;
              num_hops++;

            }
          while (from_id != to_id && num_hops < m_disconnectedNumHops);
          /////////////////////////////////////

          if (num_hops >= m_disconnectedNumHops)
            {
              m_numHops.at (from_address.GetAsInt ()).at (to_address.GetAsInt ()) = m_disconnectedNumHops;
              continue;
            }
          if (num_hops > max_num_hops) max_num_hops = num_hops;
          m_numHops.at (from_address.GetAsInt ()).at (to_address.GetAsInt ()) = num_hops;
        }

      m_maxNumHops.at (from_address.GetAsInt ()) = max_num_hops;
    }
}
void
GhnPlcRoutingTable::CalcRoutingTable (void)
{
  m_cost.clear ();
  m_route.clear ();

  Edge edge_array[m_nodes.size () * (m_nodes.size () - 1)];
  double weights[m_nodes.size () * (m_nodes.size () - 1)];
  uint32_t i = 0;
  for (auto it1 = m_nodes.begin (); it1 != m_nodes.end (); it1++)
    {
      for (auto it2 = m_nodes.begin (); it2 != m_nodes.end (); it2++)
        {
          if (it1 == it2) continue;

          edge_array[i] = Edge ((*it1)->GetVertexId (), (*it2)->GetVertexId ());
          weights[i++] = CalcCost (*it1, *it2);
        }
    }

  graph_t g (edge_array, edge_array + sizeof(edge_array) / sizeof(Edge), weights, m_nodes.size ());
  property_map<graph_t, edge_weight_t>::type weightmap = get (edge_weight, g);

  for (uint32_t i = 0; i < m_nodes.size (); i++)
    {
      std::vector<vertex_descriptor> p (num_vertices (g));
      std::vector<double> d (num_vertices (g));

      vertex_descriptor s = vertex (i, g);

      dijkstra_shortest_paths (g, s, predecessor_map (boost::make_iterator_property_map (p.begin (), get (boost::vertex_index,
              g))). distance_map (boost::make_iterator_property_map (d.begin (), get (boost::vertex_index, g))));
      m_cost.push_back (d);
      m_route.push_back (p);
    }

  for (uint32_t i = 0; i < m_nodes.size (); i++)
    {
      m_indexByIdMap.insert (std::pair<uint32_t, uint32_t> (m_nodes.at (i)->GetVertexId (), i));
      m_idByAddressMap.insert (std::pair<UanAddress, uint32_t> (LookUp ((uint32_t) m_nodes.at (i)->GetVertexId ()), m_nodes.at (
              i)->GetVertexId ()));
    }

  CalcNumHops ();

  //
  // calc route costs
  //
  uint8_t max_address = UanAddress::GetBroadcast ().GetAsInt ();
  m_routeCost .resize (max_address, std::vector<double> (max_address, 0));
  for (uint32_t i = 0; i < m_channel->GetNDevices (); i++)
    {
      auto from = m_channel->GetDevice (i)->GetObject<GhnPlcNetDevice> ()->GetDllManagement ()->GetAddress ();

      for (uint32_t j = 0; j < m_channel->GetNDevices (); j++)
        {
          if (i == j) continue;

          auto to = m_channel->GetDevice (j)->GetObject<GhnPlcNetDevice> ()->GetDllManagement ()->GetAddress ();
          m_routeCost.at (from.GetAsInt ()).at (to.GetAsInt ()) = CalcRouteCost (from, to);
        }
    }
}

}
}
