/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Author: Giovanni Marchiori (giovanni.marchiori@cern.ch)

#include "DerivationFrameworkEGamma/EGPhotonCleaningWrapper.h"
#include "xAODEgamma/Photon.h"
#include <EgammaAnalysisHelpers/PhotonHelpers.h>

namespace DerivationFramework {

StatusCode
EGPhotonCleaningWrapper::initialize()
{
  if (!m_fudgeMCTool.name().empty()) {
    CHECK(m_fudgeMCTool.retrieve());
  }
  ATH_CHECK(m_containerName.initialize());
  ATH_CHECK(m_decoratorPass.initialize());
  ATH_CHECK(m_decoratorPassDelayed.initialize());

  return StatusCode::SUCCESS;
}

StatusCode
EGPhotonCleaningWrapper::addBranches(const EventContext& ctx) const
{

  SG::ReadHandle<xAOD::PhotonContainer> photons{ m_containerName, ctx };

  // If we're applying corrections, the correction tools will give us
  // copies that we need to keep track of.  (We want to do all the copies
  // before we start writing decorations, to avoid warnings about having
  // unlocked decorations in a copy).
  // The copies we get back from the tool will have standalone aux stores.
  // We'll put them in a DataVector to get them deleted, but we don't
  // need to copy the aux data to the container, so construct it with
  // @c NEVER_TRACK_INDICES.
  xAOD::PhotonContainer pCopies (SG::OWN_ELEMENTS, SG::NEVER_TRACK_INDICES);
  if (!m_fudgeMCTool.empty()) {
    pCopies.reserve (photons->size());
    for (const xAOD::Photon* photon : *photons) {
      // apply the shower shape corrections
      CP::CorrectionCode correctionCode = CP::CorrectionCode::Ok;
      xAOD::Photon* ph = nullptr;
      correctionCode = m_fudgeMCTool->correctedCopy(*photon, ph);
      if (correctionCode == CP::CorrectionCode::Ok) {
      } else if (correctionCode == CP::CorrectionCode::Error) {
        Error("addBranches()",
              "Error applying fudge factors to current photon");
      } else if (correctionCode == CP::CorrectionCode::OutOfValidityRange) {
        Warning(
                "addBranches()",
                "Current photon has no valid fudge factors due to out-of-range");
      } else {
        Warning("addBranches()",
                "Unknown correction code %d from "
                "ElectronPhotonShowerShapeFudgeTool",
                (int)correctionCode);
      }
      pCopies.push_back (ph);
    }
  }
  else {
    pCopies.resize (photons->size());
  }

  SG::WriteDecorHandle<xAOD::PhotonContainer, char> decoratorPass{
    m_decoratorPass, ctx
  };
  SG::WriteDecorHandle<xAOD::PhotonContainer, char> decoratorPassDelayed{
    m_decoratorPassDelayed, ctx
  };

  // Write mask for each element and record to SG for subsequent selection
  for (size_t ipar = 0; const xAOD::Photon* photon : *photons) {

    const xAOD::Photon* pCopy = pCopies[ipar++];
    if (!pCopy) pCopy = photon;

    // decorate the original object
    decoratorPass(*photon) = static_cast<int> (PhotonHelpers::passOQquality(*pCopy));
    decoratorPassDelayed(*photon) = static_cast<int> (PhotonHelpers::passOQqualityDelayed(*pCopy));
  }
  return StatusCode::SUCCESS;
}
} // end namespace DerivationFramework
