/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_HGTDCLUSTERINGTOOLBASE_H
#define ACTSTRK_HGTDCLUSTERINGTOOLBASE_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "ActsToolInterfaces/IHGTDClusteringTool.h"

#include "HGTD_Identifier/HGTD_ID.h"
#include "HGTD_ReadoutGeometry/HGTD_DetectorManager.h"
#include "HGTD_Calibration/HGTD_TdcCalibrationTool.h"

#include "details/CellContainer.h"
#include "details/CellContainerProxy.h"

#include "Acts/Definitions/Units.hpp"

namespace ActsTrk {
class HgtdAuxDataCache;

template <typename T_RDOCollection>
struct HgtdCollectionAdapter;

class HgtdClusteringToolBase :
    public extends<AthAlgTool, IHGTDClusteringTool> {
public:

    HgtdClusteringToolBase(const std::string& type,
                           const std::string& name,
                           const IInterface* parent);

    virtual StatusCode initialize() override;

    virtual std::pair<unsigned int, unsigned int>
    countCells(const RDOContainerVariant& rdoContainer,
               const std::vector<IdentifierHash> &listOfIds) const override;
       
    virtual std::any createEventDataCache(xAOD::HGTDClusterContainer& cont,
                                          std::size_t nClusterRDOs) const override;

    virtual StatusCode makeClusters(const EventContext& ctx,
                                    const RDOContainerVariant& rdoContainer,
                                    const CellContainer& cellContainer,
                                    unsigned int imodule,
                                    unsigned int icluster,
                                    xAOD::HGTDClusterContainer& container,
                                    std::any& cache) const;
protected:
    using ClusterProxy = InPlaceClusterization::ClusterProxy<const IHGTDClusteringTool::CellContainer>;

    template <typename T_RDOCollection>
    StatusCode makeCluster(size_t icluster,
                           xAOD::HGTDCluster& xaodcluster,
                           const HgtdClusteringToolBase::ClusterProxy &clusterProxy,
                           const T_RDOCollection *RDOs,
                           const HgtdCollectionAdapter<T_RDOCollection> &rdoAdapter,
                           HgtdAuxDataCache* auxDataCache) const;
  
    const HGTD_DetectorManager* m_hgtd_det_mgr{nullptr};
    const HGTD_ID* m_hgtd_id{nullptr};
    ToolHandle<HGTD_TdcCalibrationTool> m_hgtd_tdc_calib_tool{this, 
      "HGTD_TdcCalibrationTool","HGTD_TdcCalibrationTool", 
      "Tool that that access TOA TDC calibration and retrieves time of arrival"};

  BooleanProperty m_use_altiroc_rdo{this, "useALTIROC_RDO", false, "Use Altiroc RDO instead of standard"};

};

}

#endif

