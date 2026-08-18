/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Author: Giovanni Marchiori (giovanni.marchiori@cern.ch)

#include "DerivationFrameworkEGamma/EGElectronLikelihoodToolWrapper.h"
//
#include "PATCore/AcceptData.h"
#include "PATCore/AcceptInfo.h"
//
#include "xAODBase/IParticleContainer.h"
#include "xAODBase/IParticleHelpers.h"
#include "xAODEgamma/EgammaContainer.h"
#include "xAODEgamma/Electron.h"
#include "xAODEgamma/Photon.h"

namespace DerivationFramework {

  StatusCode
  EGElectronLikelihoodToolWrapper::initialize()
  {
    ATH_CHECK(m_tool.retrieve());

    ATH_CHECK(m_ContainerName.initialize());
    ATH_CHECK(m_fudgedContainerName.initialize(!m_fudgedContainerName.empty()));
    //
    ATH_CHECK(m_decoratorPass.initialize());
    ATH_CHECK(m_decoratorIsEM.initialize());
    //
    ATH_CHECK(m_decoratorResult.initialize(m_storeTResult));
    ATH_CHECK(m_decoratorMultipleOutputs.initialize(m_storeMultipleOutputs));
    return StatusCode::SUCCESS;
  }

  StatusCode
  EGElectronLikelihoodToolWrapper::addBranches(const EventContext& ctx) const
  {
    // retrieve container
    SG::ReadHandle<xAOD::EgammaContainer> particles;
    if(!m_fudgedContainerName.empty()){
      SG::ReadHandle<xAOD::EgammaContainer> fudged{ m_fudgedContainerName, ctx };
      particles = fudged;
    } else {
      SG::ReadHandle<xAOD::EgammaContainer> egammas{ m_ContainerName, ctx };
      particles = egammas;
    }

    // Decorators
    SG::WriteDecorHandle<xAOD::EgammaContainer, char> decoratorPass{
      m_decoratorPass, ctx
    };
    SG::WriteDecorHandle<xAOD::EgammaContainer, unsigned int> decoratorIsEM{
      m_decoratorIsEM, ctx
    };

    std::unique_ptr<SG::WriteDecorHandle<xAOD::EgammaContainer, float>>
      decoratorResult = nullptr;
    if (m_storeTResult) {
      decoratorResult =
        std::make_unique<SG::WriteDecorHandle<xAOD::EgammaContainer, float>>(
                                                                             m_decoratorResult, ctx);
    }
    std::vector<SG::WriteDecorHandle<xAOD::EgammaContainer, float>> decoratorMultipleOutputs{};
    if (m_storeMultipleOutputs) {
      decoratorMultipleOutputs = m_decoratorMultipleOutputs.makeHandles(ctx);
    }

    // Write mask for each element and record to SG for subsequent selection
    for (const auto& egamma : *particles) {
      // compute the output of the selector
      asg::AcceptData theAccept(m_tool->accept(ctx, egamma));
      // this should work for both the
      // cut-based and the LH selectors
      const unsigned int isEM =
        static_cast<unsigned int>(theAccept.getCutResultInvertedBitSet()
				  .to_ulong());

      // decorate the original object
      const xAOD::IParticle* original = m_fudgedContainerName.empty() ?
	egamma : xAOD::getOriginalObject(*egamma);
      if (m_cut.empty()) {
        const bool pass_selection = static_cast<bool>(theAccept);
	decoratorPass(*original) = pass_selection ? 1 : 0;
      } else {
	decoratorPass(*original) = theAccept.getCutResult(m_cut) ? 1 : 0;
      }
      decoratorIsEM(*original) = isEM;
      if (decoratorResult) {
	(*decoratorResult)(*original) =
	  static_cast<float>(m_tool->calculate(ctx, egamma));
      }
      if (m_storeMultipleOutputs) {
	// calculateMultipleOutputs only supports xAOD::Electron as input
	const xAOD::Electron *eCopy = static_cast<const xAOD::Electron *>(egamma);
	std::vector<float> toolOutput = m_tool->calculateMultipleOutputs(ctx, eCopy);
	for (size_t i = 0; i < toolOutput.size(); i++){
	  decoratorMultipleOutputs.at(i)(*original) = toolOutput.at(i);
	}
      }
    }

    return StatusCode::SUCCESS;
  }
}
