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

#include "header.h"
#include "nc-routing-rules.h"
#include "coder.h"
#include "brr-tx-plan.h"
#include "brr-pkt-header.h"
#include "brr-feedback.h"
#include "brr-netdiscovery.h"
#include "sim-parameters.h"
#include "brr-header.h"

namespace ns3 {
namespace ghn {
class GhnPlcLlcCodedFlow: public GhnPlcLlcFlow {
	typedef std::shared_ptr<ncr::NcRoutingRules> routing_rules_ptr;
	typedef std::shared_ptr<ncr::encoder_queue> encoder_queue_ptr;
	typedef std::shared_ptr<ncr::decoder_queue> decoder_queue_ptr;
	typedef Callback<void,uint16_t> GenCallback;

public:
	static TypeId
	GetTypeId(void);
	GhnPlcLlcCodedFlow();
	virtual
	~GhnPlcLlcCodedFlow();

	void Configure(ncr::NodeType type, ncr::UanAddress dst, ncr::SimParameters sp, GenCallback cb);
	void NotifyRcvUp(ncr::GenId genId);
	SendTuple SendDown ();
	GroupEncAckInfo Receive (GhnBuffer buffer, ConnId connId);
	void ReceiveAck (GroupEncAckInfo info, ConnId connId);
	bool IsQueueEmpty ();

private:

	void PrepareForSend(uint64_t dataAmount);
	void ProcessDecoded(GhnBuffer buffer, ConnId connId);

	Ptr<Packet> ConvertBrrHeaderToPkt(ncr::TxPlan txPlan);
	ncr::BrrHeader ConvertPktToBrrHeader(Ptr<Packet> pkt);

	void ProcessRcvdPacket(std::vector<uint8_t> pkt, bool crc, ncr::UanAddress addr, ncr::TxPlan::iterator item, ConnId connId);

        void ProcessFeedback(ncr::FeedbackInfo f);
        void ProcessNetDiscovery(ncr::FeedbackInfo f);
        void ProcessRetransRequest(ncr::FeedbackInfo f);

	routing_rules_ptr m_brr;
	encoder_queue_ptr m_encQueue;
	decoder_queue_ptr m_decQueue;

        ncr::UanAddress m_id;
        ncr::NodeType m_nodeType;
	ncr::SimParameters m_sp;
	ncr::symb_ssn_t m_ssn;
	ncr::FeedbackInfo m_feedback;

	ncr::get_rank_func m_getRank;
	GenCallback m_genCallback;
};
}
}
#endif /* GHN_PLC_LLCCODEDFLOW_H_ */
