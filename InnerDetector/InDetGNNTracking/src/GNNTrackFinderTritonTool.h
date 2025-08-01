/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#pragma once

// System include(s).
#include <iostream>
#include <list>
#include <memory>

#include "AthOnnxInterfaces/IAthInferenceTool.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "ISpacepointFeatureTool.h"
#include "InDetRecToolInterfaces/IGNNTrackFinder.h"

class MsgStream;

namespace InDet {
/**
 * @class InDet::GNNTrackFinderTritonTool
 * @brief InDet::GNNTrackFinderTritonTool is a tool that produces track
 * candidates with graph neural networks-based pipeline using space points as
 * inputs.
 * @author xiangyang.ju@cern.ch
 */

class GNNTrackFinderTritonTool : public extends<AthAlgTool, IGNNTrackFinder> {
 public:
  using base_class::base_class;
  virtual StatusCode initialize() override;

  ///////////////////////////////////////////////////////////////////
  // Main methods for local track finding asked by the IGNNTrackFinder
  ///////////////////////////////////////////////////////////////////

  /**
   * @brief Get track candidates from a list of space points.
   * @param spacepoints a list of spacepoints as inputs to the GNN-based track
   * finder.
   * @param tracks a list of track candidates.
   *
   * @return
   */
  virtual StatusCode getTracks(
      const std::vector<const Trk::SpacePoint*>& spacepoints,
      std::vector<std::vector<uint32_t> >& tracks) const override;

  virtual MsgStream&    dump(MsgStream&    out) const override;
  virtual std::ostream& dump(std::ostream& out) const override;


 private:
  ToolHandle<AthInfer::IAthInferenceTool> m_gnnTrackingTritonTool{
      this, "TritonTool", "AthInfer::TritonTool"};
  ToolHandle<ISpacepointFeatureTool> m_spacepointFeatureTool{
      this, "SpacepointFeatureTool", "InDet::SpacepointFeatureTool"};

  StringProperty m_featureNames{this, "FeatureNames",
                                "r,phi,z,cluster_x_1,cluster_y_1,cluster_z_1,cluster_x_2,cluster_y_2,cluster_z_2,count_1,charge_count_1,loc_eta_1,loc_phi_1,localDir0_1,localDir1_1,localDir2_1,lengthDir0_1,lengthDir1_1,lengthDir2_1,glob_eta_1,glob_phi_1,eta_angle_1,phi_angle_1,count_2,charge_count_2,loc_eta_2,loc_phi_2,localDir0_2,localDir1_2,localDir2_2,lengthDir0_2,lengthDir1_2,lengthDir2_2,glob_eta_2,glob_phi_2,eta_angle_2,phi_angle_2,eta,cluster_r_1,cluster_phi_1,cluster_eta_1,cluster_r_2,cluster_phi_2,cluster_eta_2",
                                "Feature names for the GNN pipeline"};

  std::vector<std::string> m_featureNamesVec;
  MsgStream&    dumpevent     (MsgStream&    out) const;
};

MsgStream&    operator << (MsgStream&   ,const GNNTrackFinderTritonTool&);
std::ostream& operator << (std::ostream&,const GNNTrackFinderTritonTool&);

}  // namespace InDet

