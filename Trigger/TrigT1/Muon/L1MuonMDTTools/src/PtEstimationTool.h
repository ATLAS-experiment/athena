/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef L1MuonMDTTools_PTESTIMATIONTOOL_H
#define L1MuonMDTTools_PTESTIMATIONTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "L1MuonMDTTools/IPtEstimationTool.h"

namespace L0MDT {

class PtEstimationTool : public AthAlgTool, virtual public IPtEstimationTool {
public:
  using AthAlgTool::AthAlgTool;
  virtual ~PtEstimationTool() override = default;

  virtual StatusCode initialize() override;

  virtual std::optional<PtEstimate> estimatePt(const Segment* biSeg,
                                               const Segment* bmSeg,
                                               const Segment* boSeg) const override;

private:
  /// Compute a toy pT proxy from the angular deflection between two stations.
  /// @param deltaBeta  Difference in segment angle between inner and outer station [rad]
  float estimateTwoStationPt(float deltaBeta) const;

  /// Compute a toy pT proxy from the sagitta and lever arm of a three-station track.
  /// @param sagitta    Perpendicular distance of the middle station from the BI-BO line [mm]
  /// @param leverArm   Distance between inner and outer station in the (z, r) plane [mm]
  float estimateThreeStationPt(float sagitta, float leverArm) const;
};

}  // namespace L0MDT

#endif