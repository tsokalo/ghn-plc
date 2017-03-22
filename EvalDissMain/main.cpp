/*
 * main.cpp
 *
 *  Created on: Dec 1, 2015
 *      Author: tsokalo
 */
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <ctime>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <memory>
#include <stdint.h>
#include <map>
#include <math.h>

enum VisVar
{
  LATENCY_VIS_VAR, JITTER_VIS_VAR, DATARATE_VIS_VAR, AVEMPDU_VIS_VAR, COALITION_SIZE_VIS_VAR
};

struct Filter
{
public:

  Filter (uint16_t allow_coop, uint16_t square, uint16_t num_modems, uint16_t gen_size, uint16_t num_gen)
  {
    this->allow_coop = allow_coop;
    this->square = square;
    this->num_modems = num_modems;
    this->gen_size = gen_size;
    this->num_gen = num_gen;
  }

  std::string
  GetFileName ()
  {
    std::stringstream ss;
    ss << "cdf_" << allow_coop << "_" << square << "_" << num_modems << "_" << gen_size << "_" << num_gen << ".txt";
    return ss.str ();
  }

  std::map<VisVar, std::vector<double> >
  Apply (std::string sim_file_path)
  {
    std::ifstream fi (sim_file_path, std::ios_base::in);
    std::string line;
    std::map<VisVar, std::vector<double> > vals;
    while (getline (fi, line, '\n'))
      {
        std::stringstream sv (line);
        double v, w1, w2, w3, w4, w5;
        uint32_t iv;
        sv >> iv;
        std::cout << (iv != allow_coop) << "\t" << iv << "\t" << allow_coop << std::endl;
        if (iv != allow_coop) continue;
        sv >> iv;
        sv >> iv;
        std::cout << (iv != gen_size) << "\t" << iv << "\t" << gen_size << std::endl;
        if (iv != gen_size) continue;
        sv >> iv;
        std::cout << (iv != num_modems) << "\t" << iv << "\t" << num_modems << std::endl;
        if (iv != num_modems) continue;
        sv >> w5;
        if(allow_coop == 1 && w5 < 1.9)continue;
        sv >> w1;
        sv >> v;
        sv >> w2;
        sv >> v;
        sv >> w3;
        sv >> v;
        sv >> w4;
        sv >> iv;
        std::cout << (iv != square) << "\t" << iv << "\t" << square << std::endl;
        if (iv != square) continue;
        sv >> iv;
        std::cout << (iv != num_gen) << "\t" << iv << "\t" << num_gen << std::endl;
        if (iv != num_gen) continue;

        vals[LATENCY_VIS_VAR].push_back (w1 / 1000);
        vals[JITTER_VIS_VAR].push_back (w2 / 1000);
        vals[DATARATE_VIS_VAR].push_back (w3);
        vals[AVEMPDU_VIS_VAR].push_back (w4);
        vals[COALITION_SIZE_VIS_VAR].push_back (w5);

      }
    fi.close ();
    return vals;
  }

private:

  uint16_t allow_coop;
  uint16_t square;
  uint16_t num_modems;
  uint16_t gen_size;
  uint16_t num_gen;
};

void
SaveCdf (std::map<VisVar, std::vector<double> > valss, std::string f_path)
{
  uint16_t num_intervals = 300;
  std::map<VisVar, std::vector<std::pair<double, double> > > cdfs;

  for (auto v : valss)
    {
      auto vis_var = v.first;
      auto vals = v.second;

      if (vals.empty ())
        {
          std::cout << "No filter corresponding elements are found: " << f_path << std::endl;
          return;
        }
      double min = *std::min_element (vals.begin (), vals.end ());
      double max = *std::max_element (vals.begin (), vals.end ());
      double step = max / (double) num_intervals;

      std::map<uint16_t, uint16_t> freq;
      for (auto v : vals)
        {
          uint16_t i = floor (v / step);
          freq[i]++;
        }

      std::vector<double> cdf;
      uint32_t sum = 0;
      for (uint16_t i = 0; i < num_intervals; i++)
        {
          auto v = cdf.empty () ? freq[i] : cdf.at (i - 1) + freq[i];
          sum += freq[i];
          cdf.push_back (v);
        }
      for (auto &v : cdf)
        v /= (double) sum;

      cdfs[vis_var].push_back (std::pair<double, double> (step * 0.99, 0));
      for (uint16_t i = 1; i < num_intervals; i++)
        {
          cdfs[vis_var].push_back (std::pair<double, double> ((double) i * step, cdf.at (i)));
        }
    }

  std::cout << "Save to " << f_path << std::endl;
  std::ofstream fo (f_path, std::ios_base::out);
  for (uint16_t i = 0; i < num_intervals; i++)
    {
      if (cdfs[LATENCY_VIS_VAR].size () < num_intervals || cdfs[JITTER_VIS_VAR].size () < num_intervals
              || cdfs[DATARATE_VIS_VAR].size () < num_intervals || cdfs[AVEMPDU_VIS_VAR].size () < num_intervals
              || cdfs[COALITION_SIZE_VIS_VAR].size () < num_intervals) continue;

      fo << cdfs[LATENCY_VIS_VAR].at (i).first << "\t" << cdfs[LATENCY_VIS_VAR].at (i).second << "\t"
              << cdfs[JITTER_VIS_VAR].at (i).first << "\t" << cdfs[JITTER_VIS_VAR].at (i).second << "\t"
              << cdfs[DATARATE_VIS_VAR].at (i).first << "\t" << cdfs[DATARATE_VIS_VAR].at (i).second << "\t"
              << cdfs[AVEMPDU_VIS_VAR].at (i).first << "\t" << cdfs[AVEMPDU_VIS_VAR].at (i).second << "\t"
              << cdfs[COALITION_SIZE_VIS_VAR].at (i).first << "\t" << cdfs[COALITION_SIZE_VIS_VAR].at (i).second << std::endl;
    }
  fo.close ();
}

int
main (int argc, char *argv[])
{
  std::cout << "Program start" << std::endl;

  if (argc < 2)
    {
      std::cout << "Not enough arguments" << std::endl;
      exit (1);
    }
  std::string sim_file_path = argv[1];
  std::cout << "Using simulation file " << sim_file_path << std::endl;
  std::string path = sim_file_path.substr (0, sim_file_path.rfind ("/") + 1);
  if (path.empty ())
    {
      std::cout << "Please, give the full path" << std::endl;
      exit (1);
    }

  std::vector<Filter> filters;
  filters.push_back (Filter (0, 60, 3, 32, 20));
  filters.push_back (Filter (1, 60, 3, 64, 10));
  filters.push_back (Filter (1, 60, 3, 128, 5));
  filters.push_back (Filter (1, 60, 3, 32, 20));
  filters.push_back (Filter (1, 60, 3, 32, 12));
  filters.push_back (Filter (1, 60, 3, 16, 40));
  filters.push_back (Filter (1, 60, 3, 32, 10));
  filters.push_back (Filter (1, 60, 3, 32, 40));
  filters.push_back (Filter (1, 60, 3, 32, 80));
  filters.push_back (Filter (1, 60, 3, 32, 160));
  filters.push_back (Filter (1, 60, 2, 32, 40));
  filters.push_back (Filter (1, 60, 4, 32, 40));
  filters.push_back (Filter (1, 60, 5, 32, 40));
  filters.push_back (Filter (1, 60, 6, 32, 40));
  filters.push_back (Filter (1, 60, 7, 32, 40));

  filters.push_back (Filter (1, 60, 3, 8, 40));
  filters.push_back (Filter (1, 60, 3, 16, 20));
  filters.push_back (Filter (1, 60, 3, 64, 5));
  filters.push_back (Filter (1, 60, 2, 32, 12));
  filters.push_back (Filter (1, 60, 4, 32, 12));
  filters.push_back (Filter (1, 60, 5, 32, 12));
  filters.push_back (Filter (1, 60, 6, 32, 12));

  for (auto filter : filters)
    {
      auto vals = filter.Apply (sim_file_path);
      SaveCdf (vals, path + filter.GetFileName ());
    }

  std::cout << "Finished successfully" << std::endl;
  return 0;
}

