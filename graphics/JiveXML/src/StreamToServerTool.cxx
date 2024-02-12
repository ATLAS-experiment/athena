/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <JiveXML/StreamToServerTool.h>

namespace JiveXML {

  /**
   * Constructor 
   * - setup message service
   * - initialize properties
   */
  StreamToServerTool::StreamToServerTool( const std::string& type , const std::string& name, const IInterface* p):
    AthAlgTool(type,name,p),
    m_ServerSvc("JiveXML::ONCRPCServerSvc",name){
    
    //Provide interface
    declareInterface<IStreamTool>(this);

    /// The name of the stream
    declareProperty("StreamName", m_StreamName = "default");
    /// The ServerSvc
    declareProperty("ServerService", m_ServerSvc );
  }

  /**
   * Intialize - called once at the beginning
   */
  StatusCode StreamToServerTool::initialize(){

    //Make sure we can get hold of the server and the online event displays service
    if (m_ServerSvc.retrieve().isFailure()){
      ATH_MSG_ERROR("Unable to initialize server service with name " << m_ServerSvc.name());
      return StatusCode::FAILURE;
    }

    if( m_onlineEDsvc.retrieve().isFailure()){
      ATH_MSG_ERROR("Could not locate the online event displays service");
      return StatusCode::FAILURE;
    }
    
    return StatusCode::SUCCESS;
  }

  /**
   * Finalize - called once at the end
   */
  StatusCode StreamToServerTool::finalize(){
    return StatusCode::SUCCESS;
  }

  /**
   * Stream one event to the server
   * @param EventNumber the event number
   * @param RunNumber the run number
   * @param EventBuffer the string holding the complete event
   */
   StatusCode StreamToServerTool::StreamEvent( const unsigned long EventNumber, 
					       const unsigned int RunNumber, 
					       const std::ostringstream* const EventBuffer ) { 
     m_StreamName = m_onlineEDsvc->getStreamName();
     ATH_MSG_VERBOSE("Streaming event Number " << EventNumber 
		     << " from run Number " << RunNumber
		     << " to stream " << m_StreamName
		     << " on " << m_ServerSvc.name() << " service");

     /// Create an EventStreamID for this event
     EventStreamID evtStreamID(EventNumber,RunNumber,m_StreamName);

     /// Stream this event to the server
     return m_ServerSvc->UpdateEventForStream(evtStreamID,EventBuffer->str());
   }

} //namespace
