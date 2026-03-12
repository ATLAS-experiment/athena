/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TrackingDecorAlgorithms/TrackCovarianceDecoratorAlg.h"

#include "StoreGate/WriteDecorHandle.h"

#include <cmath>


namespace TrackingDecorAlgorithms {

  TrackCovarianceDecoratorAlg::TrackCovarianceDecoratorAlg(
    const std::string& name, ISvcLocator* loc)
    : AthReentrantAlgorithm(name, loc) {}

  StatusCode TrackCovarianceDecoratorAlg::initialize() {
    ATH_MSG_INFO("Initializing " << name() << "...");

    ATH_CHECK(m_trackContainerKey.initialize());
    ATH_CHECK(m_dec_phiUncertainty.initialize());
    ATH_CHECK(m_dec_thetaUncertainty.initialize());
    ATH_CHECK(m_dec_qOverPUncertainty.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode TrackCovarianceDecoratorAlg::execute(
    const EventContext& ctx) const
  {
    SG::ReadHandle<xAOD::TrackParticleContainer> tracks(
      m_trackContainerKey, ctx);
    ATH_CHECK(tracks.isValid());

    using TPC = xAOD::TrackParticleContainer;
    SG::WriteDecorHandle<TPC, float> dec_phi(m_dec_phiUncertainty, ctx);
    SG::WriteDecorHandle<TPC, float> dec_theta(m_dec_thetaUncertainty, ctx);
    SG::WriteDecorHandle<TPC, float> dec_qOverP(m_dec_qOverPUncertainty, ctx);

    for (const xAOD::TrackParticle* trk : *tracks) {
      const auto& diag = trk->definingParametersCovMatrixDiagVec();
      dec_phi(*trk) = std::sqrt(diag.at(2));
      dec_theta(*trk) = std::sqrt(diag.at(3));
      dec_qOverP(*trk) = std::sqrt(diag.at(4));
    }

    return StatusCode::SUCCESS;
  }

}
