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

#include "comparison.h"

//#define PER_BER_MAP_SIZE        500
#define MIN_PER_VAL     0.01
#define MAX_PER_VAL     0.99

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

    assert(ncr::geq(per, MIN_PER_VAL));
    assert(ncr::leq(per, MAX_PER_VAL));

    for(auto pbm : m_pbMapping)
      {
        if(ncr::geq(pbm.per, per))
          {
            return pbm;
          }
      }
    assert(0);
//
//    uint16_t i = per * 1000 - 1;
//    assert(i < PER_BER_MAP_SIZE);
//    return m_pbMapping.at (i);
  }

private:
  static std::vector<PbMapping> m_pbMapping;
};
}
}
#endif /* GHN_PLC_BITLOADINGDATE_H_ */
