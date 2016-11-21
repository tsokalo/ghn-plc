/*
 * GhnPlcWaterFillerr.cpp
 *
 *  Created on: Oct 6, 2015
 *      Author: tsokalo
 */

#include <iostream>
#include <assert.h>

#include "ghn-plc-water-filling.h"

namespace ns3 {
namespace ghn {

GhnPlcWaterFiller::GhnPlcWaterFiller (std::vector<double> levels, double energy) :
  m_levels (levels), m_energy (energy)
{
  for (uint32_t i = 0; i < m_levels.size (); i++)
    m_indexes.push_back (i);

  BubbleSort (m_levels, m_indexes);
}
GhnPlcWaterFiller::~GhnPlcWaterFiller ()
{
  m_indexes.clear ();
  m_levels.clear ();
}

std::vector<double>
GhnPlcWaterFiller::CreateFill ()
{
  assert(m_levels.size() > 0);
  double h = FindLevel (m_levels, m_energy);
  uint32_t s = m_levels.size ();
  std::vector<double> fill (s, 0);

  for (uint32_t i = 0; i < s; i++)
    {
      if (h > m_levels.at (i)) fill.at (i) = h - m_levels.at (i);
    }
  //  OrigReoder (m_levels, m_indexes);
  OrigReoder (fill, m_indexes);
  return fill;
}

void
GhnPlcWaterFiller::BubbleSort (std::vector<double> &nbrs, std::vector<uint32_t> &indexes)
{
  assert(nbrs.size() == indexes.size());

  uint32_t s = nbrs.size ();

  for (double i = 0; i < s; i++)
    {
      double temp = 0;
      uint32_t t = 0;
      for (double j = 0; j < s - i - 1; j++)
        {
          if (nbrs[j] > nbrs[j + 1])
            {
              temp = nbrs[j];
              nbrs[j] = nbrs[j + 1];
              nbrs[j + 1] = temp;

              t = indexes[j];
              indexes[j] = indexes[j + 1];
              indexes[j + 1] = t;
            }
        }
    }
}

double
GhnPlcWaterFiller::FindLevel (std::vector<double> levels, double energy)
{
  uint32_t s = levels.size ();
  uint32_t c = 0;
  while ((levels.at (c) - levels.at (0)) * s <= energy)
    {
      if (c + 1 == levels.size ()) break;
      c++;
    }

  for (; c < s && c >= 0;)
    {
      double sum = 0;
      //
      // concern sorted levels
      //
      for (uint32_t m = 0; m <= c; m++)
        {
          if (levels.at (m) <= levels.at (c)) sum += levels.at (m);
        }
      assert(levels.at (c) * (c + 1) >= sum);
      if (levels.at (c) * (c + 1) - sum > energy || c == s - 1) break;
      c++;
    }

  if (c > 0) assert(levels.at(c) > levels.at(c - 1));

  double sum = 0;
  for (uint32_t i = 0; i < c; i++)
    sum += levels.at (i);

  return (sum + energy) / (double) c;
}

void
GhnPlcWaterFiller::VisualizeLevels (std::vector<double> levels)
{
  for (uint32_t i = 0; i < levels.size (); i++)
    {
      for (uint32_t j = 0; j < (uint32_t) (levels.at (i) * 10); j++)
        {
          std::cout << "#";
        }
      std::cout << std::endl;
    }
}
void
GhnPlcWaterFiller::VisualizeLevels (std::vector<double> levels, std::vector<double> fill)
{
  for (uint32_t i = 0; i < levels.size (); i++)
    {
      for (uint32_t j = 0; j < (uint32_t) (levels.at (i) * 10); j++)
        {
          std::cout << "#";
        }
      for (uint32_t j = 0; j < (uint32_t) (fill.at (i) * 10); j++)
        {
          std::cout << ">";
        }
      std::cout << std::endl;
    }
}

void
GhnPlcWaterFiller::OrigReoder (std::vector<double> &nbrs, std::vector<uint32_t> &indexes)
{
  assert(nbrs.size() == indexes.size());
  uint32_t s = nbrs.size ();
  std::vector<double> temp (s, 0);
  temp.swap (nbrs);

  for (double i = 0; i < s; i++)
    {
      nbrs.at (indexes.at (i)) = temp.at (i);
    }
}

}
}
