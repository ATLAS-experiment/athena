/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "DerivationFrameworkEGamma/EGPhotonBDTToolDecorator.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include "PATCore/AcceptData.h"
#include "xAODEgamma/Electron.h"
#include "xAODEgamma/Photon.h"

namespace DerivationFramework {

StatusCode
EGPhotonBDTToolDecorator::initialize()
{
  ATH_CHECK(m_observableTool.retrieve());

  if (!(m_fudgeMCTool.name().empty())) {
    ATH_CHECK(m_fudgeMCTool.retrieve());
  } else {
    m_fudgeMCTool.disable();
  }

  ATH_CHECK(m_ContainerName.initialize());
  ATH_CHECK(m_decoratorScore.initialize());

  return StatusCode::SUCCESS;
}

StatusCode
EGPhotonBDTToolDecorator::addBranches(const EventContext& ctx) const
{
  // retrieve container
  SG::ReadHandle<xAOD::EgammaContainer> particles{ m_ContainerName, ctx };

  // Decorators
  SG::WriteDecorHandle<xAOD::EgammaContainer, float> decoratorScore{
    m_decoratorScore, ctx
  };

  // If we're applying corrections, the correction tools will give us
  // copies that we need to keep track of.  (We want to do all the copies
  // before we start writing decorations, to avoid warnings about having
  // unlocked decorations in a copy).
  // The copies we get back from the tool will have standalone aux stores.
  // We'll put them in a DataVector to get them deleted, but we don't
  // need to copy the aux data to the container, so construct it with
  // @c NEVER_TRACK_INDICES.
  xAOD::EgammaContainer pCopies (SG::OWN_ELEMENTS, SG::NEVER_TRACK_INDICES);
  if (!m_fudgeMCTool.empty()) {
    pCopies.reserve (particles->size());
    for (const xAOD::Egamma* par : *particles) {
      xAOD::Type::ObjectType type = par->type();
      // apply the shower shape corrections
      CP::CorrectionCode correctionCode = CP::CorrectionCode::Ok;
      xAOD::Egamma* pCopy = nullptr;
      if (type == xAOD::Type::Electron) {
        const xAOD::Electron* eg = static_cast<const xAOD::Electron*>(par);
        xAOD::Electron* el = nullptr;
        correctionCode = m_fudgeMCTool->correctedCopy(*eg, el);
        pCopy = el;
      } else {
        const xAOD::Photon* eg = static_cast<const xAOD::Photon*>(par);
        xAOD::Photon* ph = nullptr;
        correctionCode = m_fudgeMCTool->correctedCopy(*eg, ph);
        pCopy = ph;
      }
      if (correctionCode == CP::CorrectionCode::Ok) {
        // all OK
      } else if (correctionCode == CP::CorrectionCode::OutOfValidityRange) {
        Warning(
          "addBranches()",
          "Current photon has no valid fudge factors due to out-of-range");
      } else {
        Warning(
          "addBranches()",
          "Unknown correction code %d from ElectronPhotonShowerShapeFudgeTool",
          (int)correctionCode);
      }
      pCopies.push_back (pCopy);
    }
  }
  else {
    pCopies.resize (particles->size());
  }

  // Write mask for each element and record to SG for subsequent selection
  for (size_t ipar = 0; ipar < particles->size(); ipar++) {
    const xAOD::Egamma* par = particles->at(ipar);
    const xAOD::Egamma* pCopy = pCopies[ipar];
    if (!pCopy) pCopy = par;

    // compute the BDT score
    const float score = m_observableTool->evaluate(pCopy);
    
    decoratorScore(*par) = score;
  }

  return StatusCode::SUCCESS;
}
}
