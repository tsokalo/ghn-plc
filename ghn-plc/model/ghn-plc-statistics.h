/*
 * statistics.h
 *
 *  Created on: Aug 9, 2016
 *      Author: tsokalo
 */

#ifndef GHN_PLC_STATISTICS_H_
#define GHN_PLC_STATISTICS_H_

#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <stdint.h>
#include <random>
#include <math.h>
#include <memory>

#include <boost/dynamic_bitset.hpp>
#include <boost/math/distributions/chi_squared.hpp>
#include <boost/random/normal_distribution.hpp>
#include <boost/math/distributions/students_t.hpp>
#include <boost/algorithm/minmax_element.hpp>

namespace ns3 {
namespace ghn {

#define NUM_BATCHES     20

typedef std::vector<double> rv_t;
typedef std::pair<rv_t, rv_t> bi_rv_t;
typedef std::pair<double, double> st_pair_t;

//
// calculates mean and confidence interval using T-Student theorem and 95% confidence probability
//
st_pair_t
CalcStats (rv_t vals)
{
  vals.erase(vals.begin(), vals.begin() + (vals.size() >> 1));
//  std::cout << "vals size: " << vals.size () << std::endl;
  uint16_t num_batches = NUM_BATCHES;
  std::size_t batch_size = floor ((double) vals.size () / (double) (num_batches + 1)), j = 0;
  rv_t v;

  //
  // ignore the first batch as the warm up period
  //
  while (j++ < num_batches)
    {
      v.push_back (std::accumulate (vals.begin () + j * batch_size, vals.begin () + (j + 1) * batch_size, 0.0)
              / (double) batch_size);
    }

  double mean = std::accumulate (v.begin (), v.end (), 0.0) / v.size ();
  double stdev = std::sqrt (std::inner_product (v.begin (), v.end (), v.begin (), 0.0) / v.size () - mean * mean);

  using boost::math::quantile;
  using boost::math::complement;

  const double alpha = 1 - 0.95;
  const boost::math::students_t dist (num_batches - 1);

  double T = quantile (complement (dist, alpha / 2));
  //
  // Calculate width of interval (one sided)
  //
  double confInterval = T * stdev / sqrt ((double) num_batches);

  return st_pair_t (mean, confInterval);
}
//
// work for not correlated RVs following T-Student distribution
// Fieller's method
//
st_pair_t
CalcStatsByFieller (bi_rv_t vals)
{
  rv_t x = vals.first;// data
  rv_t y = vals.second;// iats
  vals.first.erase(vals.first.begin(), vals.first.begin() + (vals.first.size() >> 1));
  vals.second.erase(vals.second.begin(), vals.second.begin() + (vals.second.size() >> 1));

  assert(x.size() == y.size());

  uint16_t num_batches = NUM_BATCHES;

  auto find_stat_pair = [&](rv_t w)->st_pair_t
    {
      std::size_t batch_size = floor ((double) w.size () / (double) (num_batches + 1)), j = 0;
      rv_t v;

      //
      // ignore the first batch as the warm up period
      //
      while (j++ < num_batches)
        {
          v.push_back (std::accumulate (w.begin () + j * batch_size, w.begin () + (j + 1) * batch_size, 0.0)
                  / (double) batch_size);
        }

      double mean = std::accumulate (v.begin (), v.end (), 0.0) / v.size ();
      double stdev = std::sqrt (std::inner_product (v.begin (), v.end (), v.begin (), 0.0) / v.size () - mean * mean);

      return st_pair_t(mean, stdev);
    };;

  auto covariance = [&](rv_t v, rv_t u, double m_v, double m_u)->double
    {
      assert(u.size() == v.size());
      auto v_it = v.begin();
      auto u_it = u.begin();
      double cov = 0;
      while(v_it != v.end())
        {
          cov += (*v_it - m_v)*(*u_it - m_u);
          v_it++;
          u_it++;
        }
      cov /= (double)v.size();
      return cov;
    };;

  using boost::math::quantile;
  using boost::math::complement;

  const double alpha = 1 - 0.95;
  const boost::math::students_t dist (num_batches - 1);

  double t = quantile (complement (dist, alpha / 2));

  st_pair_t x_st = find_stat_pair (x);
  st_pair_t y_st = find_stat_pair (y);

  double m1 = x_st.first, m2 = y_st.first, sig1 = x_st.second, sig2 = y_st.second;
  double cov = covariance (x, y, m1, m2);

  double f0 = m1 * m1 - t * t * sig1;
  double f1 = m1 * m2 - t * t * cov;
  double f2 = m2 * m2 - t * t * sig2;
  double D = f1 * f1 - f0 * f2;
  assert(D > 0);

  double m = (f2 > 0) ? f1 / f2 : m1 / m2;
  double ci = (f2 > 0) ? sqrt (D) / f2 : 0;

//  std::cout << "m1: " << m1 << ", m2: " << m2 << ", sig1: " << sig1 << ", sig2: " << sig2 << ", t: " << t << ", cov: " << cov
//          << std::endl;
//  std::cout << "f0: " << f0 << ", f1: " << f1 << ", f2: " << f2 << ", D: " << D << ", m: " << m << ", ci: " << ci << std::endl;

  return st_pair_t (m, ci);
}
//
// work for not correlated RVs following T-Student distribution
// Delta method
//
st_pair_t
CalcStatsByDelta (bi_rv_t vals)
{
  rv_t x = vals.first;// data
  rv_t y = vals.second;// iats
  vals.first.erase(vals.first.begin(), vals.first.begin() + (vals.first.size() >> 1));
  vals.second.erase(vals.second.begin(), vals.second.begin() + (vals.second.size() >> 1));

  assert(x.size() == y.size());

  uint16_t num_batches = NUM_BATCHES;

  auto find_stat_pair = [&](rv_t w)->st_pair_t
    {
      std::size_t batch_size = floor ((double) w.size () / (double) (num_batches + 1)), j = 0;
      rv_t v;

      //
      // ignore the first batch as the warm up period
      //
      while (j++ < num_batches)
        {
          v.push_back (std::accumulate (w.begin () + j * batch_size, w.begin () + (j + 1) * batch_size, 0.0)
                  / (double) batch_size);
        }

      double mean = std::accumulate (v.begin (), v.end (), 0.0) / v.size ();
      double stdev = std::sqrt (std::inner_product (v.begin (), v.end (), v.begin (), 0.0) / v.size () - mean * mean);

      return st_pair_t(mean, stdev);
    };;

  auto covariance = [&](rv_t v, rv_t u, double m_v, double m_u)->double
    {
      assert(u.size() == v.size());
      auto v_it = v.begin();
      auto u_it = u.begin();
      double cov = 0;
      while(v_it != v.end())
        {
          cov += (*v_it - m_v)*(*u_it - m_u);
          v_it++;
          u_it++;
        }
      cov /= (double)v.size();
      return cov;
    };;

  using boost::math::quantile;
  using boost::math::complement;

  const double alpha = 1 - 0.95;
  const boost::math::students_t dist (num_batches - 1);

  double t = quantile (complement (dist, alpha / 2));

  st_pair_t x_st = find_stat_pair (x);
  st_pair_t y_st = find_stat_pair (y);

  double m1 = x_st.first, m2 = y_st.first, sig1 = x_st.second, sig2 = y_st.second;
  double cov = covariance (x, y, m1, m2);

  double m = m1 / m2;
  double A = (sig1 - 2 * m * cov + m * m * sig2) / (m2 * m2);
  assert(A >= 0);
  double ci = t * sqrt (A);

//  std::cout << "m1: " << m1 << ", m2: " << m2 << ", sig1: " << sig1 << ", sig2: " << sig2 << ", t: " << t << ", cov: " << cov
//          << std::endl;
//  std::cout << "A: " << A << ", m: " << m << ", ci: " << ci << std::endl;

  return st_pair_t (m, ci);
}
rv_t
GetJitter (rv_t latency)
{
  rv_t jitter;
  double al = std::accumulate (latency.begin (), latency.end (), 0.0) / latency.size ();
  for(auto l : latency)jitter.push_back(fabs(l - al));
  return jitter;
}
bi_rv_t
GetDatarate (rv_t iat, rv_t data)
{
  assert(iat.size() == data.size());

  for(auto &d : data) d*=8;;

  return std::pair<rv_t, rv_t> (data, iat);
}
void
FlatterIats (rv_t &iat)
{
  typedef rv_t::iterator iat_it;
  auto flatter = [](const iat_it &b, const iat_it &e)
    {
      assert(*b != 0);
      assert(b != e);
      double f = *b / (double) std::distance(b, e);
      for(auto i = b; i != e; i++)*i = f;
    };;

  for (iat_it i = iat.begin (); i != iat.end (); i++)
    {
      const iat_it b = i;

      if (*(i + 1) == 0)
        {
          while (*(i + 1) == 0 && (i + 1) != iat.end ())
            i++;
          const iat_it e = i + 1;
          flatter (b, e);
        }
    }
}
}
}

#endif /* GHN_PLC_STATISTICS_H_ */
