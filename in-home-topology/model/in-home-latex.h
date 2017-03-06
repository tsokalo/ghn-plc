/*
 * in-home-latex.h
 *
 *  Created on: 06.03.2017
 *      Author: tsokalo
 */

#ifndef SRC_IN_HOME_TOPOLOGY_MODEL_IN_HOME_LATEX_H_
#define SRC_IN_HOME_TOPOLOGY_MODEL_IN_HOME_LATEX_H_

#include <vector>
#include <fstream>
#include <iostream>
#include <iomanip>

#include "in-home-header.h"

namespace ns3
{
class TopologyDrawer
{
  enum DevType
  {
    ELECTRICAL_DEV_TYPE, MODEM_DEV_TYPE, DISTRBOX_DEV_TYPE
  };

  typedef uint16_t dev_i;
  struct RoomDesc
  {
    RoomDesc (uint16_t i, Coordinate c, double s)
    {
      this->i = i;
      this->c = c;
      this->s = s;
    }
    uint16_t i;
    Coordinate c;
    double s;
  };
  struct DevDesc
  {
    DevDesc (dev_i i, Coordinate c, DevType t)
    {
      this->i = i;
      this->c = c;
      this->t = t;
    }
    dev_i i;
    Coordinate c;
    DevType t;
  };
  struct Edge
  {
    Edge (dev_i i, dev_i j)
    {
      this->i = i;
      this->j = j;
    }
    dev_i i;
    dev_i j;
  };

public:

  TopologyDrawer ()
  {

  }
  ~TopologyDrawer ()
  {

  }

  void
  AddElDev (dev_i i, Coordinate c)
  {
    m_devs.push_back (DevDesc (i, c, ELECTRICAL_DEV_TYPE));
  }
  void
  AddModem (dev_i i, Coordinate c)
  {
    m_devs.push_back (DevDesc (i, c, MODEM_DEV_TYPE));
  }
  void
  AddDisBx (dev_i i, Coordinate c)
  {
    m_devs.push_back (DevDesc (i, c, DISTRBOX_DEV_TYPE));
  }
  void
  Connect (dev_i i, dev_i j)
  {
    m_edges.push_back (Edge (i, j));
  }

  void
  SetRoom (uint16_t i, Coordinate c, double s)
  {
    m_rooms.push_back (RoomDesc (i, c, s));
  }

  void
  Draw (std::string path)
  {
    std::ofstream f (path + "topology.tex", std::ios::out);

    f << "\\newcommand{\\inhometopology}[0]{" << std::endl;
    f << "\\begin{tikzpicture}" << std::endl;
    f << "[" << std::endl;
    f << "    scale=0.6, " << std::endl;
    f << "    transform shape" << std::endl;
    f << "    ] " << std::endl;

    double dim = ceil(sqrt(m_rooms.size())) * m_rooms.begin()->s;
    f << std::setprecision(3) << "\\sizearrowleft{(0,0)}{(0," << dim << ")}{" << dim << " meter};" << std::endl;
    f << std::setprecision(3) <<"\\sizearrowbelow{(0,0)}{(" << dim << ",0)}{" << dim << " meter};" << std::endl;
    f << std::setprecision(6) << std::endl;

    for (auto r : m_rooms)
      {
        f << "\\draw [fill=room backg](" << r.c.x << "," << r.c.y << ") rectangle (" << r.c.x + r.s << "," << r.c.y + r.s
                << ");" << std::endl;
      }

    for (auto e : m_edges)
      {
        Coordinate i, j;
        for (auto d : m_devs)
          if (d.i == e.i) i = d.c;
        for (auto d : m_devs)
          if (d.i == e.j) j = d.c;
        f << "\\draw[color=el cable, line width=2pt] (" << i.x << "," << i.y << ") -- (" << j.x << "," << j.y << ");"
                << std::endl;
      }

    for (auto d : m_devs)
      {
        if (d.t == ELECTRICAL_DEV_TYPE)
          {
            f << "\\node[inner sep=0pt] (n" << d.i << ") at (" << d.c.x << "," << d.c.y << ") "
                    << "{\\includegraphics[width=.09\\textwidth]{el_dev.png}};" << std::endl;
          }
        if (d.t == MODEM_DEV_TYPE)
          {
            f << "\\node[inner sep=0pt] (n" << d.i << ") at (" << d.c.x << "," << d.c.y << ") "
                    << "{\\includegraphics[width=.09\\textwidth]{modem.png}};" << std::endl;
          }
        if (d.t == DISTRBOX_DEV_TYPE)
          {
            f << "\\node[inner sep=0pt] (n" << d.i << ") at (" << d.c.x << "," << d.c.y << ") "
                    << "{\\includegraphics[width=.09\\textwidth]{dist_box.png}};" << std::endl;
          }
      }

    f << "\\end{tikzpicture}" << std::endl;
    f << "}" << std::endl;

    f.close ();
  }

private:

  std::vector<DevDesc> m_devs;
  std::vector<RoomDesc> m_rooms;
  std::vector<Edge> m_edges;

};
}

#endif /* SRC_IN_HOME_TOPOLOGY_MODEL_IN_HOME_LATEX_H_ */
