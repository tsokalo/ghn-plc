/*
 * nc-bit-loading-date.h
 *
 *  Created on: 24.09.2016
 *,,Author: tsokalo
 */

#ifndef GHN_PLC_BITLOADINGDATE_H_
#define GHN_PLC_BITLOADINGDATE_H_

#include <tuple>
#include <vector>
#include "ns3/plc-defs.h"

#define PER_BER_MAP_SIZE        500

namespace ns3
{
namespace ghn {
struct PbMapping
{
  PbMapping (double per_, CodingType ct, double ber, double per)
  {
    this->per_ = per_;
    this->ct = ct;
    this->ber = ber;
    this->per = per;
  }
  PbMapping&
  operator= (const PbMapping& other)
  {
    if (this != &other)
      {
        this->per_ = other.per_;
        this->ct = other.ct;
        this->ber = other.ber;
        this->per = other.per;
      }
    return *this;
  }
  double per_;
  CodingType ct;
  double ber;
  double per;
};

class PbMappingList
{
public:
  PbMappingList ()
  {
  }

  virtual
  ~PbMappingList ();

  static PbMapping
  get_val (double per)
  {
    uint16_t i = per * 1000;
    assert(i < PER_BER_MAP_SIZE);
    return m_pbMapping.at (i);
  }

private:
  static std::vector<PbMapping> m_pbMapping;
};
}
}
#endif /* GHN_PLC_BITLOADINGDATE_H_ */
