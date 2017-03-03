/*
 * WaterFillerr.h
 *
 *  Created on: Oct 6, 2015
 *      Author: tsokalo
 */

#ifndef WATERFILLER_H_
#define WATERFILLER_H_

#include <vector>
#include <stdint.h>

class WaterFiller
{
public:

  WaterFiller (std::vector<double> levels, double energy);
  ~WaterFiller ();

  std::vector<double>
  CreateFill ();

  void
  VisualizeLevels (std::vector<double> levels);
  void
  VisualizeLevels (std::vector<double> levels, std::vector<double> fill);

private:

  void
  BubbleSort (std::vector<double> &nbrs, std::vector<uint32_t> &indexes);
  double
  FindLevel (std::vector<double> levels, double energy);
  void
  OrigReoder (std::vector<double> &nbrs, std::vector<uint32_t> &indexes);

  std::vector<double> m_levels;
  double m_energy;
  std::vector<uint32_t> m_indexes;
};


#endif /* WATERFILLER_H_ */
