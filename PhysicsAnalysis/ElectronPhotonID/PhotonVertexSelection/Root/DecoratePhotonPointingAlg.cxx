/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "PhotonVertexSelection/DecoratePhotonPointingAlg.h"

#include <xAODEgamma/EgammaContainer.h>

#include "AsgDataHandles/ReadHandle.h"

DecoratePhotonPointingAlg::DecoratePhotonPointingAlg(const std::string& name,
                                                     ISvcLocator* svcLoc)
    : EL::AnaAlgorithm(name, svcLoc) {}

StatusCode DecoratePhotonPointingAlg::initialize() {
  ATH_CHECK(m_photonContainerKey.initialize());
  ATH_CHECK(m_pointingTool.retrieve());
  return StatusCode::SUCCESS;
}

StatusCode DecoratePhotonPointingAlg::execute() {
  SG::ReadHandle<xAOD::EgammaContainer> egammas{m_photonContainerKey};
  if (!egammas.isValid()) {
    ATH_MSG_ERROR("Invalid egamma container from " << m_photonContainerKey.key());
    return StatusCode::FAILURE;
  }

  ATH_CHECK(m_pointingTool->updatePointingAuxdata(*egammas));

  return StatusCode::SUCCESS;
}