/*
  Copyright (C) 2020-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
//   Implementation file for class TruthHSRoISeedTool
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////


#include "ZWindowRoISeedTool/TruthHSRoISeedTool.h"
#include "TVector2.h"
#include <map>


///////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////

InDet::TruthHSRoISeedTool::TruthHSRoISeedTool
(const std::string& t,const std::string& n,const IInterface* p)
  : base_class(t,n,p)
{
}

///////////////////////////////////////////////////////////////////
// Initialization
///////////////////////////////////////////////////////////////////

StatusCode InDet::TruthHSRoISeedTool::initialize()
{
  StatusCode sc = AlgTool::initialize();   

  ATH_CHECK( m_inputTruthEventsKey.initialize() );

  return sc;
}

/////////////////////////////////////////////////////////////////////
// Compute RoI
/////////////////////////////////////////////////////////////////////

std::vector<InDet::IZWindowRoISeedTool::ZWindow> InDet::TruthHSRoISeedTool::getRoIs(const EventContext& ctx) const
{

  // prepare output
  std::vector<InDet::IZWindowRoISeedTool::ZWindow> listRoIs;  
  listRoIs.clear();

  //retrieve truth collection
  SG::ReadHandle<xAOD::TruthEventContainer> truthEvents(m_inputTruthEventsKey, ctx);
  if (not truthEvents.isValid()) {
    ATH_MSG_ERROR("Cannot retrieve xAOD truth event information. Bailing out with empty RoI list.");
    return listRoIs;
  }
  ATH_MSG_DEBUG("xAOD Truth Event available!");

  //get HS position
  for (const xAOD::TruthEvent *evt : *truthEvents) { 
    const xAOD::TruthVertex *hsPos = evt->signalProcessVertex();
    if (hsPos == nullptr) {
      ATH_MSG_DEBUG("Invalid signal process vertex! Trying next TruthEvent.");
      continue;
    }
    InDet::IZWindowRoISeedTool::ZWindow RoI;
    RoI.zReference = hsPos->z();
    RoI.zWindow[0] = RoI.zReference - m_z0Window; 
    RoI.zWindow[1] = RoI.zReference + m_z0Window;
    listRoIs.push_back(RoI);
    ATH_MSG_DEBUG("Found RoI: " << RoI.zReference << " [" << RoI.zWindow[0] << ", " << RoI.zWindow[1] << "]");
    //use only the first one
    break;
  }

  return listRoIs;
  
}

