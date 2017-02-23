/*
 * PLC_FullDuplexOfdmPhy.h
 *
 *  Created on: 07.09.2016
 *      Author: tsokalo
 */

#ifndef PLC_FULLDUPLEXOFDMPHY_H_
#define PLC_FULLDUPLEXOFDMPHY_H_

#include <cmath>
#include <vector>
#include <deque>
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

#include "ns3/plc-phy.h"

namespace ns3
{

class PLC_FullDuplexOfdmPhy : public PLC_Phy
{
public:

  /**
   * Three states of the half duplex phy
   */
  enum State
  {
    IDLE, TX, RX, TXRX
  };

  static TypeId
  GetTypeId (void);

  /**
   * Constructor
   */
  PLC_FullDuplexOfdmPhy ();
  virtual
  ~PLC_FullDuplexOfdmPhy () = 0;

  static void
  SetGuardIntervalDuration (Time duration);
  static Time
  GetGuardIntervalDuration (void);

  /**
   * Creates rx and tx interface, respectively, on outlet
   *
   * If an impedance has been assigned previously to outlet, the value will be
   * treated as shunt impedance to the device.
   * By initializing txPsd the SpectrumModel and the OFDM subbands are defined
   *
   * @param outlet
   * @param txPsd
   */
  void
  CreateInterfaces (Ptr<PLC_Outlet> outlet, Ptr<SpectrumValue> txPsd, Ptr<PLC_Impedance> rxImpedance = 0,
          Ptr<PLC_Impedance> txImpedance = 0);

  /**
   * @return Outlet the device is attached to
   */
  Ptr<PLC_Outlet>
  GetOutlet (void)
  {
    return m_outlet;
  }

  /**
   * Set the power spectral density to be used for the outgoing waveform
   * @param txPsd
   */
  void
  SetTxPowerSpectralDensity (Ptr<SpectrumValue> txPsd);

  /**
   * @return PSD used for transmission
   */
  Ptr<const SpectrumValue>
  GetTxPowerSpectralDensity (void)
  {
    return m_txPsd;
  }

  /**
   * @return TX interface of the PHY
   */
  Ptr<PLC_TxInterface>
  GetTxInterface (void);

  /**
   * @return RX interface of the PHY
   */
  Ptr<PLC_RxInterface>
  GetRxInterface (void);

  /**
   * @return number of OFDM subcarriers used for data transmission
   */
  size_t
  GetNumSubcarriers (void)
  {
    return m_numSubcarriers;
  }

  /**
   * Set shunt impedance to the node the device is located on
   * @param shuntImpedance
   */
  void
  SetShuntImpedance (Ptr<PLC_Impedance> shuntImpedance);

  /**
   * Set access impedance for the device being in receive state
   * @param rxImpedance
   */
  void
  SetRxImpedance (Ptr<PLC_Impedance> rxImpedance);

  /**
   * Set access impedance for the device being in transmit state
   * @param rxImpedance
   */
  void
  SetTxImpedance (Ptr<PLC_Impedance> txImpedance);

  /**
   * @return Shunt impedance of the node the device is located on
   */
  Ptr<PLC_Impedance>
  GetShuntImpedance (void)
  {
    return m_shuntImpedance;
  }

  /**
   * @return Access impedance used while receiving
   */
  Ptr<PLC_Impedance>
  GetRxImpedance (void)
  {
    return m_rxImpedance;
  }

  /**
   * @return Access impedance used while transmitting
   */
  Ptr<PLC_Impedance>
  GetTxImpedance (void)
  {
    return m_txImpedance;
  }

  void
  SetNoiseFloor (Ptr<const SpectrumValue> noiseFloor);

  /**
   * Clear Channel Assessment request
   * Typically called by MAC layer
   */
  void
  CcaRequest (void);

  /**
   * Cancel previous CcaRequest
   */
  void
  CancelCca (void);

  /**
   * Clear Channel Assessment listening end
   */
  void
  EndCca (void);

  /**
   * Confirmation callback after Clear Channel Assessment request
   * This is part of the interconnection between MAC and PHY layer
   * @param c
   */
  void
  SetCcaConfirmCallback (PLC_PhyCcaConfirmCallback c);

  /**
   * Change the PHY's state to newState
   * @param newState
   */
  void
  ChangeState (State newState);

  /**
   * Get current state of the PHY
   * @return Current state
   */
  State
  GetState (void);

  /**
   * @return True if PHY is not IDLE
   */
  bool
  IsBusy (void)
  {
    return m_state != IDLE;
  }

  void
  SendFrame (Ptr<PLC_TrxMetaInfo> metaInfo);

  void
  NoiseStart (uint32_t txId, Ptr<const SpectrumValue> psd, Time duration);
  void
  NoiseStop (uint32_t txId);

protected:
  virtual void
  DoStart (void);
  virtual void
  DoDispose (void);
  virtual PLC_ChannelTransferImpl *
  DoGetChannelTransferImpl (Ptr<PLC_Phy> rxPhy);
  virtual Ptr<PLC_ChannelTransferImpl>
  DoGetChannelTransferImplPtr (Ptr<PLC_Phy> rxPhy);

  virtual void
  DoUpdateRxPsd (uint32_t txId, Ptr<const SpectrumValue> newRxPsd);

  static Time guard_interval_duration;

  void
  ComputeEquivalentImpedances (void);
  virtual PLC_PhyCcaResult
  ClearChannelAssessment (void) = 0;

  Time
  CalculateTransmissionDuration (size_t nSymbols);

  /**
   * Switch access impedance of the device according to state
   * @warning will not change the state of the PHY, but only the impedance value;
   * for a state change call ChangeState ()
   * @param state
   */
  void
  SwitchImpedance (State state);

  Ptr<PLC_Outlet> m_outlet;
  Ptr<SpectrumValue> m_txPsd;
  Ptr<PLC_TxInterface> m_txInterface;
  Ptr<PLC_RxInterface> m_rxInterface;
  Ptr<PLC_Impedance> m_shuntImpedance;
  Ptr<PLC_Impedance> m_txImpedance;
  Ptr<PLC_Impedance> m_rxImpedance;
  Ptr<PLC_Impedance> m_eqRxImpedance;
  Ptr<PLC_Impedance> m_eqTxImpedance;
  size_t m_numSubcarriers;

  Time m_rxEnd;
  Time m_txEnd;
  //
  // channel transfer function between s and me, where s is the last or current node, from whom I'm receiving data
  //
  Ptr<SpectrumValue> m_chTrF;

  // The PHY has to be aware of all receive PSDs to
  // update the interference model when a signal changes
  std::map<uint32_t, std::pair<EventId, Ptr<const SpectrumValue> > > m_rxNoisePsdMap;

  uint32_t m_locked_txId;

  EventId m_ccaEndEvent;
  EventId m_changeStateEvent;
  EventId m_txFrameEvent;
  PLC_PhyCcaConfirmCallback m_ccaConfirmCallback;
  TracedCallback<Time, State> m_PhyStateLogger;

private:

  State m_state;
};

std::ostream&
operator<< (std::ostream& os, PLC_FullDuplexOfdmPhy::State state);
}
#endif /* PLC_FULLDUPLEXOFDMPHY_H_ */
