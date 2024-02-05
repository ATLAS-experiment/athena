/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKANALYSIS_ESTIMATEDTRACKPARAMSANALYSISALG_H
#define ACTSTRKANALYSIS_ESTIMATEDTRACKPARAMSANALYSISALG_H

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "StoreGate/ReadHandleKey.h"
#include "ActsEvent/TrackParameters.h"

namespace ActsTrk {

  class EstimatedTrackParamsAnalysisAlg final :
    public AthMonitorAlgorithm {
  public:
    EstimatedTrackParamsAnalysisAlg(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~EstimatedTrackParamsAnalysisAlg() override = default;

    virtual StatusCode initialize() override;
    virtual StatusCode fillHistograms(const EventContext& ctx) const override;

  private:
    SG::ReadHandleKey< ActsTrk::BoundTrackParametersContainer > m_inputTrackParamsColletionKey {this,  "InputTrackParamsCollection", "", ""}; 

    Gaudi::Property< std::string > m_monGroupName
      {this, "MonGroupName", "ActsEstimatedTrackParamsAnalysisAlg"};
  };

}

#endif
