/**
 * Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration.
 *
 * @file SinglePadClusterTool.h
 * @author Alexander Leopold <alexander.leopold@cern.ch>
 *
 * @brief Translates HGTD_RDO objects of a given detector element into
 * HGTD_PrepRawData clusters. Since currently no or only very small crosstalk
 * between neighbouring pixels of LGAD sensors is expected, a single RDO gets
 * translated directly into a HGTD_Cluster.
 *
 */

#ifndef HGTD_SINGLEPADCLUSTERTOOL_H
#define HGTD_SINGLEPADCLUSTERTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "HGTD_RecToolInterfaces/IHGTD_PadClusterizationTool.h"

#include "HGTD_ClusterMakerTool.h"

#include "HGTD_ReadoutGeometry/HGTD_DetectorManager.h"
#include "HGTD_Calibration/HGTD_TdcCalibrationTool.h"

#include <memory>

namespace HGTD {

class SinglePadClusterTool : public extends<AthAlgTool, IHGTD_PadClusterizationTool> {

public:
  SinglePadClusterTool(const std::string& type, const std::string& name,
                       const IInterface* parent);

  virtual StatusCode initialize() override final;

  virtual std::unique_ptr<HGTD_ClusterCollection>
  clusterize(const HGTD_RDO_Collection& rdo_coll,
             DataPool<HGTD_Cluster>* dataItemsPool) const override final;

  virtual std::unique_ptr<HGTD_ClusterCollection>
  clusterize(const HGTD_ALTIROC_RDO_Collection& rdo_coll,
            DataPool<HGTD_Cluster>* dataItemsPool) const override final;

private:
  ToolHandle<HGTD_ClusterMakerTool> m_cluster_maker{this, "ClusterMakerTool", "HGTD_ClusterMakerTool", "Tool for creating HGTD_Cluster objects"};
  ToolHandle<HGTD_TdcCalibrationTool> m_hgtd_tdc_calib_tool{this, "HGTD_TdcCalibrationTool","HGTD_TdcCalibrationTool", "Tool that that access TOA TDC calibration and retrieves time of arrival"};

  BooleanProperty m_use_altiroc_rdo{this, "useALTIROC_RDO", false, "Use Altiroc RDO instead of standard"};

  const HGTD_DetectorManager* m_hgtd_det_mgr{nullptr};
};

} // namespace HGTD

#endif // HGTD_SINGLEPADCLUSTERTOOL_H
