/*
 * GhnPlcBitLoading.h
 *
 *  Created on: Jun 13, 2016
 *      Author: tsokalo
 */

#ifndef GHN_PLC_BITLOADING_H_
#define GHN_PLC_BITLOADING_H_

#include <memory>
#include <vector>
#include <iostream>
#include <map>

#include <ns3/type-id.h>
#include <ns3/callback.h>
#include "ns3/ptr.h"
#include "ns3/object.h"
#include <ns3/spectrum-value.h>
#include "ns3/nstime.h"
#include "ns3/traced-callback.h"
#include "ns3/file-aggregator.h"

#include "ns3/plc-defs.h"
#include "ns3/plc-node.h"
#include "ns3/plc-phy.h"
#include "ns3/plc-channel.h"
#include "ns3/plc-link-performance-model.h"

#include "ns3/ghn-plc-water-filling.h"

namespace ns3 {
namespace ghn {

// <carrier index> <number of bits corresponding to certain modulation minus 1>
typedef std::vector<std::vector<double> > ser_map_t;
typedef std::vector<std::vector<double> > cap_map_t;
// <src id> <dst id>
typedef std::vector<std::vector<Ptr<SpectrumValue> > > SpectrumTable;

class GhnPlcBitLoading: public Object {
public:

	static TypeId
	GetTypeId(void);

	GhnPlcBitLoading();
	virtual
	~GhnPlcBitLoading();

	void SetTxEnvelope(Ptr<SpectrumValue> txEnvelope) {
		m_txEnvelope = txEnvelope;
	}
	void SetNoiseEnvelope(Ptr<const SpectrumValue> noiseEnvelope) {
		m_noiseEnvelope = noiseEnvelope;
	}
	void SetChannel(Ptr<PLC_Channel> channel) {
		this->m_channel = channel;
	}
	virtual void
	CalcBitLoadingTable() = 0;
	void
	AddNode(Ptr<PLC_Node> node);

	ModulationAndCodingScheme GetModulationAndCodingScheme(uint32_t src_id, uint32_t dst_id);
	Ptr<SpectrumValue> GetTxPsd(uint32_t src_id, uint32_t dst_id);
	double GetNumEffBits(uint32_t src_id, uint32_t dst_id);
	void PrintBitLoadingTable();

	uint32_t GetDataAmount(Time txTime, uint8_t src_id, uint8_t dst_id);
	void SetResDirectory(std::string resDir) {
		m_resDir = resDir;
	}

protected:

	static PLC_InformationRateModel::McsInfo s_mcs_info[13];

	virtual void CalcModulationAndCodingScheme() = 0;

	double GetNumEffBits(ModulationAndCodingScheme mcs, Ptr<SpectrumValue> sinr);
	virtual double GetNumEffBits(ModulationAndCodingScheme mcs, SpectrumValue sinr) = 0;

	void CreateLogger();
	void PrintCapacity();
	double ModToDouble(CodingType cr);

	Ptr<PLC_ChannelTransferImpl> GetChannalTransferImpl(uint16_t src_id, uint16_t dst_id);

	Ptr<SpectrumValue> m_txEnvelope;
	Ptr<const SpectrumValue> m_noiseEnvelope;
	Ptr<PLC_Channel> m_channel;
	std::vector<Ptr<PLC_Node> > m_nodes;

	// <index of source node> <index of destination node>
	SpectrumTable m_blts;
	SpectrumTable m_sinr;
	std::vector<std::vector<ModulationAndCodingScheme> > m_mcs;
	std::vector<std::vector<double> > m_capacityPerSymbol;
	std::map<uint16_t, std::map<uint16_t, double> > m_bitsPerSymbol;

	Ptr<FileAggregator> m_aggr;
	//
	// <from ID> <to ID> <capacity in bits/second>
	//
	TracedCallback<double, double, double> m_capacityTrace;

	std::string m_resDir;
};
/*
 * NcBlVarTxPsd calculates the TX PSD using Water filling algorithm; the calculation is done
 * for each region with constant PSD envelope separately; bit loading algorithm uses the same
 * number of bits per sub-carrier
 */
class NcBlVarTxPsd: public GhnPlcBitLoading {
public:

	static TypeId GetTypeId(void);

	NcBlVarTxPsd();
	virtual ~NcBlVarTxPsd();

	void CalcBitLoadingTable();

private:

	void CalcModulationAndCodingScheme();
	double GetNumEffBits(ModulationAndCodingScheme mcs, SpectrumValue sinr);

    std::shared_ptr<GhnPlcWaterFiller> m_waterFiller;
};
/*
 * NcBlVarBatMap uses the same TX PSD for each connection - equal to the TX PSD envelope, i.e. maximum
 * transmission power; bit loading algorithm uses [1] for calculation of bits per each sub-carrier;
 * tolerable BER is taken as a parameter
 * [1] @masterthesis{thesisBar,
 * author={Roee Bar},
 * school={The University of British Columbia}
 * title={In-vehicle Powerline Communication Using Software-Defined Radio},
 * month={August},
 * year={2016}
 * }
 */
class NcBlVarBatMap: public GhnPlcBitLoading {
public:

	static TypeId GetTypeId(void);

	NcBlVarBatMap();
	virtual ~NcBlVarBatMap();

	void SetPer(uint16_t src_id, double per);
	void CalcBitLoadingTable();
	BitAllocationTable CalculateBat(double p, SpectrumValue sinr);
	ser_map_t CalculateSerMap(SpectrumValue sinr);
	cap_map_t CalculateCapMap (SpectrumValue sinr);

        uint32_t CalcBitsPerSymbol(BitAllocationTable bat);
        double GetOfdmSymbolCapacity(BitAllocationTable bat, SpectrumValue sinr);

private:

	void CalcModulationAndCodingScheme();
	double GetNumEffBits(ModulationAndCodingScheme mcs, SpectrumValue sinr);
	double GetOfdmSymbolCapacity(ModulationAndCodingScheme mcs, SpectrumValue sinr);
	// functions in plc-dcmc-capacity should give the same results
	double CalcSer(ModulationType m, double sinr);

	std::map<uint16_t, double> m_desiredPer;
};
}
}
#endif /* GHN_PLC_BITLOADING_H_ */
