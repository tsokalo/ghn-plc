/*
 * GhnPlcLlcCodedFlow.h
 *
 *  Created on: Jul 26, 2016
 *      Author: tsokalo
 */

#ifndef GHN_PLC_LLCCODEDFLOW_H_
#define GHN_PLC_LLCCODEDFLOW_H_

#include <memory>
#include <functional>

#include "ghn-plc-llc-flow.h"
#include "ghn-plc-utilities.h"
#include "ghn-plc-data-rate-calculator.h"

#include "header.h"
#include "nc-routing-rules.h"
#include "coder.h"
#include "brr-tx-plan.h"
#include "brr-pkt-header.h"
#include "brr-feedback.h"
#include "brr-netdiscovery.h"
#include "sim-parameters.h"
#include "brr-header.h"
#include "ordering-queue.h"

namespace ns3
{
namespace ghn
{
class GhnPlcLlcCodedFlow : public GhnPlcLlcFlow
{
  typedef std::shared_ptr<ncr::NcRoutingRules> routing_rules_ptr;
  typedef std::shared_ptr<ncr::encoder_queue> encoder_queue_ptr;
  typedef std::shared_ptr<ncr::decoder_queue> decoder_queue_ptr;
  typedef ncr::OrderingQueue<MAX_SYM_SSN> ordering_queue;
  typedef std::shared_ptr<ordering_queue> ordering_queue_ptr;

  typedef std::function<void
  (ncr::LogItem item, ncr::UanAddress node_id)> add_log_func;

public:
  static TypeId
  GetTypeId (void);
  GhnPlcLlcCodedFlow ();
  virtual
  ~GhnPlcLlcCodedFlow ();

  void
  Configure (ncr::NodeType type, ncr::UanAddress dst, ncr::SimParameters sp);
  void
  NotifyRcvUp (ncr::GenId genId);
  SendTuple
  SendDown ();
  GroupEncAckInfo
  Receive (GhnBuffer buffer, ConnId connId);
  void
  ReceiveAck (GroupEncAckInfo info, ConnId connId);
  bool
  IsQueueEmpty ();
  void
  SetLogCallback (add_log_func addLog);
  void
  SetNextHopVertex(UanAddress addr);

private:

  void
  PrepareForSend ();
  void
  ProcessDecoded (GhnBuffer buffer, ConnId connId);

  GhnBuffer
  ConvertBrrHeaderToPkt (ncr::TxPlan txPlan);
  ncr::BrrHeader
  ConvertPktToBrrHeader (GhnBuffer &buffer, std::deque<SegmentState> &state);

  void
  ProcessRcvdPacket (std::vector<uint8_t> pkt, bool crc, ncr::UanAddress addr, ncr::TxPlan::iterator item, ConnId connId);

  void
  ProcessFeedback (ncr::FeedbackInfo f);
  void
  ProcessNetDiscovery (ncr::FeedbackInfo f);
  void
  ProcessRetransRequest (ncr::FeedbackInfo f);
  void
  UpdateFeedback();
  void
  GenerateGreedy();

  routing_rules_ptr m_brr;
  encoder_queue_ptr m_encQueue;
  decoder_queue_ptr m_decQueue;
  ordering_queue_ptr m_oQueue;

  ncr::UanAddress m_id;
  ncr::NodeType m_nodeType;
  ncr::SimParameters m_sp;
  ncr::symb_ssn_t m_ssn;
  ncr::FeedbackInfo m_feedback;
  DataRateCalculator m_drCalc;

  ncr::get_rank_func m_getRank;
  add_log_func m_addLog;

  std::deque<Ssn> m_notDesegm;
};
}
}
#endif /* GHN_PLC_LLCCODEDFLOW_H_ */
