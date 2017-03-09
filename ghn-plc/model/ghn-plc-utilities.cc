/*
 * nc-utilities.cc
 *
 *  Created on: May 27, 2016
 *      Author: tsokalo
 */

#include "ns3/assert.h"
#include "ns3/log.h"

#include <algorithm>
#include <string>
#include <sstream>

#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>
#include <math.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <array>
#include <stdexcept>
#include <utility>

#include "ghn-plc-utilities.h"

typedef struct stat Stat;

#define DELIMITER "\t"

#define SetBit(x,y) (x|=(1<<y))
#define ClrBit(x,y) (x&=~(1<<y))

namespace ns3
{
namespace ghn
{
NS_LOG_COMPONENT_DEFINE("NC_Utilities");
void
CreateLineTopology (PLC_NodeList &node_list, Ptr<PLC_Cable> cable, Ptr<const SpectrumModel> sm, std::vector<uint32_t> distance)
{
  std::cout << "Creating line topology for " << distance.size () + 1 << " nodes" << std::endl;
  uint16_t num_modems = distance.size () + 1;
  node_list.resize (num_modems);

  //
  // Create node_list
  //
  for (uint16_t i = 0; i < num_modems; i++)
    {
      NS_LOG_DEBUG("Creating a modem " << i);
      node_list.at (i) = CreateObject<PLC_Node> ();
      node_list.at (i)->SetName (std::string ("Node ") + std::to_string (i + 1));
    }

  NS_ASSERT(distance.size () + 1 == node_list.size ());

  //
  // Connect node_list with cable
  //
  node_list.at (0)->SetPosition (0, 0, 0);
  for (uint16_t i = 1; i < node_list.size (); i++)
    {
      std::cout << "Set position of node " << i << " at distance of "
              << node_list.at (i - 1)->GetPosition ().x + distance.at (i - 1) << " from the node 0" << std::endl;
      node_list.at (i)->SetPosition (node_list.at (i - 1)->GetPosition ().x + distance.at (i - 1), 0, 0);
      CreateObject<PLC_Line> (cable, node_list.at (i - 1), node_list.at (i));
    }
}
void
CreateStarTopology (PLC_NodeList &node_list, Ptr<PLC_Cable> cable, Ptr<const SpectrumModel> sm, std::vector<uint32_t> distance)
{
  std::cout << "Creating a star topology for " << distance.size () + 1 << " nodes" << std::endl;
  uint16_t num_modems = distance.size () + 1;
  node_list.resize (num_modems);

  //
  // Create node_list
  //
  for (uint16_t i = 0; i < num_modems; i++)
    {
      NS_LOG_DEBUG("Creating a modem " << i);
      node_list.at (i) = CreateObject<PLC_Node> ();
      node_list.at (i)->SetName (std::string ("Node ") + std::to_string (i + 1));
    }

  NS_ASSERT(distance.size () + 1 == node_list.size ());

  //
  // Connect node_list with cable
  //
  node_list.at (0)->SetPosition (0, 0, 0);
  double shift_degree = 2 * 3.14159 / (double) (num_modems - 1);
  double curr_degree = 0;
  for (uint16_t i = 1; i < node_list.size (); i++)
    {
      double X = cos (curr_degree) * distance.at (i - 1) + distance.at (0);
      double Y = sin (curr_degree) * distance.at (i - 1) + distance.at (0);
      std::cout << "Set position of node " << i << " at X " << X << " and Y " << Y << std::endl;

      node_list.at (i)->SetPosition (X, Y, 0);
      CreateObject<PLC_Line> (cable, node_list.at (0), node_list.at (i));
      curr_degree += shift_degree;
    }
}

Ptr<Packet>
GroupEncAckInfoToPkt (GroupEncAckInfo info)
{
  std::stringstream ss;

  //
  // convert G.hn ACK
  //
  ss << info.winStart << DELIMITER;
  ss << info.numRcvSym << DELIMITER;
  ss << info.invalid << DELIMITER;

  ss << info.details.size () << DELIMITER;
  for (auto d : info.details)
    {
      ss << (uint8_t) d << DELIMITER;
    };;

  ss << info.ncAckInfo.size () << DELIMITER;
  for (auto inf : info.ncAckInfo)
    {
      ss << inf.groupId << DELIMITER;
      ss << inf.rcv << DELIMITER;
      ss << inf.use << DELIMITER;
    };;

  std::string str = ss.str ();

  auto bs = GHN_BLKSZ_540;
  auto pkt_size = str.size () + 1;
  uint8_t n_bs = ceil ((double) pkt_size / (double) bs);

  auto pkt = Create<Packet> ((const uint8_t*) (&n_bs), 1);
  pkt->AddAtEnd (Create<Packet> ((const uint8_t*) str.c_str (), str.size ()));

  auto padding_length = n_bs * bs - pkt->GetSize ();
  if (padding_length != 0) pkt->AddPaddingAtEnd (padding_length);

  //
  // convert ANChOR ACK if present
  //
  if (!info.brrFeedback.empty ())
    {
      NS_LOG_UNCOND("Send ANChOR ACK!");
      auto it = info.brrFeedback.begin ();
      while (it != info.brrFeedback.end ())
        {
          pkt->AddAtEnd (*it);
          it++;
        }
    }
  else
    {
      NS_LOG_UNCOND("Send NO ANChOR ACK!");
    }

  return pkt;
}

GroupEncAckInfo
PktToGroupEncAckInfo (Ptr<Packet> pkt)
{
  //
  // convert G.hn ACK
  //
  uint8_t n_bs = 0;
  pkt->CopyData (&n_bs, 1);
  assert(n_bs != 0);

  auto bs = GHN_BLKSZ_540;
  auto pkt_frag = pkt->CreateFragment (0, n_bs * bs);
  pkt_frag->RemoveAtStart (1);

  uint8_t * buffer = new uint8_t[pkt_frag->GetSize () + 1];
  pkt_frag->CopyData (buffer, pkt_frag->GetSize ());

  std::stringstream ss (std::string ((char*) buffer, pkt_frag->GetSize ()));

  GroupEncAckInfo info;

  ss >> info.winStart;
  ss >> info.numRcvSym;
  ss >> info.invalid;

  int16_t s = 0;
  ss >> s;
  while (s-- > 0)
    {
      uint8_t d = false;
      ss >> d;
      info.details.push_back ((bool) d);
    }

  ss >> s;
  while (s-- > 0)
    {
      NcAckInfoItem i;
      ss >> i.groupId;
      ss >> i.rcv;
      ss >> i.use;
      info.ncAckInfo.push_back (i);
    }

  //
  // convert ANChOR ACK if present
  //
  if (pkt->GetSize () > n_bs * bs)
    {
      NS_LOG_UNCOND("Receive ANChOR ACK!");
      auto start = n_bs * bs;
      while (start != pkt->GetSize ())
        {
          assert(start + bs <= pkt->GetSize ());
          auto p = pkt->CreateFragment (start, bs);
          info.brrFeedback.push_back (p);
          start += bs;
        }
    }
  else
    {
      NS_LOG_UNCOND("Receive NO ANChOR ACK!");
    }

  delete[] buffer;
  return info;
}

void
PrintGroupEncAckInfo (GroupEncAckInfo info)
{
  std::cout << "ACK INFO -------------------->>> " << std::endl;
  std::cout << "Details: ";
  for (auto d : info.details)
    {
      std::cout << (uint32_t) d << " ";
    }
  std::cout << std::endl;
  std::cout << "Win start: " << info.winStart << std::endl;
  std::cout << "Number of received symbols: " << info.numRcvSym << std::endl;
  std::cout << "ACK info item: ";
  for (auto i : info.ncAckInfo)
    {
      std::cout << i.groupId << " " << i.rcv << DELIMITER << i.use << DELIMITER << std::endl;
    };;
  std::cout << std::endl;
  std::cout << "ACK INFO <<<-------------------- " << std::endl;
}

FecRateType
ConvertPlcRateToGhnRate (CodingType rate)
{
  switch (rate)
    {
  case CODING_RATE_1_4:
    {
      assert(0);
    }
  case CODING_RATE_1_2:
    {
      return FEC_RATE_1_2;
    }
  case CODING_RATE_2_3:
    {
      return FEC_RATE_2_3;
    }
  case CODING_RATE_16_21:
    {
      return FEC_RATE_16_21;
    }
  case CODING_RATE_5_6:
    {
      return FEC_RATE_5_6;
    }
  case CODING_RATE_16_18:
    {
      return FEC_RATE_16_18;
    }
  case CODING_RATE_20_21:
    {
      return FEC_RATE_20_21;
    }
  case CODING_RATE_RATELESS:
    {
      assert(0);
    }
    }
  assert(0);
}
CodingType
ConvertGhnRateToPlcRate (FecRateType rate)
{
  switch (rate)
    {
  case FEC_RATE_1_2:
    {
      return CODING_RATE_1_2;
    }
  case FEC_RATE_2_3:
    {
      return CODING_RATE_2_3;
    }
  case FEC_RATE_5_6:
    {
      return CODING_RATE_5_6;
    }
  case FEC_RATE_16_18:
    {
      return CODING_RATE_16_18;
    }
  case FEC_RATE_20_21:
    {
      return CODING_RATE_20_21;
    }
    }
  assert(0);
}

double
ConvertCodingTypeToDouble (CodingType ct)
{
  std::array<double, 8> fec_rate =
    { 1.0 / 4.0, 1.0 / 2.0, 2.0 / 3.0, 16.0 / 21.0, 5.0 / 6.0, 16.0 / 18.0, 20.0 / 21.0, 1.0 };

  assert(ct - 1 < fec_rate.size ());
  return fec_rate.at (ct - 1);
}

void
GetDirListing (FileList& result, const std::string& dirpath)
{
  DIR* dir = opendir (dirpath.c_str ());
  if (dir)
    {
      struct dirent* entry;
      while ((entry = readdir (dir)))
        {
          struct stat entryinfo;
          std::string entryname = entry->d_name;
          std::string entrypath = dirpath + "/" + entryname;
          if (!stat (entrypath.c_str (), &entryinfo)) result.push_back (entrypath);
        }
      closedir (dir);
    }
}

int16_t
CreateDirectory (std::string path)
{
  //mode_t mode = 0x0666;
  std::cout << "Creating directory: " << path << std::endl;
  Stat st;
  int32_t status = 0;

  if (stat (path.c_str (), &st) != 0)
    {
      /* Directory does not exist. EEXIST for race condition */
      if (mkdir (path.c_str (), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0 && errno != EEXIST) status = -1;  //, mode
    }
  else if (!S_ISDIR(st.st_mode))
    {
      errno = ENOTDIR;
      status = -1;
    }

  return status;
}

bool
RemoveDirectory (std::string folderPath)
{
  std::cout << "Deleting directory: " << folderPath << std::endl;
  FileList dirtree;
  GetDirListing (dirtree, folderPath);
  int32_t numofpaths = dirtree.size ();

  for (int32_t i = 0; i < numofpaths; i++)
    {
      std::string str (dirtree[i]);
      std::string fullPath = str;

      int32_t pos = 0;
      while (pos != -1)
        {
          pos = str.find ("/");
          str = str.substr (pos + 1);
        }
      if (str == "" || str == "." || str == "..")
        {
          continue;
        }
      struct stat st_buf;
      stat (fullPath.c_str (), &st_buf);
      if (S_ISDIR(st_buf.st_mode))
        {
          RemoveDirectory (fullPath);
        }
      else
        {
          std::remove (fullPath.c_str ());
        }
      rmdir (fullPath.c_str ());
    }
  return true;
}
std::string
GetLogIdent (std::vector<double> vals)
{
  std::stringstream ss;
  for (auto v : vals)
    ss << "_" << v;
  return ss.str ();
}

template<typename T1, typename T2>
  T1
  CopyBits (int start_data, int end_data, T2 data, int start_target, T1 target)
  {
    if (start_data > sizeof(T2) - 1 || end_data > sizeof(T2))
      {
        return 0;
      }

    if (end_data - start_data > sizeof(T1) - start_target)
      {
        return 0;
      }
    int k = 0;
    for (int i = start_data; i < end_data; i++)
      {
        if (data & (1 << i))
          {
            SetBit(target, (start_target + k));
          }
        else
          {
            ClrBit(target, (start_target + k));
          }
        k++;
      }
    return target;
  }
uint8_t
CopyBits (int start_data, int end_data, uint64_t data, int start_target, uint8_t target)
{
  if (start_data > 63 || end_data > 64)
    {
      return 0;
    }

  if (end_data - start_data > 8 - start_target)
    {
      return 0;
    }
  int k = 0;
  for (int i = start_data; i < end_data; i++)
    {
      if (data & (1 << i))
        {
          SetBit(target, (start_target + k));
        }
      else
        {
          ClrBit(target, (start_target + k));
        }
      k++;
    }
  return target;
}
uint64_t
CopyBits (int start_data, int end_data, uint8_t data, int start_target, uint64_t target)
{
  if (start_data > 7 || end_data > 8)
    {
      return 0;
    }
  if (end_data - start_data > 64 - start_target)
    {
      return 0;
    }
  int k = 0;
  for (int i = start_data; i < end_data; i++)
    {
      if (data & (1 << i))
        {
          SetBit(target, (start_target + k));
        }
      else
        {
          ClrBit(target, (start_target + k));
        }
      k++;
    }
  return target;
}
uint8_t
CopyBits (int start_data, int end_data, uint8_t data, int start_target, uint8_t target)
{
  if (start_data > 7 || end_data > 8)
    {
      return 0;
    }

  if (end_data - start_data > 8 - start_target)
    {
      return 0;
    }
  int k = 0;
  for (int i = start_data; i < end_data; i++)
    {
      if (data & (1 << i))
        {
          SetBit(target, (start_target + k));
        }
      else
        {
          ClrBit(target, (start_target + k));
        }
      k++;
    }
  return target;
}

uint8_t
CopyBits (int start_data, int end_data, unsigned int data, int start_target, uint8_t target)
{
  if (start_data > 15 || end_data > 16)
    {
      return 0;
    }

  if (end_data - start_data > 8 - start_target)
    {
      return 0;
    }
  int k = 0;
  for (int i = start_data; i < end_data; i++)
    {
      if (data & (1 << i))
        {
          SetBit(target, (start_target + k));
        }
      else
        {
          ClrBit(target, (start_target + k));
        }
      k++;
    }
  return target;
}
unsigned int
CopyBits (int start_data, int end_data, uint8_t data, int start_target, unsigned int target)
{
  if (start_data > 7 || end_data > 8)
    {
      return 0;
    }

  if (end_data - start_data > 16 - start_target)
    {
      return 0;
    }
  int k = 0;
  for (int i = start_data; i < end_data; i++)
    {
      if (data & (1 << i))
        {
          SetBit(target, (start_target + k));
        }
      else
        {
          ClrBit(target, (start_target + k));
        }
      k++;
    }
  return target;
}
VirtSsn
RotateVarFwrd (VirtSsn toRotate, VirtSsn howFar, VirtSsn cycleSize)
{
  NS_ASSERT_MSG(toRotate < cycleSize, "to rotate:" << toRotate << ", cycle size: " << cycleSize);
  NS_ASSERT(howFar <= cycleSize);

  if (toRotate + howFar < cycleSize)
    return (toRotate + howFar);
  else
    return (toRotate + howFar - cycleSize);
}
VirtSsn
RotateVarBck (VirtSsn toRotate, VirtSsn howFar, VirtSsn cycleSize)
{
  NS_ASSERT(toRotate < cycleSize);
  NS_ASSERT(howFar <= cycleSize);

  if (toRotate >= howFar)
    return (toRotate - howFar);
  else
    return (cycleSize + toRotate - howFar);
}

Ptr<Packet>
ConvertVecToPacket (std::vector<uint8_t> vec)
{
  return Create<Packet> (vec.data (), vec.size ());
}
std::vector<uint8_t>
ConvertPacketToVec (Ptr<Packet> pkt)
{
  std::vector<uint8_t> v (pkt->GetSize ());
  pkt->CopyData (v.data (), pkt->GetSize ());
  return v;
}
GhnBuffer
ConvertVecsToBuffer (std::vector<std::vector<uint8_t> > vecs)
{
  GhnBuffer buf;
  for (auto vec : vecs)
    buf.push_back (ConvertVecToPacket (vec));
  return buf;
}
void
ThrowToFile (std::string line, std::string path)
{
  std::ofstream f (path, std::ios_base::out | std::ios_base::app);
  f << line << std::endl;
  f.close ();
}

std::string
ConstructResFoldName (int argc, char *argv[])
{
  std::stringstream ss;
  std::string path = argv[0]; // get path from argument 0
  path = path.substr (0, path.rfind ("/") + 1);
  ss << path << "Results";

  //  for (uint16_t i = 1; i < argc; i++)
  //    {
  //      ss << "_" << argv[1];
  //    }
  ss << "/";
  return ss.str ();
}

}
}
