/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "DerivationFrameworkEGamma/EGPhotonBDTToolWrapper.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include "PATCore/AcceptData.h"
#include "xAODEgamma/Electron.h"
#include "xAODEgamma/Photon.h"

namespace DerivationFramework {

StatusCode
EGPhotonBDTToolWrapper::initialize()
{
  ATH_CHECK(m_selectorTool.retrieve());

  if (!(m_fudgeMCTool.name().empty())) {
    ATH_CHECK(m_fudgeMCTool.retrieve());
  } else {
    m_fudgeMCTool.disable();
  }

  ATH_CHECK(m_ContainerName.initialize());
  ATH_CHECK(m_decoratorPass.initialize());
  ATH_CHECK(m_decoratorIsEM.initialize());

  return StatusCode::SUCCESS;
}

StatusCode
EGPhotonBDTToolWrapper::addBranches(const EventContext& ctx) const
{
  // retrieve container
  SG::ReadHandle<xAOD::EgammaContainer> particles{ m_ContainerName, ctx };

  // Decorators
  SG::WriteDecorHandle<xAOD::EgammaContainer, char> decoratorPass{
    m_decoratorPass, ctx
  };
  SG::WriteDecorHandle<xAOD::EgammaContainer, unsigned int> decoratorIsEM{
    m_decoratorIsEM, ctx
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

    // compute the output of the selector
    asg::AcceptData theAccept(m_selectorTool->accept(ctx, pCopy));
    // compute the is EM word
    unsigned int isEM = 0;
    ATH_CHECK(m_selectorTool->execute(ctx, pCopy, isEM));
    
    // decorate the original object
    if (m_cut.empty()) {
      decoratorPass(*par) = static_cast<bool>(theAccept) ? 1 : 0;  
    } else {
      decoratorPass(*par) = theAccept.getCutResult(m_cut) ? 1 : 0;
    }
    decoratorIsEM(*par) = isEM;
  }

  return StatusCode::SUCCESS;
}
}
