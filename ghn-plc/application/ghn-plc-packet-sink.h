
#ifndef GHN_PLC_PACKET_SINK_H
#define GHN_PLC_PACKET_SINK_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"
#include "ns3/address.h"
#include "ns3/socket.h"

#include "ns3/ghn-plc-cut-log.h"

namespace ns3 {
namespace ghn {

class GhnPlcPacketSink : public Application
{
public:

  static TypeId GetTypeId (void);
  GhnPlcPacketSink ();

  virtual ~GhnPlcPacketSink ();

  uint32_t GetTotalRx () const;

  Ptr<Socket> GetListeningSocket (void) const;

  std::list<Ptr<Socket> > GetAcceptedSockets (void) const;

  void
  SetResDirectory (std::string resDir)
  {
    m_resDir = resDir;
  }
  void
  SetLogId(std::string logId)
  {
    m_logId = logId;
  }

protected:
  virtual void DoDispose (void);
private:

  virtual void StartApplication (void);    // Called at time specified by Start
  virtual void StopApplication (void);     // Called at time specified by Stop


  void HandleRead (Ptr<Socket> socket);
  void HandleAccept (Ptr<Socket> socket, const Address& from);
  void HandlePeerClose (Ptr<Socket> socket);
  void HandlePeerError (Ptr<Socket> socket);

  Ptr<Socket>     m_socket;       //!< Listening socket
  std::list<Ptr<Socket> > m_socketList; //!< the accepted sockets

  Address         m_local;        //!< Local address to bind to
  uint32_t        m_totalRx;      //!< Total bytes received
  TypeId          m_tid;          //!< Protocol TypeId

  TracedCallback<Ptr<const Packet>, const Address &> m_rxTrace;

  Ptr<GhnPlcCutLog> m_cutLog;
  std::string m_resDir;
  std::string m_logId;

};
}
} // namespace ns3

#endif /* GHN_PLC_PACKET_SINK_H */

