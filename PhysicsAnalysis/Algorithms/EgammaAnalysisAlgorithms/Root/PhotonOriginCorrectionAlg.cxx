/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <AsgDataHandles/ReadHandle.h>
#include <EgammaAnalysisAlgorithms/PhotonOriginCorrectionAlg.h>

#include "egammaUtils/egPhotonWrtPoint.h"
//
// method implementations
//

namespace CP {
PhotonOriginCorrectionAlg ::PhotonOriginCorrectionAlg(const std::string& name,
                                                      ISvcLocator* pSvcLocator)
    : AnaReentrantAlgorithm(name, pSvcLocator) {}

StatusCode PhotonOriginCorrectionAlg ::initialize() {
  ANA_CHECK(m_PhotonHandle.initialize(m_systematicsList));
  ANA_CHECK(m_preselection.initialize(m_systematicsList, m_PhotonHandle,
                                      SG::AllowEmpty));

  // We do not have systematics in principle from this operation
  ANA_CHECK(m_systematicsList.initialize());
  ANA_CHECK(m_primVertices.initialize());
  return StatusCode::SUCCESS;
}

StatusCode PhotonOriginCorrectionAlg ::execute(const EventContext &ctx) const {

  // If we can not rely on Event Cleaning or something
  // else having run before for the PHYSLITE.
  // We have to see if we have a PriVtx.
  SG::ReadHandle<xAOD::VertexContainer> vertices(m_primVertices, ctx);
  const xAOD::Vertex* primary = nullptr;
  for (const xAOD::Vertex* vtx : *vertices) {
    if (vtx->vertexType() == xAOD::VxType::PriVtx) {
      primary = vtx;
      break;
    }
  }
  if (!primary) {
    ATH_MSG_WARNING("Could not find a Primary vertex");
  }

  for (const auto& sys : m_systematicsList.systematicsVector()) {

    xAOD::PhotonContainer* photons = nullptr;

    ANA_CHECK(m_PhotonHandle.getCopy(photons, sys));

    // Actually here we rely on the egamma preselection
    // to have run. Otherwise there is high danger
    // or garbage for the LarEM "gap" region.
    // It seems to be the case for PHYSLITE
    for (xAOD::Photon* ph : *photons) {
      if (m_preselection.getBool(*ph, sys) && primary) {
        // We have a primary vertex so we should be able
        // to correct the Photon Origin from (0,0,0)
        // to (0,0,z)
        photonWrtPoint::correctForZ(*ph, primary->z());
      }
    }
  }
  return StatusCode::SUCCESS;
}
}  // namespace CP
