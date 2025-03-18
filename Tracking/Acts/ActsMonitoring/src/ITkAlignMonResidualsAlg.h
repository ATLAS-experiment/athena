/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#pragma once

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "ActsEvent/TrackContainer.h"
#include "xAODTracking/TrackParticleContainer.h"

namespace ActsTrk {

  class ITkAlignMonResidualsAlg final :
    public AthMonitorAlgorithm {
  public:
    ITkAlignMonResidualsAlg(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~ITkAlignMonResidualsAlg() override = default;

    virtual StatusCode initialize() override;
    virtual StatusCode fillHistograms(const EventContext& ctx) const override;

  private:
    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_trackParticlesKey{this, "TrackParticles", "", "Input xAOD::TrackParticles"};

    // Decorators
    SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_measurement_det {this, "measurement_det", "measurement_det"};
    SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_measurement_region {this, "measurement_region", "measurement_region"};
    SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_measurement_type {this, "measurement_type", "measurement_type"};
    SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_measurement_layer {this, "measurement_iLayer", "measurement_iLayer"};
    SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_hitResiduals_residualLocX {this, "hitResiduals_residualLocX", "hitResiduals_residualLocX"};
    SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_hitResiduals_pullLocX {this, "hitResiduals_pullLocX", "hitResiduals_pullLocX"};
    SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_hitResiduals_residualLocY {this, "hitResiduals_residualLocY", "hitResiduals_residualLocY"};
    SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_hitResiduals_pullLocY {this, "hitResiduals_pullLocY", "hitResiduals_pullLocY"};
    SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_hitResiduals_phiWidth {this, "hitResiduals_phiWidth", "hitResiduals_phiWidth"};
    SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_hitResiduals_etaWidth {this, "hitResiduals_etaWidth", "hitResiduals_etaWidth"};

    Gaudi::Property< std::string > m_monGroupName
      {this, "MonGroupName", "ActsResAnalysisAlg"};

    
    static const int m_nSiBlayers{5}; //
    std::vector<int> m_pixResidualX;
    std::vector<int> m_pixResidualY;
    std::vector<int> m_pixPullX;
    std::vector<int> m_pixPullY;
    std::vector<int> m_stripResidualX;
    std::vector<int> m_stripPullX;
    
  };
  
}
