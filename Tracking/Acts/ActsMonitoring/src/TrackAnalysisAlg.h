/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKANALYSIS_TRACKANALYSISALG_H
#define ACTSTRKANALYSIS_TRACKANALYSISALG_H

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "ActsEvent/TrackContainer.h"
#include "StoreGate/ReadHandleKey.h"

namespace ActsTrk {

  class TrackAnalysisAlg final :
    public AthMonitorAlgorithm {
  public:
    TrackAnalysisAlg(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~TrackAnalysisAlg() override = default;

    virtual StatusCode initialize() override;
    virtual StatusCode fillHistograms(const EventContext& ctx) const override;
      
  private:
    SG::ReadHandleKey<ActsTrk::TrackContainer> m_tracksKey {this, "TracksLocation", "",
	"Input track collection"};

    Gaudi::Property< std::string > m_monGroupName
      {this, "MonGroupName", "ActsSeedAnalysisAlg"};
  };

}

#endif
