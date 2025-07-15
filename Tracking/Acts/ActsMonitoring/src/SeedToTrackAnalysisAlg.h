/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKANALYSIS_SEEDTOTRACKANALYSISALG_H
#define ACTSTRKANALYSIS_SEEDTOTRACKANALYSISALG_H

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "ActsEvent/SeedContainer.h"
#include "ActsEvent/TrackParametersContainer.h"
#include "BeamSpotConditionsData/BeamSpotData.h"

namespace ActsTrk {

  class SeedToTrackAnalysisAlg final :
    public AthMonitorAlgorithm {
  public:
    SeedToTrackAnalysisAlg(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~SeedToTrackAnalysisAlg() override = default;

    virtual StatusCode initialize() override;
    virtual StatusCode fillHistograms(const EventContext& ctx) const override;

  private:
    SG::ReadHandleKey< ActsTrk::SeedContainer > m_seedsKey {this, "InputSeedCollection", ""};
    SG::ReadHandleKey< ActsTrk::BoundTrackParametersContainer > m_paramsKey {this, "InputTrackParamsCollection", ""};
    SG::ReadHandleKey< std::vector<int> > m_destiniesKey {this, "InputDestinyCollection", ""};
    SG::ReadCondHandleKey< InDet::BeamSpotData > m_beamSpotKey{this, "BeamSpotKey", "BeamSpotData", "SG key for beam spot"};

    static const int m_nLayers{5};
    std::vector<int> m_seedVars {};
  };

}

#endif
