/*
 * in-home-header.h
 *
 *  Created on: 06.03.2017
 *      Author: tsokalo
 */

#ifndef SRC_IN_HOME_TOPOLOGY_MODEL_IN_HOME_HEADER_H_
#define SRC_IN_HOME_TOPOLOGY_MODEL_IN_HOME_HEADER_H_

#define DELIMITER '\t'

#include <fstream>
#include <stdint.h>

namespace ns3
{

struct Coordinate
{
  double x;
  double y;

  friend std::ostream&
  operator<< (std::ostream& fo, const Coordinate& c)
  {
    fo << c.x << DELIMITER;
    fo << c.y << DELIMITER;
    return fo;
  }

  friend std::istream&
  operator>> (std::istream& fi, Coordinate& c)
  {
    fi >> c.x;
    fi >> c.y;
    return fi;
  }
};

struct Dev
{
  /*
   * dev id
   */
  uint16_t id;
  /*
   * id of neighbour dev
   */
  uint16_t neighbour;
  /*
   * index of node that associates with this dev
   */
  int16_t nodeIndex;
  Coordinate c;
  double energy;

  friend std::ostream&
  operator<< (std::ostream& fo, const Dev& d)
  {
    fo << d.id << DELIMITER;
    fo << d.neighbour << DELIMITER;
    fo << d.nodeIndex << DELIMITER;
    fo << d.c;
    fo << d.energy << DELIMITER;
    return fo;
  }

  friend std::istream&
  operator>> (std::istream& fi, Dev& d)
  {
    fi >> d.id;
    fi >> d.neighbour;
    fi >> d.nodeIndex;
    fi >> d.c;
    fi >> d.energy;
    return fi;
  }
};

enum InhomeTopologyType
{
  START_TOPOLOGY_TYPE, START_PERIMETER_TOPOLOGY_TYPE, BUS_TOPOLOGY_TYPE
};
}



#endif /* SRC_IN_HOME_TOPOLOGY_MODEL_IN_HOME_HEADER_H_ */
