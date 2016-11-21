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

#include <ns3/fatal-error.h>
#include <math.h>
#include "plc-dcmc-capacity.h"

namespace ns3 {

SpectrumValue GetCapacity(const SpectrumValue& SINR_dB, Modulation mod, short cardinality)
{
	SpectrumValue capacityPerHertz(SINR_dB.GetSpectrumModel());

	const double *cap;
	if (mod == QAM)
	{
		switch (cardinality)
		{
			case 2:
			{
				cap = qamCap[0];
				break;
			}
			case 4:
			{
				cap = qamCap[1];
				break;
			}
			case 8:
			{
				cap = qamCap[2];
				break;
			}
			case 16:
			{
				cap = qamCap[3];
				break;
			}
			case 32:
			{
				cap = qamCap[4];
				break;
			}
			case 64:
			{
				cap = qamCap[5];
				break;
			}
                        case 128:
                        {
                                cap = qamCap[6];
                                break;
                        }
                        case 256:
                        {
                                cap = qamCap[7];
                                break;
                        }
                        case 512:
                        {
                                cap = qamCap[8];
                                break;
                        }
                        case 1024:
                        {
                                cap = qamCap[9];
                                break;
                        }
                        case 2048:
                        {
                                cap = qamCap[10];
                                break;
                        }
                        case 4096:
                        {
                                cap = qamCap[11];
                                break;
                        }
			default:
			{
				NS_FATAL_ERROR("QAM" << cardinality << " not supported");
				break;
			}
		}
	}
	else if (mod == PSK)
	{
		switch (cardinality)
		{
			case 16:
			{
				cap = pskCap[0];
				break;
			}
			case 32:
			{
				cap = pskCap[1];
				break;
			}
			case 64:
			{
				cap = pskCap[2];
				break;
			}
			default:
			{
				NS_FATAL_ERROR("PSK" << cardinality << " not supported");
				break;
			}
		}
	}
	else
	{
		NS_FATAL_ERROR("Unsupported modulation type");
	}

	Values::const_iterator SINR_dB_it = SINR_dB.ConstValuesBegin();
	Values::iterator cit = capacityPerHertz.ValuesBegin();
	while (SINR_dB_it != SINR_dB.ConstValuesEnd() && cit != capacityPerHertz.ValuesEnd())
	{
		// subchannel SINR_dB
		double sc_SINR_dB = *SINR_dB_it;
		if (sc_SINR_dB < -10)
		{
			*cit = log2(1+pow(10, sc_SINR_dB/10));
		}
		else if (sc_SINR_dB >= 40)
		{
			*cit = log2(cardinality);
		}
		else
		{
			short x1 = floor(sc_SINR_dB+10);
			short x2 = ceil(sc_SINR_dB+10);

			if (x1 == x2)
			{
				*cit = cap[x1];
			}
			else
			{
				double y1 = cap[x1];
				double y2 = cap[x2];

				// linear interpolation
				*cit = y1 + (sc_SINR_dB-x1+10)*((y2-y1)/(x2-x1));
			}
		}
		++SINR_dB_it;
		++cit;
	}

	return capacityPerHertz;
}
//tsokalo
SpectrumValue
GetCapacity (const SpectrumValue& SINR_dB, BitAllocationTable bat)
{
  SpectrumValue capacityPerHertz (SINR_dB.GetSpectrumModel ());

  Values::const_iterator SINR_dB_it = SINR_dB.ConstValuesBegin ();
  BitAllocationTable::iterator bat_it = bat.begin ();
  Values::iterator cit = capacityPerHertz.ValuesBegin ();
  while (SINR_dB_it != SINR_dB.ConstValuesEnd () && cit != capacityPerHertz.ValuesEnd ())
    {
      *cit = GetCapPerChannel (*SINR_dB_it, *bat_it);
      ++SINR_dB_it;
      ++cit;
      ++bat_it;
    }

  return capacityPerHertz;
}
double
GetCapPerChannel (double sinr_db, ModulationType m)
{
  if(m == NOMOD)return 0;

  double success_bits = 0;
  double b = m;
  uint16_t mi = (uint16_t) m - 1;

  if (sinr_db < -10)
    {
      success_bits = log2 (1 + pow (10, sinr_db / 10));
    }
  else if (sinr_db >= 40)
    {
      success_bits = b;
    }
  else
    {
      short x1 = floor (sinr_db + 10);
      short x2 = ceil (sinr_db + 10);

      if (x1 == x2)
        {
          success_bits = qamCap[mi][x1];
        }
      else
        {
          double y1 = qamCap[mi][x1];
          double y2 = qamCap[mi][x2];

          // linear interpolation
          success_bits = y1 + (sinr_db - x1 + 10) * ((y2 - y1) / (x2 - x1));
        }
    }

   return success_bits;
}
} // namespace ns3
