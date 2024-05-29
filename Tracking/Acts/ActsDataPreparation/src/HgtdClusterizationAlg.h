/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_DATAPREPARATION_HGTD_CLUSTERIZATIONALG_H
#define ACTSTRK_DATAPREPARATION_HGTD_CLUSTERIZATIONALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "ActsToolInterfaces/IHGTDClusteringTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "HGTD_Identifier/HGTD_ID.h"
#include "HGTD_ReadoutGeometry/HGTD_DetectorManager.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "xAODInDetMeasurement/HGTDClusterContainer.h"
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"

namespace ActsTrk {

class HgtdClusterizationAlg
    : public AthReentrantAlgorithm {
public:
    HgtdClusterizationAlg(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~HgtdClusterizationAlg() override = default;

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;
    
private:
  ToolHandle< IHGTDClusteringTool > m_clusteringTool {this, "ClusteringTool", "", "The Clustering Tool"};
  ToolHandle< GenericMonitoringTool > m_monTool {this, "MonTool", "", "Monitoring tool"};

  SG::ReadHandleKey<HGTD_RDO_Container> m_rdoContainerKey{this, "RDOContainerName", "", "Name of the HGTD_RDO container"};
  SG::WriteHandleKey<xAOD::HGTDClusterContainer> m_clusterContainerKey{this, "ClusterContainerName", "", "Name of the HGTD cluster container"}; 
};

} // namespace ActsTrk

#endif

