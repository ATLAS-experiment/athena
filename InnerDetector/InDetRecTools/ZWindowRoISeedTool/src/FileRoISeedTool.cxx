/*
  Copyright (C) 2020-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
//   Implementation file for class FileRoISeedTool (get RoI for low-pt tracking from a file)
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////


#include "ZWindowRoISeedTool/FileRoISeedTool.h"

#include "TVector2.h"

#include <map>
#include <sstream>
#include <fstream>
#include <string>
#include <filesystem>

///////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////

InDet::FileRoISeedTool::FileRoISeedTool
(const std::string& t,const std::string& n,const IInterface* p)
  : base_class(t,n,p)
{
}

///////////////////////////////////////////////////////////////////
// Initialization
///////////////////////////////////////////////////////////////////

StatusCode InDet::FileRoISeedTool::initialize()
{
  StatusCode sc = AlgTool::initialize();

  ATH_CHECK( m_eventInfoKey.initialize() );

  if (not std::filesystem::exists(std::filesystem::path(m_filename.value()))) {
    ATH_MSG_ERROR( "RoI File DOES NOT Exist!");
    return StatusCode::FAILURE;
  }

  return sc;
}

/////////////////////////////////////////////////////////////////////
// Compute RoI
/////////////////////////////////////////////////////////////////////
std::vector<InDet::IZWindowRoISeedTool::ZWindow> InDet::FileRoISeedTool::getRoIs(const EventContext& ctx) const
{

  // prepare output
  std::vector<InDet::IZWindowRoISeedTool::ZWindow> listRoIs;  
  listRoIs.clear();

  unsigned long long evtN = 0;
  int runN = 0;

  SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfoKey, ctx);
  if ( eventInfo.isValid() ){
    evtN = eventInfo->eventNumber();
    runN = eventInfo->runNumber();
  }

  std::string line;
  std::ifstream inFile(m_filename.value());
  if (inFile.is_open()){

    while (std::getline(inFile, line)){
      
      std::istringstream iss(line);

      int runnum;
      unsigned long long eventnum;
      float zref;

      while( iss >> runnum >> eventnum >> zref){
	//no need to go past the right line
	if(runnum == runN && eventnum == evtN) break;
      }

      if(runnum == runN && eventnum == evtN){ //No need to fill if there isn't an ROI
	InDet::IZWindowRoISeedTool::ZWindow readinref;
	readinref.zReference = zref;
	readinref.zWindow[0] = zref -m_z0Window;
	readinref.zWindow[1] = zref + m_z0Window;
	listRoIs.push_back(readinref);
      }
      
    }
  }

  inFile.close();

  return listRoIs;
  
}

