/*
 *      Author: tsokalo
 */

#include <iostream>
#include <sstream>
#include <fstream>
#include <time.h>

#include <ns3/core-module.h>
#include <ns3/nstime.h>
#include <ns3/simulator.h>
#include <ns3/output-stream-wrapper.h>
#include "ns3/plc.h"
#include <boost/assign/list_of.hpp>

using namespace ns3;

int
main (int argc, char *argv[])
{
  std::vector<ModulationType> mts =
    { BPSK, QAM4, QAM8, QAM16, QAM32, QAM64, QAM128, QAM256, QAM512, QAM1024, QAM2048, QAM4096 };

  double min_sinr_db = -5;
  double max_sinr_db = 20;
  double step_sinr_db = 1;

  for(auto mt : mts)
    {
      double sinr_db = min_sinr_db;
      while(sinr_db < max_sinr_db)
        {
          double c = GetCapPerChannel (sinr_db, mt);
          double loaded = mt;
          double ber = (c < mt) ? (mt - c) / mt : 0;

          std::cout << loaded << "\t" << sinr_db << "\t" << ber << std::endl;
          sinr_db += step_sinr_db;
      }
    }

  return EXIT_SUCCESS;
}
