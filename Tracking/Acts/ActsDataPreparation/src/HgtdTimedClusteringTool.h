/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_DATAPREPARATION_HGTD_TIMEDCLUSTERING_TOOL_H
#define ACTSTRK_DATAPREPARATION_HGTD_TIMEDCLUSTERING_TOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "ActsToolInterfaces/IHGTDClusteringTool.h"

#include "HGTD_Identifier/HGTD_ID.h"
#include "HGTD_ReadoutGeometry/HGTD_DetectorManager.h"
#include "HGTD_Calibration/HGTD_TdcCalibrationTool.h"

#include "Acts/Definitions/Units.hpp"

namespace Hgtd {
  struct UnpackedHgtdRDO {
    UnpackedHgtdRDO(int ncl, int row, int col, float toa, int tot, Identifier id)
      : NCL(ncl), ROW(row), COL(col), TOA(toa), TOT(tot), ID(id)
    {};
    
    int        NCL{0};
    int        ROW{0};
    int        COL{0};
    float      TOA{0.f};
    int        TOT{0};
    Identifier ID {};
  };
}

namespace ActsTrk {
struct HgtdAuxDataCache;

class HgtdTimedClusteringTool :
    public extends<AthAlgTool, IHGTDClusteringTool> {
public:

  using Cell = Hgtd::UnpackedHgtdRDO;
  using CellCollection = std::vector<Cell>;
  using Cluster = IHGTDClusteringTool::Cluster;
  using ClusterCollection = IHGTDClusteringTool::ClusterCollection;

public:
    HgtdTimedClusteringTool(const std::string& type,
			    const std::string& name,
			    const IInterface* parent);

    virtual StatusCode initialize() override;

    virtual StatusCode clusterize(const EventContext& ctx,
                                  const RawDataCollection& RDOs,
                                  std::vector<ClusterCollection>& collection) const override;

    virtual StatusCode clusterize(const EventContext& ctx,
                                  const HGTD_ALTIROC_RDO_Collection& RDOs,
                                  std::vector<ClusterCollection>& collection) const override;

    virtual std::any createEventDataCache(xAOD::HGTDClusterContainer& cont,
                                          std::size_t nClusterRDOs) const override;

    virtual StatusCode makeClusters(const EventContext& ctx,
                                    const ClusterCollection& clusters,
                                    xAOD::HGTDClusterContainer& container,
                                    size_t& icluster,
                                    std::any& cache) const override;

private:
  // N.B. the cluster is added to the container
  StatusCode makeCluster(const EventContext& ctx,
			 const typename HgtdTimedClusteringTool::Cluster &cluster,
			 xAOD::HGTDCluster& xaodcluster,
                         HgtdAuxDataCache*cache) const;

private:
    const HGTD_DetectorManager* m_hgtd_det_mgr{nullptr};
    const HGTD_ID* m_hgtd_id{nullptr};
    ToolHandle<HGTD_TdcCalibrationTool> m_hgtd_tdc_calib_tool{this, 
      "HGTD_TdcCalibrationTool","HGTD_TdcCalibrationTool", 
      "Tool that that access TOA TDC calibration and retrieves time of arrival"};

  BooleanProperty m_use_altiroc_rdo{this, "useALTIROC_RDO", false, "Use Altiroc RDO instead of standard"};

  Gaudi::Property<double> m_timeTollerance {this, "TimeTollerance", 0.035 * Athena::Units::nanosecond};
  Gaudi::Property<bool> m_addCorners {this, "AddCorners", true};
};

}

#endif

