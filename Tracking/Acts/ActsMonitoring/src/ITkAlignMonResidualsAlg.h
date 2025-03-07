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
    
    SG::ReadHandleKey<ActsTrk::TrackContainer> m_tracksKey{this, "ActsTracks","","Input Acts::Track Collection"};
    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_trackParticlesKey{this, "InDetTrackParticles","InDetTrackParticles","Input xAOD::TrackParticles"};

    SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_acc_measurement_regionAcc {this, "measurement_region", m_trackParticlesKey, "measurement_region"};
    
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
