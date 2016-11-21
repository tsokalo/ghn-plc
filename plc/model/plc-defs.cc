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

#include <ns3/abort.h>
#include "plc-defs.h"

namespace ns3 {

PLC_Mutex g_log_mutex;
PLC_Mutex g_smartpointer_mutex;

std::ostream&
operator<<(std::ostream& os, ModulationType type)
{
	switch (type)
	{
	case (NOMOD):
		{
			os << "NOMOD";
			break;
		}
	case (BPSK):
		{
			os << "BPSK";
			break;
		}
	case (QAM4):
		{
			os << "QAM4";
			break;
		}
	case (QAM8):
		{
			os << "QAM8";
			break;
		}
	case (QAM16):
		{
			os << "QAM16";
			break;
		}
	case (QAM32):
		{
			os << "QAM32";
			break;
		}
	case (QAM64):
		{
			os << "QAM64";
			break;
		}
	case (QAM128):
		{
			os << "QAM128";
			break;
		}
	case (QAM256):
		{
			os << "QAM256";
			break;
		}
	case (QAM512):
		{
			os << "QAM512";
			break;
		}
        case (QAM1024):
                {
                        os << "QAM1024";
                        break;
                }
        case (QAM2048):
                {
                        os << "QAM2048";
                        break;
                }
        case (QAM4096):
                {
                        os << "QAM4096";
                        break;
                }
	default:
		{
			os << "INVALID";
			break;
		}
	}
	return os;
}

std::ostream&
operator<<(std::ostream& os, CodingType type)
{
        switch (type)
        {
        case (CODING_RATE_1_4):
                {
                        os << "CODING_RATE_1_4";
                        break;
                }
        case (CODING_RATE_1_2):
                {
                        os << "CODING_RATE_1_2";
                        break;
                }
        case (CODING_RATE_2_3):
                {
                        os << "CODING_RATE_2_3";
                        break;
                }
        case (CODING_RATE_16_21):
                {
                        os << "CODING_RATE_16_21";
                        break;
                }
        case (CODING_RATE_5_6):
                {
                        os << "CODING_RATE_5_6";
                        break;
                }
        case (CODING_RATE_16_18):
                {
                        os << "CODING_RATE_16_18";
                        break;
                }
        case (CODING_RATE_20_21):
                {
                        os << "CODING_RATE_20_21";
                        break;
                }
        case (CODING_RATE_RATELESS):
                {
                        os << "CODING_RATE_RATELESS";
                        break;
                }
        default:
                {
                        os << "INVALID";
                        break;
                }
        }
        return os;
}

std::ostream&
operator<<(std::ostream& os, ModulationAndCodingScheme mcs)
{
	os << "(ModulationType: " << mcs.mt << ", CodingType: "<< mcs.ct << ", Gap: " << mcs.gap2Capacity_dB <<"dB)";
	return os;
}
std::ostream&
operator<< (std::ostream& os, const BitAllocationTable& tm)
{
  for (uint16_t i = 0; i < tm.size (); i++)
    std::cout << "Channel " << i << ":" << "\t" << tm.at (i) << "\n";
  return os;
}
double
GetCodeRate (CodingType ct)
{
	NS_ASSERT_MSG (ct < CODING_RATE_RATELESS, "Rateless codes do not have fixed code rate");

	double code_rate;
	switch (ct)
	{
		case CODING_RATE_1_4:
		{
			code_rate = 0.25;
			break;
		}
		case CODING_RATE_1_2:
		{
			code_rate = 0.5;
			break;
		}
		case CODING_RATE_2_3:
		{
			code_rate = (double) 2/3;
			break;
		}
                case CODING_RATE_16_21:
                {
                        code_rate = (double) 16/21;
                        break;
                }
		case CODING_RATE_5_6:
		{
			code_rate = (double) 5/6;
			break;
		}
                case CODING_RATE_16_18:
                {
                        code_rate = (double) 16/18;
                        break;
                }
		case CODING_RATE_20_21:
		{
			code_rate = (double) 20/21;
			break;
		}

		default:
		{
			NS_ABORT_MSG("Coding type " << ct << " not supported");
			break;
		}
	}

	return code_rate;
}

size_t
GetBitsPerSymbol (ModulationType mt)
{
	size_t bits;
	switch(mt)
	{
		case NOMOD:
	        {
	                bits = 0;
	                break;
	        }
		case BPSK:
		{
			bits = 1;
			break;
		}
		case QAM4:
		{
			bits = 2;
			break;
		}
		case QAM8:
		{
			bits = 3;
			break;
		}
		case QAM16:
		{
			bits = 4;
			break;
		}
		case QAM32:
		{
			bits = 5;
			break;
		}
                case QAM64:
                {
                        bits = 6;
                        break;
                }
                case QAM128:
                {
                        bits = 7;
                        break;
                }
                case QAM256:
                {
                        bits = 8;
                        break;
                }
                case QAM512:
                {
                        bits = 9;
                        break;
                }
                case QAM1024:
                {
                        bits = 10;
                        break;
                }
                case QAM2048:
                {
                        bits = 11;
                        break;
                }
                case QAM4096:
                {
                        bits = 12;
                        break;
                }
		default:
		{
			NS_ABORT_MSG("Unsupported Modulation Type");
			bits = 0;
			break;
		}
	}
	return bits;
}

}
