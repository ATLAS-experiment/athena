/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Author: Giovanni Marchiori (giovanni.marchiori@cern.ch)
//

#include "DerivationFrameworkEGamma/EGSelectionToolWrapper.h"
#include "PATCore/AcceptData.h"
#include "xAODBase/IParticleContainer.h"
#include "xAODBase/IParticleHelpers.h"
#include "xAODEgamma/EgammaContainer.h"
#include "xAODEgamma/Electron.h"
#include "xAODEgamma/Photon.h"

namespace DerivationFramework {

StatusCode
EGSelectionToolWrapper::initialize()
{
  ATH_CHECK(m_tool.retrieve());

  ATH_CHECK(m_ContainerName.initialize());
  ATH_CHECK(m_fudgedContainerName.initialize(!m_fudgedContainerName.empty()));

  ATH_CHECK(m_decoratorPass.initialize());
  ATH_CHECK(m_decoratorIsEM.initialize());

  return StatusCode::SUCCESS;
}

StatusCode
EGSelectionToolWrapper::execute(const EventContext& ctx) const
{
  // retrieve container
  SG::ReadHandle<xAOD::EgammaContainer> particles;
  if(!m_fudgedContainerName.empty()){
    SG::ReadHandle<xAOD::EgammaContainer> fudged{ m_fudgedContainerName, ctx };
    particles = std::move(fudged);
  } else {
    SG::ReadHandle<xAOD::EgammaContainer> egammas{ m_ContainerName, ctx };
    particles = std::move(egammas);
  }
  
  // Decorators
  SG::WriteDecorHandle<xAOD::EgammaContainer, char> decoratorPass{
    m_decoratorPass, ctx
  };
  SG::WriteDecorHandle<xAOD::EgammaContainer, unsigned int> decoratorIsEM{
    m_decoratorIsEM, ctx
  };

  for (const auto& egamma : *particles) {
    // compute the output of the selector with the fudged object
    asg::AcceptData theAccept(m_tool->accept(ctx, egamma));
    // this should work for both the
    // cut-based and the LH selectors
    unsigned int isEM =
      static_cast<unsigned int>(theAccept.getCutResultInvertedBitSet()
				.to_ulong());

    // decorate the original object
    const xAOD::IParticle* original = m_fudgedContainerName.empty() ?
      egamma : xAOD::getOriginalObject(*egamma);
    if (m_cut == "") {
      bool pass_selection = static_cast<bool>(theAccept);
      decoratorPass(*original) = pass_selection ? 1 : 0;
    } else {
      decoratorPass(*original) = theAccept.getCutResult(m_cut) ? 1 : 0;
    }
    decoratorIsEM(*original) = isEM;
  }

  return StatusCode::SUCCESS;
}
}
