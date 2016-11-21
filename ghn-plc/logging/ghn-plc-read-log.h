/*
 * read-log.h
 *
 *  Created on: 12.09.2016
 *      Author: tsokalo
 */

#ifndef GHN_PLC_READLOG_H_
#define GHN_PLC_READLOG_H_

#include <string.h>
#include <vector>
#include <fstream>
#include <sstream>

namespace ns3 {
namespace ghn {
typedef std::vector<double> Vals;

void
ReadFileContents (std::string path, Vals &data, Vals &loss, Vals &latency, Vals &iat)
{
  std::string line;
  uint32_t sid, seqNum, lost, l, i, s;
  std::ifstream myfile (path);
  if (myfile.is_open ())
    {
      while (getline (myfile, line))
        {
          std::stringstream ss (line);
          //          m_appLogTrace (sid, m_seqNum[sid], lostPkts, latency.GetMicroSeconds (), iat.GetMicroSeconds(), pkt->GetSize ());
          ss >> sid;
          ss >> seqNum;
          ss >> lost;
          ss >> l;
          ss >> i;
          ss >> s;
          data.push_back(s);
          loss.push_back(lost);
          latency.push_back(l);
          iat.push_back(i);
        }
      myfile.close ();
    }
}
}
}

#endif /* GHN_PLC_READLOG_H_ */
