/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKANALYSIS_TRACKPARTICLEANALYSISALG_H
#define ACTSTRKANALYSIS_TRACKPARTICLEANALYSISALG_H

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "StoreGate/ReadHandleKey.h"

namespace ActsTrk {

  class TrackParticleAnalysisAlg final :
    public AthMonitorAlgorithm {
  public:
    TrackParticleAnalysisAlg(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~TrackParticleAnalysisAlg() override = default;

    virtual StatusCode initialize() override;
    virtual StatusCode fillHistograms(const EventContext& ctx) const override;

  private:
    void monitorTrackStateCounts(const xAOD::TrackParticle &track_particle) const;

    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_tracksKey {this, "TrackParticleLocation", "",
        "Input track particle collection"};
    Gaudi::Property< std::string > m_monGroupName
      {this, "MonGroupName", "TrackParticleAnalysisAlg"};

    Gaudi::Property< bool > m_monitorTrackStateCounts
      {this, "MonitorTrackStateCounts", true};
  };

}

#endif
