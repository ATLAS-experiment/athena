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
#include "HGTD_RawData/HGTD_ALTIROC_RDO_Container.h"

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
    virtual StatusCode finalize() override;
    
private:
  ToolHandle< IHGTDClusteringTool > m_clusteringTool {this, "ClusteringTool", "", "The Clustering Tool"};
  ToolHandle< GenericMonitoringTool > m_monTool {this, "MonTool", "", "Monitoring tool"};

  SG::ReadHandleKey<HGTD_RDO_Container> m_rdoContainerKey{this, "RDOContainerName", "", "Name of the HGTD_RDO container"};
  SG::ReadHandleKey<HGTD_ALTIROC_RDO_Container> m_altiroc_rdo_rh_key{this, "AltirocRDOContainerName", "", "Name of the HGTD_ALTIROC_RDO container"};
  SG::WriteHandleKey<xAOD::HGTDClusterContainer> m_clusterContainerKey{this, "ClusterContainerName", "", "Name of the HGTD cluster container"}; 

  BooleanProperty m_use_altiroc_rdo{this, "useALTIROC_RDO", false, "Use Altiroc RDO instead of standard"};

private:
  enum EStat {
    kNRdo,
    kNClusters,
    kNStat
  };
  
  mutable std::array<std::atomic<unsigned int>, kNStat> m_stat ATLAS_THREAD_SAFE {}; 
};

} // namespace ActsTrk

#endif

