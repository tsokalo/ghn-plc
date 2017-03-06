/*
 * plc-defs-extension.cpp
 *
 *  Created on: Sep 8, 2014
 *      Author: tsokalo
 */

#include "ns3/plc-defs-extension.h"
#include "ns3/log.h"
#include "ns3/assert.h"
#include "ns3/random-variable-stream.h"
#include "math.h"
#include "ns3/nstime.h"
#include "ns3/plc-time.h"
#include "ns3/simulator.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("PLC_Defs_Electrical_Device");
NS_OBJECT_ENSURE_REGISTERED(PLC_CustomImpulsiveNoiseSource);
/////////////////////////////////////////////////////////////////////////////////////////////////////////
TypeId
PLC_CustomImpulsiveNoiseSource::GetTypeId (void)
{
  static TypeId tid = ns3::TypeId ("ns3::PLC_CustomImpulsiveNoiseSource").SetParent<PLC_NoiseSource> ();
  return tid;
}

PLC_CustomImpulsiveNoiseSource::PLC_CustomImpulsiveNoiseSource (Ptr<PLC_Node> src_node, PLC_TimeVaryingSpectrumValue tvSm) :
        PLC_NoiseSource (src_node, tvSm.at (0), PLC_NoiseSource::TIMEVARIANT)
{
  m_tvSm = tvSm;
  m_currSlot = 0;
}

void
PLC_CustomImpulsiveNoiseSource::Start (void)
{
  PLC_LOG_FUNCTION(this);

  NS_ASSERT_MSG(m_is_initialized, "Noise Source not initialized! Call Init() before starting noise source!");

  NS_LOG_DEBUG("Starting the noise source on node " << this->GetNode()->GetVertexId());

  PLC_NoiseSource::Enable ();

  AlterPsd ();
}
void
PLC_CustomImpulsiveNoiseSource::Stop (void)
{
  PLC_LOG_FUNCTION(this);
  NS_ASSERT_MSG(m_is_initialized, "Noise Source not initialized! Call Init() before starting noise source!");

  NS_LOG_DEBUG("Stopping the noise source on node " << this->GetNode()->GetVertexId());

  PLC_NoiseSource::Disable ();
}
void
PLC_CustomImpulsiveNoiseSource::AlterPsd (void)
{
  PLC_LOG_FUNCTION(this);

  if (!IsEnabled ()) return;

  (*m_noisePsd) = (*(m_tvSm.at (m_currSlot)));
  m_currSlot++;
  if ((size_t) (m_currSlot + 1) == m_tvSm.size ()) m_currSlot = 0;

  //  NS_LOG_DEBUG("Impulsive noise PSD: " << (*m_noisePsd));

  Time duration = Seconds (PLC_Time::GetResolutionS ());
  //  NS_LOG_DEBUG("Transmitting this PSD for: " << duration);

  Simulator::ScheduleNow (&PLC_TxInterface::StartTx, m_txInterface, m_noisePsd, duration, Ptr<PLC_TrxMetaInfo> ());
  Simulator::Schedule (duration, &PLC_CustomImpulsiveNoiseSource::AlterPsd, this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

void
InitImpedanceProperties (PLC_ImpedanceProperties &impedanceProperties)
{
  using boost::math::constants::pi;

  UniformRandomVariable uniRV;

  impedanceProperties.impedanceType = ImpedanceType (uniRV.GetInteger (0, 3));

  impedanceProperties.offStateImpedanceValue = PLC_Value (10000000, 0);

  ExponentialRandomVariable expRV;
  impedanceProperties.power = expRV.GetValue (impedanceProperties.firstMomentPower, 100 * impedanceProperties.firstMomentPower);

  impedanceProperties.realZ = pow (230, 2) / impedanceProperties.power;

  impedanceProperties.qualityFactor = uniRV.GetValue (impedanceProperties.bottomLimitQualityFactor,
          impedanceProperties.topLimitQualityFactor);
  impedanceProperties.resonanceImpedance = uniRV.GetValue (impedanceProperties.minResonanceImpedance,
          impedanceProperties.maxResonanceImpedance);

  impedanceProperties.resonanceFrequency = uniRV.GetValue (impedanceProperties.bottomLimitResonanceFrequency,
          impedanceProperties.topLimitResonanceFrequency);

  impedanceProperties.openDuration = round (uniRV.GetValue (1, impedanceProperties.numSlots / 4)) * PLC_Time::GetResolutionS ();
  impedanceProperties.openingMoment = round (
          uniRV.GetValue (0, impedanceProperties.numSlots / 2 - impedanceProperties.openDuration))
          * PLC_Time::GetResolutionS ();
  impedanceProperties.phi = uniRV.GetValue (0, pi<double> ());
}
void
InitColouredNoiseProperties (PLC_ColouredNoiseProperties &noiseProperties)
{
  NormalRandomVariable normRV;
  noiseProperties.constantPart = normRV.GetValue (noiseProperties.firstMomentConstantPart,
          noiseProperties.secondMomentConstantPart);

  UniformRandomVariable uniRV;
  noiseProperties.changingPart = uniRV.GetValue (noiseProperties.bottomLimitChangingPart, noiseProperties.topLimitChangingPart);

  ExponentialRandomVariable expRV;
  noiseProperties.frequencyFactor = expRV.GetValue (noiseProperties.firstMomemntFrequencyFactor,
          100 * noiseProperties.firstMomemntFrequencyFactor) - noiseProperties.shiftFrequencyFactor;
}

void
InitNarrowbandNoiseProperties (PLC_NarrowbandNoiseProperties &noiseProperties)
{
  UniformRandomVariable uniRV;

  NormalRandomVariable normRV1;
  double val = round (
          normRV1.GetValue (noiseProperties.n0010.firstMomentSpreadFactor, noiseProperties.n0010.secondMomentSpreadFactor));
  noiseProperties.n0010.spreadFactor = (val < 0) ? 0 : val;

  for (uint16_t harmonic = 0; harmonic < noiseProperties.n0010.spreadFactor; harmonic++)
    {
      noiseProperties.n0010.amplitude.push_back (
              uniRV.GetValue (noiseProperties.n0010.bottomLimitAmplitude, noiseProperties.n0010.topLimitAmplitude));

      ExponentialRandomVariable expRV;
      noiseProperties.n0010.bandwidth.push_back (
              expRV.GetValue (noiseProperties.n0010.firstMomentBandwidth, 100 * noiseProperties.n0010.firstMomentBandwidth)
                      + noiseProperties.n0010.minBandwidth);

      noiseProperties.n0010.centerFrequency.push_back (
              uniRV.GetValue (noiseProperties.n0010.bottomLimitCenterFrequency, noiseProperties.n0010.topLimitCenterFrequency));
    }

  NormalRandomVariable normRV2;
  val = round (
          normRV2.GetValue (noiseProperties.n1020.firstMomentSpreadFactor, noiseProperties.n1020.secondMomentSpreadFactor));
  noiseProperties.n1020.spreadFactor = (val < 0) ? 0 : val;

  for (uint16_t harmonic = 0; harmonic < noiseProperties.n0010.spreadFactor; harmonic++)
    {
      noiseProperties.n1020.amplitude.push_back (
              uniRV.GetValue (noiseProperties.n1020.bottomLimitAmplitude, noiseProperties.n1020.topLimitAmplitude));

      ExponentialRandomVariable expRV;
      noiseProperties.n1020.bandwidth.push_back (
              expRV.GetValue (noiseProperties.n1020.firstMomentBandwidth, 100 * noiseProperties.n1020.firstMomentBandwidth)
                      + noiseProperties.n1020.minBandwidth);

      noiseProperties.n1020.centerFrequency.push_back (
              uniRV.GetValue (noiseProperties.n1020.bottomLimitCenterFrequency, noiseProperties.n1020.topLimitCenterFrequency));
    }

  NormalRandomVariable normRV3;
  val = round (
          normRV3.GetValue (noiseProperties.n2030.firstMomentSpreadFactor, noiseProperties.n2030.secondMomentSpreadFactor));
  noiseProperties.n2030.spreadFactor = (val < 0) ? 0 : val;

  for (uint16_t harmonic = 0; harmonic < noiseProperties.n2030.spreadFactor; harmonic++)
    {
      noiseProperties.n2030.amplitude.push_back (
              uniRV.GetValue (noiseProperties.n2030.bottomLimitAmplitude, noiseProperties.n2030.topLimitAmplitude));

      ExponentialRandomVariable expRV;
      noiseProperties.n2030.bandwidth.push_back (
              expRV.GetValue (noiseProperties.n2030.firstMomentBandwidth, 100 * noiseProperties.n2030.firstMomentBandwidth)
                      + noiseProperties.n2030.minBandwidth);

      noiseProperties.n2030.centerFrequency.push_back (
              uniRV.GetValue (noiseProperties.n2030.bottomLimitCenterFrequency, noiseProperties.n2030.topLimitCenterFrequency));
    }
}

void
InitImpulseNoiseProperties (PLC_ImpulseNoiseProperties &noiseProperties, PLC_ImpedanceProperties impedanceProperties)
{
  UniformRandomVariable uniRV;
  GammaRandomVariable gammaRV;

  noiseProperties.amplitude = gammaRV.GetValue (noiseProperties.firstMomentAmplitude, noiseProperties.secondMomentAmplitude);

  WeibullRandomVariable weibRV;
  noiseProperties.pulsation = weibRV.GetValue (noiseProperties.firstMomentPulsation, noiseProperties.secondMomentPulsation,
          100 * noiseProperties.firstMomentPulsation);
  if (noiseProperties.pulsation > 25) noiseProperties.pulsation = 25;

  noiseProperties.duration = impedanceProperties.openDuration * 1000;

  noiseProperties.attenuation = weibRV.GetValue (noiseProperties.firstMomentAttenuation,
          noiseProperties.secondMomentAttenuation, 100 * noiseProperties.firstMomentAttenuation);

  NormalRandomVariable normRV2;
  noiseProperties.frequency = normRV2.GetValue (noiseProperties.firstMomentFrequency, noiseProperties.secondMomentFrequency);
  if (noiseProperties.frequency > 200) noiseProperties.frequency = 200;

  NS_LOG_DEBUG(
          "Switching frequency (kHz): " << noiseProperties.frequency << ", Train duration (ms): " << noiseProperties.duration << ", Amplitude (mV): " << noiseProperties.amplitude << ", Pulsation frequency (MHz): " << noiseProperties.pulsation << ", Fading factor (1/ms): " << noiseProperties.attenuation << ", or fading factor: " << noiseProperties.attenuation);
}
double
GetColouredNoiseValue (PLC_ColouredNoiseProperties colouredNoiseProperties, uint64_t frequency)
{
  double noisedBmHz = colouredNoiseProperties.constantPart
          + colouredNoiseProperties.changingPart
                  * std::exp (-static_cast<long double> (frequency) / colouredNoiseProperties.frequencyFactor / pow (10, 6));
  return noisedBmHz;
}
double
GetNarrowbandNoiseValue (PLC_NarrowbandNoiseProperties noiseProperties, uint64_t frequency)
{
  double noisedBmHz = 0;
  if (frequency > 20 * pow (10, 6))
    {
      for (uint16_t harmonic = 0; harmonic < noiseProperties.n2030.spreadFactor; harmonic++)
        {
          noisedBmHz += noiseProperties.n2030.amplitude.at (harmonic)
                  * std::exp (
                          -pow (
                                  static_cast<double> (frequency) / pow (10, 6)
                                          - noiseProperties.n2030.centerFrequency.at (harmonic), 2) / 2
                                  / pow (noiseProperties.n2030.bandwidth.at (harmonic), 2));
          //          NS_LOG_DEBUG("Harmonic[" << harmonic << "] -> amplitude: " << noiseProperties.n2030.amplitude.at (harmonic)
          //                  << ", frequency (Hz): " << frequency << ", center frequency (MHz): " << noiseProperties.n2030.centerFrequency.at (harmonic)
          //                  << ", noise bandwidth (MHz): " << noiseProperties.n2030.bandwidth.at (harmonic));
        }
    }
  else if (frequency > 10 * pow (10, 6))
    {
      for (uint16_t harmonic = 0; harmonic < noiseProperties.n1020.spreadFactor; harmonic++)
        {
          noisedBmHz += noiseProperties.n1020.amplitude.at (harmonic)
                  * std::exp (
                          -pow (
                                  static_cast<double> (frequency) / pow (10, 6)
                                          - noiseProperties.n1020.centerFrequency.at (harmonic), 2) / 2
                                  / pow (noiseProperties.n1020.bandwidth.at (harmonic), 2));
          //          NS_LOG_DEBUG("Harmonic[" << harmonic << "] -> amplitude: " << noiseProperties.n1020.amplitude.at (harmonic)
          //                  << ", frequency (Hz): " << frequency << ", center frequency (MHz): " << noiseProperties.n1020.centerFrequency.at (harmonic)
          //                  << ", noise bandwidth (MHz): " << noiseProperties.n1020.bandwidth.at (harmonic));
        }
    }
  else
    {
      for (uint16_t harmonic = 0; harmonic < noiseProperties.n0010.spreadFactor; harmonic++)
        {
          noisedBmHz += noiseProperties.n0010.amplitude.at (harmonic)
                  * std::exp (
                          -pow (
                                  static_cast<double> (frequency) / pow (10, 6)
                                          - noiseProperties.n0010.centerFrequency.at (harmonic), 2) / 2
                                  / pow (noiseProperties.n0010.bandwidth.at (harmonic), 2));
          //          NS_LOG_DEBUG("Harmonic[" << harmonic << "] -> amplitude: " << noiseProperties.n0010.amplitude.at (harmonic)
          //                  << ", frequency (Hz): " << frequency << ", center frequency (MHz): " << noiseProperties.n0010.centerFrequency.at (harmonic)
          //                  << ", noise bandwidth (MHz): " << noiseProperties.n0010.bandwidth.at (harmonic));
        }
    }
  return noisedBmHz;
}
double
GetImpulseNoiseValue (PLC_ImpulseNoiseProperties noiseProperties, PLC_ImpedanceProperties impProp, Bands::const_iterator band,
        Time t)
{
  double noiseV2Hz = 0;
  Time halfMainsPeriod = Seconds (PLC_Time::GetPeriodS () / 2);
  if ((t >= halfMainsPeriod)) t = t - halfMainsPeriod;

  long double noisemV2 = GetImpulsiveNoiseFreqDomain (noiseProperties, t, band->fc);

  NS_LOG_DEBUG("Bandwidth (Hz): " << band->fh - band->fl << ", V2: " << noisemV2);

  noiseV2Hz = (noisemV2 / pow (10, 6)) * (band->fh - band->fl);

  //  switch (impProp.impedanceType)
  //    {
  //  case CONSTANT_TYPE:
  //    {
  //      break;
  //    }
  //  case FREQ_SELECTIVE_TYPE:
  //    {
  //      //      long double noisemV = GetImpulsiveNoiseFreqDomain (noiseProperties, t, band->fc);
  //      //      noiseWHz = noisemV * noisemV / pow (10, 6) * (band->fh - band->fl);
  //      break;
  //    }
  //  case FREQ_SELECTIVE_TIME_VARIANT_COMMUTED_TYPE:
  //    {
  //      if (t.GetSeconds () > impProp.openingMoment && t.GetSeconds () < (impProp.openDuration + impProp.openingMoment))
  //        {
  //
  //        }
  //      else
  //        {
  //          long double noisemV = GetImpulsiveNoiseFreqDomain (noiseProperties, t, band->fc);
  //          noiseWHz = noisemV * noisemV / pow (10, 6) * (band->fh - band->fl);
  //        }
  //      break;
  //    }
  //  case FREQ_SELECTIVE_TIME_VARIANT_HARMONIC_TYPE:
  //    {
  //      //      long double noisemV = GetImpulsiveNoiseFreqDomain (noiseProperties, t, band->fc);
  //      //      //      NS_LOG_DEBUG("band->fc: " << band->fc << ", t (ms): " << t.GetMilliSeconds() << ", Zwischenzahl mV: " << noisemV);
  //      //      //
  //      //      // unit [W / Hz]
  //      //      // Assuming transfer function almost the same in one band
  //      //      // dBm = 10*Log10 ((mV^2)/ 1000)
  //      //      // dBm/Hz = dBm - 10log (BW)
  //      //      //
  //      //      //  (*txPsd) = Pow (10.0, ((*txPsd) - 30) / 10.0);
  //      //      //
  //      //      noiseWHz = noisemV * noisemV / pow (10, 6) * (band->fh - band->fl);
  //      break;
  //    }
  //    }
  return noiseV2Hz;
}

PLC_Value
GetImpedanceValue (PLC_ImpedanceProperties impProp, uint64_t f, Time t)
{
  using boost::math::constants::pi;

  //    NS_LOG_DEBUG("Resonance Frequency (kHz): " << impProp.resonanceFrequency << ", quality factor: " << impProp.qualityFactor
  //            << ", real resistance (Ohm): " << impProp.realZ << ", off impedance value (Ohm): " << impProp.offStateImpedanceValue
  //            << ", f (Hz): " << f << ", open duration: " << openDuration.GetMilliSeconds() << ", actual time: " << t.GetMilliSeconds());

  PLC_Value value;
  Time halfMainsPeriod = Seconds (PLC_Time::GetPeriodS () / 2);
  if ((t >= halfMainsPeriod)) t = t - halfMainsPeriod;

  double k = f / impProp.resonanceFrequency / pow (10, 6);
  double resistanceBase = impProp.resonanceImpedance / (1 + pow (impProp.qualityFactor * (k - 1 / k), 2));
  double reactanceBase = -impProp.resonanceImpedance * impProp.qualityFactor * (k - 1 / k)
          / (1 + pow (impProp.qualityFactor * (k - 1 / k), 2));
  switch (impProp.impedanceType)
    {
  case CONSTANT_TYPE:
    {
      value.real (impProp.realZ);
      value.imag (0);
      break;
    }
  case FREQ_SELECTIVE_TYPE:
    {
      value.real (resistanceBase);
      value.imag (reactanceBase);
      break;
    }
  case FREQ_SELECTIVE_TIME_VARIANT_COMMUTED_TYPE:
    {
      if (t.GetSeconds () > impProp.openingMoment && t.GetSeconds () < (impProp.openDuration + impProp.openingMoment))
        {
          value.real (resistanceBase);
          value.imag (reactanceBase);
        }
      else
        {
          value.real (resistanceBase * impProp.zaTozbRatio);
          value.imag (reactanceBase * impProp.zaTozbRatio);
        }
      break;
    }
  case FREQ_SELECTIVE_TIME_VARIANT_HARMONIC_TYPE:
    {
      double phase = 2 * pi<double> () * t.GetSeconds () / PLC_Time::GetPeriodS () + impProp.phi;
      //      NS_ASSERT_MSG(sin (phase) >= 0, "Phase: " << phase);
      value.real (impProp.za + resistanceBase * sin (phase));
      value.imag (reactanceBase * sin (phase));
      break;
    }
    }

  return value;
}

Ptr<SpectrumValue>
CreateNarrowbandNoiseValue (Ptr<const SpectrumModel> sm)
{
  PLC_NarrowbandNoiseProperties nbNoiseProperties;
  InitNarrowbandNoiseProperties (nbNoiseProperties);

  Ptr<SpectrumValue> narrowbandNoiseValue = Create<SpectrumValue> (sm);
  Values::iterator value = narrowbandNoiseValue->ValuesBegin ();
  //  uint32_t carrierIndex = 0;
  for (Bands::const_iterator band = sm->Begin (); band != sm->End (); band++)
    {
      double noisedBmHz = GetNarrowbandNoiseValue (nbNoiseProperties, band->fc);
      (*value) = noisedBmHz;
      value++;
      //      NS_LOG_DEBUG("Narrowband noise f<" << carrierIndex++ << "> : " << noisedBmHz << " dBm/Hz>");
    }

  return narrowbandNoiseValue;
}

Ptr<SpectrumValue>
CreateColouredNoiseValue (Ptr<const SpectrumModel> sm)
{
  PLC_ColouredNoiseProperties colNoiseProperties;
  InitColouredNoiseProperties (colNoiseProperties);

  Ptr<SpectrumValue> colouredNoiseValue = Create<SpectrumValue> (sm);
  Values::iterator value = colouredNoiseValue->ValuesBegin ();
  //  uint32_t carrierIndex = 0;
  for (Bands::const_iterator band = sm->Begin (); band != sm->End (); band++)
    {
      double noisedBmHz = GetColouredNoiseValue (colNoiseProperties, band->fc);
      (*value) = noisedBmHz;
      value++;
      //      NS_LOG_DEBUG("Coloured noise f<" << carrierIndex++ << "> : " << noisedBmHz << " dBm/Hz>");
    }

  return colouredNoiseValue;
}
Ptr<SpectrumValue>
CreateInhomeBackgroundNoise (Ptr<const SpectrumModel> sm)
{
  Ptr<SpectrumValue> bckgNoise = Create<SpectrumValue> (sm);

  (*bckgNoise) = (*CreateColouredNoiseValue (sm)) + (*CreateNarrowbandNoiseValue (sm));

  //  uint32_t carrierIndex = 0;
  //  for (Values::iterator value = bckgNoise->ValuesBegin (); value != bckgNoise->ValuesEnd (); value++)
  //    {
  //      NS_LOG_DEBUG("Background noise f<" << carrierIndex << "> : " << (*value) << " dBm/Hz>");
  //      carrierIndex++;
  //    }
  (*bckgNoise) = Pow (10.0, ((*bckgNoise) - 30) / 10.0);
  return bckgNoise;
}

template<typename value_type, typename function_type>
  inline value_type
  integral (const value_type a, const value_type b, const value_type tol, function_type func)
  {
    unsigned n = 1U;

    value_type h = (b - a);
    value_type I = (func (a) + func (b)) * (h / 2);
    //    NS_LOG_DEBUG("I : " << I);
    for (unsigned k = 0U; k < 30U; k++)
      {
        h /= 2;

        value_type sum (0);
        for (unsigned j = 1U; j <= n; j++)
          {
            sum += func (a + (value_type ((j * 2) - 1) * h));
          }

        const value_type I0 = I;
        I = (I / 2) + (h * sum);
        //        NS_LOG_DEBUG("I : " << I);
        //        NS_LOG_DEBUG("I0 : " << I0);

        const value_type ratio = I0 / I;
        //        NS_LOG_DEBUG("ratio : " << ratio);
        const value_type delta = ratio - 1;
        //        NS_LOG_DEBUG("delta : " << delta);
        const value_type delta_abs = ((delta < 0) ? -delta : delta);
        //        NS_LOG_DEBUG("delta_abs : " << delta_abs);
        //        NS_LOG_DEBUG("(k > 1U) && (delta_abs < tol) : " << k << " " << delta_abs << " " << tol);
        if ((k > 1U) && (delta_abs < tol))
          {
            break;
          }

        n *= 2U;
      }

    return I;
  }

template<typename value_type>
  value_type
  ImpulsiveNoiseTimeDomain<value_type>::operator() (const value_type& t) const
  {
    using boost::math::constants::pi;

    value_type noisemV = 0;
    double r = m_offsetTime + t;
    uint16_t numBurst = (round (r / m_tau) + 1 < m_nmax) ? (round (r / m_tau) + 1) : m_nmax;

    //    NS_LOG_DEBUG ("numBurst: " << numBurst);
    //            NS_LOG_DEBUG ("t: " << t << ", Switching period (s): " << m_tau
    //                    << ", Number of switching periods: " << m_nmax << ", Amplitude (mV): " << m_A << ", Pulsation frequency (Hz): " << m_w
    //                    << ", Fading factor (1/s): " << m_alpha << ", Frequency (Hz): " << m_s);

    //
    // Consider this and previous period
    //

    for (uint16_t n = 0; n < numBurst; n++)
      {
        noisemV += sin (m_w * (r - n * m_tau)) * std::exp (-m_alpha * (r - n * m_tau));
        //        NS_LOG_DEBUG ("Time (s): " << r << ", r - n * m_tau: " << r - n * m_tau << ", sin (..): " << sin (m_w * (r - n * m_tau))
        //                << ", std::exp (-m_alpha * ..): " << std::exp (-m_alpha * (r - n * m_tau))
        //                << ", std::exp (-m_s * ..): " << std::exp (-m_s * t));
      }
    //
    // TODO: Actually it costs much computation time to consider also the previous period
    //

    //    r += m_T;
    //    for (uint16_t n = 0; n < m_nmax; n++)
    //      {
    //        //
    //        // r will be always > then (n * m_tau - m_T)
    //        //
    //        noisemV += sin (m_w * (r - n * m_tau)) * std::exp (-m_alpha * (r - n * m_tau));
    //        //        NS_LOG_DEBUG ("t: " << t << ", n * m_tau: " << n * m_tau);
    //        //        NS_LOG_DEBUG ("Time (s): " << t << ", sin (m_w * (t - n * m_tau)): " << sin (m_w * (t - n * m_tau))
    //        //                << ", std::exp (-m_alpha * (t - n * m_tau)): " << std::exp (-m_alpha * (t - n * m_tau))
    //        //                << ", std::exp (-m_s * t): " << std::exp (-m_s * t));
    //      }
    noisemV *= m_A * std::exp (-m_s * t);

    return noisemV;
  }
///*
// * returns laplace transformation of impulsive noise
// */
//long double
//GetImpulsiveNoiseFreqDomain (PLC_ImpulseNoiseProperties noiseProperties, Time t, double s)
//{
//  //
//  // with the accuracy we understand the number of meaning digits, which are correct
//  // e.x. by 1.0E-3 only 3 digits
//  //
//  double accuracy = 1.0E-3;
//  return (integral ((double) (0), (double) (PLC_Time::GetResolutionS ()), accuracy, ImpulsiveNoiseTimeDomain<double> (
//          noiseProperties, s, t.GetSeconds ())));
//}

///*
// * returns laplace transformation of impulsive noise
// */
//long double
//GetImpulsiveNoiseFreqDomain (PLC_ImpulseNoiseProperties noiseProperties, Time t, double s)
//{
//  double noisemV = 0;
//
//  Timeslot m = PLC_Time::GetTimeslot (t);
//  double Ts = PLC_Time::GetResolutionS ();
//  double tau = 1 / noiseProperties.frequency / pow (10, 3);
//  uint16_t nmax = round (noiseProperties.duration / tau / pow (10, 3));
//  double A = noiseProperties.amplitude;
//  double w = noiseProperties.pulsation * pow (10, 6);
//  double alpha = noiseProperties.attenuation * pow (10, 3);
//
//  NS_LOG_DEBUG("Number of time slot: " << m
//          << ", Time slot duration (s): " << Ts
//          << ", Switching period (s): " << tau
//          << ", Number of switching periods: " << nmax
//          << ", Amplitude (mV): " << A
//          << ", Pulsation frequency (Hz): " << w
//          << ", Fading factor (1/s): " << alpha
//          << ", Frequency (Hz): " << s);
//  //
//  // in order to simplify calculations we consider that the noise fades completely
//  // after 10 ms. So, the noise voltage burst in current time slot consists of a sum of the
//  // noise voltage bursts produced for a period !at least! 10 ms before
//  //
//  double a = m * Ts;
//  uint16_t numBurst = (round (m * Ts / tau) < nmax) ? (round (m * Ts / tau)) : nmax;
//  double zz5 = 0;
//  for (uint16_t n = 0; n < numBurst; n++)
//    {
//      double zz1 = cos (w * (a - n * tau));
//      double zz2 = sin (w * (a - n * tau));
//      double zz3 = pow (s + alpha, 2) + pow (w, 2);
//      double zz4 = std::exp (alpha * (n * tau - a));
//
//      zz5 += zz4 * (zz1 * w / zz3 + zz2 * (s + alpha) / zz3);
//      //      NS_LOG_DEBUG("exp alpha: " << zz5);
//    }
//
//  NS_LOG_DEBUG("Zwischenzahl1: " << zz5);
//
//  double zz6 = zz5;
//
//  a = (m + 1) * Ts;
//  numBurst = (round ((m + 1) * Ts / tau) < nmax) ? (round ((m + 1) * Ts / tau)) : nmax;
//  zz5 = 0;
//  for (uint16_t n = 0; n < numBurst; n++)
//    {
//      double zz1 = cos (w * (a - n * tau));
//      double zz2 = sin (w * (a - n * tau));
//      double zz3 = pow (s + alpha, 2) + pow (w, 2);
//      double zz4 = std::exp (alpha * (n * tau - a));
//
//      zz5 += zz4 * (zz1 * w / zz3 + zz2 * (s + alpha) / zz3);
//      NS_LOG_DEBUG("exp alpha: " << zz5);
//    }NS_LOG_DEBUG("Zwischenzahl3: " << zz5);
//
//  double zz7 = std::exp (-Ts * s) * zz5;
//  NS_LOG_DEBUG("Zwischenzahl4: " << zz7);
//
//  noisemV = A * (zz6 - zz7);
//  NS_LOG_DEBUG("Zwischenzahl5: " << noisemV);
//
//  return noisemV;
//}

///*
// * returns laplace transformation of impulsive noise
// */
//long double
//GetImpulsiveNoiseFreqDomain (PLC_ImpulseNoiseProperties noiseProperties, Time t, double s)
//{
//  double noisemV = 0;
//
//  double tau = 1 / noiseProperties.frequency / pow (10, 3);
//  uint16_t nmax = round (noiseProperties.duration / tau / pow (10, 3));
//  double A = noiseProperties.amplitude;
//  double w = noiseProperties.pulsation * pow (10, 6);
//  double alpha = noiseProperties.attenuation * pow (10, 3);
//
//  //  NS_LOG_DEBUG("Switching period (s): " << tau
//  //          << ", Number of switching periods: " << nmax
//  //          << ", Amplitude (mV): " << A
//  //          << ", Pulsation frequency (Hz): " << w
//  //          << ", Fading factor (1/s): " << alpha
//  //          << ", Frequency (Hz): " << s);
//
//  for (uint16_t n = 0; n < nmax; n++)
//    {
//      double zz1 = cos (w * n * tau);
//      double zz2 = sin (w * n * tau);
//      double zz3 = pow (s + alpha, 2) + pow (w, 2);
//      double zz4 = std::exp (alpha * n * tau);
//
//      noisemV += zz4 * (zz1 * w / zz3 - zz2 * (s + alpha) / zz3);
//      //        NS_LOG_DEBUG("n: " << n << ", zz1: " << zz1 << ", zz2: " << zz2 << ", zz3: " << zz3 << ", zz4: " << zz4 << ", s: " << s << ", noisemV: " << noisemV);
//    }
//
//  noisemV *= A;
//
//  //  NS_LOG_DEBUG("noisemV: " << noisemV);
//
//  return noisemV;
//}
/*
 * returns laplace transformation of impulsive noise
 */
//long double
//GetImpulsiveNoiseFreqDomain (PLC_ImpulseNoiseProperties noiseProperties, Time t, double s)
//{
//  double noisemV = 0;
//
//  Timeslot m = PLC_Time::GetTimeslot (t) + 1;
//  double Ts = PLC_Time::GetResolutionS ();
//  double tau = 1 / noiseProperties.frequency / pow (10, 3);
//  uint16_t nmax = round (noiseProperties.duration / tau / pow (10, 3));
//  double A = noiseProperties.amplitude;
//  double w = noiseProperties.pulsation * pow (10, 6);
//  double alpha = noiseProperties.attenuation * pow (10, 3);
//
//  NS_LOG_DEBUG("Number of time slot: " << m
//          << ", Time slot duration (s): " << Ts
//          << ", Switching period (s): " << tau
//          << ", Number of switching periods: " << nmax
//          << ", Amplitude (mV): " << A
//          << ", Pulsation frequency (Hz): " << w
//          << ", Fading factor (1/s): " << alpha
//          << ", Frequency (Hz): " << s);
//  //
//  // in order to simplify calculations we consider that the noise fades completely
//  // after 10 ms. So, the noise voltage burst in current time slot consists of a sum of the
//  // noise voltage bursts produced for a period !at least! 10 ms before
//  //
//  double a = 0;
//
//  uint16_t numBurst = (round (m * Ts / tau) < nmax) ? (round (m * Ts / tau)) : nmax;
//  double zz5 = 0;
//  double zz3 = pow (s + alpha, 2) + pow (w, 2);
//  for (uint16_t n = 0; n < numBurst; n++)
//    {
//      zz5 = 0;
//        {
//          a = m * Ts;
//
//          double zz1 = cos (w * (a - n * tau));
//          double zz2 = sin (w * (a - n * tau));
//          double zz4 = std::exp (alpha * (n * tau - a));
//
//          zz5 += zz4 * (zz1 * w / zz3 + zz2 * (s + alpha) / zz3);
//        }
//      //        {
//      //          a = (m + 1) * Ts;
//      //          double zz1 = cos (w * (a - n * tau));
//      //          double zz2 = sin (w * (a - n * tau));
//      //          double zz3 = pow (s + alpha, 2) + pow (w, 2);
//      //          double zz4 = std::exp (alpha * (n * tau - a));
//      //
//      //          zz5 -= zz4 * (zz1 * w / zz3 + zz2 * (s + alpha) / zz3);
//      //        }
//      noisemV += zz5 * zz5;
//      NS_LOG_DEBUG("n: " << n << ", s: " << s << "exp alpha: " << zz5);
//    }
//
//  //  noisemV = std::exp (-Ts * s) * noisemV;
//  NS_LOG_DEBUG("noisemV: " << noisemV);
//
//  return noisemV;
//}
//long double
//GetImpulsiveNoiseFreqDomain (PLC_ImpulseNoiseProperties noiseProperties, Time t, double s)
//{
//  double noisemV = 0;
//
//  Timeslot m = PLC_Time::GetTimeslot (t) + 1;
//  double Ts = PLC_Time::GetResolutionS ();
//  double tau = 1 / noiseProperties.frequency / pow (10, 3);
//  uint16_t nmax = round (noiseProperties.duration / tau / pow (10, 3));
//  double A = noiseProperties.amplitude;
//  double w = noiseProperties.pulsation * pow (10, 6);
//  double alpha = noiseProperties.attenuation * pow (10, 3);
//
//  //  NS_LOG_DEBUG("Number of time slot: " << m
//  //          << ", Time slot duration (s): " << Ts
//  //          << ", Switching period (s): " << tau
//  //          << ", Number of switching periods: " << nmax
//  //          << ", Amplitude (mV): " << A
//  //          << ", Pulsation frequency (Hz): " << w
//  //          << ", Fading factor (1/s): " << alpha
//  //          << ", Frequency (Hz): " << s);
//  //
//  // in order to simplify calculations we consider that the noise fades completely
//  // after 10 ms. So, the noise voltage burst in current time slot consists of a sum of the
//  // noise voltage bursts produced for a period !at least! 10 ms before
//  //
//  double a = 0;
//
//  uint16_t numBurst = (round (m * Ts / tau) < nmax) ? (round (m * Ts / tau)) : nmax;
//  double zz5 = 0;
//  double zz3 = pow (s + alpha, 2) + pow (w, 2);
//  for (uint16_t n = 0; n < numBurst; n++)
//    {
//      zz5 = 0;
//        {
//          a = (m) * Ts;
//          double zz1 = cos (w * (a - n * tau));
//          double zz2 = sin (w * (a - n * tau));
//          double zz4 = std::exp (alpha * (n * tau - a));
//
//          zz5 = zz4 * (zz1 * w / zz3 + zz2 * (s + alpha) / zz3);
//        }
//      //        {
//      //          a = (m + 1) * Ts;
//      //          double zz1 = cos (w * (a - n * tau));
//      //          double zz2 = sin (w * (a - n * tau));
//      //          double zz3 = pow (s + alpha, 2) + pow (w, 2);
//      //          double zz4 = std::exp (alpha * (n * tau - a));
//      //
//      //          zz5 -= zz4 * (zz1 * w / zz3 + zz2 * (s + alpha) / zz3);
//      //        }
//      noisemV += zz5;
//      //      NS_LOG_DEBUG("n: " << n << ", s: " << s << "exp alpha: " << zz5);
//    }
//
//  noisemV *= A;
//  //  NS_LOG_DEBUG("noisemV: " << noisemV);
//
//  return noisemV;
//}
long double
GetImpulsiveNoiseFreqDomain (PLC_ImpulseNoiseProperties noiseProperties, Time t, double s)
{
  double noisemV2 = 0;

  Timeslot m = PLC_Time::GetTimeslot (t) + 1;
  double Ts = PLC_Time::GetResolutionS ();
  double tau = 1 / noiseProperties.frequency / pow (10, 3);
  uint16_t nmax = round (noiseProperties.duration / tau / pow (10, 3));
  double A = noiseProperties.amplitude;
  double w0 = noiseProperties.pulsation * pow (10, 6);
  double alpha = -noiseProperties.attenuation * pow (10, 3);
  double w = 2 * pi<double> () * s;

  double w02 = pow (w0, 2);
  double w2 = pow (w, 2);
  double alpha2 = pow (alpha, 2);

  double a1 = pow (alpha2 + w02 - w2, 2) + 4 * w2 * alpha2;
  double a2 = w0 * (alpha2 + w02 - w2);
  double a3 = 2 * w0 * w * alpha;
  double a4 = alpha * (alpha2 + w02 + w2);
  double a5 = w * (-alpha2 + w02 - w2);
  double a6 = pow (a2 / a1, 2) + pow (a3 / a1, 2);
  double a7 = pow (a4 / a1, 2) + pow (a5 / a1, 2);

  //  NS_LOG_DEBUG("Number of time slot: " << m
  //          << ", Time slot duration (s): " << Ts
  //          << ", Switching period (s): " << tau
  //          << ", Number of switching periods: " << nmax
  //          << ", Amplitude (mV): " << A
  //          << ", Pulsation frequency (Hz): " << w
  //          << ", Fading factor (1/s): " << alpha
  //          << ", Frequency (Hz): " << s);
  //
  // in order to simplify calculations we consider that the noise fades completely
  // after 10 ms. So, the noise voltage burst in current time slot consists of a sum of the
  // noise voltage bursts produced for a period !at least! 10 ms before
  //

  uint16_t numBurst = (round (m * Ts / tau) < nmax) ? (round (m * Ts / tau)) : nmax;

  for (uint16_t n = 0; n < numBurst; n++)
    {
      double k = -alpha * n * tau;
      double beta = -w0 * n * tau;

      noisemV2 += std::exp (2 * k) * (pow (cos (beta), 2) * a6 + pow (sin (beta), 2) * a7);
      //      NS_LOG_DEBUG("n: " << n << ", s: " << s << "exp alpha: " << zz5);
    }

  NS_LOG_DEBUG("noisemV2 / mV2: " << noisemV2);
  noisemV2 *= (A * A);

  return noisemV2;
}

}
