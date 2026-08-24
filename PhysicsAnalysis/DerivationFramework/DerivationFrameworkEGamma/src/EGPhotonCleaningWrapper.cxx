/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Author: Giovanni Marchiori (giovanni.marchiori@cern.ch)

#include "DerivationFrameworkEGamma/EGPhotonCleaningWrapper.h"
#include "xAODEgamma/Photon.h"
#include <EgammaAnalysisHelpers/PhotonHelpers.h>
#include "xAODBase/IParticleHelpers.h"

namespace DerivationFramework {

StatusCode
EGPhotonCleaningWrapper::initialize()
{
  ATH_CHECK(m_containerName.initialize());
  ATH_CHECK(m_fudgedContainerName.initialize(!m_fudgedContainerName.empty()));

  ATH_CHECK(m_decoratorPass.initialize());
  ATH_CHECK(m_decoratorPassDelayed.initialize());

  return StatusCode::SUCCESS;
}

StatusCode
EGPhotonCleaningWrapper::addBranches(const EventContext& ctx) const
{

  SG::ReadHandle<xAOD::EgammaContainer> particles;
    if(!m_fudgedContainerName.empty()){
      SG::ReadHandle<xAOD::EgammaContainer> fudged{ m_fudgedContainerName, ctx };
      particles = std::move(fudged);
    } else {
      SG::ReadHandle<xAOD::EgammaContainer> photons{ m_containerName, ctx };
      particles = std::move(photons);
    }

  SG::WriteDecorHandle<xAOD::EgammaContainer, char> decoratorPass{
    m_decoratorPass, ctx
  };
  SG::WriteDecorHandle<xAOD::EgammaContainer, char> decoratorPassDelayed{
    m_decoratorPassDelayed, ctx
  };

  // Write mask for each element and record to SG for subsequent selection
  for (const auto& egamma : *particles) {
    // decorate the original object
    const xAOD::IParticle* original = m_fudgedContainerName.empty() ?
	egamma : xAOD::getOriginalObject(*egamma);
    const xAOD::Photon* gCopy = static_cast<const xAOD::Photon*>(egamma);
    decoratorPass(*original) =
      static_cast<int> (PhotonHelpers::passOQquality(*gCopy));
    decoratorPassDelayed(*original) =
      static_cast<int> (PhotonHelpers::passOQqualityDelayed(*gCopy));
  }
  return StatusCode::SUCCESS;
}
} // end namespace DerivationFramework
