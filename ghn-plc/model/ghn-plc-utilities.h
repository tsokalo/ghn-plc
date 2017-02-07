/*
 * nc-utiliites.h
 *
 *  Created on: May 27, 2016
 *      Author: tsokalo
 */

#ifndef GHN_PLC_UTILIITES_H_
#define GHN_PLC_UTILIITES_H_

#include <string.h>

#include "ns3/ptr.h"
#include "ns3/plc-graph.h"
#include "ns3/packet.h"

#include "ghn-plc-defs.h"
#include "ghn-plc-header.h"
#include "ghn-plc-phy-header.h"


namespace ns3
{
namespace ghn {

typedef std::vector<std::string> FileList;

void
CreateLineTopology (PLC_NodeList &node_list, Ptr<PLC_Cable> cable, Ptr<const SpectrumModel> sm, std::vector<uint32_t> distance);
void
CreateStarTopology (PLC_NodeList &node_list, Ptr<PLC_Cable> cable, Ptr<const SpectrumModel> sm, std::vector<uint32_t> distance);

Ptr<Packet>
GroupEncAckInfoToPkt (GroupEncAckInfo info);
GroupEncAckInfo
PktToGroupEncAckInfo (Ptr<Packet> packet);
void
PrintGroupEncAckInfo (GroupEncAckInfo info);

FecRateType
ConvertPlcRateToGhnRate (CodingType rate);
CodingType
ConvertGhnRateToPlcRate (FecRateType rate);

void
GetDirListing (FileList& result, const std::string& dirpath);
int16_t
CreateDirectory (std::string path);
bool
RemoveDirectory (std::string folderPath);

std::string GetLogIdent(std::vector<double> vals);

template<typename T1, typename T2>
  T1
  CopyBits (int start_data, int end_data, T2 data, int start_target, T1 target);

uint8_t
CopyBits (int start_data, int end_data, uint64_t data, int start_target, uint8_t target);
uint64_t
CopyBits (int start_data, int end_data, uint8_t data, int start_target, uint64_t target);
//uint8_t
//CopyBits (int start_data, int end_data, uint32_t data, int start_target, uint8_t target);
//uint32_t
//CopyBits (int start_data, int end_data, uint8_t data, int start_target, uint32_t target);

uint8_t
CopyBits (int start_data, int end_data, uint8_t data, int start_target, uint8_t target);
uint8_t
CopyBits (int start_data, int end_data, unsigned int data, int start_target, uint8_t target);
unsigned int
CopyBits (int start_data, int end_data, uint8_t data, int start_target, unsigned int target);

VirtSsn
RotateVarFwrd (VirtSsn toRotate, VirtSsn howFar, VirtSsn cycleSize);
VirtSsn
RotateVarBck (VirtSsn toRotate, VirtSsn howFar, VirtSsn cycleSize);

Ptr<Packet> ConvertVecToPacket(std::vector<uint8_t> vec);
std::vector<uint8_t> ConvertPacketToVec(Ptr<Packet> pkt);
GhnBuffer ConvertVecsToBuffer(std::vector<std::vector<uint8_t> > vec);
}
}
#endif /* GHN_PLC_UTILIITES_H_ */
