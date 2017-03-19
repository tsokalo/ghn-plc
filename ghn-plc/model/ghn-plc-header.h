/*
 * ghn-header.h
 *
 *  Created on: Jul 1, 2015
 *      Author: tsokalo
 */

#ifndef GHN_PLC_HEADER_H_
#define GHN_PLC_HEADER_H_

#include <stdint.h>
#include <vector>
#include <deque>

#include <ns3/ptr.h>
#include <ns3/packet.h>
#include <ns3/uan-address.h>

namespace ns3
{
namespace ghn
{
/*
 * *************************************************************
 *                 VERSIONS
 * *************************************************************
 */
///*
// * in this version the channel sensing is done only after TS_DURATION
// */
//#define VERSION_NOSENSE
/*
 * in this version the channel sensing is done in two cases:
 * - like if version VERSION_NOSENSE
 * - before each transmission start
 */

/*
 * *************************************************************
 *                 SIMULATION LAYER PARAMETERS
 * *************************************************************
 */
/*
 * MASTER_NODE port better not change
 */

#define MASTER_NODE 0
#define MULTICAST_PORT 252
#define UNDEFINED_NODE 254
#define GHN_BROADCAST_PORT 255

/*
 * The period in which all the nodes in the network will make a handshake with the master
 * time unit [s]
 * You have to increase/decrease this period manually. You HAVE TO ENSURE!!!
 * that all queues of all nodes does not contain any packets at the
 * end of this period
 * This adaption should be made every time when you change the network topology
 * (of course it does not cocern the topology changes during the simulation run after WARM-UP)
 * For this purpose set the traffic type to "Simple" and set a few Packets to be transmitted
 * Run one of the routines. If the short statistics of your simulation shows transmission of
 * exactly the amount of packet that you have set - than you can decrease this period
 */
#define GHN_WARMUP_PERIOD 1.0
/*
 * The period is needed by TCP sockets to be released
 */
#define GHN_TCP_SERVER_CLOSE 0
#define GHN_TCP_CLIENT_CLOSE 1
//10
#define GHN_NS3_DEVICE_CLOSE 2
//20
#define GHN_NS3_SIMULATION_CLOSE 0.5

#if GHN_TCP_CLIENT_CLOSE > GHN_TCP_SERVER_CLOSE
#define GHN_CALMDOWN_PERIOD (GHN_TCP_CLIENT_CLOSE + GHN_NS3_DEVICE_CLOSE)
#else
#define GHN_CALMDOWN_PERIOD (GHN_TCP_SERVER_CLOSE + GHN_NS3_DEVICE_CLOSE)
#endif
/*
 * Duration of the simiulation excluding the warm-up period
 * time unit [s]
 */
#define GHN_SIM_DURATION 2
/*
 * This is a boolean parameter
 * Depending on it value the writting of the measurement data to the files may be switched off
 */
#define GHN_WRITE_MEAS_RESULTS 1
/*
 * save only the measurement on the APP layer
 */
#define GHN_WRITE_RESULTS_ONLY_APP 1
/*
 * period to output real time to the screen
 * unit [s]
 */
#define GHN_SHOWTIME_PERIOD 0.5
/*
 * *************************************************************
 *                    LAYER MANAGEMENT BLOCK PARAMETERS
 * *************************************************************
 */
/*
 * unit description: if GHN_LM_OVERHEAD = 0, we get 100% bandwidth
 * [%]
 */
#define GHN_LM_OVERHEAD 20.4
/*
 * *************************************************************
 *                    GhnMacLayer LAYER PARAMETERS
 * *************************************************************
 */
#define NODE_MAC_ADDRESS 0
/*
 * A period to sense the channel
 * unit [symbols]
 */
#define GHN_DELAY_TXOP 1
/*
 * update period of the information about the lenght of the Slaves' queues
 * time unit [ms]
 */
#define GHN_UPDATE_PERIOD 1
/*
 * g.9961
 * unit [ns]
 */
#define TICK 10
/*
 * unit [Hz]
 */
#define GHN_POWER_FREQUENCY 50.0
/*
 * G.9961
 * unit [s]
 */
#define GHN_CYCLE_MIN (2/GHN_POWER_FREQUENCY)
#define GHN_CYCLE_MAX (2/GHN_POWER_FREQUENCY)
/*
 * G.9961
 * unit [us]
 */
//#define GHN_TS_DURATION 35.84
#define GHN_TS_DURATION 120
#define GHN_TIFG_MIN 90

#define GHN_MAP_TX_SETUP_TIME 2000
#define GHN_INTER_MAP_RMAP_GAP 1000
/*****************************************************************************
 * G.9961
 * group of constants relative ot ACK procedures
 */
/*
 * wait before send ACK
 * unit [us]
 */
#define GHN_TAIFG_D 122.88
#define GHN_TAIFG_MIN 20.48
#define GHN_TAIFG_MAX 122.88
#define MIN_SYM_VAR_AIFG 2

#define GHN_BLKSZ_120 120
#define GHN_BLKSZ_540 540

#define GHN_ACK_MAX_WINDOW_SIZE_MANAGE_120 32
#define GHN_ACK_MAX_WINDOW_SIZE_MANAGE_540 16

#define GHN_ACK_MAX_WINDOW_SIZE_DATA(blockSize) (blockSize == GHN_BLKSZ_120) ? 564 : 376
typedef uint16_t Ssn;
typedef Ssn VirtSsn;
typedef Ssn GenSize;
struct CodingUnit
{
  std::deque<Ssn> ssn;
  Ssn numCoded;
};

/*
 * the maximum time interval a segment shall be kept in the ACK_TX_WINDOW
 * value to calibrate
 * unit [ms]
 */
#define GHN_ACK_BLOCK_LIFETIME 1000

//ACK_TX_WINDOW_START - SSN of the oldest unacknowledged segment
//ACK_TX_NEXT_SSN - SSN of the next segment to send
//ACK_TX_WINDOW - he range of SSNs between the oldest unacknowledged segment and the and newest unacknowledged segment, inclusive
//ACK_TX_CONF_WINDOW - the maximum range of SSNs the transmitter is permitted to send
//ACK_TX_CONF_WINDOW_SIZE - is less or equal to GHN_ACK_MAX_WINDOW_SIZE
//ACK_TX_RESET - forbid or allows sending segments

// RPRQ field - inidcates type of the ACK
// RX_WIN_TYPE
// START_SSN field - transmitter initializes the ACK_RX_WINDOW_START with its ACK_TX_WINDOW_START
/*
 * unit [ms]
 */
#define CNM_TIMEOUT 200

/*
 * ***************************************************************************/
/*
 * Ghn to calibrate
 * unit [ms]
 */
#if GHN_TS_DURATION > 1000
#define GHN_MIN_ACCESS_TIME GHN_TS_DURATION/1000
#else
#define GHN_MIN_ACCESS_TIME 1
#endif
#define GHN_MAX_ACCESS_TIME GHN_CYCLE_MAX
/*
 * Ghn to calibrate
 * unit [s]
 */
#define TIME_CONNECTION_TO_DIE 1
/*
 * Maximal value of the connection SSN
 */
#define GHN_MAX_CONNECTION_SSN (1 << 16)

/*
 * *************************************************************
 *                    GHN_DLL LAYER PARAMETERS
 * *************************************************************
 */
/*
 * G.9961
 */
#define DEFAULT_PRIORITY 0
#define MIN_PRIOR_CONN_ID DEFAULT_PRIORITY
#define MAX_PRIOR_CONN_ID 7
#define MIN_CONN_ID 8
#define MAX_CONN_ID 250
#define MANAGMENT_CONN_ID 251
#define MULTICAST_CONN_ID 252
#define BROADCAST_CONN_ID 255

#define MANAGE_PHY_FRAME_PRIOR_CONN_ID MAX_PRIOR_CONN_ID

#define MIN_FLOWCONTROL_VALUE 0b00000
#define MAX_FLOWCONTROL_VALUE 0b11111

#define MANAGE_MESS_RESEND_ATTEMPTS 2

#define MAX_IMPORTANCE_VALUE MAX_FLOW_ID

/*
 * The maximum time within which a node shall respond to the received management message
 *
 * unit [ms]
 */
#define MAX_RESP_TIME 100
/*
 * The time that a node shall wait for an expected response after transmitting a management
 * message before inferring the loss of the transmitted message or the response from the
 * responding node or both
 *
 * unit [ms]
 */
#define MAX_WAIT_TIME 200
/*
 * unit [s]
 */
#define DEFAULT_CONN_LIFE_DURATION GHN_CYCLE_MAX
/*
 * *************************************************************
 *                    GHN_LLC LAYER PARAMETERS
 * *************************************************************
 */

#define GHN_LPDU_HEADER_SIZE 4
#define MAX_LCDU_SIZE 1500
#define MIN_MM_SIZE 8
#define MIN_CM_SIZE 2
#define LCDU_HEADER_SIZE 14
#define MAX_LCC_SEQ_NUM ((1 << 16) - 1)

#define GHN_MAX_LPDU_TTL 63
/*
 * assumption on the real buffer on the GhnLlcLayer layer
 * unit [number of packet]
 */
#define GHN_LLC_QUEUE_SIZE 20000
/*
 * uint [bytes]
 */
#define MIN_SEGMENTING_LCDU_SIZE 1500

#define MANAGEMENT_PROTOCOL_NUMBER 0x22E3
/*
 * unit [bytes]
 */
#define FCS_SIZE 4
/*
 * *************************************************************
 *                    CONV LAYER PARAMETERS
 * *************************************************************
 */
/*
 * assumption on the real buffer on the convergence layer
 * unit [number of packet]
 */
#define GHN_CONV_QUEUE_SIZE 10000
/*
 * *************************************************************
 *                    Ghn_PHY LAYER PARAMETERS
 * *************************************************************
 */
#define PERMANENTLY_MASKED_SUBCARRIERS 75
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////         MUSOR       /////////////////////////////////////////////////////////
#define MAX_Ghn_PHY_SPEED 1500
/*
 * OPERA Specification 1
 * see Figure 4 PPDU for details
 * time unit [us]
 */
#define SOT_DURATION 40
/*
 * OPERA Specification 1
 * see Figure 4 PPDU for details
 * time unit [us]
 */
#define SOT_TO_SYNCH_SYMBOL_GAP 2.75
/*
 * OPERA Specification 1
 * During this period the actual data is transmitted
 * time unit [us]
 */
#define IDFT_INTERVAL 51.2
/*
 * OPERA Specification 1
 * time unit [us]
 */
#define CYCLIC_PREFIX 20
/*
 * OPERA Specification 1
 * time unit [us]
 */
#define SYMBOL_INTERVAL 71.2
/*
 * OPERA Specification 1
 * time unit [us]
 */
#define TXRX_SWITCH_TIME 356
/*
 * OPERA Specification 1 [Table 37]
 * unit [bytes]
 */
#define AVE_FEC_PAYLOAD 192
/*
 * OPERA Specification 1
 * unit [bits]
 */
#define MIN_BIT_PER_SYMBOL 24
/*
 * OPERA Specification 1
 * unit [bits]
 */
#define MAX_BIT_PER_SYMBOL 14592
/*
 * Limit SNR
 * When the node receives signal with SNR less then SNR_LIMIT
 * the Ghn_PHY will drop this packet
 * Parameter range: [0, 1]
 */
#define SNR_LIMIT 1
/*
 * Period of Ghn_PHY tables update
 * The period is updated each Ghn_PHY_GHN_UPDATE_PERIOD but not earlier than the new
 * GhnMacLayer cycle will start
 * unit [s]
 */
#define Ghn_PHY_GHN_UPDATE_PERIOD 60
/*
 * Port value that corresponds to the absence of port connection
 */
#define GHN_OFF_PORT 33333
/*
 * G.9961. Reduction from 06.2010
 * Table 8-48
 * The nodes may report on the possible datarate of PtP connection with the MIN_GHN_PHY_SPEED granularity
 *
 * unit [Mbps]
 */
#define MIN_GHN_PHY_SPEED 0.5
/////////////////////////////////////////////////////////////////////////////////////////////////
/*
 * TEMPORARY VARIABLES
 */
#define GHN_BLKSZ_CONTROL 120
#define GHN_BLKSZ_DATA 540
#define GHN_FEC_CODING_RATE 0.5

/*
 * 32 bit - 4 bytes
 */
#define GHN_CRC_SIZE 4
/*
 * G.9961
 * unit [us]
 */
#define GHN_TX_ON 1
/*
 * Taken from the ceiling
 * unit [us]
 */
#define BUSY_DELAY 50
/*
 * correction of the PHY layer estimation of the amount of data
 * which can be sent withing the given time
 * INACCURACY_CORRECTION says that the maximal expected error
 * in the PHY layer feedback is (1 - INACCURACY_CORRECTION) / 2 * 100%
 */
#define INACCURACY_CORRECTION 0.9

//
// Differentiated Services Field
// unit [bits]
//
#define DSP_SIZE        6

#define SSN_UNDEF       (1 << 13)

enum SegmentState
{
  NOT_SENT_SEGMENT_STATE, // receiver and transmitter
  WAIT_ACK_SEGMENT_STATE, // only for transmitter
  WAIT_RETRANSMISSION_SEG_STATE, //only for receiver
  DISCARDED_SEGMENT_STATE, // receiver and transmitter
  DONE_SEGMENT_STATE
// receiver and transmitter
};
enum AckCompressType
{
  NO_ACK_COMPRESS, GROUP_ACK_COMPRESS
};

#define DELETE_PTR( ptr )                       \
    do {                                          \
    if ( ptr != NULL ) {                        \
    delete ptr;                               \
    ptr = NULL;                               \
    }                                           \
    } while( false )

#define DELETE_ARRAY( ptr )                     \
    do {                                          \
    if ( ptr != NULL ) {                        \
    delete [] ptr;                            \
    ptr = NULL;                               \
    }                                           \
    } while( false )

enum ArqType
{
  GHN_ARQ_TYPE, RLNC_ARQ_TYPE, ON_THE_FLY_RLNC_ARQ_TYPE, SLIDED_WINDOW_RLNC_ARQ_TYPE
};

typedef std::deque<Ptr<Packet> > GhnBuffer;

//
// unit [bytes]
//
#define GHN_CRC_LENGTH	4

#define MAX_LLC_QUEUE_LENGTH	100000
struct GhnSeg
{
  Ptr<Packet> pkt;

  uint16_t posLlcFrame;
  bool validSeg;
  Ssn ssn;
};
typedef std::deque<GhnSeg> SegGhnBuffer;

#define NO_PARTIAL_DESEGEMENTED (1<<15)

enum FlowOriginationType
{
  OWN_FLOW_ORIGINATION_TYPE = 0, REPEAT_FLOW_ORIGINATION_TYPE = 1
};

struct ConnIdentifier
{
  uint16_t sid;
  uint16_t did;
  uint16_t flowId;
  FlowOriginationType origType;
};

#define UNDEF_FLOW_INDEX   -1
#define UNDEF_GROUP_INDEX   -1

#include <ns3/packet.h>
#include <ns3/ptr.h>
#include <ns3/callback.h>

struct TestParameters
{
  Ssn winConfSize;
  double per;
  uint64_t llcBufferLength;
  Callback<void, Ptr<Packet>, ConnIdentifier> rcvX1RefPointReport;
  Callback<void, Ptr<Packet>, ConnIdentifier> inIndexedTrace;
  Callback<void, Ptr<Packet>, ConnIdentifier> outIndexedTrace;
};

#define WITH_NC 1

struct NcAckInfoItem
{
  uint16_t groupId;
  uint16_t rcv; // number of correctly received packets
  uint16_t use; // number of useful correctly received packets
};
typedef std::deque<NcAckInfoItem> NcAckInfo;

struct GroupEncAckInfo
{
  GroupEncAckInfo ()
  {
    winStart = 0;
    groupSize = 1;
    numRcvSym = 0;
    invalid = false;
    details.clear ();
    ncAckInfo.clear ();
    brrFeedback.clear ();
  }
  GroupEncAckInfo (bool inv)
  {
    invalid = !inv;
    winStart = 0;
    groupSize = 1;
    numRcvSym = 0;
    details.clear ();
    ncAckInfo.clear ();
    brrFeedback.clear ();
  }
  GroupEncAckInfo (const GroupEncAckInfo &arg)
  {
    details.clear ();
    details.insert (details.begin (), arg.details.begin (), arg.details.end ());
    winStart = arg.winStart;
    groupSize = arg.groupSize;
    numRcvSym = arg.numRcvSym;
    ncAckInfo.clear ();
    ncAckInfo.insert (ncAckInfo.begin (), arg.ncAckInfo.begin (), arg.ncAckInfo.end ());
    brrFeedback.clear ();
    brrFeedback.insert (brrFeedback.begin (), arg.brrFeedback.begin (), arg.brrFeedback.end ());
    invalid = false;
  }
  GroupEncAckInfo&
  operator= (GroupEncAckInfo arg)
  {
    details.clear ();
    details.insert (details.begin (), arg.details.begin (), arg.details.end ());
    winStart = arg.winStart;
    groupSize = arg.groupSize;
    numRcvSym = arg.numRcvSym;
    ncAckInfo.clear ();
    ncAckInfo.insert (ncAckInfo.begin (), arg.ncAckInfo.begin (), arg.ncAckInfo.end ());
    brrFeedback.clear ();
    brrFeedback.insert (brrFeedback.begin (), arg.brrFeedback.begin (), arg.brrFeedback.end ());
    return *this;
  }

  friend std::ostream&
  operator<< (std::ostream& o, GroupEncAckInfo& m)
  {
    o << "[" << m.winStart << " / " << m.groupSize << " / " << m.numRcvSym << " / ";
    for (auto a : m.details)
      o << (uint16_t)a << " ";
    o << " / " << m.brrFeedback.size();
    o << "]";
    return o;
  }

  bool invalid;

  std::deque<bool> details;
  VirtSsn winStart;
  uint16_t groupSize;
  VirtSsn numRcvSym;

  GhnBuffer brrFeedback;
  NcAckInfo ncAckInfo;
};

typedef Callback<void, Callback<void, GhnBuffer, ConnIdentifier>, Callback<void, GhnBuffer, ConnIdentifier>,
        Callback<void, GroupEncAckInfo, ConnIdentifier>, Callback<void, GroupEncAckInfo, ConnIdentifier> > ExternArqOutputs;
typedef Callback<void, Callback<void, GhnBuffer, ConnIdentifier>, Callback<void, GroupEncAckInfo, ConnIdentifier>,
        Callback<void, GhnBuffer, ConnIdentifier>, Callback<void, GroupEncAckInfo, ConnIdentifier>, Callback<void, uint64_t> > ExternArqInputs;

#define LSS_N_MAX       15
#define LSS_MAX         (1 << LSS_N_MAX)

struct LlcLogs
{
  uint64_t totalRcv;
  uint64_t totalSnd;
  uint64_t totalRcvSegs;
  uint64_t totalSndSegs;
};
//
//#define WITH_NC 1
//
//typedef std::vector<bool> SeenMap;
//struct Group
//{
//  uint16_t groupId;
//  uint16_t rank;
//  uint16_t size;
//  SeenMap seen;
//};
//typedef std::vector<Group> Groups;
//struct CoopNodeDesc
//{
//  CoopNodeDesc(UanAddress addr, double priority, double e, double l)
//  {
//    this->addr = addr;
//    this->priority = priority;
//    this->e = e;
//    this->l = l;
//  }
//  UanAddress addr;
//  double priority;
//  double e;
//  double l;
//};
//typedef std::vector<CoopNodeDesc> CoalitionDescr;
//struct NcBckwrdFeedback
//{
//  NcBckwrdFeedback&
//  operator= (NcBckwrdFeedback arg)
//  {
//    myAddr = arg.myAddr;
//    myPriority = arg.myPriority;
//    myCoalition.swap (arg.myCoalition);
//    groups.swap (arg.groups);
//    return *this;
//  }
//
//  UanAddress myAddr;
//  double myPriority;
//  CoalitionDescr myCoalition;
//  Groups groups;
//};
//
//struct GroupEncAckInfo
//{
//  GroupEncAckInfo ()
//  {
//  }
//  GroupEncAckInfo (bool inv)
//  {
//    invalid = !inv;
//  }
//  GroupEncAckInfo&
//  operator= (GroupEncAckInfo arg)
//  {
//    details.swap (arg.details);
//    winStart = arg.winStart;
//    groupSize = arg.groupSize;
//    numRcvSym = arg.numRcvSym;
//    ncAckInfo.swap (arg.ncAckInfo);
//    return *this;
//  }
//
//  bool invalid;
//
//  std::deque<bool> details;
//  VirtSsn winStart;
//  uint16_t groupSize;
//  VirtSsn numRcvSym;
//
//  NcBckwrdFeedback ncBckwrdFeedback;
//};
//
//typedef Callback<void, Callback<void, GhnBuffer, ConnIdentifier> , Callback<void, GhnBuffer, ConnIdentifier> , Callback<void,
//        GroupEncAckInfo, ConnIdentifier> , Callback<void, GroupEncAckInfo, ConnIdentifier> > ExternArqOutputs;
//typedef Callback<void, Callback<void, GhnBuffer, ConnIdentifier> , Callback<void, GroupEncAckInfo, ConnIdentifier> , Callback<
//        void, GhnBuffer, ConnIdentifier> , Callback<void, GroupEncAckInfo, ConnIdentifier> , Callback<void, uint64_t> >
//        ExternArqInputs;
//
//#define LSS_N_MAX       12
//
//struct LlcLogs
//{
//  uint64_t totalRcv;
//  uint64_t totalSnd;
//  uint64_t totalRcvSegs;
//  uint64_t totalSndSegs;
//};

enum GhnPlcCsmaNodeState
{
  READY = 0, BACKOFF = 1, CCA = 2, TX = 3, RX = 4, SEND_ACK = 5, WAIT_ACK = 6, GAP = 7
};
}
}
#endif /* GHN_PLC_HEADER_H_ */
