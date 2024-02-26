/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKANALYSIS_HGTDCLUSTERANALYSISALG_H
#define ACTSTRKANALYSIS_HGTDCLUSTERANALYSISALG_H

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODInDetMeasurement/HGTDClusterContainer.h"
#include "HGTD_ReadoutGeometry/HGTD_DetectorElementCollection.h"
#include "HGTD_Identifier/HGTD_ID.h"

namespace ActsTrk {

  class HgtdClusterAnalysisAlg final: 
    public AthMonitorAlgorithm {
  public:
    HgtdClusterAnalysisAlg(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~HgtdClusterAnalysisAlg() override = default;

    virtual StatusCode initialize() override;
    virtual StatusCode fillHistograms(const EventContext& ctx) const override;

  private:
    SG::ReadCondHandleKey<InDetDD::HGTD_DetectorElementCollection> m_HGTDDetEleCollKey
    {this, "HGTDDetEleCollKey", "HGTD_DetectorElementCollection",
	"Key of HGTD_DetectorElementCollection for HGTD"};

    SG::ReadHandleKey< xAOD::HGTDClusterContainer > m_hgtdClusterContainerKey
    {this, "ClusterContainerKey", "HGTD_Clusters",
	"Key of input hgtd clusters"};    
    
    Gaudi::Property< std::string > m_monGroupName
      {this, "MonGroupName", "ActsHgtdClusterAnalysisAlg"};

    const HGTD_ID *m_hgtdID {};
  };

}

#endif
