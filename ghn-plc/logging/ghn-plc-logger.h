/*
 * GhnPlcLogger.h
 *
 *  Created on: May 31, 2016
 *      Author: tsokalo
 */

#ifndef GHN_PLC_LOGGER_H_
#define GHN_PLC_LOGGER_H_

#include "ns3/object.h"
#include "ns3/core-module.h"
#include "ns3/stats-module.h"
#include <string>

#include "ns3/ghn-plc-defs.h"

namespace ns3
{
namespace ghn {
class TimeProbe;

class GhnPlcLogger
{
public:
  GhnPlcLogger ();
  GhnPlcLogger (std::string orig)
  {
    m_originatorName = orig;
  }
  virtual
  ~GhnPlcLogger ();

  template<class T>
    void
    AddProbeOriginator (std::string orig, T* o)
    {
      m_originatorName = "/Names/" + orig;
      Names::Add (m_originatorName, o);
    }

  std::string
  GetProbeOriginator ()
  {
    return m_originatorName;
  }

  void
  SetupLog (FileLogData logData);

private:

  std::string m_originatorName;
  FileHelper m_fileHelper;

};
}
}
#endif /* GHN_PLC_LOGGER_H_ */
