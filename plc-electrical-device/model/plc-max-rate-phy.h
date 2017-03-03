/*
 * PlcMaxRatePhy.h
 *
 *  Created on: Oct 6, 2015
 *      Author: tsokalo
 */

#ifndef PLCMAXRATEPHY_H_
#define PLCMAXRATEPHY_H_

#include "ns3/plc-phy.h"
#include "ns3/plc-max-rate.h"

namespace ns3
{
class PlcMaxRatePhy : public PLC_HalfDuplexOfdmPhy
{
public:
  static TypeId
  GetTypeId (void);

  PlcMaxRatePhy (void);

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

  virtual void
  PreambleDetectionSuccessful (uint32_t txId, Ptr<const SpectrumValue> rxPsd, Time duration,
          Ptr<const PLC_TrxMetaInfo> metaInfo);

  double
  GetTransmissionRateLimit (Ptr<SpectrumValue> rxPsd);
  Ptr<SpectrumValue>
  GetBpc ();

protected:
  static size_t modulation_symbols_per_code_block;
  static double rateless_coding_overhead;

  virtual void
  DoStart (void);
  virtual void
  DoDispose (void);
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

  void
  PrepareTransmission (Ptr<PLC_TrxMetaInfo> metaInfo);
  void
  PrepareFixedRateTransmission (Ptr<PLC_TrxMetaInfo> metaInfo);
  void
  PrepareRatelessTransmission (Ptr<PLC_TrxMetaInfo> metaInfo);

  PLC_PhyCcaResult
  ClearChannelAssessment (void);

  size_t
  RequiredChunks (size_t num_blocks);
  size_t
  ChunksInByte (size_t num_chunks, size_t raw_bits_per_symbol);

  Ptr<PlcMaxRate> m_information_rate_model;

  Ptr<Packet> m_incoming_frame;
  PLC_PhyFrameControlHeader m_txFch;
  PLC_PhyFrameControlHeader m_rxFch;
  PLC_PhyFrameControlHeader m_lastDataFch;
  ModulationAndCodingScheme m_header_mcs;
  ModulationAndCodingScheme m_payload_mcs;
  uint16_t m_txMessageId;

  size_t m_maxRxQueueSize;

  PLC_PayloadReceptionFailedCallback m_payload_reception_failed_cb;
};
}
#endif /* PLCMAXRATEPHY_H_ */
