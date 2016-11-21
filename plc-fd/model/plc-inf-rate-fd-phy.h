/*
 * PLC_InfRateFDPhy.h
 *
 *  Created on: 07.09.2016
 *      Author: tsokalo
 */

#ifndef PLC_INFRATEFDPHY_H_
#define PLC_INFRATEFDPHY_H_

#include <cmath>
#include <vector>
#include <deque>
#include <functional>
#include <ns3/object.h>
#include <ns3/nstime.h>
#include <ns3/traced-value.h>
#include <ns3/trace-source-accessor.h>
#include <ns3/event-id.h>
#include "ns3/plc-interface.h"
#include "ns3/plc-header.h"
#include "ns3/plc-outlet.h"
#include "ns3/plc-link-performance-model.h"
#include "ns3/plc-channel.h"
#include <array>

#include "plc-full-duplex-ofdm-phy.h"

//
// channel attenuation to self interference cancellation mapping
// dB to dB
//
//static std::array<double, 56> sca_ecg =
//  { 33.287851983865, 33.9974462737789, 34.7107947619187, 35.4278530674704, 36.1486851804404, 36.8734872143585, 37.602611397994,
//      38.3365884766783, 39.0761460936207, 39.8222201405802, 40.5759555829201, 41.3386929710344, 42.1119368528043,
//      42.8973027027241, 43.6964398681046, 44.5109294516685, 45.3421579998857, 46.1911702754402, 47.0585071093491,
//      47.9440371232713, 48.8467936874984, 49.7648304938553, 50.6951102277699, 51.6334407123757, 52.5744713514826,
//      53.5117596352923, 54.4379129813818, 55.344805538995, 56.2238632405146, 57.0664039362469, 57.8640135756672,
//      58.608934781319, 59.2944413969429, 59.9151721063663, 60.4673982052706, 60.9492049801955, 61.3605725460777,
//      61.7033498103892, 61.9811236901616, 62.1989939503514, 62.3632712273013, 62.4811212473551, 62.5601814578949,
//      62.6081770311803, 62.632561536667, 62.6402038209678, 62.6371373077968, 62.6283816829827, 62.6178404524475,
//      62.6082718000982, 62.6013250587155, 62.5976313072674, 62.5969343039581, 62.5982471555233, 62.6000206542916,
//      62.6003108161735 };
static std::array<double, 85> sca_ecg =
  { 10.2972492044, 11.0900286106, 11.8828080168, 12.675587423, 13.4683668292, 14.2611462354, 15.0539256416, 15.8467050477,
      16.6394844539, 17.4322638601, 18.2250432663, 19.0178226725, 19.8106020787, 20.6033814849, 21.3961608911, 22.1889402972,
      22.9817197034, 23.7744991096, 24.5672785158, 25.360057922, 26.1528373282, 26.9456167344, 27.7383961406, 28.5311755467,
      29.3239549529, 30.1167343591, 30.9095137653, 31.7022931715, 32.4950725777, 33.2878519839, 33.9974462738, 34.7107947619,
      35.4278530675, 36.1486851804, 36.8734872144, 37.602611398, 38.3365884767, 39.0761460936, 39.8222201406, 40.5759555829,
      41.338692971, 42.1119368528, 42.8973027027, 43.6964398681, 44.5109294517, 45.3421579999, 46.1911702754, 47.0585071093,
      47.9440371233, 48.8467936875, 49.7648304939, 50.6951102278, 51.6334407124, 52.5744713515, 53.5117596353, 54.4379129814,
      55.344805539, 56.2238632405, 57.0664039362, 57.8640135757, 58.6089347813, 59.2944413969, 59.9151721064, 60.4673982053,
      60.9492049802, 61.3605725461, 61.7033498104, 61.9811236902, 62.1989939504, 62.3632712273, 62.4811212474, 62.5601814579,
      62.6081770312, 62.6325615367, 62.640203821, 62.6371373078, 62.628381683, 62.6178404524, 62.6082718001, 62.6013250587,
      62.5976313073, 62.596934304, 62.5982471555, 62.6000206543, 62.6003108162 };

namespace ns3
{
typedef Callback<void> PLC_CollisionDetection;
typedef Callback<void, uint32_t, Ptr<const SpectrumValue>, Ptr<const PLC_TrxMetaInfo> > EndRxHeaderCallback;
//typedef std::function<void(uint32_t, Ptr<const SpectrumValue>, Ptr<const PLC_TrxMetaInfo>) > EndRxHeaderCallback;

/**
 * \class PLC_InfRateFDPhy
 *
 * Simple implementation of a ful duplex phy using the information rate model
 */
class PLC_InfRateFDPhy : public PLC_FullDuplexOfdmPhy
{
public:
  static TypeId
  GetTypeId (void);

  PLC_InfRateFDPhy (void);

  /**
   * A second fixed rate modulation and coding scheme has to be defined for the
   * transmission of control frames and frame header.
   * Only the payload of data frames can be encoded rateless.
   * The robustness of control frame and frame header transmission
   * can be increased by choosing a lower mcs
   */
  void
  SetHeaderModulationAndCodingScheme (ModulationAndCodingScheme mcs);
  ModulationAndCodingScheme
  GetHeaderModulationAndCodingScheme (void);

  void
  SetPayloadModulationAndCodingScheme (ModulationAndCodingScheme mcs);
  ModulationAndCodingScheme
  GetPayloadModulationAndCodingScheme (void);

  /**
   * Define number of modulation symbols needed to map one code block
   * @param spb
   */
  static void
  SetOfdmSymbolsPerCodeBlock (size_t spb);

  /**
   * @return Modulation symbols needed to map one code block
   */
  size_t
  GetOfdmSymbolsPerCodeBlock (void);

  /**
   * Define the minimal overhead induced by rateless encoding.
   * default: 0.2
   *
   * TODO: non static estimation with respect to message length
   * and claimed decoding propability
   *
   * @param overhead (1 = 100%)
   */
  static void
  SetRatelessCodingOverhead (double overhead);
  static double
  GetRatelessCodingOverhead (void)
  {
    return rateless_coding_overhead;
  }

  /**
   * Callback for a failed header reception
   * @param c
   */
  void
  SetPayloadReceptionFailedCallback (PLC_PayloadReceptionFailedCallback c);

  virtual void
  EndRxHeader (uint32_t txId, Ptr<const SpectrumValue> rxPsd, Ptr<const PLC_TrxMetaInfo> metaInfo);
  virtual void
  EndRxPayload (Ptr<const PLC_TrxMetaInfo> metaInfo);
  void
  ReceptionFailure (void);

  virtual bool
  SendRedundancy (void);

  void
  PreambleDetection (uint32_t txId, Ptr<const SpectrumValue> rxPsd, Time duration, Ptr<const PLC_TrxMetaInfo> metaInfo);
  void
  PreambleDetectionSuccessful (uint32_t txId, Ptr<const SpectrumValue> rxPsd, Time duration,
          Ptr<const PLC_TrxMetaInfo> metaInfo);
  void
  PreambleCollisionDetection ();

  double
  GetTransmissionRateLimit (Ptr<SpectrumValue> rxPsd);

  void
  SetCollisionDetection (PLC_CollisionDetection cb)
  {
    m_collisionDetection = cb;
  }
  void
  SetEndRxHeaderCallback(EndRxHeaderCallback cb)
  {
    m_endRxHeaderCallback = cb;
  }
  void
  AbortReception ();

protected:
  static size_t modulation_symbols_per_code_block;
  static double rateless_coding_overhead;

  virtual void
  DoStart (void);
  virtual void
  DoDispose (void);
  virtual bool
  DoStartTx (Ptr<PLC_TrxMetaInfo> metaInfo);
  virtual bool
  DoStartTx (Ptr<const Packet> p);
  virtual void
  DoStartRx (uint32_t txId, Ptr<const SpectrumValue> rxPsd, Time duration, Ptr<const PLC_TrxMetaInfo> metaInfo);
  virtual Ptr<PLC_LinkPerformanceModel>
  DoGetLinkPerformanceModel (void);

  virtual void
  StartReception (uint32_t txId, Ptr<const SpectrumValue> rxPsd, Time duration, Ptr<const PLC_TrxMetaInfo> metaInfo);
  virtual void
  NotifySuccessfulReception (void);
  virtual void
  NotifyPayloadReceptionFailed (Ptr<const PLC_TrxMetaInfo> metaInfo);

  PLC_PhyCcaResult
  ClearChannelAssessment (void);
  PLC_PhyCcaResult
  CollisionDetection ();
  double
  GetMeanSelfInterferencePower (Ptr<const SpectrumValue> rxPsd);

  size_t
  RequiredChunks (size_t num_blocks);
  size_t
  ChunksInByte (size_t num_chunks, size_t raw_bits_per_symbol);

  Ptr<SpectrumValue>
  ComputeSelfInterference (Ptr<SpectrumValue> channelImp, Ptr<const SpectrumValue> rxPsd);
  /*
   * self-interference noise
   */
  void
  SiNoiseStart ();
  void
  CheckCollision ();

  Ptr<PLC_InformationRateModel> m_information_rate_model;

  Ptr<Packet> m_incoming_frame;
  PLC_PhyFrameControlHeader m_txFch;
  PLC_PhyFrameControlHeader m_rxFch;
  PLC_PhyFrameControlHeader m_lastDataFch;
  ModulationAndCodingScheme m_header_mcs;
  ModulationAndCodingScheme m_payload_mcs;
  uint16_t m_txMessageId;
  EventId m_preambleDetectionEvent;
  EventId m_collisionCheckEvent;
  EventId m_receptionEndEvent;
  EventId m_stateChangeEvent;

  size_t m_maxRxQueueSize;

  PLC_PayloadReceptionFailedCallback m_payload_reception_failed_cb;
  PLC_CollisionDetection m_collisionDetection;
  EndRxHeaderCallback m_endRxHeaderCallback;
};
}

#endif /* PLC_INFRATEFDPHY_H_ */
