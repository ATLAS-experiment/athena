/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRACKINGDECORALGORITHMS_TRACKCOVARIANCEDECORATORALG_H
#define TRACKINGDECORALGORITHMS_TRACKCOVARIANCEDECORATORALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"

#include "xAODTracking/TrackParticleContainer.h"


namespace TrackingDecorAlgorithms {

  class TrackCovarianceDecoratorAlg : public AthReentrantAlgorithm {
  public:
    TrackCovarianceDecoratorAlg(const std::string& name,
                                ISvcLocator* pSvcLocator);

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext&) const override;

  private:

    // Input container
    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_trackContainerKey {
      this, "TrackContainer", "InDetTrackParticles",
        "Key for the input track collection"};

    // Output decorations
    SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_dec_phiUncertainty {
      this, "phiUncertainty", m_trackContainerKey, "phiUncertainty",
        "sqrt of phi diagonal element of track covariance matrix"};
    SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_dec_thetaUncertainty {
      this, "thetaUncertainty", m_trackContainerKey, "thetaUncertainty",
        "sqrt of theta diagonal element of track covariance matrix"};
    SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_dec_qOverPUncertainty {
      this, "qOverPUncertainty", m_trackContainerKey, "qOverPUncertainty",
        "sqrt of qOverP diagonal element of track covariance matrix"};
  };

}

#endif
