/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "DerivationFrameworkEGamma/EGPhotonBDTToolWrapper.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include "PATCore/AcceptData.h"
#include "xAODEgamma/Electron.h"
#include "xAODEgamma/Photon.h"
#include "xAODBase/IParticleHelpers.h"

namespace DerivationFramework {

StatusCode
EGPhotonBDTToolWrapper::initialize()
{
  ATH_CHECK(m_selectorTool.retrieve());

  ATH_CHECK(m_ContainerName.initialize());
  ATH_CHECK(m_fudgedContainerName.initialize(!m_fudgedContainerName.empty()));

  ATH_CHECK(m_decoratorPass.initialize());
  ATH_CHECK(m_decoratorIsEM.initialize());

  return StatusCode::SUCCESS;
}

StatusCode
EGPhotonBDTToolWrapper::execute(const EventContext& ctx) const
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
  SG::WriteDecorHandle<xAOD::EgammaContainer, char> decoratorPass{
    m_decoratorPass, ctx
  };
  SG::WriteDecorHandle<xAOD::EgammaContainer, unsigned int> decoratorIsEM{
    m_decoratorIsEM, ctx
  };

  for (const auto& photon : *particles) {
    // compute the output of the selector with the fudged photon
    asg::AcceptData theAccept(m_selectorTool->accept(ctx, photon));
    // compute the is EM word
    unsigned int isEM = 0;
    ATH_CHECK(m_selectorTool->execute(ctx, photon, isEM));
    
    // decorate the original object
    const xAOD::IParticle* original = m_fudgedContainerName.empty() ?
      photon : xAOD::getOriginalObject(*photon);
    if (m_cut.empty()) {
      decoratorPass(*original) = static_cast<bool>(theAccept) ? 1 : 0;
    } else {
      decoratorPass(*original) = theAccept.getCutResult(m_cut) ? 1 : 0;
    }
    decoratorIsEM(*original) = isEM;
  }

  return StatusCode::SUCCESS;
}
}
