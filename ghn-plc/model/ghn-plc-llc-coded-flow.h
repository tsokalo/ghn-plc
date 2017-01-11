/*
 * GhnPlcLlcCodedFlow.h
 *
 *  Created on: Jul 26, 2016
 *      Author: tsokalo
 */

#ifndef GHN_PLC_LLCCODEDFLOW_H_
#define GHN_PLC_LLCCODEDFLOW_H_

#include <memory>

#include "ghn-plc-llc-flow.h"

#include "header.h"
#include "nc-routing-rules.h"
#include "coder.h"
#include "brr-tx-plan.h"
#include "brr-pkt-header.h"
#include "brr-feedback.h"
#include "brr-netdiscovery.h"

namespace ns3 {
namespace ghn {
class GhnPlcLlcCodedFlow: public GhnPlcLlcFlow {
	typedef std::shared_ptr<ncr::NcRoutingRules> routing_rules_ptr;
	typedef std::shared_ptr<ncr::encoder_queue> encoder_queue_ptr;
	typedef std::shared_ptr<ncr::decoder_queue> decoder_queue_ptr;

public:
	static TypeId
	GetTypeId(void);
	GhnPlcLlcCodedFlow();
	virtual
	~GhnPlcLlcCodedFlow();

	void Configure(ncr::NodeType type, ncr::UanAddress dst);

	void NotifyRcvUp(ncr::GenId genId);

private:

	void PrepareForSend(uint64_t dataAmount);
	Ptr<Packet> ConvertBrrHeaderToPkt(ncr::TxPlan txPlan);
	ncr::HeaderInfo ConvertPktToBrrHeader(Ptr<Packet> pkt);
	void ProcessDecoded(GhnBuffer buffer);
	bool HaveFeedback();
	GhnBuffer ConvertFeedbackToBuffer(ncr::FeedbackInfo f);
	ncr::FeedbackInfo ConvertBufferToFeedback(GhnBuffer b);
	Ptr<Packet> ConvertVecToPacket(std::vector<uint8_t> vec);
	std::vector<uint8_t> ConvertPacketToVec(Ptr<Packet> pkt);
	void ProcessRcvdPacket(Ptr<Packet> pkt, bool crc, ncr::UanAddress addr, std::map<ncr::GenId,ncr::TxPlanItem>::iterator item);

	ncr::UanAddress m_id;
	ncr::NodeType m_nodeType;

	routing_rules_ptr m_brr;

	encoder_queue_ptr m_encQueue;
	decoder_queue_ptr m_decQueue;

	uint16_t m_numGen;
	uint32_t m_genSize;
	uint32_t m_symbolSize;
	ncr::Datarate m_sendRate;

	ncr::get_rank_func m_getRank;

	ncr::symb_ssn_t m_ssn;
	std::map<MessType, GhnBuffer> m_feedback;
};
}
}
#endif /* GHN_PLC_LLCCODEDFLOW_H_ */
