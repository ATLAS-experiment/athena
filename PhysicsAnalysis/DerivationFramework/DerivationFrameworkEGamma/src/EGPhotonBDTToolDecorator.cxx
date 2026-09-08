/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "DerivationFrameworkEGamma/EGPhotonBDTToolDecorator.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include "PATCore/AcceptData.h"
#include "xAODEgamma/Electron.h"
#include "xAODEgamma/Photon.h"
#include "xAODBase/IParticleHelpers.h"

namespace DerivationFramework {

StatusCode
EGPhotonBDTToolDecorator::initialize()
{
  ATH_CHECK(m_observableTool.retrieve());

  ATH_CHECK(m_ContainerName.initialize());
  ATH_CHECK(m_fudgedContainerName.initialize(!m_fudgedContainerName.empty()));

  ATH_CHECK(m_decoratorScore.initialize());

  return StatusCode::SUCCESS;
}

StatusCode
EGPhotonBDTToolDecorator::addBranches(const EventContext& ctx) const
{
  // retrieve container
  SG::ReadHandle<xAOD::EgammaContainer> particles;
  if(!m_fudgedContainerName.empty()){
    SG::ReadHandle<xAOD::EgammaContainer> fudged{ m_fudgedContainerName, ctx };
    particles = std::move(fudged);
  } else {
    SG::ReadHandle<xAOD::EgammaContainer> photons{ m_ContainerName, ctx };
    particles = std::move(photons);
  }

  // Decorators
  SG::WriteDecorHandle<xAOD::EgammaContainer, float> decoratorScore{
    m_decoratorScore, ctx
  };

  for (const auto& photon : *particles) {
    // compute the BDT score with the fudged photon
    const float score = m_observableTool->evaluate(photon);

    // decorate the original object
    const xAOD::IParticle* original = m_fudgedContainerName.empty() ?
      photon : xAOD::getOriginalObject(*photon);
    decoratorScore(*original) = score;
  }

  return StatusCode::SUCCESS;
}
}
