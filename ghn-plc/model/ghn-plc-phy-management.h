/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 TUD
 *
 *  Created on: 25.08.2015
 *      Author: Stanislav Mudriievskyi <stanislav.mudriievskyi@tu-dresden.de>
 */

#ifndef GHN_PLC_PHY_MANAGEMENT_H_
#define GHN_PLC_PHY_MANAGEMENT_H_

#include <random>

#include "ns3/object.h"
#include "ns3/callback.h"

#include "ns3/plc-phy.h"

#include "ghn-plc-phy-pcs.h"
#include "ghn-plc-phy-pma.h"

namespace ns3 {
namespace ghn {
class GhnPlcPhyPcs;
class GhnPlcPhyPma;

typedef Callback<uint32_t> FrameSizeCallback;
typedef Callback<double> GatheredInfBitsCallback;
typedef Callback<Ptr<PLC_TxInterface> > TxInterfaceCallback;
typedef Callback<Ptr<PLC_RxInterface> > RxInterfaceCallback;

class GhnPlcPhyManagement : public Object
{
public:
  static TypeId GetTypeId (void);
  GhnPlcPhyManagement ();
  virtual ~GhnPlcPhyManagement ();

  //PHY sub-layers
  void SetPhyPcs (Ptr<GhnPlcPhyPcs> ghnPhyPcs);
  Ptr<GhnPlcPhyPcs> GetPhyPcs (void);
  void SetPhyPma (Ptr<GhnPlcPhyPma> ghnPhyPma);
  Ptr<GhnPlcPhyPma> GetPhyPma (void);

  //TX parameters
  void SetTxPacketSize (uint32_t txPacketSize);
  uint32_t GetTxPacketSize (void);
  void SetTxMulticastIndication (uint8_t txMulticastIndication);
  uint8_t GetTxMulticastIndication (void);
  void SetTxHeaderSegmentationIndication (uint8_t txHeaderSegmentationIndication);
  uint8_t GetTxHeaderSegmentationIndication (void);
  void SetTxExtendedHeaderIndication (uint8_t txExtendedHeaderIndication);
  uint8_t GetTxExtendedHeaderIndication (void);
  void SetTxFecBlockSize (FecBlockSizeType txFecBlockSize);
  FecBlockSizeType GetTxFecBlockSize (void);
  void SetTxPayloadFecRate (FecRateType txPayloadFecRate);
  FecRateType GetTxPayloadFecRate (void);
  void SetTxRepetitionsNumber (EncodingRepetitionsNumberType txRepetitionsNumber);
  EncodingRepetitionsNumberType GetTxRepetitionsNumber (void);
  void SetTxReplyRequired (uint8_t replyRequired);
  uint8_t GetTxReplyRequired (void);
  void SetTxDomainId (uint8_t txDomainId);
  uint8_t GetTxDomainId (void);
  void SetTxSourceId (uint8_t txSourceId);
  uint8_t GetTxSourceId (void);
  void SetTxDestinationId (uint8_t txDestinationId);
  uint8_t GetTxDestinationId (void);

  //Are used to find out number of padded bytes in MPDU (is transferred in MSG header as duration)
  void SetTxFrameDuration (uint16_t txFrameDuration);
  uint16_t GetTxFrameDuration (void);

  void SetTxConnectionIdentifier (uint8_t txConnectionIdentifier);
  uint8_t GetTxConnectionIdentifier (void);

  void SetTxCtmgData16 (uint16_t txCtmgData16);
  uint16_t GetTxCtmgData16 (void);

  //Are used to find out number of padded bytes in MPDU (is transferred in MSG header as duration)
  void SetRxFrameDuration (uint16_t rxFrameDuration);
  uint16_t GetRxFrameDuration (void);
  void SetRxTime (Time rxTime);
  Time GetRxTime (void);
  void SetRxReplyRequired (uint8_t replyRequired);
  uint8_t GetRxReplyRequired (void);
  void SetRxAckReceived (bool ackReceived);
  bool GetRxAckReceived (void);
  void SetRxConnectionIdentifier (uint8_t rxConnectionIdentifier);
  uint8_t GetRxConnectionIdentifier (void);
  void SetRxDomainId (uint8_t rxDomainId);
  uint8_t GetRxDomainId (void);
  void SetMcAckSlotsNumber (uint8_t mcAckSlotsNumber) { m_mcAckSlotsNumber = mcAckSlotsNumber; }
  uint8_t GetMcAckSlotsNumber (void) { return m_mcAckSlotsNumber; }

  void SetRxCtmgData16 (uint16_t rxCtmgData16);
  uint16_t GetRxCtmgData16 (void);


  //service functions
  uint16_t GetTxFecBlockSizeFromHeader (void);
  uint8_t GetTxRepetitionsNumberFromHeader (void);

  //for NetDevice
  Time GetTxTime (uint32_t mpduLength, uint8_t sourceId, uint8_t destinationId);//mpduLength in Byte
  uint32_t GetDataAmount (Time txTime, uint8_t sourceId, uint8_t destinationId);
  uint32_t GetDatarate (uint8_t sourceId, uint8_t destinationId);//return datarate in bps

  bool IsBlockSuccess();

  void SetFrameSizeCallback(FrameSizeCallback cb){m_frameSizeCallback = cb;}
  void SetGatheredInfBitsCallback(GatheredInfBitsCallback cb){m_gatheredInfBitsCallback = cb;}
  void SetTxInterfaceCallback(TxInterfaceCallback cb){m_txInterfaceCallback = cb;}
  void SetRxInterfaceCallback(RxInterfaceCallback cb){m_rxInterfaceCallback = cb;}

  Ptr<PLC_TxInterface> GetTxInterface(){NS_ASSERT(!m_txInterfaceCallback.IsNull());return m_txInterfaceCallback();}
  Ptr<PLC_RxInterface> GetRxInterface(){NS_ASSERT(!m_rxInterfaceCallback.IsNull());return m_rxInterfaceCallback();}

private:
  Ptr<GhnPlcPhyPcs> m_ghnPhyPcs;
  Ptr<GhnPlcPhyPma> m_ghnPhyPma;

  //TX parameters
  uint32_t m_txPacketSize;
  uint8_t m_txMulticastIndication;
  uint8_t m_txHeaderSegmentationIndication;
  uint8_t m_txExtendedHeaderIndication;
  FecBlockSizeType m_txFecBlockSize;
  FecRateType m_txPayloadFecRate;
  EncodingRepetitionsNumberType m_txRepetitionsNumber;
  uint8_t m_txReplyRequired;
  uint8_t m_txDomainId;
  uint8_t m_txSourceId;
  uint8_t m_txDestinationId;

  uint8_t m_txFecConcatenationFactorH;
  uint8_t m_txFecConcatenationFactorZ;

  uint16_t m_txFrameDuration;
  uint8_t m_txConnectionIdentifier; //used to transfer current MAC
  uint16_t m_txCtmgData16; //used to transfer current MAC in CTMG


  //RX parameters
  uint32_t m_rxPacketSize;
  uint8_t m_rxMulticastIndication;
  uint8_t m_rxHeaderSegmentationIndication;
  uint8_t m_rxExtendedHeaderIndication;
  FecBlockSizeType m_rxFecBlockSize;
  FecRateType m_rxPayloadFecRate;
  EncodingRepetitionsNumberType m_rxRepetitionsNumber;
  uint8_t m_rxReplyRequired;
  uint8_t m_rxDomainId;
  uint8_t m_mcAckSlotsNumber;

  uint8_t m_rxFecConcatenationFactorH;
  uint8_t m_rxFecConcatenationFactorZ;

  uint16_t m_rxFrameDuration;
  Time m_rxTime; //the time channel is busy in current MAC cycle
  bool m_rxAckReceived;
  uint8_t m_rxConnectionIdentifier; //used to transfer current MAC
  uint16_t m_rxCtmgData16; //used to transfer current MAC in CTMG

  std::default_random_engine m_randGen;

  FrameSizeCallback m_frameSizeCallback;
  GatheredInfBitsCallback m_gatheredInfBitsCallback;
  TxInterfaceCallback m_txInterfaceCallback;
  RxInterfaceCallback m_rxInterfaceCallback;
};
}
} // namespace ns3

#endif /* GHN_PLC_PHY_MANAGEMENT_H_ */
