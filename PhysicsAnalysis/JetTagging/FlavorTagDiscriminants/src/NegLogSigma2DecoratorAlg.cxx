/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Header file
#include "FlavorTagDiscriminants/NegLogSigma2DecoratorAlg.h"

// Read and write handles
#include "StoreGate/ReadHandle.h"
#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/WriteDecorHandle.h"

#include <cmath>
#include <limits>


namespace FlavorTagDiscriminants {

  NegLogSigma2DecoratorAlg::NegLogSigma2DecoratorAlg(
    const std::string& name, ISvcLocator* loc)
    : AthReentrantAlgorithm(name, loc) {}


  StatusCode NegLogSigma2DecoratorAlg::initialize() {
    ATH_MSG_DEBUG("Initializing " << name());

    ATH_CHECK(m_jetCollectionKey.initialize());
    ATH_CHECK(m_sigmaKey.initialize());
    ATH_CHECK(m_outputKey.initialize());

    return StatusCode::SUCCESS;
  }


  StatusCode NegLogSigma2DecoratorAlg::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG("Executing " << name());

    SG::ReadHandle<xAOD::IParticleContainer> jets(m_jetCollectionKey, ctx);
    if(!jets.isValid()) {
      ATH_MSG_ERROR("Failed to retrieve jet container with key " << m_jetCollectionKey.key());
      return StatusCode::FAILURE;
    }

    SG::ReadDecorHandle<xAOD::IParticleContainer, float> sigma(m_sigmaKey, ctx);
    SG::WriteDecorHandle<xAOD::IParticleContainer, float> negLogSigma2(m_outputKey, ctx);

    for(const xAOD::IParticle* jet : *jets) {
      const double sig = sigma(*jet);
      negLogSigma2(*jet) = (std::isfinite(sig) && sig > 0.)
        ? static_cast<float>(-2. * std::log(sig))
        : std::numeric_limits<float>::quiet_NaN();
    }

    return StatusCode::SUCCESS;
  }
}
