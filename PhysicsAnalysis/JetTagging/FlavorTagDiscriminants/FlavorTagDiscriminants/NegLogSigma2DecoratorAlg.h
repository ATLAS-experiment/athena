/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef NEG_LOG_SIGMA2_DECORATOR_ALG_HH
#define NEG_LOG_SIGMA2_DECORATOR_ALG_HH

// FrameWork includes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"

// Containers
#include "xAODBase/IParticleContainer.h"

// Read and write handle keys
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"

namespace FlavorTagDiscriminants {

  class NegLogSigma2DecoratorAlg: public AthReentrantAlgorithm {
    /** @name NegLogSigma2DecoratorAlg
     *  @brief Turn a per-jet standard deviation into -2*log(sigma), the log
     *         precision that the DipZ maximum-likelihood trigger condition
     *         expects, since it recovers the variance as exp(-negLogSigma2).
     *         A non-positive or non-finite input is passed through as NaN.
     */

    public:
      NegLogSigma2DecoratorAlg(const std::string& name,
                               ISvcLocator* pSvcLocator);

      virtual StatusCode initialize() override;
      virtual StatusCode execute(const EventContext& ctx) const override;

    private:
      SG::ReadHandleKey<xAOD::IParticleContainer> m_jetCollectionKey {
        this, "jetContainer", "AntiKt4EMPFlowJets", "Key for the jet collection"};

      SG::ReadDecorHandleKey<xAOD::IParticleContainer> m_sigmaKey {
        this, "sigmaDecor", m_jetCollectionKey, "HitZ_z0_sigma",
        "Jet decoration holding the regressed standard deviation"};

      SG::WriteDecorHandleKey<xAOD::IParticleContainer> m_outputKey {
        this, "negLogSigma2Decor", m_jetCollectionKey, "HitZ_negLogSigma2",
        "Key for the output log precision, -2*log(sigma)"};
  };
}

#endif
