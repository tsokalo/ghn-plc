/*
 * main.cpp
 *
 *  Created on: 24.06.2016
 *      Author: tsokalo
 */
#include <iostream>
#include <fstream>
#include <sstream>
#include <array>
#include <thread>
#include <memory>
#include <limits>
#include <random>
#include <vector>
#include <stdexcept>
#include <map>
#include <utility>
#include <algorithm>
#include <functional>
#include <cmath>

#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>

#include "ghn-plc-statistics.h"

using namespace std;

#define MAC_WARMUP_LINES        15
#define APP_WARMUP_LINES        50

typedef std::vector<std::string> FileList;
typedef struct stat Stat;

struct log_data_t
{
  std::vector<uint32_t> mac_bytes;
  std::vector<double> iat;
  std::vector<double> app_bytes;
};

typedef uint16_t csma_t;// 0 = CA, 4 = CD
typedef uint16_t num_senders_t;
typedef uint16_t cw_t;
typedef std::map<csma_t, std::map<num_senders_t, std::map<cw_t, log_data_t> > > database_t;

struct ResStat
{
  uint64_t macBytes;
  uint32_t numMpdus;
  st_pair_t appGain;
};
typedef std::map<csma_t, std::map<num_senders_t, std::map<cw_t, ResStat> > > res_stat_t;
typedef std::map<csma_t, std::map<num_senders_t, std::map<cw_t, uint64_t> > > sim_duration_t;

struct folder_name_mask
{
  folder_name_mask&
  operator= (const folder_name_mask& other)
  {
    if (this != &other)
      {
        this->ns = other.ns;
        this->cw = other.cw;
        this->t = other.t;
      }
    return *this;
  }
  num_senders_t ns;
  cw_t cw;
  csma_t t;
};

//////////////////////////////////////////////////////////////////////////////

void
GetDirListing (FileList& result, const std::string& dirpath);
int16_t
CreateDirectory (std::string path);
bool
RemoveDirectory (std::string folderPath);
FileList
FindFiles (std::string searchPath, std::string filePartName);
folder_name_mask
GetFolderNameMask (std::string dir);

//////////////////////////////////////////////////////////////////////////////

double
GetMacEfficiency (uint64_t totalMacBytes, uint64_t sim_dur);
double
AverageTp (uint64_t totalMacBytes, uint32_t numMpdus);

int
main (int argc, char **args)
{
  //
  // Create results folder
  //
  std::string path = args[0]; // get path from argument 0
  path = path.substr (0, path.rfind ("/") + 1);
  std::string pf = path + "Results/";
  RemoveDirectory (pf);
  CreateDirectory (pf);
  std::cout << "Results are saved to " << pf << std::endl;

  //
  // Read the input data
  //
  assert(argc > 1);

  std::string resDir = args[1];

  FileList dirs;
  GetDirListing (dirs, resDir);
  for (FileList::iterator it = dirs.begin (); it != dirs.end ();)
    {
      if (it->find ("Results") == std::string::npos)
        {
          dirs.erase (it);
        }
      else
        {
          it++;
        }
    }

  database_t db;
  sim_duration_t sd;
  for(auto d : dirs)
    {
      std::cout <<"Reading log in " << d << std::endl;

      auto m = GetFolderNameMask(d);
      log_data_t ld;

        {
          std::string name_key = "app_data_0";
          FileList fl = FindFiles (d, name_key);
          assert(fl.size() == 1);

          ifstream f (fl.at(0), ios_base::in);
          string line;
          int16_t c = APP_WARMUP_LINES;

          while (getline (f, line, '\n'))
            {
              if(c-- > 0)continue;
              stringstream ss (line);
              uint32_t v, app_bytes, iat;
              ss >> v;
              ss >> v;
              ss >> v;
              ss >> v;
              ss >> iat;
              ss >> app_bytes;
              ld.iat.push_back(iat);
              ld.app_bytes.push_back(app_bytes * 8);
            }
          f.close ();
        }

      uint64_t sim_dur = 0;
      uint64_t sim_start = 0;
        {
          std::string name_key = "mac_rcv_bits_0";
          FileList fl = FindFiles (d, name_key);
          assert(fl.size() == 1);

          ifstream f (fl.at(0), ios_base::in);
          string line;
          int16_t c = MAC_WARMUP_LINES;

          while (getline (f, line, '\n'))
            {
              stringstream ss (line);
              if(c-- > 0)
                {
                  ss >> sim_start;
                  continue;
                }

              uint32_t v;
              uint32_t mb;
              ss >> sim_dur;
              ss >> v;
              ss >> v;
              ss >> v;
              ss >> mb;
              ld.mac_bytes.push_back(mb);
            }
          f.close ();
        }

      auto check_ld_empty = [](log_data_t ld)->bool
        {
          return (ld.mac_bytes.empty() && ld.iat.empty() && ld.app_bytes.empty());
        };;

      assert(check_ld_empty(db[m.t][m.ns][m.cw]));
      db[m.t][m.ns][m.cw] = ld;
      sd[m.t][m.ns][m.cw] = sim_dur - sim_start;
    }

  //
  // Evaluate input data
  //
  res_stat_t rs;

  typedef std::map<num_senders_t, std::map<cw_t, log_data_t> >::iterator s_it;
  typedef std::map<cw_t, log_data_t>::iterator ss_it;

  for (database_t::iterator di = db.begin (); di != db.end (); di++)
    for (s_it dii = di->second.begin (); dii != di->second.end (); dii++)
      for (ss_it diii = dii->second.begin (); diii != dii->second.end (); diii++)
        {
          rs[di->first][dii->first][diii->first].macBytes = std::accumulate (diii->second.mac_bytes.begin (),
                  diii->second.mac_bytes.end (), 0);
          rs[di->first][dii->first][diii->first].numMpdus = diii->second.mac_bytes.size ();
          rs[di->first][dii->first][diii->first].appGain = CalcStatsByFieller (bi_rv_t (diii->second.app_bytes,
                  diii->second.iat));
        }

  //
  // Write results to the file
  //
  ofstream f (pf + "stats.txt", ios_base::out);

  typedef std::map<num_senders_t, std::map<cw_t, ResStat> >::iterator rs_it;
  typedef std::map<cw_t, ResStat>::iterator rss_it;

  for (res_stat_t::iterator di = rs.begin (); di != rs.end (); di++)
    for (rs_it dii = di->second.begin (); dii != di->second.end (); dii++)
      for (rss_it diii = dii->second.begin (); diii != dii->second.end (); diii++)
        {
          uint64_t sim_dur = sd.at (di->first).at (dii->first).at (diii->first);
          f << di->first << "\t" << dii->first << "\t" << diii->first << "\t" << diii->second.macBytes << "\t"
                  << diii->second.numMpdus << "\t" << diii->second.appGain.first << "\t" << diii->second.appGain.second << "\t"
                  << GetMacEfficiency (diii->second.macBytes, sim_dur) << "\t" << AverageTp (diii->second.macBytes,
                  diii->second.numMpdus) << std::endl;
        }

  f.close ();

  cout << "Program finished successfully" << endl;
  return 0;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


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
  Stat st;
  int32_t status = 0;

  if (stat (path.c_str (), &st) != 0)
    {
      /* Directory does not exist. EEXIST for race condition */
      if (mkdir (path.c_str (), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0 && errno != EEXIST) status = -1;//, mode
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
  cout << "Deleting directory: " << folderPath << endl;
  FileList dirtree;
  GetDirListing (dirtree, folderPath);
  int32_t numofpaths = dirtree.size ();

  for (int32_t i = 0; i < numofpaths; i++)
    {
      string str (dirtree[i]);
      string fullPath = str;

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
      if (S_ISDIR (st_buf.st_mode))
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
FileList
FindFiles (std::string searchPath, std::string filePartName)
{
  //  cout << "Searching in directory: " << searchPath << " for file with " << filePartName << " in its name" << endl;
  FileList matchFullPath;
  FileList dirtree;
  GetDirListing (dirtree, searchPath);
  int32_t numofpaths = dirtree.size ();

  for (int32_t i = 0; i < numofpaths; i++)
    {
      string str (dirtree[i]);
      string fullPath = str;

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
      if (S_ISDIR (st_buf.st_mode))
        {
          continue;
        }
      else
        {
          std::size_t found = str.find (filePartName);
          if (found != std::string::npos)
            {
              if (str.find ('~') == std::string::npos)
                {
                  matchFullPath.push_back (fullPath);
                  //                  cout << "Found file: " << fullPath << endl;
                }
            }
        }
    }
  return matchFullPath;
}

folder_name_mask
GetFolderNameMask (std::string dir)
{
  folder_name_mask m;

  auto extract_num = [&]()
    {
      uint32_t pos = 0;
      pos = dir.rfind ("_");
      assert(pos != std::string::npos);
      auto v = atoi (dir.substr (pos + 1, dir.size()).c_str ());
      dir = dir.substr (0, pos);

      return v;
    };;

  m.t = extract_num ();
  m.cw = extract_num ();
  m.ns = extract_num () - 1;

  //  std::cout << m.ns << "\t" << m.cw << "\t" << m.t << "\t" << std::endl;
  return m;
}
double
GetMacEfficiency (uint64_t totalMacBytes, uint64_t sim_dur)
{
  uint32_t bitsPerOfdmSymb = 10800;
  //
  // unit [us]
  //
  uint32_t t_symbol = 42.24;

  double v = (long double) totalMacBytes * 8 / (double) sim_dur;
  return v / (double) bitsPerOfdmSymb * (double) t_symbol;
}

double
AverageTp (uint64_t totalMacBytes, uint32_t numMpdus)
{
  uint32_t bitsPerOfdmSymb = 10800;
  //
  // unit [us]
  //
  uint32_t t_symbol = 42.24;

  double num_symb = ceil ((long double) totalMacBytes * 8 / (double) bitsPerOfdmSymb);
  return num_symb * (double) t_symbol / (double) numMpdus;
}
