/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JIVEXML_ExternalONCRPCSERVERSVC_H
#define JIVEXML_ExternalONCRPCSERVERSVC_H

#include "AthenaBaseComps/AthService.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/MsgStream.h"
#include "JiveXML/EventStream.h"
#include "JiveXML/IServerSvc.h"

#include <rpc/clnt.h>

namespace JiveXML {

  /**
   * This athena service will provide a link to an external ONCRPC server, e.g
   * running as standalone application or in another athena application. It only
   * provides means for streaming events to that server.
   */
  class ExternalONCRPCServerSvc : public extends<AthService, IServerSvc> {

  public:
    
    /** Default constructor */
    ExternalONCRPCServerSvc(const std::string& name, ISvcLocator* sl);
    /** Destructor */
    virtual ~ExternalONCRPCServerSvc();

    /** Gaudi default methods */
    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;

    /** @name Server methods */
    //@{
    /** Put this event as new current event for stream given by name */
    virtual StatusCode UpdateEventForStream( const EventStreamID& evtStreamID, const std::string & event) override;
    //@}
     
  private:

    //Instantiate and release the client handle
    StatusCode GetClient();
    StatusCode ReleaseClient();

    //Get a timeval struct from the timeout double
    struct timeval GetTimeout();

    //The name of the machine the external server is running on
    //defaults to "localhost"
    std::string m_hostName;

    //The timeout when calling the server in seconds
    double m_timeOut;

    //A client handle
    CLIENT* m_client;
  };
 
} //namespace

#endif

