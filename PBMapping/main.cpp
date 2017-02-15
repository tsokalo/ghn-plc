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
#include <random>
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
  return (k > n - k) ? ((long double) factorial_partial (n, k) / (long double) factorial (n - k)) :
          ((long double) factorial_partial (n, n - k) / (long double) factorial (k));
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
  return (k == 0) ? (pow (1 - p, n)) : (binom_distr (n, k - 1, p) + binom_coef (n, k) * pow (p, k) * pow (1 - p, n - k));
}

double
binom_distr_r (uint16_t n, uint16_t k, double p)
{
  std::random_device rd;
  std::mt19937 gen (rd ());
  std::binomial_distribution<> d (n, p);

  uint32_t c = 100000, s = 0;
  for (uint32_t i = 0; i < c; i++)
    {
      auto v = d (gen);
      if (v <= k) s++;
    }
  return (double) s / (double) c;
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
  PbMapping ()
  {
  }
  PbMapping (cr_t cr, double ber, double per)
  {
    this->cr = cr;
    this->ber = ber;
    this->per = per;
  }
  PbMapping&
  operator= (const PbMapping& other) // copy assignment
  {
    if (this != &other)
      { // self-assignment check expected
        this->cr.first = other.cr.first;
        this->cr.second = other.cr.second;
        this->ber = other.ber;
        this->per = other.per;
      }
    return *this;
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

  PbMappingC (cr_t cr, double ber, double per, double per_o)
  {
    this->cr = cr;
    this->ber = ber;
    this->per = per;
    this->per_o = per_o;
  }
  PbMappingC (PbMapping p, double per_o)
  {
    this->cr = p.cr;
    this->ber = p.ber;
    this->per = p.per;
    this->per_o = per_o;
  }
  friend bool
  operator < (const PbMappingC& l, const PbMappingC& r)
  {
    return l.cr < r.cr;
  }

  cr_t cr;
  double ber;
  double per;
  double per_o;
};

int
main ()
{
  cout << "Start" << endl;
  std::vector<cr_t> crs = boost::assign::list_of<cr_t> (cr_t (1.0, 4.0)) (cr_t (1.0, 2.0)) (cr_t (2.0, 3.0)) (cr_t (5.0, 6.0)) (
          cr_t (16.0, 21.0)) (cr_t (16.0, 18.0)) (cr_t (20.0, 21.0));
  uint16_t msg_size = 540;

  std::cout.flush ();
  auto print_progress = [](uint32_t m, uint32_t c)
    {
      float progress = (double) c / (double) m;

      int barWidth =100;

      std::cout << "[";
      int pos = barWidth * progress;
      for (int i = 0; i < barWidth; ++i)
        {
          if (i < pos) std::cout << "=";
          else if (i == pos) std::cout << ">";
          else std::cout << " ";
        }
      std::cout << "] " << ceil(progress * 100.0) << " %\r";
      std::cout.flush();
    };

  std::vector<PbMapping> ms;
  uint16_t ber_digits = 5;
  auto n = pow (10, ber_digits);
  auto cr = cr_t (20.0, 21.0);
  for (uint32_t i = 0; i < n; i++)
    {
      uint16_t coded_msg_size = (double) msg_size * (double) cr.second / (double) cr.first * 8;
      uint16_t elig_loss_size = coded_msg_size - msg_size * 8;
      auto ber = (double) i / (double)n;
      auto per = 1 - binom_distr_r (coded_msg_size, elig_loss_size, ber);
      ms.push_back (PbMapping (cr, ber, per));

      if (fabs (per - 1.0) < 0.000001) break;
      print_progress (n * 0.07, i);

//      std::cout << coded_msg_size << "\t" << elig_loss_size << "\t" << ber << "\t" << per << std::endl;
    }

//  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////// TEST
//  for (uint16_t i = 0; i < 10000; i++)
//    {
//      auto cr = cr_t (20.0, 21.0);
//      uint16_t coded_msg_size = (double) msg_size * (double) cr.second / (double) cr.first * 8;
//      uint16_t elig_loss_size = coded_msg_size - msg_size * 8;
//      auto ber = (double) i / 10000.0;
//      auto per = 1 - binom_distr_r (coded_msg_size, elig_loss_size, ber);
//      if (fabs (per - 1.0) < 0.000001) break;
//      std::cout << coded_msg_size << "\t" << elig_loss_size << "\t" << ber << "\t" << per << std::endl;
//    }
//
//  return 0;
//  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////// OLD STUFF
//  std::vector<PbMapping> ms;
//
//  long double d = pow (10, -3);
//  uint16_t ber_l = 1;
//  uint16_t fidelity = 80;
//  while (ber_l < 720 * fidelity)
//    {
//      print_progress (720 * fidelity, ber_l);
//      long double ber = ber_l * d / (double) fidelity;
//
//      auto dr = [](PbMapping pbm)
//        {
//          return (double)pbm.cr.first / (double)pbm.cr.second * (1 - pbm.per);
//        };
//
//      double v = 0;
//      PbMapping pbm;
//
//      for (auto cr : crs)
//        {
//          uint16_t coded_msg_size = (double) msg_size * (double) cr.second / (double) cr.first * 8;
//          uint16_t elig_loss_size = coded_msg_size - msg_size * 8;
//          double per = 1 - binom_distr_r (coded_msg_size, elig_loss_size, ber);
//
//          std::cout << ber << "\t" << cr.first << "/" << cr.second << "\t" << per << std::endl;
//          double vv = dr (PbMapping (cr, ber, per));
//          if (vv > v)
//            {
//              pbm = PbMapping (cr, ber, per);
//              v = vv;
//            }
//        };;
//
//      ms.push_back (pbm);
//
//      ber_l += 10;
//    }
//
//  std::sort (ms.begin (), ms.end ());

  std::vector<PbMappingC> msfin;
  uint16_t per_l = 0;
  while (per_l++ < 100)
    {
      for (auto m : ms)
        {
          if (m.per < (double) per_l / 100.0 + 0.005 && m.per > (double) per_l / 100.0 - 0.0005)
            {
              msfin.push_back (PbMappingC (m, (double) per_l / 100.0));
              break;
            }
        };;
    }

  cout << endl << endl;
  for (auto m : msfin)
    {
//      cout << (double) (++per_l) / 1000 << "\t" << m.cr.first << "/" << m.cr.second << "\t" << m.ber << "\t" << m.per << endl;
      cout << "<" << m.per_o << "\t" << m.cr.first << "/" << m.cr.second << "\t" << m.ber << "|\t" << m.per << ">" << endl;
    }

  cout << "End" << endl;
  return 0;
}

