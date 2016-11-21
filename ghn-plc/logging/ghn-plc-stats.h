/*
 * GhnPlcStats.h
 *
 *  Created on: Jun 15, 2016
 *      Author: tsokalo
 */

#ifndef GHN_PLC_STATS_H_
#define GHN_PLC_STATS_H_

#include <iostream>
#include <map>
#include <stdint.h>
#include "ns3/object.h"
#include "ns3/type-id.h"

namespace ns3
{
namespace ghn {
class GhnPlcStats : public Object
{
public:
  static TypeId
  GetTypeId (void);

  GhnPlcStats ();
  virtual
  ~GhnPlcStats ();

  void
  NotifyPhySuccess (uint32_t vertex_id)
  {
    m_phySuccess[vertex_id]++;
  }
  void
  NotifyPhyFailure (uint32_t vertex_id)
  {
    m_phyFailure[vertex_id]++;
  }
  void
  PrintStats ()
  {
    for (auto it = m_phySuccess.begin (); it != m_phySuccess.end (); it++)
      std::cout << "Vertex " << it->first << " -> PHY successes: " << it->second << std::endl;
    for (auto it = m_phyFailure.begin (); it != m_phyFailure.end (); it++)
      std::cout << "Vertex " << it->first << " -> PHY failure: " << it->second << std::endl;
  }

private:

  // <vertex id> <counter>
  std::map<uint32_t, uint32_t> m_phySuccess;
  std::map<uint32_t, uint32_t> m_phyFailure;

};
}
}
#endif /* GHN_PLC_STATS_H_ */
