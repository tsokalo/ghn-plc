/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef __INHOMETOPOLOGY_H__
#define __INHOMETOPOLOGY_H__

#include <fstream>
#include <vector>
#include <deque>
#include <stdint.h>
#include <math.h>
#include "ns3/plc-channel.h"

#define DERIVATION_BOX_ID       0
#define PLC_MODEM_ENERGY        1

#define DELIMITER '\t'

namespace ns3
{

/*
 * These classes are supported with data and ideas from following publications:
 *
 * 1. Bottom-Up Statistical PLC Channel Modeling—Part I: Random Topology Model and Efficient Transfer Function Computation
 *    Andrea M. Tonello, Member, IEEE, and Fabio Versolatto, Student Member, IEEE
 *
 * 2. Energie-Info BDEW Bundesverband der Energie- und Wasserwirtschaft e.V. Stromverbrauch im Haushalt, Berlin, Oktober 2013
 *
 * 3. Ausstattung mit Gebrauchsgütern. Ausstattung privater Haushalte mit Haushaltsß und sonstigen Geräten Deutschland. Statistisches Bundesamt.
 */

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

class Room
{
public:
  Room (double L);
  virtual
  ~Room ();

  void
  AddDevice (uint16_t devId)
  {
    Dev dev;
    dev.id = devId;
    m_dev.push_back (dev);
  }

  void
  Create ();

  std::deque<Dev>
  GetDevs ()
  {
    return m_dev;
  }

  friend std::ostream&
  operator<< (std::ostream& fo, const Room& r)
  {
    fo << r.m_vb << DELIMITER;
    fo << (uint32_t)r. m_dev.size () << DELIMITER;
    for (auto dev : r.m_dev)
      fo << dev;
    fo << r.m_L << DELIMITER;
    fo << (uint32_t)r.m_topologyType << DELIMITER;
    return fo;
  }

  friend std::istream&
  operator>> (std::istream& fi, Room& r)
  {
    fi >> r.m_vb;
    uint32_t n = 0;
    fi >> n;
    r.m_dev.clear();
    for (uint32_t i = 0; i < n; i++)
      {
        Dev dev;
        fi >> dev;
        r.m_dev.push_back (dev);
      }
    fi >> r.m_L;
    fi >> n;
    r.m_topologyType = InhomeTopologyType(n);
    return fi;
  }


  double
  GetLength (double maxLength);
  /*
   * Verteilungsbox / Anschlusskasten
   */
  Dev m_vb;
  /*
   * indexes of electrical devices
   */
  std::deque<Dev> m_dev;

  /*
   * room width/length
   * unit [m]
   */
  double m_L;

  InhomeTopologyType m_topologyType;
};

class InhomeTopology
{
public:

  InhomeTopology (double totalSquare, uint16_t numModems, uint16_t numDevs);
  virtual
  ~InhomeTopology ();

  std::vector<Dev>
  GetPositions ()
  {
    return m_devs;
  }

  void
  Create (PLC_NodeList &nodes, Ptr<PLC_Cable> cable);
  void
  Save (std::ostream &fo);
  void
  Load (PLC_NodeList &nodes, Ptr<PLC_Cable> cable, std::istream &fi);

private:

  void
  CreateRooms (uint16_t roomNum);

  uint16_t
  GetNumPersons ();
  /*
   * calculate total consumption of a flat/house in kWh per year
   */
  double
  GetTotalEnery (uint16_t numPerson);
//  void
//  InitDevs (double totalkWh);
  void
  InitDevs ();
  void
  InitModems ();
  void
  SetupModems(PLC_NodeList &nodes, Ptr<PLC_Cable> cable);
  /*
   * all rooms of the same size and have a form of a square
   * unit [m2]
   */
  double m_roomSqaure;
  double m_L;
  /*
   * i - room number
   * j - device index (referred to m_devEnergy);
   */
  std::vector<Room*> m_rooms;

  std::vector<double> m_devEnergy;

  std::vector<Dev> m_devs;
  std::vector<Coordinate> m_roomOrigins;

  uint16_t m_numModems;
  uint16_t m_numDevs;
};

}

#endif /* __IN_HOME_TOPOLOGY_H__ */

