/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_DATAPREPARATION_HGTD_CLUSTERING_TOOL_H
#define ACTSTRK_DATAPREPARATION_HGTD_CLUSTERING_TOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "ActsToolInterfaces/IHGTDClusteringTool.h"

#include "HGTD_Identifier/HGTD_ID.h"
#include "HGTD_ReadoutGeometry/HGTD_DetectorManager.h"
#include "HGTD_Calibration/HGTD_TdcCalibrationTool.h"

namespace ActsTrk {

class HgtdClusteringTool : public extends<AthAlgTool, IHGTDClusteringTool> {
public:
    HgtdClusteringTool(const std::string& type,
		       const std::string& name,
		       const IInterface* parent);

    virtual StatusCode initialize() override;

    virtual StatusCode clusterize(const EventContext& ctx,
				  const RawDataCollection& RDOs,
				  ClusterContainer& container) const override;

    virtual StatusCode clusterize(const EventContext& ctx,
          const HGTD_ALTIROC_RDO_Collection& RDOs,
          ClusterContainer& container) const override;

private:
    const HGTD_DetectorManager* m_hgtd_det_mgr{nullptr};
    ToolHandle<HGTD_TdcCalibrationTool> m_hgtd_tdc_calib_tool{this, 
      "HGTD_TdcCalibrationTool","HGTD_TdcCalibrationTool", 
      "Tool that that access TOA TDC calibration and retrieves time of arrival"};
    
    BooleanProperty m_use_altiroc_rdo{this, "useALTIROC_RDO", 
      false, "Use Altiroc RDO instead of standard"};
};

}

#endif

