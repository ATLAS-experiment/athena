/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):

// local include
#include "DerivationFrameworkEGamma/EGammaFudgeAlgorithm.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/PhotonContainer.h"

#include "xAODCore/ShallowCopy.h"
#include "xAODBase/IParticleHelpers.h"
#include "PATInterfaces/CorrectionCode.h"

namespace DerivationFramework {

  StatusCode EGammaFudgeAlgorithm::initialize() {
    ATH_CHECK(m_inputKey.initialize());
    ATH_CHECK(m_outputKey.initialize());
    ATH_CHECK(m_fudgeTool.retrieve());
    return StatusCode::SUCCESS;
  }

  StatusCode EGammaFudgeAlgorithm::execute(const EventContext& ctx) const{

    SG::ReadHandle<xAOD::EgammaContainer> readHandle{m_inputKey, ctx};
    if (!readHandle.isValid()) {
      ATH_MSG_FATAL("No EGamma container found");
      return StatusCode::FAILURE;
    }
    const xAOD::EgammaContainer* egammas{readHandle.cptr()};
    const xAOD::ElectronContainer* electrons = dynamic_cast<const xAOD::ElectronContainer*>(egammas);
    const xAOD::PhotonContainer* photons = dynamic_cast<const xAOD::PhotonContainer*>(egammas);

    xAOD::EgammaContainer* outcontainer{nullptr};
    if (electrons) {
      xAOD::ShallowCopyResult_t<xAOD::ElectronContainer> output = xAOD::shallowCopy(*electrons, ctx);
      if (!output.first || !output.second) {
            ATH_MSG_FATAL("Creation of shallow copy failed");
            return StatusCode::FAILURE;
      }
      outcontainer = output.first.get();
      SG::WriteHandle<xAOD::EgammaContainer> writeHandle{m_outputKey, ctx};
      ATH_CHECK(writeHandle.recordNonConst(std::move(output.first), std::move(output.second)));

    } else if (photons) {
         xAOD::ShallowCopyResult_t<xAOD::PhotonContainer> output = xAOD::shallowCopy(*photons, ctx);
         if (!output.first || !output.second) {
               ATH_MSG_FATAL("Creation of shallow copy failed");
               return StatusCode::FAILURE;
         }
         outcontainer = output.first.get();
         SG::WriteHandle<xAOD::EgammaContainer> writeHandle{m_outputKey, ctx};
         ATH_CHECK(writeHandle.recordNonConst(std::move(output.first), std::move(output.second)));
    } else {
         ATH_MSG_FATAL("Unknown Egamma container "<<m_inputKey.fullKey());
         return StatusCode::FAILURE;
    }

    if (!setOriginalObjectLink(*egammas, *outcontainer)) {
      ATH_MSG_ERROR("Failed to add original object links to shallow copy of " << m_inputKey);
      return StatusCode::FAILURE;
    }

    for(xAOD::Egamma* iParticle : *outcontainer) {
      if (electrons) {
	auto* electron = static_cast<xAOD::Electron*>(iParticle);
	if(m_fudgeTool->applyCorrection(*electron)==CP::CorrectionCode::Error)
	  return StatusCode::FAILURE;
      } else {
	auto* photon = static_cast<xAOD::Photon*>(iParticle);
	if (m_fudgeTool->applyCorrection(*photon) == CP::CorrectionCode::Error)
	  return StatusCode::FAILURE;
      }
    }

    return StatusCode::SUCCESS;
  }

}
