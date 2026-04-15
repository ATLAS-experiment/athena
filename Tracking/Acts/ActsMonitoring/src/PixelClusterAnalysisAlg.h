/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKANALYSIS_PIXELCLUSTERANALYSISALG_H
#define ACTSTRKANALYSIS_PIXELCLUSTERANALYSISALG_H

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "InDetIdentifier/PixelID.h"

namespace ActsTrk {

  class PixelClusterAnalysisAlg final: 
    public AthMonitorAlgorithm {
  public:
    PixelClusterAnalysisAlg(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~PixelClusterAnalysisAlg() override = default;

    virtual StatusCode initialize() override;
    virtual StatusCode fillHistograms(const EventContext& ctx) const override;

  private:
    SG::ReadHandleKey< xAOD::PixelClusterContainer > m_pixelClusterContainerKey
    {this, "ClusterContainerKey", "ITkPixelClusters",
	"Key of input pixel clusters"};    
    
    Gaudi::Property< std::string > m_monGroupName
      {this, "MonGroupName", "ActsClusterAnalysisAlg"};

    const PixelID *m_pixelID {};
  };

}

#endif // ACTSTRKANALYSIS_PIXELCLUSTERANALYSISALG_H
