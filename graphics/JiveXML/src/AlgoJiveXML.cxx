/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "JiveXML/AlgoJiveXML.h"
#include "JiveXML/IDataRetriever.h"
#include "JiveXML/IGeometryWriter.h"
#include "JiveXML/XMLFormatTool.h"
#include "JiveXML/StreamToFileTool.h"
#include "JiveXML/StreamToServerTool.h"

#include "xAODEventInfo/EventInfo.h"

#include "GaudiKernel/ServiceHandle.h"

#include "RootUtils/PyAthenaGILStateEnsure.h"

#include <algorithm>

namespace JiveXML{

  AlgoJiveXML::AlgoJiveXML(const std::string& name, ISvcLocator* pSvcLocator) :
    AthAlgorithm(name, pSvcLocator) {}

  /**
   * Initialize - called once in the beginning
   * - Get StoreGate service
   * - Get GeometryWriters and write geometry if asked for
   * - Get the formatting tool
   * - Get the data retrievers
   * - Get the streaming tools, including defaults if asked for   */
  StatusCode AlgoJiveXML::initialize(){

    //be verbose
    ATH_MSG_VERBOSE("Initialize()");

    /**
     * Get the geometry writer tools and write geometries
     */
    if(m_writeGeometry){

      ATH_MSG_DEBUG("Writing geometry to file");

      /// Loop over names
      std::vector<std::string>::iterator GeoWriterNameItr = m_GeoWriterNames.begin();
      for ( ; GeoWriterNameItr != m_GeoWriterNames.end(); ++GeoWriterNameItr){

        /// Get a tool handle and retrieve the tool

        ToolHandle<IGeometryWriter> GeometryWriter(*GeoWriterNameItr);
        if ( GeometryWriter.retrieve().isFailure() ){
          ATH_MSG_WARNING("Unable to locate "<<GeometryWriter.name()<<" tool");
        } else {
          /// Write geometry
          if ( GeometryWriter->writeGeometry().isFailure() ){
            ATH_MSG_WARNING("Unable to write geometry");
          }
          /// Release tool
          if ( GeometryWriter.release().isFailure() ){
            ATH_MSG_WARNING("Unable to release tool GeometryWriter");
          }
        }
      }
    }

    /**
     * Get the formatting tools
     */
    ATH_MSG_DEBUG("Retrieving XML format tool");

    //Retrieve the format tool
    ATH_CHECK(m_FormatTool.retrieve());

    //Setting the tags
    if ( m_FormatTool->SetTag( TagType("Release",m_AtlasRelease)).isFailure() ){
      ATH_MSG_WARNING( "Couldn't set Release version" );
    }

    /**
     * Add default file streaming tools if requested
     */
    ATH_MSG_DEBUG("Retrieving streaming tools");
    if (m_writeToFile){
      ATH_MSG_INFO("Retrieving default file streaming tool");
      ATH_CHECK(m_StreamToFileTool.retrieve());
    }
    ATH_MSG_INFO("online " << m_onlineMode);
    /// Get the streaming tools
    if (m_onlineMode){
      ATH_MSG_INFO("Retrieving default server streaming tool");
      ATH_CHECK(m_StreamToServerTool.retrieve());
   }

  
    ATH_MSG_INFO("Retrieving data from " << m_DataRetrieverTools.size() << " tools" );

    ATH_MSG_INFO("List property settings: ");
    ATH_MSG_INFO("AtlasRelease: " << m_AtlasRelease);
    ATH_MSG_INFO("WriteToFile: " << m_writeToFile);
    ATH_MSG_INFO("OnlineMode: " << m_onlineMode);
    ATH_MSG_INFO("WriteGeometry: " << m_writeGeometry);
    ATH_MSG_INFO("GeometryVersion: " << m_geometryVersionIn);
    ATH_MSG_INFO("GeoWriterNames: "  << m_GeoWriterNames );

    return StatusCode::SUCCESS;
  }

  /**
   * Execute - called for every event
   * - retrive general event information
   * - call data retrievers
   * - pass formatted events to streamers
   */
  StatusCode AlgoJiveXML::execute() {

    if(m_onlineMode){
      bool isR4PinThisEvent = false;
      // Get the ready4physics bool at the start of this job and for the current time, during the job ATLAS might have become ready for physics, if this is the case we need to stop the jobs and let the partition restart them
      
      // Acquire the Global Interpreter Lock (GIL) to ensure thread safety when interacting with Python objects
      RootUtils::PyGILStateEnsure ensure;

      // Import the EventDisplaysOnlineHelpers Python module
      PyObject* pHelper = PyImport_ImportModule("EventDisplaysOnline.EventDisplaysOnlineHelpers");
      if (!pHelper) {
	// Error handling: Print Python exception if import fails
	PyErr_Print();
	ATH_MSG_WARNING("Failed to import EventDisplaysOnline.EventDisplaysOnlineHelpers module");
	return StatusCode::FAILURE;
      } else {
	  ATH_MSG_INFO("ready4Physics at start of job: " << m_ready4PhysicsAtStart);
	  // Get the "Ready4Physics" function from the module
          PyObject* Ready4Physics = PyObject_GetAttrString(pHelper, "Ready4Physics");
          if (!Ready4Physics || !PyCallable_Check(Ready4Physics)) {
            // Error handling: Print warning if function not found or not callable
            ATH_MSG_WARNING("Could not find or call Ready4Physics function in EventDisplaysOnline.EventDisplaysOnlineHelpers module");
          } else {
            // Call the "Ready4Physics" function
            PyObject* r4p = PyObject_CallFunctionObjArgs(Ready4Physics, NULL);
            if (!r4p) {
              PyErr_Print();
              ATH_MSG_WARNING("Failed to call Ready4Physics function");
            } else {
              // Convert the result to a boolean value
              isR4PinThisEvent = PyObject_IsTrue(r4p);
	      ATH_MSG_INFO("isR4PinThisEvent: " << isR4PinThisEvent);
	      if(!m_ready4PhysicsAtStart && isR4PinThisEvent){
		ATH_MSG_INFO("ATLAS was not ready for physics at the start of this job, but is now, so stopping job so that the partition can restart the jobs with any new configuration, i.e. after a warm start to pick back up pixel hits");
		return StatusCode::FAILURE;
	      }
              Py_DECREF(r4p); // Decrement reference count of the r4p object
            }
          }
          Py_XDECREF(Ready4Physics);
      }
      Py_DECREF(pHelper);
    }// End of if online
    
    /**
     * Retrieve all the event header information
     */
    //The run and event number of the current event
    unsigned int runNo = 0, lumiBlock = 0;
    uint64_t eventNo=0;
    //Date and time string of the current event
    char dateTime[32];
    //general event property, unused yet
    std::string eventProperty = "default";

    // geometry-version/geometry-tag from Athena (sourced via jOs)
    std::string geometryVersion = "default";
    geometryVersion = DataType(m_geometryVersionIn).toString();
    
    //Retrieve eventInfo from StoreGate
    const xAOD::EventInfo* eventInfo = nullptr;
    if (evtStore()->retrieve(eventInfo).isFailure()){
      ATH_MSG_FATAL("Could not find xAODEventInfo" );
      return StatusCode::FAILURE;
    }else{
     ATH_MSG_DEBUG(" xAODEventInfo: runNumber: "  << eventInfo->runNumber()
          << ", eventNumber: " << eventInfo->eventNumber()
          << ", mcChannelNumber: " << eventInfo->mcChannelNumber()
          << ", mcEventNumber: "  << eventInfo->mcEventNumber() // MC: use this instead of runNumber
          << ", lumiBlock: "  << eventInfo->lumiBlock()
          << ", timeStamp: "  << eventInfo->timeStamp()
          << ", bcid: "  << eventInfo->bcid()
          << ", eventTypeBitmask: "  << eventInfo->eventTypeBitmask()
          << ", actualInteractionsPerCrossing: "  << eventInfo->actualInteractionsPerCrossing()
          << ", averageInteractionsPerCrossing: "  << eventInfo->averageInteractionsPerCrossing()
          );
    }

    //when running online, if a new run has started stop the jobs and let the partition restart the jobs to pick up any new configuration for the run
    if(m_onlineMode){
      ATH_MSG_INFO("m_previousRunNumber: " << m_previousRunNumber);
      ATH_MSG_INFO("eventInfo->runNumber(): " << eventInfo->runNumber());
      //don't check for first event
      if(m_previousRunNumber!=0){
	if(m_previousRunNumber != eventInfo->runNumber()){
	  ATH_MSG_INFO("Got a new run number, a new run has started whilst this job was running, stopping now to let the partition restart the jobs and pick up any new configuration");
	  return StatusCode::FAILURE;
	}
      }
      //store run number to look at in the next event
      m_previousRunNumber = eventInfo->runNumber();
    }
    // new treatment of mc_channel_number for mc12
    // from: https://twiki.cern.ch/twiki/bin/viewauth/Atlas/PileupDigitization#Contents_of_Pileup_RDO
    unsigned int mcChannelNo = 0;
    bool firstEv = true;

    //+++ Get sub-event info object
    ATH_MSG_DEBUG( "Sub Event Infos: " );
    for (const xAOD::EventInfo::SubEvent& subevt : eventInfo->subEvents()) {
      const xAOD::EventInfo* sevt = subevt.ptr();
      if (sevt) {
        if (firstEv){
          mcChannelNo =  sevt->mcChannelNumber(); // the 'real' mc-channel
          ATH_MSG_DEBUG( " mc_channel from SubEvent   : " << sevt->mcChannelNumber() );
          firstEv = false;
        }
        ATH_MSG_VERBOSE("Sub Event Info:\n  Time         : " << subevt.time()
                        << "  Index        : " << subevt.index()
                        << "  Provenance   : " << subevt.type()                         // This is the provenance stuff: signal, minbias, cavern, etc
                        << "  Run Number   : " << sevt->runNumber()
                        << "  Event Number : " << sevt->eventNumber()
                        << "  ns Offset    : " << sevt->timeStampNSOffset()
                        << "  Lumi Block   : " << sevt->lumiBlock()
                        << "  mc_channel   : " << sevt->mcChannelNumber()
                        << "  BCID         : " << sevt->bcid()
                        << "  Geo version  : " << m_geometryVersionIn
                        );
      }
      else ATH_MSG_VERBOSE("Subevent is null ptr ");
    }

    //Get run and event numbers
    runNo   = eventInfo->runNumber();
    eventNo = eventInfo->eventNumber();

// Note: 4294967293 is the maximum value for a unsigned long

    if ( mcChannelNo != 0 ){ runNo = mcChannelNo + 140000000; } // indicating 'mc14'
    ATH_MSG_DEBUG( " runNumber for filename: " << runNo << ", eventNumber: " << eventNo);

    if ( eventInfo->lumiBlock() ){
      lumiBlock = eventInfo->lumiBlock();
    }else{
      lumiBlock = -1; // placeholder
    }
    if ( mcChannelNo != 0 ) lumiBlock = -1; // mask for mc11a

    // lumiBlock from mc can be just huge number, ignore then
    if ( lumiBlock > 1000000 ) { lumiBlock = 0; }

    //Get timestamp of the event
    //If Grid job not running in CEST or CET, change to UTC
    //Only option to avoid odd timezones. jpt 29Mar11
    size_t found1;
    size_t found2;
    if (eventInfo->timeStamp() > 0) {
      time_t unixtime = (time_t) eventInfo->timeStamp();
      struct tm time;
      localtime_r(&unixtime, &time);
      strftime(dateTime, 32, "%Y-%m-%d %H:%M:%S %Z", &time);
      struct tm utctime;
      gmtime_r(&unixtime, &utctime);
      found1 = (DataType(dateTime).toString().find("CEST"));
      found2 = (DataType(dateTime).toString().find("CET"));
      if ( int(found1)<0 && int(found2)<0 ){ // not found is -1
         strftime(dateTime, 32, "%Y-%m-%d %H:%M:%S UTC", &utctime);
         ATH_MSG_DEBUG( " TIME NOT CET/CEST. Adjusted to:" << dateTime );
      }
    } else {
      dateTime[0] = '\0'; // empty string
    }
    if ( mcChannelNo != 0 ){ dateTime[0] = '\0'; } // mask for mc11a

    /**
     * Then start a new event with this header information
     */
    if ( m_FormatTool->StartEvent(eventNo, runNo, dateTime, lumiBlock, eventProperty, geometryVersion).isFailure() ){
      ATH_MSG_FATAL("Couldn't start event in FormatTool");
      return StatusCode::FAILURE;
    }

    /**
     * Now iterate over all the IDataRetrievers and
     * write their data to the xml file by giving it the XMLWriter
     */
    ATH_MSG_DEBUG("Starting loop over data retrievers" );
    //Loop over data retrievers
    ToolHandleArray<IDataRetriever>::iterator DataRetrieverItr = m_DataRetrieverTools.begin();
    for(; DataRetrieverItr != m_DataRetrieverTools.end(); ++DataRetrieverItr)  {
      //Add try-catch to avoid retrieval to fail on single retriever
      try {
        //Retrieve information and pass it to formatting tool object
        if ((*DataRetrieverItr)->retrieve(m_FormatTool).isFailure()) {
          ATH_MSG_WARNING( "Failed to fill " << (*DataRetrieverItr)->dataTypeName() );
        } else {
          ATH_MSG_DEBUG("Filled: " << (*DataRetrieverItr)->dataTypeName() );
        }
      //Only catch std::exception
      } catch ( std::exception& ex ){
        //Now show some message
        ATH_MSG_FATAL("Caught exception in " << (*DataRetrieverItr)->name()
                          << " while retrieving data for " << (*DataRetrieverItr)->dataTypeName()
                          << " : " << ex.what() );
        //and return with an error
        return StatusCode::FAILURE;
      }
    }
    ATH_MSG_DEBUG( "Finished loop over data retrievers" );

    /**
     * Finish the event with a proper footer
     */
    if ( m_FormatTool->EndEvent().isFailure() ){
      ATH_MSG_WARNING( "Couldn't end event in FormatTool" );
      return StatusCode::FAILURE;
    }

    /**
     * Now stream the events to all registered streaming tools
     */
    if(m_writeToFile){
      ATH_MSG_DEBUG("Streaming event to file");
      if ( (m_StreamToFileTool->StreamEvent(eventNo, runNo, m_FormatTool->getFormattedEvent()).isFailure() )){
        ATH_MSG_WARNING( "Could not stream event to file" );
      }
    }
    if(m_onlineMode){
      ATH_MSG_DEBUG("Streaming event to server");
      if ( (m_StreamToServerTool->StreamEvent(eventNo, runNo, m_FormatTool->getFormattedEvent()).isFailure() )){
        ATH_MSG_WARNING( "Could not stream event to server" );
      }
    }

    return StatusCode::SUCCESS;
  }

  /**
   * Finalize called once in the end
   * - release all tools
   */
  StatusCode AlgoJiveXML::finalize() {

    ATH_MSG_VERBOSE( "finalize()" );

    /// Release all the tools
    m_DataRetrieverTools.release().ignore();
    m_FormatTool.release().ignore();
    m_StreamToFileTool.release().ignore();
    m_StreamToServerTool.release().ignore();

    return StatusCode::SUCCESS;
  }
} //namespace
