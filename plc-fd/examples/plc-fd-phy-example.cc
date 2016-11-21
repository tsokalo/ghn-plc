/*
 * csma-cd-example.cc
 *
 *  Created on: 07.09.2016
 *      Author: tsokalo
 */

/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 University of British Columbia, Vancouver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Alexander Schloegl <alexander.schloegl@gmx.de>
 */

//CXXFLAGS="-std=c++0x" ./waf configure --disable-python --with-nsc /home/tsokalo/workspace/ns3sims/ns-3-dev/nsc
//export 'NS_LOG=PLC_FullDuplexOfdmPhy=level_logic|prefix_time|prefix_node|prefix_func:PLC_InfRateFDPhy=level_logic|prefix_time|prefix_node|prefix_func:PLC_Interference=level_logic|prefix_time|prefix_node|prefix_func:PLC_Channel=level_logic|prefix_time|prefix_node|prefix_func'

#include <iostream>
#include <sstream>
#include <fstream>
#include <time.h>

#include <ns3/core-module.h>
#include <ns3/nstime.h>
#include <ns3/simulator.h>
#include <ns3/output-stream-wrapper.h>
#include "ns3/plc-module.h"
#include "ns3/csma-module.h"
#include "ns3/plc-fd-module.h"

using namespace ns3;

void
ReceiveSuccess (Ptr<const Packet> p, uint16_t msgId)
{
  NS_LOG_UNCOND(Simulator::Now() << ": Packet received!");
}
void
CollisionDetection ()
{
  NS_LOG_UNCOND(Simulator::Now() << ": Collision detection!");
}
enum PhyTestCase
{
  NORMAL_MODE_TEST_CASE, COLLISION_TEST_CASE, RECOVERY_AFTER_COLLISION_TEST_CASE, MULTIPLE_COLLISIONS_TEST_CASE
};
int
main (int argc, char *argv[])
{
//  LogComponentEnable ("PLC_FullDuplexOfdmPhy", LOG_LOGIC);
//  LogComponentEnable ("PLC_InfRateFDPhy", LOG_LOGIC);
//  LogComponentEnable ("PLC_Interference", LOG_LOGIC);
//  LogComponentEnable ("PLC_Channel", LOG_LOGIC);

  // Define spectrum model
  PLC_SpectrumModelHelper smHelper;
  Ptr<const SpectrumModel> sm;
  sm = smHelper.GetSpectrumModel (0, 10e6, 100);

  // Define transmit power spectral density
  Ptr<SpectrumValue> txPsd = Create<SpectrumValue> (sm);
  (*txPsd) = 1e-8; // -50dBm/Hz

  // Create cable types
  Ptr<PLC_Cable> cable = CreateObject<PLC_NAYY150SE_Cable> (sm);
  Ptr<SpectrumValue> noiseFloor = CreateWorstCaseBgNoise (sm)->GetNoisePsd ();
  uint16_t numNodes = 3;
  PLC_NodeList nodes;
  std::vector<Ptr<PLC_Outlet> > outlets;
  std::vector<Ptr<PLC_InfRateFDPhy> > phys;
  PLC_ChannelHelper channelHelper (sm);

  // Setup nodes
  for (uint16_t i = 0; i < numNodes; i++)
    {
      // Create nodes
      auto node = CreateObject<PLC_Node> ();
      nodes.push_back (node);
      node->SetName (std::string ("Node ") + std::to_string (i + 1));
      // Connect nodes
      node->SetPosition (500 * i, 0, 0);
      if (i > 0)
        {
          std::cout << "Connecting " << (*(nodes.begin () + i - 1))->GetName () << " with " << node->GetName () << std::endl;
          CreateObject<PLC_Line> (cable, *(nodes.begin () + i - 1), node);
        }
      channelHelper.Install (node);

      // Create outlets
      auto outlet = CreateObject<PLC_Outlet> (node);
      outlets.push_back (outlet);

      // Create PHYs
      auto phy = CreateObject<PLC_InfRateFDPhy> ();
      phys.push_back (phy);
      phy->CreateInterfaces (outlet, txPsd);
      phy->SetNoiseFloor (noiseFloor);
      phy->SetHeaderModulationAndCodingScheme (ModulationAndCodingScheme (BPSK, CODING_RATE_1_2, 0));
      phy->SetPayloadModulationAndCodingScheme (ModulationAndCodingScheme (QAM4, CODING_RATE_2_3, 0));
      phy->GetRxInterface ()->AggregateObject (CreateObject<Node> ());
      phy->SetReceiveSuccessCallback (MakeCallback (&ReceiveSuccess));
      phy->SetCollisionDetection (MakeCallback (&CollisionDetection));
    }

  Ptr<PLC_Channel> channel = channelHelper.GetChannel ();

  // Since we do not run an ns-3 simulation, the channel computation has to be triggered manually
  channel->InitTransmissionChannels ();
  channel->CalcTransmissionChannels ();

  // Create packet to send
  Ptr<Packet> p = Create<Packet> (1024);

  PhyTestCase t = MULTIPLE_COLLISIONS_TEST_CASE;

  switch (t)
    {
  case NORMAL_MODE_TEST_CASE:
    {
      //
      // the packet should be successfully delivered
      //
      Simulator::Schedule (Seconds (1), &PLC_Phy::StartTx, phys.at (0), p);
      break;
    }
  case COLLISION_TEST_CASE:
    {
      //
      // collision should be recognized by both nodes after the duration
      // equal to the preamble duration
      //
      Simulator::Schedule (Seconds (1), &PLC_Phy::StartTx, phys.at (0), p);
      Simulator::Schedule (Seconds (1), &PLC_Phy::StartTx, phys.at (1), p);
      break;
    }
  case RECOVERY_AFTER_COLLISION_TEST_CASE:
    {
      //
      // similar to previous scenario the collision should be detected and
      // the nodes should already recover after the duration equal to the
      // preamble duration
      //
      Simulator::Schedule (Seconds (1), &PLC_Phy::StartTx, phys.at (0), p);
      Simulator::Schedule (Seconds (1), &PLC_Phy::StartTx, phys.at (1), p);
      Simulator::Schedule (Seconds (1) + 2 * PLC_Preamble::GetDuration (), &PLC_Phy::StartTx, phys.at (2), p);
      break;
    }
  case MULTIPLE_COLLISIONS_TEST_CASE:
    {
      //
      // similar to previous scenario but repeat it many times
      //
      uint16_t numRepeat = 0;
      while (numRepeat++ != 100)
        {
          Simulator::Schedule (Seconds (1) + numRepeat * Seconds(1), &PLC_Phy::StartTx, phys.at (0), p);
          Simulator::Schedule (Seconds (1) + numRepeat * Seconds(1), &PLC_Phy::StartTx, phys.at (1), p);
          Simulator::Schedule (Seconds (1) + numRepeat * Seconds(1) + 2 * PLC_Preamble::GetDuration (),
                  &PLC_Phy::StartTx, phys.at (2), p);
        }
      break;
    }
    }

  // Start simulation
  Simulator::Run ();

  // Cleanup simulation
  Simulator::Destroy ();

  return EXIT_SUCCESS;
}
