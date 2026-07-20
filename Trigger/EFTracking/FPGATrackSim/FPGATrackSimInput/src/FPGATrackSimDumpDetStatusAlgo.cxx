/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "FPGATrackSimDumpDetStatusAlgo.h"

#include <sstream>
#include <iostream>
#include <fstream>

/////////////////////////////////////////////////////////////////////////////
FPGATrackSimDumpDetStatusAlgo::FPGATrackSimDumpDetStatusAlgo(const std::string& name, ISvcLocator* pSvcLocator) :
  AthAlgorithm(name, pSvcLocator)
{}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
StatusCode FPGATrackSimDumpDetStatusAlgo::initialize(){
  // select how the input is obtained
  if (m_DumpBadModules || m_DumpGlobalToLocalMap) {
    // Use the SG to retrieve the hits, this also means other Athena tools can be used
    ATH_CHECK(m_detectorTool.retrieve());    
  }
  return StatusCode::SUCCESS;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
StatusCode FPGATrackSimDumpDetStatusAlgo::execute(const EventContext& /*ctx*/) {    
  
  if (m_DumpBadModules) {
    // These are not available yet
    ATH_MSG_WARNING( "FPGATrackSimDumpDetStatusAlgo DumpBadModules not available yet");
    //m_detectorTool->makeBadModuleMap(); //Dump bad SS map
    //m_detectorTool->dumpDeadModuleSummary(); //Dump bad module map
    m_DumpBadModules = false; // in case you run on more than 1 event, only dump once
  }

  if (m_DumpGlobalToLocalMap) {
    m_detectorTool->dumpGlobalToLocalModuleMap(); // Dump the map of the module of each tower
    m_DumpGlobalToLocalMap=false;
  }

  if (m_DumpIDMap) {
    // This is not available yet
    ATH_MSG_WARNING( "FPGATrackSimDumpDetStatusAlgo DumpIDMap not available yet" );
    //m_detectorTool->dumpIDMap();
    m_DumpIDMap = false; // in case you run on more than 1 event, only dump once
  }

  if(m_DumpModulePositions) {
    //m_detectorTool->dumpModulePositions();
     m_DumpModulePositions=false;
  }

  return StatusCode::SUCCESS;
}

