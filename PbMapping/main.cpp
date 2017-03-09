//
// ping.cpp
// ~~~~~~~~
//#include "Ping.h"
#include "string.h"
#include <sstream>
#include <stdint.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <algorithm>

#include <iostream>
#include <fstream>
#include <string>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <limits>
#include <vector>
#include <math.h>
#include <boost/assign/list_of.hpp>

using namespace std;

long double
factorial_partial (uint16_t n, uint16_t k)
{
  return (n == k) ? 1 : factorial_partial (n - 1, k) * n;
}
long double
factorial (uint16_t n)
{
  return factorial_partial (n, 0);
}

long double
binom_coef (uint16_t n, uint16_t k)
{
  assert(n >= k);
  return (k > n - k) ? ((long double) factorial_partial (n, k) / (long double) factorial (n - k))
          : ((long double) factorial_partial (n, n - k) / (long double) factorial (k));
}
//
// if p is the success probability then k is the number of failures after n attempts
//
long double
binom_distr (uint16_t n, uint16_t k, double p)
{
  assert(n >= k);
  //  cout << "k: " << k << ", n: " << n << ", e1: " << e1 << ", e3: " << e3 << ", binom_coef: " << binom_coef (n, k)
  //          << ", pow (1 - e1 * e3, k): " << pow (1 - e1 * e3, k) << ", pow (e1 * e3, n - k): " << pow (e1 * e3, n - k)
  //          << ", prob: " << binom_coef (n, k) * pow (1 - e1 * e3, k) * pow (e1 * e3, n - k) << endl;
  return (k == 0) ? 0 : (binom_distr (n, k - 1, p) + binom_coef (n, k) * pow (1 - p, k) * pow (p, n - k));
}

typedef std::pair<uint16_t, uint16_t> cr_t;
//template<>
bool
operator < (const cr_t& l, const cr_t& r)
{
  //swap only if they're unequal to avoid infinite recursion
  if (l.first == l.second && r.first == r.second) return false;

  //actual comparison is done here
  return ((double) l.first / (double) l.second > (double) r.first / (double) r.second);
}

struct PbMapping
{
  PbMapping (cr_t cr, double ber, double per)
  {
    this->cr = cr;
    this->ber = ber;
    this->per = per;
  }
  friend bool
  operator < (const PbMapping& l, const PbMapping& r)
  {
    return l.per < r.per;
  }
  cr_t cr;
  double ber;
  double per;
};

struct PbMappingC
{
  PbMappingC (cr_t cr, double ber, double per)
  {
    this->cr = cr;
    this->ber = ber;
    this->per = per;
  }
  PbMappingC (PbMapping p)
  {
    this->cr = p.cr;
    this->ber = p.ber;
    this->per = p.per;
  }
  friend bool
  operator < (const PbMappingC& l, const PbMappingC& r)
  {
    return l.cr < r.cr;
  }

  cr_t cr;
  double ber;
  double per;
};

int
main ()
{
  std::vector<cr_t> crs = boost::assign::list_of<cr_t> (cr_t (1.0, 4.0)) (cr_t (1.0, 2.0)) (cr_t (2.0, 3.0)) (cr_t (5.0, 6.0)) (
          cr_t (16.0, 21.0)) (cr_t (16.0, 18.0)) (cr_t (20.0, 21.0));

  std::vector<PbMapping> ms;

  long double d = pow (10, -3);
  uint16_t ber_l = 1;
  uint16_t fidelity = 1000;
  while (ber_l++ < 35 * fidelity)
    {
      long double ber = ber_l * d / (double) fidelity;
      for (auto cr : crs)
        {
          double per = binom_distr(cr.second, cr.second - cr.first, 1 - ber);
          ms.push_back(PbMapping(cr, ber, per));
        };;
    }

  std::sort (ms.begin (), ms.end ());

  std::vector<PbMappingC> msfin;
  uint16_t per_l = 0;
  while (per_l++ < 500)
    {
      std::vector<PbMappingC> msub;
      for (auto m : ms)
        {
          if(m.per < (double)per_l / 1000 + 0.05 / (double)fidelity && m.per > (double)per_l / 1000 - 0.05/ (double)fidelity)
            {
              msub.push_back(PbMappingC(m));
            }
        };;
      std::sort (msub.begin (), msub.end ());
      if (!msub.empty ()) msfin.push_back (*msub.begin ());
    }

  per_l = 0;
  for(auto m : msfin)
    {
      cout << (double)(++per_l) / 1000 << "\t" << m.cr.first << "/" << m.cr.second << "\t" << m.ber << "\t" << m.per << endl;
    }

  cout << "End" << endl;
  return 0;
}

