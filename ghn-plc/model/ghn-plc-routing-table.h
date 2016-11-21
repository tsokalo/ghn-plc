/*
 * GhnPlcRoutingTable.h
 *
 *  Created on: Jun 2, 2016
 *      Author: tsokalo
 */

#ifndef GHN_PLC_ROUTINGTABLE_H_
#define GHN_PLC_ROUTINGTABLE_H_

#include <limits>

#include <boost/graph/copy.hpp>
#include <boost/config.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/property_map/property_map.hpp>

#include "ns3/uan-address.h"
#include <ns3/type-id.h>
#include "ns3/plc-defs.h"
#include "ns3/plc-node.h"
#include "ns3/plc-channel.h"
#include "ns3/ptr.h"
#include "ns3/object.h"
#include "ns3/plc-phy.h"

#include "ghn-plc-bit-loading.h"

namespace ns3
{
namespace ghn {
typedef boost::adjacency_list<boost::listS, boost::vecS, boost::directedS, boost::no_property, boost::property<
        boost::edge_weight_t, double> > graph_t;
typedef boost::graph_traits<graph_t>::vertex_descriptor vertex_descriptor;
typedef std::pair<int32_t, int32_t> Edge;

class GhnPlcRoutingTable : public Object
{
public:

  static TypeId
  GetTypeId (void);

  GhnPlcRoutingTable ();
  virtual
  ~GhnPlcRoutingTable ();

  void
  CalcRoutingTable (void);

  UanAddress
  GetNextHopAddress (uint32_t from, uint32_t to);
  UanAddress
  GetNextHopAddress (UanAddress from, UanAddress to);
  UanAddress
  GetNextHopAddress (Ptr<PLC_Node> from, Ptr<PLC_Node> to);
  bool
  IsOnRoute (UanAddress from, UanAddress to, UanAddress my);
  bool
  DoesRouteExist (UanAddress from, UanAddress to);
  bool
  IsNextOnRoute (UanAddress from, UanAddress to, UanAddress my);
  uint32_t
  GetNumHops (UanAddress from, UanAddress to);
  uint32_t
  GetMaxNumHops (UanAddress from);

  double
  GetCost (uint32_t from, uint32_t to);
  double
  GetCost (UanAddress from, UanAddress to);
  double
  GetCost (Ptr<PLC_Node> from, Ptr<PLC_Node> to);

  void
  AddNode (Ptr<PLC_Node> node);
  void
  SetChannel (Ptr<PLC_Channel> channel)
  {
    this->m_channel = channel;
  }
  Ptr<PLC_Channel>
  GetChannel (void)
  {
    NS_ASSERT_MSG(m_channel, "Channel for the graph has not been created yet");
    return this->m_channel;
  }


  void
  PrintRoutingTable ();

  void
  SetBitLoadingTable (Ptr<GhnPlcBitLoading> bitLoadingTable)
  {
    m_bitLoadingTable = bitLoadingTable;
  }
  uint32_t GetIdByAddress(UanAddress address)
  {
    NS_ASSERT_MSG(m_idByAddressMap.size() > address.GetAsInt(), m_idByAddressMap.size() << " " << (uint32_t)address.GetAsInt());
    return m_idByAddressMap[address];
  }
  double
  GetRouteCost (UanAddress src, UanAddress dst);
  void
  SetResDirectory (std::string resDir)
  {
    m_resDir = resDir;
  }

private:

  UanAddress
  LookUp (uint32_t id);
  //
  // unit [bits per symbols]
  //
  double
  CalcCost (Ptr<PLC_Node> node1, Ptr<PLC_Node> node2);
  double
  CalcCost (uint32_t from, uint32_t to);
  double
  CalcRouteCost (UanAddress src, UanAddress dst);
  void
  CalcNumHops ();

  Ptr<SpectrumValue>
  GetTxPsd (uint32_t vertex_id);

  std::vector<std::vector<double> > m_cost;
  std::vector<std::vector<vertex_descriptor> > m_route;
  std::vector<std::vector<double> > m_routeCost;

  Ptr<PLC_Channel> m_channel;

  std::vector<Ptr<PLC_Node> > m_nodes;

  std::vector<uint32_t> m_maxNumHops;
  std::vector<std::vector<uint32_t> > m_numHops;
  //
  // node index and PLC node ID
  //
  std::map<uint32_t, uint32_t> m_indexByIdMap;
  std::map<UanAddress, uint32_t> m_idByAddressMap;

  Ptr<GhnPlcBitLoading> m_bitLoadingTable;

  double m_disconnectedLinkCost;
  uint32_t m_disconnectedNumHops;

  std::string m_resDir;
};
}
}
#endif /* GHN_PLC_ROUTINGTABLE_H_ */
