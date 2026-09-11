/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
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
StatusCode TauRecToolBase::execute(xAOD::TauJet&) const {
  ATH_MSG_ERROR("function not implemented");
  return StatusCode::FAILURE;
}

#ifdef XAOD_ANALYSIS
StatusCode TauRecToolBase::executeDev(xAOD::TauJet&) {
  ATH_MSG_ERROR("function not implemented");
  return StatusCode::FAILURE;
}
#else
StatusCode TauRecToolBase::executePi0CreateROI(xAOD::TauJet& /*pTau*/, CaloConstCellContainer& /*caloCellContainer*/, boost::dynamic_bitset<>& /*map*/ ) const {
  ATH_MSG_ERROR("function not implemented");
  return StatusCode::FAILURE;
}
#endif

StatusCode TauRecToolBase::executeVertexFinder(xAOD::TauJet&, const xAOD::VertexContainer*) const {
  ATH_MSG_ERROR("function not implemented");
  return StatusCode::FAILURE;
}

StatusCode TauRecToolBase::executeTrackFinder(xAOD::TauJet&, xAOD::TauTrackContainer&) const {
  ATH_MSG_ERROR("function not implemented");
  return StatusCode::FAILURE;
}

StatusCode TauRecToolBase::executeTrackClassifier(xAOD::TauJet&, xAOD::TauTrackContainer&) const {
  ATH_MSG_ERROR("function not implemented");
  return StatusCode::FAILURE;
}

StatusCode TauRecToolBase::executeShotFinder(xAOD::TauJet& /*pTau*/, xAOD::CaloClusterContainer& /*shotClusterContainer*/, xAOD::PFOContainer& /*PFOContainer*/ ) const {
  ATH_MSG_ERROR("function not implemented");
  return StatusCode::FAILURE;
}

StatusCode TauRecToolBase::executePi0ClusterCreator(xAOD::TauJet& /*pTau*/, xAOD::PFOContainer& /*neutralPFOContainer*/, 
					      xAOD::PFOContainer& /*hadronicPFOContainer*/, 
					      const xAOD::CaloClusterContainer& /*pCaloClusterContainer*/ ) const {
  ATH_MSG_ERROR("function not implemented");
  return StatusCode::FAILURE;
}

StatusCode TauRecToolBase::executeVertexVariables(xAOD::TauJet& /*pTau*/, xAOD::VertexContainer& /*vertexContainer*/ ) const {
  ATH_MSG_ERROR("function not implemented");
  return StatusCode::FAILURE;
}

StatusCode TauRecToolBase::executePi0ClusterScaler(xAOD::TauJet& /*pTau*/, xAOD::PFOContainer& /*neutralPFOContainer*/, xAOD::PFOContainer& /*chargedPFOContainer*/ ) const {
  ATH_MSG_ERROR("function not implemented");
  return StatusCode::FAILURE;
} 

StatusCode TauRecToolBase::executePi0nPFO(xAOD::TauJet& /*pTau*/, xAOD::PFOContainer& /*neutralPFOContainer*/) const {
  ATH_MSG_ERROR("function not implemented");
  return StatusCode::FAILURE;
}

StatusCode TauRecToolBase::executePanTau(xAOD::TauJet& /*pTau*/, xAOD::ParticleContainer& /*particleContainer*/, xAOD::PFOContainer& /*neutralPFOContainer*/) const {
  ATH_MSG_ERROR("function not implemented");
  return StatusCode::FAILURE;
}

StatusCode TauRecToolBase::eventFinalize() {
  return StatusCode::SUCCESS;
}

StatusCode TauRecToolBase::finalize(){
  return StatusCode::SUCCESS;
}
