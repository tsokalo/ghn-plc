/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/random-variable-stream.h"

#include "ns3/log.h"
#include "ns3/assert.h"

#include "in-home-topology.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("InhomeTopology");

Room::Room (double L) :
        m_L (L)
{
  UniformRandomVariable uniRV;
  m_vb.id = DERIVATION_BOX_ID;
  m_vb.c.x = uniRV.GetValue (0, m_L / 4);
  m_vb.c.y = uniRV.GetValue (0, m_L / 4);
  m_vb.energy = 0;
  m_vb.neighbour = DERIVATION_BOX_ID;

  m_topologyType = InhomeTopologyType (uniRV.GetInteger (0, 2));
}
Room::~Room ()
{
  m_dev.clear ();
}
void
Room::Create ()
{
  NS_LOG_DEBUG("Creating room with topology type: " << m_topologyType);
  UniformRandomVariable uniRV;

  switch (m_topologyType)
    {
  case START_TOPOLOGY_TYPE:
    {
      for (uint16_t devIndex = 0; devIndex < m_dev.size (); devIndex++)
        {
          double length = GetLength (sqrt (2) * m_L);

          Coordinate c;
          if (length < m_L)
            {
              c.x = 0;
              c.y = length;
            }
          else
            {
              c.x = sqrt (length * length - m_L * m_L);
              c.y = m_L;
            }
          if (uniRV.GetInteger (0, 1) == 0)
            {
              double val = c.x;
              c.x = c.y;
              c.y = val;
            }
          m_dev.at (devIndex).c = c;
          m_dev.at (devIndex).neighbour = m_vb.id;
          NS_LOG_DEBUG(
                  "Device index: " << devIndex << ", id: " << m_dev.at (devIndex).id << ", neighbour: " << m_dev.at (devIndex).neighbour << ", x: " << m_dev.at (devIndex).c.x << ", y: " << m_dev.at (devIndex).c.y);
        }
      break;
    }
  case START_PERIMETER_TOPOLOGY_TYPE:
    {
      for (uint16_t devIndex = 0; devIndex < m_dev.size (); devIndex++)
        {
          double length = GetLength (2 * m_L);

          Coordinate c;
          if (length < m_L)
            {
              c.x = 0;
              c.y = length;
            }
          else
            {
              c.x = length - m_L;
              c.y = m_L;
            }
          if (uniRV.GetInteger (0, 1) == 0)
            {
              double val = c.x;
              c.x = c.y;
              c.y = val;
            }
          m_dev.at (devIndex).c = c;
          m_dev.at (devIndex).neighbour = m_vb.id;
          NS_LOG_DEBUG(
                  "Device index: " << devIndex << ", id: " << m_dev.at (devIndex).id << ", neighbour: " << m_dev.at (devIndex).neighbour << ", x: " << m_dev.at (devIndex).c.x << ", y: " << m_dev.at (devIndex).c.y);
        }
      break;
    }
  case BUS_TOPOLOGY_TYPE:
    {
      double maxLength = 4 * m_L;
      std::vector<double> parts;
      for (uint16_t devIndex = 0; devIndex < m_dev.size (); devIndex++)
        {
          double length = GetLength (maxLength / static_cast<double> (m_dev.size () - devIndex));
          maxLength -= length;
          NS_ASSERT(maxLength > 0);
          parts.push_back (maxLength);

          if (devIndex == 0)
            m_dev.at (devIndex).neighbour = m_vb.id;
          else
            m_dev.at (devIndex).neighbour = m_dev.at (devIndex - 1).id;
        }

      NS_ASSERT(parts.size () == m_dev.size ());

      for (uint16_t devIndex = 0; devIndex < m_dev.size (); devIndex++)
        {
          Coordinate c;
          if (parts.at (devIndex) < m_L)
            {
              c.x = 0;
              c.y = parts.at (devIndex);
            }
          else if (parts.at (devIndex) < 2 * m_L)
            {
              c.x = parts.at (devIndex) - m_L;
              c.y = m_L;
            }
          else if (parts.at (devIndex) < 3 * m_L)
            {
              c.x = m_L;
              c.y = parts.at (devIndex) - 2 * m_L;
            }
          else
            {
              c.x = parts.at (devIndex) - 3 * m_L;
              c.y = 0;
            }
          m_dev.at (devIndex).c = c;
          NS_LOG_DEBUG(
                  "Device index: " << devIndex << ", id: " << m_dev.at (devIndex).id << ", neighbour: " << m_dev.at (devIndex).neighbour << ", x: " << m_dev.at (devIndex).c.x << ", y: " << m_dev.at (devIndex).c.y);
        }
      break;
    }
    }

  m_dev.push_front (m_vb);
}

double
Room::GetLength (double maxLength)
{
  UniformRandomVariable uniRV;
  return uniRV.GetValue (0, maxLength);
}
InhomeTopology::InhomeTopology (double totalSquare, uint16_t numModems, uint16_t numDevs) :
        m_numModems (numModems), m_numDevs (numDevs)
{
  UniformRandomVariable uniRV;
  uniRV.SetStream(1);
  m_roomSqaure = uniRV.GetValue (15, 45);
  NS_LOG_DEBUG("Sqaure of each room: " << m_roomSqaure);
  m_L = sqrt (m_roomSqaure);
  uint16_t roomNum = floor (totalSquare / m_roomSqaure);
  roomNum = (roomNum < 3) ? 3 : roomNum;

  CreateRooms (roomNum);
}
InhomeTopology::~InhomeTopology ()
{
  for (uint16_t roomIndex = 0; roomIndex < m_rooms.size (); roomIndex++)
    delete m_rooms.at (roomIndex);
  m_rooms.clear ();
  m_devEnergy.clear ();
  m_devs.clear ();
  m_roomOrigins.clear ();
}
void
InhomeTopology::Create (PLC_NodeList &nodes, Ptr<PLC_Cable> cable)
{
  //  InitDevs (GetTotalEnery (GetNumPersons ()));
  InitDevs ();
  InitModems ();

  uint16_t numDevPerRoom = ceil (static_cast<double> (m_devEnergy.size ()) / static_cast<double> (m_rooms.size ()));
  NS_LOG_DEBUG("Number of devices per room " << numDevPerRoom);
  uint16_t roomIndex = 0;
  for (uint16_t devIndex = 0; devIndex < m_devEnergy.size (); devIndex++)
    {
      m_rooms.at (roomIndex)->AddDevice (devIndex + 1);
      if (m_rooms.at (roomIndex)->GetDevs ().size () == numDevPerRoom || devIndex == m_devEnergy.size () - 1)
        {
          roomIndex++;
        }
    }
  for (uint16_t roomIndex = 0; roomIndex < m_rooms.size (); roomIndex++)
    {
      m_rooms.at (roomIndex)->Create ();
      NS_LOG_DEBUG("In room " << roomIndex << " there is " << m_rooms.at (roomIndex)->GetDevs ().size() << " device(s)");
    }

  UniformRandomVariable uniRV;
  uint16_t mainRoom = uniRV.GetInteger (0, m_rooms.size () - 1);
  uint16_t vertBoxCounter = 0;
  NS_LOG_DEBUG("Selected main room with index " << mainRoom);

  uint16_t devIndexG = 0;
  for (uint16_t roomIndex = 0; roomIndex < m_rooms.size (); roomIndex++)
    {
      std::deque<Dev> devs = m_rooms.at (roomIndex)->GetDevs ();
      for (uint16_t devIndex = 0; devIndex < devs.size (); devIndex++)
        {
          devs.at (devIndex).c.x += m_roomOrigins.at (roomIndex).x;
          devs.at (devIndex).c.y += m_roomOrigins.at (roomIndex).y;
          m_devs.push_back (devs.at (devIndex));
//          NS_ASSERT (devIndexG < m_devEnergy.size ());
          if (devIndex != 0)
            {
              m_devs.at (m_devs.size () - 1).energy = m_devEnergy.at (devIndexG++);
              if (roomIndex != mainRoom)
                {
                  if (m_devs.at (m_devs.size () - 1).neighbour == DERIVATION_BOX_ID)
                    {
                      m_devs.at (m_devs.size () - 1).neighbour = m_devEnergy.size () + vertBoxCounter;
                    }
                }
            }
          else // if a distribution box
            {
              if (roomIndex != mainRoom)
                {
                  m_devs.at (m_devs.size () - 1).energy = 0;
                  m_devs.at (m_devs.size () - 1).id = m_devEnergy.size () + vertBoxCounter + 1;
                  m_devs.at (m_devs.size () - 1).neighbour = DERIVATION_BOX_ID;
                  vertBoxCounter++;
                }
            }
        }
    }

    {
      uint16_t emptyRoomsCounter = 0;
      for (uint16_t roomIndex = 0; roomIndex < m_rooms.size (); roomIndex++)
        {
          std::deque<Dev> devs = m_rooms.at (roomIndex)->GetDevs ();
          if (devs.size () == 0) emptyRoomsCounter++;
        }

      NS_ASSERT(m_devs.size () == m_devEnergy.size () + m_rooms.size () - emptyRoomsCounter);
    }
    SetupModems(nodes, cable);
}
void
InhomeTopology::SetupModems(PLC_NodeList &nodes, Ptr<PLC_Cable> cable)
{
  for (uint16_t devIndex = 0; devIndex < m_devs.size (); devIndex++)
    {
      if (m_devs.at (devIndex).energy == 1)
        {
          NS_LOG_DEBUG("Device " << m_devs.at (devIndex).id << " is a PLC modem");
          m_draw.AddModem(m_devs.at (devIndex).id, m_devs.at (devIndex).c);
        }
      else if (m_devs.at (devIndex).energy == 0)
        {
          NS_LOG_DEBUG("Device " << m_devs.at (devIndex).id << " is a distribution box");
          m_draw.AddDisBx(m_devs.at (devIndex).id, m_devs.at (devIndex).c);
        }
      else
        {
          NS_LOG_DEBUG("Device " << m_devs.at (devIndex).id << " is an in-home appliance");
          m_draw.AddElDev(m_devs.at (devIndex).id, m_devs.at (devIndex).c);
        }

    }
  NS_LOG_DEBUG("Id" << " " << "Neighbour" << "\t" << "X" << "\t" << "Y");
  for (uint16_t devIndex = 0; devIndex < m_devs.size (); devIndex++)
    {
      NS_LOG_DEBUG(
              m_devs.at (devIndex).id << "\t" << m_devs.at (devIndex).neighbour << "\t" << m_devs.at (devIndex).c.x << "\t" << m_devs.at (devIndex).c.y);
      m_draw.Connect(m_devs.at (devIndex).id, m_devs.at (devIndex).neighbour);
    }

  //
  // create additional nodes, which will work as distribution boxes
  //
  uint16_t numDistrBoxes = m_devs.size () - nodes.size ();
  nodes.resize (nodes.size () + numDistrBoxes);
  for (uint16_t nodeIndex = nodes.size () - numDistrBoxes; nodeIndex < nodes.size (); nodeIndex++)
    {
      NS_LOG_DEBUG("Creating a distribution box node " << nodeIndex);
      nodes.at (nodeIndex) = CreateObject<PLC_Node> ();
      nodes.at (nodeIndex)->SetName (std::string ("Distribution box ") + std::to_string (nodeIndex + 1 - nodes.size () + numDistrBoxes));
    }
  //
  // assign positions to the nodes
  //
  for (uint16_t nodeIndex = 0; nodeIndex < nodes.size ();)
    {
      for (uint16_t devIndex = 0; devIndex < m_devs.size (); devIndex++)
        {
          if (m_devs.at (devIndex).energy == 1)
            {
              NS_LOG_DEBUG(
                      "Setting position of modem dev " << m_devs.at (devIndex).id << ", which associates with node (index) " << nodeIndex);
              nodes.at (nodeIndex)->SetPosition (m_devs.at (devIndex).c.x, m_devs.at (devIndex).c.y, 0);
              m_devs.at (devIndex).nodeIndex = nodeIndex;
              nodeIndex++;
            }
        }
      for (uint16_t devIndex = 0; devIndex < m_devs.size (); devIndex++)
        {
          if (m_devs.at (devIndex).energy != 0 && m_devs.at (devIndex).energy != 1)
            {
              NS_LOG_DEBUG(
                      "Setting position of electrical device  dev " << m_devs.at (devIndex).id << ", which associates with node (index) " << nodeIndex);
              nodes.at (nodeIndex)->SetPosition (m_devs.at (devIndex).c.x, m_devs.at (devIndex).c.y, 0);
              m_devs.at (devIndex).nodeIndex = nodeIndex;
              nodeIndex++;
            }
        }
      for (uint16_t devIndex = 0; devIndex < m_devs.size (); devIndex++)
        {
          if (m_devs.at (devIndex).energy == 0)
            {
              NS_LOG_DEBUG(
                      "Setting position of distribution box  dev " << m_devs.at (devIndex).id << ", which associates with node (index) " << nodeIndex);
              nodes.at (nodeIndex)->SetPosition (m_devs.at (devIndex).c.x, m_devs.at (devIndex).c.y, 0);
              m_devs.at (devIndex).nodeIndex = nodeIndex;
              nodeIndex++;
            }
        }

    }
  //
  // Connected nodes with cables
  //
  for (uint16_t devIndex = 0; devIndex < m_devs.size (); devIndex++)
    {
      for (uint16_t devIndex2 = 0; devIndex2 < m_devs.size (); devIndex2++)
        {
          if (devIndex == devIndex2) continue;

          if (m_devs.at (devIndex).neighbour == m_devs.at (devIndex2).id)
            {
              NS_LOG_DEBUG(
                      "Connecting devs with ids " << m_devs.at (devIndex).id << " and " << m_devs.at (devIndex2).id << " associated with nodes (indexes) " << m_devs.at (devIndex).nodeIndex << " and " << m_devs.at (devIndex2).nodeIndex);
              CreateObject<PLC_Line> (cable, nodes.at (m_devs.at (devIndex).nodeIndex),
                      nodes.at (m_devs.at (devIndex2).nodeIndex));
              //
              // assuming tree structure. No Loops!
              //
              break;
            }
        }
    }
}
void
InhomeTopology::CreateRooms (uint16_t roomNum)
{
  m_rooms.resize (roomNum);
  m_roomOrigins.resize (roomNum);
  uint16_t roomMatrixSize = ceil (sqrt (static_cast<double> (roomNum)));
  for (uint16_t roomIndex = 0; roomIndex < m_rooms.size (); roomIndex++)
    {
      Coordinate c;
      c.x = floor (static_cast<double> (roomIndex) / static_cast<double> (roomMatrixSize)) * m_L;
      c.y = fmod (static_cast<double> (roomIndex), static_cast<double> (roomMatrixSize)) * m_L;
      m_rooms.at (roomIndex) = new Room (m_L);
      m_roomOrigins.at (roomIndex) = c;
      m_draw.SetRoom(roomIndex, c, m_L);
      NS_LOG_DEBUG("Room " << roomIndex << ": width/length - " << m_L << ", orig.x: " << c.x << ", orig.y: " << c.y);
    }
}

uint16_t
InhomeTopology::GetNumPersons ()
{
  NS_ASSERT(m_rooms.size () > 2);
  return m_rooms.size () - 2;
}
/*
 * calculate total consumption of a flat/house in kWh per year
 */
double
InhomeTopology::GetTotalEnery (uint16_t numPerson)
{
  NS_LOG_DEBUG("Number of persons per flat/house: " << numPerson);
  std::vector<double> energyPerPerson;
  /*
   * kWh per year per person for different number of persons in a flat/house
   * i - number of persons in a flat/house
   */
  energyPerPerson.push_back (2050);
  energyPerPerson.push_back (1720);
  energyPerPerson.push_back (1350);
  energyPerPerson.push_back (1188);
  energyPerPerson.push_back (1074);

  if (numPerson >= energyPerPerson.size ()) return energyPerPerson.at (energyPerPerson.size () - 1) * numPerson;
  return energyPerPerson.at (numPerson - 1) * numPerson;
}
//void
//InhomeTopology::InitDevs (double totalkWh)
//{
//  NS_LOG_DEBUG("Total energy (kWh): " << totalkWh);
//  totalkWh = totalkWh / 365 * 1000 / 24 * 5;
//  NS_LOG_DEBUG("Maximal expected power (W): " << totalkWh);
//  ExponentialVariable expRV (709.7424);
//  double devEnergy = 0;
//  do
//    {
//      devEnergy = expRV.GetValue ();
//      if (totalkWh - devEnergy <= 0)
//        {
//          m_devEnergy.push_back (totalkWh);
//          NS_LOG_DEBUG("Energy for the dev " << (m_devEnergy.size() - 1) << ": " << devEnergy);
//          break;
//        }
//      m_devEnergy.push_back (devEnergy);
//      totalkWh -= devEnergy;
//      NS_LOG_DEBUG("Energy for the dev " << (m_devEnergy.size() - 1) << ": " << devEnergy);
//    }
//  while (1);
//}
void
InhomeTopology::InitDevs ()
{
  ExponentialRandomVariable expRV;
  double devEnergy = 0;
  uint16_t counter = 0;
  double totalkWh = 0;
  do
    {
      devEnergy = expRV.GetValue (709.7424, 100 * 709.7424);
      m_devEnergy.push_back (devEnergy);
      totalkWh += devEnergy;
      NS_LOG_DEBUG("Power for the device (W): " << (m_devEnergy.size() - 1) << ": " << devEnergy);
    }
  while (++counter != m_numDevs);NS_LOG_DEBUG("Total power (W): " << totalkWh);
}

void
InhomeTopology::InitModems ()
{
  UniformRandomVariable uniRV;
  for (uint16_t modemIndex = 0; modemIndex < m_numModems; modemIndex++)
    {
      m_devEnergy.insert (m_devEnergy.begin () + uniRV.GetInteger (0, m_devEnergy.size () - 1), (double) PLC_MODEM_ENERGY);
    }
}

void
InhomeTopology::Save (std::ostream &fo)
{
  fo << m_roomSqaure << DELIMITER;
  fo << m_L << DELIMITER;
  fo << (uint32_t) m_rooms.size () << DELIMITER;
  for (auto room : m_rooms)
    fo << *room;
  fo << (uint32_t) m_devEnergy.size () << DELIMITER;
  for (auto en : m_devEnergy)
    fo << en << DELIMITER;
  fo << (uint32_t) m_devs.size () << DELIMITER;
  for (auto dev : m_devs)
    fo << dev;
  fo << (uint32_t) m_roomOrigins.size () << DELIMITER;
  for (auto rc : m_roomOrigins)
    fo << rc;
}
void
InhomeTopology::Load (PLC_NodeList &nodes, Ptr<PLC_Cable> cable, std::istream &fi)
{
  fi >> m_roomSqaure;
  fi >> m_L;
  uint32_t n = 0;
  fi >> n;
  m_rooms.clear ();
  for (uint32_t i = 0; i < n; i++)
    {
      Room* room = new Room (m_L);
      fi >> *room;
      m_rooms.push_back (room);
    }
  fi >> n;
  m_devEnergy.clear ();
  for (uint32_t i = 0; i < n; i++)
    {
      double v = 0;
      fi >> v;
      m_devEnergy.push_back (v);
    }
  fi >> n;
  m_devs.clear ();
  for (uint32_t i = 0; i < n; i++)
    {
      Dev dev;
      fi >> dev;
      m_devs.push_back (dev);
    }
  fi >> n;
  m_roomOrigins.clear ();
  for (uint32_t i = 0; i < n; i++)
    {
      Coordinate c;
      fi >> c;
      m_roomOrigins.push_back (c);
    }

  SetupModems(nodes, cable);
}
void
InhomeTopology::Draw(std::string path)
{
  m_draw.Draw(path);
}

}

