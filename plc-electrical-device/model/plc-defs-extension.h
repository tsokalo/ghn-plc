/*
 * plc-defs-extension.h
 *
 *  Created on: Sep 8, 2014
 *      Author: tsokalo
 */

#ifndef PLCDEFSEXTENSION_H_
#define PLCDEFSEXTENSION_H_

#include <iostream>
#include "ns3/plc-defs.h"
#include "ns3/object.h"
#include "ns3/nstime.h"
#include "ns3/plc-noise.h"

#include <boost/math/constants/constants.hpp>
#define DELIMITER '\t'

using boost::math::constants::pi;

namespace ns3
{
enum ImpedanceType
{
  CONSTANT_TYPE, FREQ_SELECTIVE_TYPE, FREQ_SELECTIVE_TIME_VARIANT_COMMUTED_TYPE, FREQ_SELECTIVE_TIME_VARIANT_HARMONIC_TYPE
};
struct PLC_ImpedanceProperties
{
  /****************************************************************
   *******************      Basic options   ***********************
   ***************************************************************/
  ImpedanceType impedanceType;

  PLC_Value offStateImpedanceValue;

  /*
   * Reference:
   * Statistische Modelle elektrischer Geräte für Anwendung in
   * einem PLC-Simulator
   * Haihui Wu, Studentische Arbeit, TUD, Ifn/TK
   */
  /*
   * resistance for constant impedance type or resonance impedance for
   * frequency selective type
   *
   * unit [Ohm]
   */
  double realZ;
  /*
   * Consumed power (considering only active power)
   *
   * using exponential distribution
   * unit [Watt]
   */
  static constexpr double firstMomentPower = 709.7424;
  double power;

  /*
   * Reference: (Everything below in the PLC_ImpedanceProperties)
   * A Channel Model Proposal for Indoor Power Line Communications
   * Francisco J. Can~ete, José A. Cortés, Luis Díez, and
   * José T. Entrambasaguas, Universidad de Málaga
   */
  /*
   * impedance of transmitter and receiver is considered as real and constant
   */
  static constexpr double transieverImp = 50;
  /*
   * unit [Ohm]
   * uniform distribution
   */
  static constexpr double minResonanceImpedance = 200;
  static constexpr double maxResonanceImpedance = 1800;
  double resonanceImpedance;

  /****************************************************************
   ****************   Frequency selectiveness   *******************
   ***************************************************************/
  /*
   * using uniform distribution
   * no unit
   */
  static constexpr double bottomLimitQualityFactor = 5;
  static constexpr double topLimitQualityFactor = 25;
  double qualityFactor;
  /*
   * only one resonance frequency is considered per device
   * using uniform distribution
   *
   * unit [MHz]
   */
  static constexpr double bottomLimitResonanceFrequency = 2;
  static constexpr double topLimitResonanceFrequency = 28;
  double resonanceFrequency;

  /****************************************************************
   ***********************   Time variance   **********************
   ***************************************************************/

  /*
   ***********************   Commuted impedance  ******************
   */
  /*
   * Number of uncorrelated slots
   */
  static constexpr double numSlots = 50;
  /*
   * unit [s]
   */
  double openDuration;
  double openingMoment;
  /*
   * zb - frequency selective impedance
   */
  static constexpr double zaTozbRatio = 0.5;
  /*
   ***********************   Harmonic impedance  ******************
   */
  /*
   * zb - frequency selective impedance
   * phi uniform in (0, pi)
   */
  double phi;
  static constexpr double za = 50;

  friend std::ostream&
  operator<< (std::ostream& fo, const PLC_ImpedanceProperties& c)
  {
    fo << (uint32_t) c.impedanceType << DELIMITER;
    fo << c.offStateImpedanceValue.real () << DELIMITER;
    fo << c.offStateImpedanceValue.imag () << DELIMITER;
    fo << c.realZ << DELIMITER;
    fo << c.power << DELIMITER;
    fo << c.resonanceImpedance << DELIMITER;
    fo << c.qualityFactor << DELIMITER;
    fo << c.resonanceFrequency << DELIMITER;
    fo << c.openDuration << DELIMITER;
    fo << c.openingMoment << DELIMITER;
    fo << c.phi << DELIMITER;

    return fo;
  }

  friend std::istream&
  operator>> (std::istream& fi, PLC_ImpedanceProperties& c)
  {
    uint32_t n = 0;
    fi >> n;
    c.impedanceType = ImpedanceType (n);
    double r, i;
    fi >> r;
    fi >> i;
    c.offStateImpedanceValue = PLC_Value (r, i);
    fi >> c.realZ;
    fi >> c.power;
    fi >> c.resonanceImpedance;
    fi >> c.qualityFactor;
    fi >> c.resonanceFrequency;
    fi >> c.openDuration;
    fi >> c.openingMoment;
    fi >> c.phi;
    return fi;
  }
};
struct PLC_ColouredNoiseProperties
{
  /*
   * using normal distribution
   * unit [dBm/Hz]
   */
  static constexpr double firstMomentConstantPart = -135.0;
  static constexpr double secondMomentConstantPart = 1.29;
  double constantPart;
  /*
   * using uniform distribution
   * unit [dBm/Hz]
   */
  static constexpr double bottomLimitChangingPart = 23.06;
  static constexpr double topLimitChangingPart = 74.97;
  double changingPart;
  /*
   * using exponential distribution
   * unit [MHz]
   */
  static constexpr double firstMomemntFrequencyFactor = 1.300;
  static constexpr double shiftFrequencyFactor = 0.096;
  double frequencyFactor;
};

/*
 * 0 - 10 MHz
 */
struct PLC_NarrowbandNoise0010Properties
{
  /*
   * using rounded value from normal distribution
   * no unit
   */
  static constexpr double firstMomentSpreadFactor = 6.94;
  static constexpr double secondMomentSpreadFactor = 1.00;
  uint16_t spreadFactor;
  /*
   * using normal distribution
   * unit [dBm/Hz]
   */
  static constexpr double bottomLimitAmplitude = 6.5;
  static constexpr double topLimitAmplitude = 19.2;
  std::vector<double> amplitude;
  /*
   * using exponential distribution
   * unit [MHz]
   */
  static constexpr double firstMomentBandwidth = 0.30;
  static constexpr double minBandwidth = 0.19;
  std::vector<double> bandwidth;
  /*
   * using uniform distribution
   * unit [MHz]
   */
  static constexpr double bottomLimitCenterFrequency = 0.19;
  static constexpr double topLimitCenterFrequency = 10;
  std::vector<double> centerFrequency;
};
/*
 * 10 - 20 MHz
 */
struct PLC_NarrowbandNoise1020Properties
{
  /*
   * using rounded value from normal distribution
   * no unit
   */
  static constexpr double firstMomentSpreadFactor = 0.69;
  static constexpr double secondMomentSpreadFactor = 3.64;
  uint16_t spreadFactor;
  /*
   * using noraml distribution
   * unit [dBm/Hz]
   */
  static constexpr double bottomLimitAmplitude = 4.8;
  static constexpr double topLimitAmplitude = 16.60;
  std::vector<double> amplitude;
  /*
   * using exponential distribution
   * unit [MHz]
   */
  static constexpr double firstMomentBandwidth = 0.45;
  static constexpr double minBandwidth = 0.19;
  std::vector<double> bandwidth;
  /*
   * using uniform distribution
   * unit [MHz]
   */
  static constexpr double bottomLimitCenterFrequency = 10;
  static constexpr double topLimitCenterFrequency = 20;
  std::vector<double> centerFrequency;
};
/*
 * 20 - 30 MHz
 */
struct PLC_NarrowbandNoise2030Properties
{
  /*
   * using rounded value from normal distribution
   * no unit
   */
  static constexpr double firstMomentSpreadFactor = 0.52;
  static constexpr double secondMomentSpreadFactor = 1.34;
  uint16_t spreadFactor;
  /*
   * using logarithmic normal distribution
   * unit [dBm/Hz]
   */
  static constexpr double bottomLimitAmplitude = 0.7;
  static constexpr double topLimitAmplitude = 1.5;
  std::vector<double> amplitude;
  /*
   * using exponential distribution
   * unit [MHz]
   */
  static constexpr double firstMomentBandwidth = 0.20;
  static constexpr double minBandwidth = 0.19;
  std::vector<double> bandwidth;
  /*
   * using uniform distribution
   * unit [MHz]
   */
  static constexpr double bottomLimitCenterFrequency = 20;
  static constexpr double topLimitCenterFrequency = 30;
  std::vector<double> centerFrequency;
};
struct PLC_NarrowbandNoiseProperties
{
  PLC_NarrowbandNoise0010Properties n0010;
  PLC_NarrowbandNoise1020Properties n1020;
  PLC_NarrowbandNoise2030Properties n2030;
};
struct PLC_ImpulseNoiseProperties
{
  /*
   * using gamma distribution
   * unit [mV]
   */
  static constexpr double firstMomentAmplitude = 3.9812;
  static constexpr double secondMomentAmplitude = 1.90478;
  double amplitude;
  /*
   * using Weibull distribution
   * unit [MHz]
   */
  static constexpr double firstMomentPulsation = 9.10925;
  static constexpr double secondMomentPulsation = 1.24796;
  double pulsation;
  /*
   * using gamma distribution
   * unit [ms]
   */
  static constexpr double firstMomentDuration = 5.24484;
  static constexpr double secondMomentDuration = 0.692745;
  double duration;
  /*
   * using Weibull distribution
   * unit [1 / ms]
   */
  static constexpr double firstMomentAttenuation = 4.17981;
  static constexpr double secondMomentAttenuation = 2.809;
  double attenuation;
  /*
   * using normal distribution
   * unit [kHz]
   */
  static constexpr double firstMomentFrequency = 65.8824;
  static constexpr double secondMomentFrequency = 34.956;
  double frequency;

  friend std::ostream&
  operator<< (std::ostream& fo, const PLC_ImpulseNoiseProperties& c)
  {
    fo << c.amplitude << DELIMITER;
    fo << c.pulsation << DELIMITER;
    fo << c.duration << DELIMITER;
    fo << c.attenuation << DELIMITER;
    fo << c.frequency << DELIMITER;
    return fo;
  }

  friend std::istream&
  operator>> (std::istream& fi, PLC_ImpulseNoiseProperties& c)
  {
    fi >> c.amplitude;
    fi >> c.pulsation;
    fi >> c.duration;
    fi >> c.attenuation;
    fi >> c.frequency;
    return fi;
  }
};

void
InitColouredNoiseProperties (PLC_ColouredNoiseProperties &noiseProperties);
void
InitNarrowbandNoiseProperties (PLC_NarrowbandNoiseProperties &noiseProperties);
void
InitImpulseNoiseProperties (PLC_ImpulseNoiseProperties &noiseProperties, PLC_ImpedanceProperties impedanceProperties);
void
InitImpedanceProperties (PLC_ImpedanceProperties &impedanceProperties);

/*
 * frequency unit [Hz]
 * time unit [us]
 * return unit [dBm/Hz]
 */
double
GetColouredNoiseValue (PLC_ColouredNoiseProperties colouredNoiseProperties, uint64_t frequency);
double
GetNarrowbandNoiseValue (PLC_NarrowbandNoiseProperties narrowbandNoiseProperties, uint64_t frequency);
double
GetImpulseNoiseValue (PLC_ImpulseNoiseProperties impulseNoiseProperties, PLC_ImpedanceProperties impProp,
        Bands::const_iterator band, Time t);
PLC_Value
GetImpedanceValue (PLC_ImpedanceProperties impedanceProperties, uint64_t f, Time t);

/*
 * This class implements impulsive noise, which is randomly created for all duration of mains cycle
 * The used SpectrumValue is updated each time slot with a pre-calculated SpectrumValue for the current time slot
 */
typedef std::vector<Ptr<SpectrumValue> > PLC_TimeVaryingSpectrumValue;

class PLC_CustomImpulsiveNoiseSource : public PLC_NoiseSource
{
public:
  static TypeId
  GetTypeId (void);

  PLC_CustomImpulsiveNoiseSource ()
  {
    m_currSlot = 0;
  }
  PLC_CustomImpulsiveNoiseSource (Ptr<PLC_Node> src_node, PLC_TimeVaryingSpectrumValue tvSm);

  PLC_TimeVaryingSpectrumValue
  GetTimeVaryingSpectrumValue ()
  {
    return m_tvSm;
  }

  void
  Start (void);
  void
  Stop (void);

protected:
  // pure virtual dummy function to keep pybindgen happy
  virtual void
  pureVirtualDummy (void)
  {
  }

private:

  void
  AlterPsd (void);
  PLC_TimeVaryingSpectrumValue m_tvSm;
  uint16_t m_currSlot;
};

Ptr<SpectrumValue>
CreateNarrowbandNoiseValue (Ptr<const SpectrumModel> sm);
Ptr<SpectrumValue>
CreateColouredNoiseValue (Ptr<const SpectrumModel> sm);
Ptr<SpectrumValue>
CreateInhomeBackgroundNoise (Ptr<const SpectrumModel> sm);

/*
 * the integral function is taken from http://www.boost.org/doc/libs/1_53_0/libs/multiprecision/doc/html/boost_multiprecision/tut/floats/fp_eg/gi.html
 */
template<typename value_type, typename function_type>
  inline value_type
  integral (const value_type a, const value_type b, const value_type tol, function_type func);

template<typename value_type>
  class ImpulsiveNoiseTimeDomain
  {
  public:
    ImpulsiveNoiseTimeDomain (PLC_ImpulseNoiseProperties noiseProperties, double s, double offsetTime) :
            m_offsetTime (offsetTime), m_s (s), m_noiseProperties (noiseProperties)
    {
      m_T = PLC_Time::GetPeriodS () / 2;
      m_tau = 1 / m_noiseProperties.frequency / pow (10, 3);
      m_nmax = round (m_noiseProperties.duration * m_noiseProperties.frequency);
      m_A = m_noiseProperties.amplitude;
      m_w = m_noiseProperties.pulsation * pow (10, 6);
      m_alpha = m_noiseProperties.attenuation * pow (10, 6);
    }

    value_type
    operator() (const value_type& t) const;

  private:

    double m_T;
    double m_tau;
    uint16_t m_nmax;
    double m_offsetTime;
    double m_A;
    double m_w;
    double m_alpha;
    double m_s;
    PLC_ImpulseNoiseProperties m_noiseProperties;
  };

/*
 * returns laplace transformation of impulsive noise
 */
long double
GetImpulsiveNoiseFreqDomain (PLC_ImpulseNoiseProperties noiseProperties, Time t, double s);
}

#endif /* PLCDEFSEXTENSION_H_ */
