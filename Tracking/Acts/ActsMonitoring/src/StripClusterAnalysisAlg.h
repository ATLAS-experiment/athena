/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKANALYSIS_STRIPCLUSTERANALYSISALG_H
#define ACTSTRKANALYSIS_STRIPCLUSTERANALYSISALG_H

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"
#include "InDetIdentifier/SCT_ID.h"

namespace ActsTrk {

  class StripClusterAnalysisAlg final : 
    public AthMonitorAlgorithm {
  public:
    StripClusterAnalysisAlg(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~StripClusterAnalysisAlg() override = default;

    virtual StatusCode initialize() override;
    virtual StatusCode fillHistograms(const EventContext& ctx) const override;

  private:
    SG::ReadHandleKey<xAOD::StripClusterContainer> m_stripClusterContainerKey
    {this, "ClusterContainerKey", "ITkStripClusters",
	"Key of input pixel clusters"};

    Gaudi::Property< std::string > m_monGroupName
      {this, "MonGroupName", "ActsClusterAnalysisAlg"};
    
    const SCT_ID *m_stripID {};
  };

}

#endif // ACTSTRKANALYSIS_STRIPCLUSTERANALYSIS_H
