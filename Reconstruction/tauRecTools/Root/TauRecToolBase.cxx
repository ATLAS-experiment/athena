/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "tauRecTools/TauRecToolBase.h"
#include "PathResolver/PathResolver.h"

// ROOT include(s)
#include "TEnv.h"
#include "THashList.h"

#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <sstream>
#include <cstdlib>


std::string TauRecToolBase::find_file(const std::string& fname) const {
  std::string full_path;
  //offline calib files are in GroupData
  //online calib files are in release
  full_path = PathResolverFindCalibFile(m_tauRecToolsTag+"/"+fname);
  if(full_path.empty()) full_path = PathResolverFindCalibFile(fname);
  return full_path;
}

TauRecToolBase::TauRecToolBase(const std::string& name) :
  asg::AsgTool(name) {
}

StatusCode TauRecToolBase::initialize(){
  return StatusCode::SUCCESS;
}

StatusCode TauRecToolBase::eventInitialize(){
  return StatusCode::SUCCESS;
}

//________________________________________
StatusCode TauRecToolBase::executeTool(xAOD::TauJet& ,
				       const EventContext&) const {
  ATH_MSG_ERROR("function not implemented");
  return StatusCode::FAILURE;
}

#ifdef XAOD_ANALYSIS
StatusCode TauRecToolBase::executeDev(xAOD::TauJet&) {
  ATH_MSG_ERROR("function not implemented");
  return StatusCode::FAILURE;
}
#else
StatusCode TauRecToolBase::executeTool(xAOD::TauJet& ,
				       const EventContext& ,
				       CaloConstCellContainer& ,
				       boost::dynamic_bitset<>& ) const {
  ATH_MSG_ERROR("function not implemented");
  return StatusCode::FAILURE;
}
#endif

StatusCode TauRecToolBase::executeTool(xAOD::TauJet& ,
				       const EventContext& ,
				       const xAOD::VertexContainer*) const {
  ATH_MSG_ERROR("function not implemented");
  return StatusCode::FAILURE;
}

StatusCode TauRecToolBase::executeTool(xAOD::TauJet& ,
                                       const EventContext& ,
                                       xAOD::VertexContainer& ) const {
  ATH_MSG_ERROR("function not implemented");
  return StatusCode::FAILURE;
}

StatusCode TauRecToolBase::executeTool(xAOD::TauJet& ,
				       const EventContext& ,
				       xAOD::TauTrackContainer&) const {
  ATH_MSG_ERROR("function not implemented");
  return StatusCode::FAILURE;
}

StatusCode TauRecToolBase::executeTool(xAOD::TauJet& ,
				       const EventContext& ,
				       xAOD::CaloClusterContainer& ,
				       xAOD::PFOContainer& ) const {
  ATH_MSG_ERROR("function not implemented");
  return StatusCode::FAILURE;
}

StatusCode TauRecToolBase::executeTool(xAOD::TauJet& ,
				       const EventContext& ,
				       xAOD::PFOContainer& ,
				       xAOD::PFOContainer& ,
				       const xAOD::CaloClusterContainer& ) const {
  ATH_MSG_ERROR("function not implemented");
  return StatusCode::FAILURE;
}

StatusCode TauRecToolBase::executeTool(xAOD::TauJet& ,
				       const EventContext& ,
				       xAOD::PFOContainer& ,
				       xAOD::PFOContainer& ) const {
  ATH_MSG_ERROR("function not implemented");
  return StatusCode::FAILURE;
} 

StatusCode TauRecToolBase::executeTool(xAOD::TauJet& ,
				       const EventContext& ,
				       xAOD::PFOContainer& ) const {
  ATH_MSG_ERROR("function not implemented");
  return StatusCode::FAILURE;
}

StatusCode TauRecToolBase::executeTool(xAOD::TauJet& ,
				       const EventContext& ,
				       xAOD::ParticleContainer& ,
				       xAOD::PFOContainer& ) const {
  ATH_MSG_ERROR("function not implemented");
  return StatusCode::FAILURE;
}

StatusCode TauRecToolBase::eventFinalize() {
  return StatusCode::SUCCESS;
}

StatusCode TauRecToolBase::finalize(){
  return StatusCode::SUCCESS;
}
