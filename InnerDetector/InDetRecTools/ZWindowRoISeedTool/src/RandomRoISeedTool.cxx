/*
  Copyright (C) 2020-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
//   Implementation file for class RandomRoISeedTool
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////


#include "ZWindowRoISeedTool/RandomRoISeedTool.h"
#include "AthenaKernel/RNGWrapper.h"

#include "TVector2.h"
#include "CLHEP/Random/RandGauss.h"

#include <map>


///////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////

InDet::RandomRoISeedTool::RandomRoISeedTool
(const std::string& t,const std::string& n,const IInterface* p)
  : base_class(t,n,p)
{
}

///////////////////////////////////////////////////////////////////
// Initialization
///////////////////////////////////////////////////////////////////

StatusCode InDet::RandomRoISeedTool::initialize()
{
  StatusCode sc = AlgTool::initialize();   

  ATH_CHECK( m_beamSpotKey.initialize() );

  return sc;
}

/////////////////////////////////////////////////////////////////////
// Compute RoI
/////////////////////////////////////////////////////////////////////

std::vector<InDet::IZWindowRoISeedTool::ZWindow> InDet::RandomRoISeedTool::getRoIs(const EventContext& ctx) const
{

  // Prepare output                                                                                                                                                         
  std::vector<InDet::IZWindowRoISeedTool::ZWindow> listRoIs;
  listRoIs.clear();

  // Retrieve beamspot information
  SG::ReadCondHandle<InDet::BeamSpotData> beamSpotHandle{m_beamSpotKey, ctx};
  if (not beamSpotHandle.isValid()) {
    ATH_MSG_ERROR("Cannot retrieve beam spot data. Bailing out with empty RoI list.");
    return listRoIs;
  }
  float bsSigZ = 0.0;
  bsSigZ = beamSpotHandle->beamSigma(2);
  ATH_MSG_DEBUG("Beam spot data available!");

  // Initialize random engine and find z-value of RoI
  ATHRNG::RNGWrapper *rndmEngine = m_atRndmSvc->getEngine(this, m_rndmEngineName);
  rndmEngine->setSeed (name(), ctx);
  CLHEP::HepRandomEngine* engine = rndmEngine->getEngine (ctx);  
  float zVal;
  zVal = CLHEP::RandGauss::shoot(engine, 0.0, 1.0) * bsSigZ; //This effectively samples from a beamspot with the correct beamspot sigma_z

  InDet::IZWindowRoISeedTool::ZWindow RoI;
  RoI.zReference = zVal;
  RoI.zWindow[0] = RoI.zReference - m_z0Window; 
  RoI.zWindow[1] = RoI.zReference + m_z0Window;
  listRoIs.push_back(RoI);
  ATH_MSG_DEBUG("Random RoI: " << RoI.zReference << " [" << RoI.zWindow[0] << ", " << RoI.zWindow[1] << "]");
  
  return listRoIs;
  
}

