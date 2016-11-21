/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 TUD
 *
 *  Created on: 25.08.2015
 *      Author: Stanislav Mudriievskyi <stanislav.mudriievskyi@tu-dresden.de>
 */

#ifndef GHN_PLC_DLL_MANAGEMENT_H_
#define GHN_PLC_DLL_MANAGEMENT_H_

#include "ns3/object.h"
#include "ns3/uan-address.h"
#include "ns3/queue.h"
#include "ns3/stats-module.h"

#include "ghn-plc-dll-apc.h"
#include "ghn-plc-dll-llc.h"
#include "ghn-plc-dll-mac.h"
#include "ghn-plc-phy-management.h"
#include "ghn-plc-dll-management.h"
#include "ghn-plc-phy-pcs.h"
#include "ghn-plc-routing-table.h"
#include "ghn-plc-bit-loading.h"


#define GDOTHN_MAC_CYCLE                40 //ms
#define GDOTHN_TICK                     10 //ns
#define GDOTHN_MAX_TXOP_NUMBER          65536 //TXOPs
#define GDOTHN_MAX_MSG_FRAME_DURATION   6 //ms
#define GDOTHN_MAX_BMSG_FRAME_DURATION  6 //ms
#define GDOTHN_MAX_BACK_FRAME_DURATION  6 //ms
#define GDOTHN_MIN_TIFG_DURATION        90 //mus, minimal duration of inter frame gap
#define GDOTHN_IST                      35840 //ns, duration of idle time slot
#define GDOTHN_TAIFGD                   122880 //ns, default inter frame gap before immmediate ACK
#define GDOTHN_ACK_DURATION             102400 //ns, duration of ACK frame

//#define ALS_MAC_CYCLES_INTEGRATION      1 //number of MAC cycles to be averaged for switching desicion
//#define ALS_SWITCH_POINT                0.2 //rho at which it will be switched for 3 nodes
//#define ALS_SWITCH_POINT                0.1 //rho at which it will be switched for 4 nodes
#define ALS_SWITCH_POINT                0.175 //rho at which it will be switched for 25 nodes
#define ALS_HYSTERESIS_WIDTH            5 //hysteresis width for switching

namespace ns3
{
namespace ghn {

class GhnPlcDllApc;
class GhnPlcDllLlc;
class GhnPlcDllMac;
class GhnPlcDllMacTdma;
class GhnPlcDllMacCsma;
class GhnPlcPhyManagement;
class GhnPlcPhyPcs;

enum GhnPlcDllMacProtocol
{
  CSMA_CA    = 0,
  TDMA       = 1,
  ALS        = 2, //adaptive layer switching
  COMB       = 3, //combination of CSMA/CA and TDMA in the same frame
  CSMA_CD    = 4
};


class GhnPlcDllManagement : public Object
{
public:
  static TypeId
  GetTypeId (void);
  GhnPlcDllManagement ();
  virtual
  ~GhnPlcDllManagement ();
  void
  SetDllApc (Ptr<GhnPlcDllApc> ncDllApc);
  Ptr<GhnPlcDllApc>
  GetDllApc (void);
  void
  SetDllLlc (Ptr<GhnPlcDllLlc> ncDllLlc);
  Ptr<GhnPlcDllLlc>
  GetDllLlc (void);
  void
  SetDllMac (Ptr<GhnPlcDllMacCsma> ncDllMac);
  Ptr<GhnPlcDllMacCsma>
  GetDllMac (void);
  void
  CreateDllMac();
  void
  SetPhyManagement (Ptr<GhnPlcPhyManagement> phyManagement);
  Ptr<GhnPlcPhyManagement>
  GetPhyManagement (void);
  void
  SetPhyPcs (Ptr<GhnPlcPhyPcs> ghnPhyPcs);
  Ptr<GhnPlcPhyPcs>
  GetPhyPcs (void);

  /**
   * Set MAC address (DEVICE_ID)
   *
   * @param addr 8 bit DEVICE_ID
   */
  void
  SetAddress (UanAddress address);

  /**
   * @return 8 bit DEVICE_ID
   */
  UanAddress
  GetAddress (void);
  UanAddress
  GetBroadcast (void) const;

  void
  SetNNodes (uint8_t nNodes);
  uint8_t
  GetNNodes (void);

  void
  SetRoutingTable (Ptr<GhnPlcRoutingTable> routingTable)
  {
    m_routingTable = routingTable;
  }
  Ptr<GhnPlcRoutingTable>
  GetRoutingTable ()
  {
    return m_routingTable;
  }

  void
  SetBitLoadingTable (Ptr<GhnPlcBitLoading> bitLoadingTable)
  {
    m_bitLoadingTable = bitLoadingTable;
  }
  Ptr<GhnPlcBitLoading>
  GetBitLoadingTable ()
  {
    return m_bitLoadingTable;
  }

  typedef Callback<void, Ptr<SpectrumValue> > TxPsdCallback;
  void
  SetTxPsdCallback (TxPsdCallback c)
  {
    m_setTxPsd = c;
  }

  void
  SetTxPsd (Ptr<SpectrumValue> txPsd);
  void
  DefineMacType (TypeId tid)
  {
    m_macTid = tid;
  }


protected:

  void
  NewMacCycle (void);

  Ptr<GhnPlcDllApc> m_ncDllApc;
  Ptr<GhnPlcDllLlc> m_ncDllLlc;
  Ptr<GhnPlcDllMacCsma> m_ncDllMacCsma;
  Ptr<GhnPlcPhyManagement> m_phyManagement;
  Ptr<GhnPlcPhyPcs> m_ghnPhyPcs;
  Ptr<GhnPlcRoutingTable> m_routingTable;
  Ptr<GhnPlcBitLoading> m_bitLoadingTable;

  /**
   * The DEVICE_ID which has been assigned to this device.
   */
  UanAddress m_address;
  uint8_t m_nNodes;
  float m_rho;

  TxPsdCallback m_setTxPsd;

  TypeId m_macTid;
};
}
} // namespace ns3

#endif /* GHN_PLC_DLL_MANAGEMENT_H_ */
