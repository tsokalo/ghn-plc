/*
 * GhnPlcWaterFillerr.h
 *
 *  Created on: Oct 6, 2015
 *      Author: tsokalo
 */

#ifndef GHN_PLC_WATERFILLER_H_
#define GHN_PLC_WATERFILLER_H_

#include <vector>
#include <stdint.h>


namespace ns3 {
namespace ghn {

class GhnPlcWaterFiller
{
public:

  GhnPlcWaterFiller (std::vector<double> levels, double energy);
  ~GhnPlcWaterFiller ();

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
}
}

#endif /* GHN_PLC_WATERFILLER_H_ */
