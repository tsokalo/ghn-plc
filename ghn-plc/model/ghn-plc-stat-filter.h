/*
 * filter.h
 *
 *  Created on: Jul 20, 2016
 *      Author: tsokalo
 */

#ifndef GHN_PLC_FILTER_H_
#define GHN_PLC_FILTER_H_

#include <functional>
#include <random>
#include <vector>
#include <deque>
#include <map>
#include <memory>
#include <algorithm>
#include <tuple>
#include <bitset>
#include <assert.h>
#include <iostream>

#include <boost/math/distributions/chi_squared.hpp>
#include <boost/random/normal_distribution.hpp>
#include <boost/math/distributions/students_t.hpp>
#include <boost/algorithm/minmax_element.hpp>
namespace ns3 {
namespace ghn {

template<std::size_t N>
  class FloatingFilter
  {
  public:
    FloatingFilter () :
      m_size (N)
    {
      m_filtered = 0;
    }
    ~FloatingFilter ()
    {
    }

    void
    add (double val)
    {
      m_vals.push_back (val);
      if (m_vals.size () > m_size) m_vals.pop_front ();
      apply ();
    }
    double
    val ()
    {
      return m_filtered;
    }

    void
    reset ()
    {
      m_vals.clear ();
    }
  protected:

    virtual void
    apply () = 0;

    std::deque<double> m_vals;
    uint16_t m_size;
    double m_filtered;
  };

template<std::size_t N>
  class AveFloatingFilter : public FloatingFilter<N>
  {
    using FloatingFilter<N>::m_filtered;
    using FloatingFilter<N>::m_vals;

  public:
    AveFloatingFilter ()
    {
    }
  private:

    void
    apply ()
    {
      m_filtered = std::accumulate (m_vals.begin (), m_vals.end (), 0.0) / (double) m_vals.size ();
    }
  };
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<std::size_t N>
  class BinaryFilter
  {
  public:
    BinaryFilter ()
    {
      m_filtered = 0;
      m_default = 1;
      m_is_ready = false;
      m_pos = 0;
      m_is_init = false;
      assert(m_vals.size() >= 16);
    }
    ~BinaryFilter ()
    {
    }
    bool
    is_init ()
    {
      return m_is_init;
    }

    void
    set_default (double v)
    {
      m_default = v;
    }

    void
    add (bool val)
    {
      SIM_LOG (FILTER_LOG, "value: " << val);
      m_is_init = true;
      if (m_pos == m_vals.size ())
        {
          m_pos--;
          m_vals >>= 1;
        }
      (val) ? m_vals.set (m_pos) : m_vals.reset (m_pos);

      m_pos++;
    }
    void
    add (int16_t num, bool val)
    {
      while (num-- > 0)
        add (val);
    }
    void
    update ()
    {
      if (is_ready ()) apply ();
    }

    double
    val ()
    {
      return (m_is_ready) ? m_filtered : m_default;
    }

    void
    reset ()
    {
      for (size_t i = 0; i < m_vals.size (); i++)
        {
          m_vals.reset (i);
        }
    }

  protected:

    virtual bool
    is_ready ()
    {
      m_is_ready = m_pos * 2 > m_vals.size ();
      return m_is_ready;
    }

    virtual void
    apply () = 0;

    double m_filtered;
    std::bitset<N> m_vals;
    std::size_t m_pos;
    bool m_is_init;
    bool m_is_ready;
    double m_default;
  };

template<std::size_t N>
  class AveBinaryFilter : public BinaryFilter<N>
  {
    using BinaryFilter<N>::m_filtered;
    using BinaryFilter<N>::m_vals;
    using BinaryFilter<N>::m_pos;
    using BinaryFilter<N>::m_is_ready;

  public:
    AveBinaryFilter ()
    {
      m_num_batches_bits = 4;
    }

  private:

    bool
    is_ready ()
    {
      SIM_LOG (FILTER_LOG, "current position: " << m_pos << " max size: " << m_vals.size ());
      if (m_pos < m_vals.size ()) return false;

      std::size_t batch_size = m_vals.size () >> m_num_batches_bits, j = 0;
      std::vector<double> v;

      SIM_LOG (FILTER_LOG, m_vals.to_string () << " batch size: " << batch_size);

      while (j++ < (1 << m_num_batches_bits) - 1)
        {
          auto temp_vals = ((m_vals << (batch_size * j)) >> (m_vals.size () - batch_size));
          v.push_back ((double) temp_vals.count () / (double) batch_size);
          SIM_LOG (FILTER_LOG, temp_vals.to_string () << " -> " << (double) temp_vals.count () / (double) batch_size);
        }

      double mean = std::accumulate (v.begin (), v.end (), 0.0) / v.size ();
      double stdev = std::sqrt (std::inner_product (v.begin (), v.end (), v.begin (), 0.0) / v.size () - mean * mean);

      SIM_LOG (FILTER_LOG, "mean: " << mean << " stdev: " << stdev);

      m_is_ready = stdev < 0.2;
      return m_is_ready;
      //
      //      using boost::math::quantile;
      //      using boost::math::complement;
      //
      //      const double alpha = 1 - 0.95;
      //      const boost::math::students_t dist ((1 << m_num_batches_bits) - 1);
      //
      //
      //      double T = quantile (complement (dist, alpha / 2));
      //      //
      //      // Calculate width of interval (one sided)
      //      //
      //      double confInterval = T * stdev / sqrt (double (1 << m_num_batches_bits));
      //      double accuracyIndex = 1 - confInterval / mean;
      //
      //      SIM_LOG(FILTER_LOG, alpha << "\t" << T << "\t" << confInterval << "\t" << accuracyIndex);
      //
      //      return accuracyIndex > 0.8;
    }

    void
    apply ()
    {
      m_filtered = (double) m_vals.count () / (double) ((m_pos != m_vals.size ()) ? m_pos : m_vals.size ());
    }
    std::size_t m_num_batches_bits;
  };
}
}
#endif /* GHN_PLC_FILTER_H_ */
