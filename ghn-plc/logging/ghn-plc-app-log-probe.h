
#ifndef GHN_PLC_APP_LOG_PROBE_H
#define GHN_PLC_APP_LOG_PROBE_H

#include "ns3/probe.h"
#include "ns3/object.h"
#include "ns3/callback.h"
#include "ns3/boolean.h"
#include "ns3/traced-value.h"
#include "ns3/simulator.h"
#include "ns3/nstime.h"

#include "ns3/ghn-plc-defs.h"

namespace ns3 {
namespace ghn {
class GhnPlcAppLogProbe : public Probe
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId ();
  GhnPlcAppLogProbe ();
  virtual ~GhnPlcAppLogProbe ();


  /**
   * \brief connect to a trace source attribute provided by a given object
   *
   * \param traceSource the name of the attribute TraceSource to connect to
   * \param obj ns3::Object to connect to
   * \return true if the trace source was successfully connected
   */
  virtual bool ConnectByObject (std::string traceSource, Ptr<Object> obj);

  /**
   * \brief connect to a trace source provided by a config path
   *
   * \param path Config path to bind to
   *
   * Note, if an invalid path is provided, the probe will not be connected
   * to anything.
   */
  virtual void ConnectByPath (std::string path);

private:
  /**
   * \brief Method to connect to an underlying ns3::TraceSource of type Time 
   *
   * \param oldData previous value of the Time 
   * \param newData new value of the Time 
   */
  void TraceSink (AppLog oldData, AppLog newData);

  TracedValue<AppLog> m_output; //!< Output trace source.
};
}
} // namespace ns3

#endif // GHN_PLC_APP_LOG_PROBE_H
