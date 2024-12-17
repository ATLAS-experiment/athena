/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef SiGNNTrackFinder_H
#define SiGNNTrackFinder_H

// System include(s).
#include <list>
#include <iostream>
#include <memory>

#include "AthenaBaseComps/AthAlgTool.h"
#include "InDetRecToolInterfaces/IGNNTrackFinder.h"
#include "ISpacepointFeatureTool.h"

// ONNX Runtime include(s).
#include "AthOnnxInterfaces/IOnnxRuntimeInferenceTool.h"
#include <onnxruntime_cxx_api.h>

class MsgStream;

namespace InDet{
  /**
   * @class InDet::SiGNNTrackFinderTool
   * @brief InDet::SiGNNTrackFinderTool is a tool that produces track candidates
   * with graph neural networks-based pipeline using 3D space points as inputs.
   * @author xiangyang.ju@cern.ch
   */

  class SiGNNTrackFinderTool: public extends<AthAlgTool, IGNNTrackFinder>
  {
    public:
    SiGNNTrackFinderTool(const std::string& type, const std::string& name, const IInterface* parent);
    virtual StatusCode initialize() override;

    ///////////////////////////////////////////////////////////////////
    // Main methods for local track finding asked by the ISiMLTrackFinder
    ///////////////////////////////////////////////////////////////////

    /**
     * @brief Get track candidates from a list of space points.
     * @param spacepoints a list of spacepoints as inputs to the GNN-based track finder.
     * @param tracks a list of track candidates.
     *
     * @return
     */
    virtual StatusCode getTracks(
      const std::vector<const Trk::SpacePoint*>& spacepoints,
      std::vector<std::vector<uint32_t> >& tracks) const override;

    ///////////////////////////////////////////////////////////////////
    // Print internal tool parameters and status
    ///////////////////////////////////////////////////////////////////
    virtual MsgStream&    dump(MsgStream&    out) const override;
    virtual std::ostream& dump(std::ostream& out) const override;

    protected:

    SiGNNTrackFinderTool() = delete;
    SiGNNTrackFinderTool(const SiGNNTrackFinderTool&) =delete;
    SiGNNTrackFinderTool &operator=(const SiGNNTrackFinderTool&) = delete;

    /// @name Exa.TrkX pipeline configurations, which will not be changed after construction
    StringProperty m_inputMLModuleDir{this, "inputMLModelDir", ""};
    UnsignedIntegerProperty m_embeddingDim{this, "embeddingDim", 8};
    FloatProperty m_rVal{this, "rVal", 0.12};
    UnsignedIntegerProperty m_knnVal{this, "knnVal", 1000};
    FloatProperty m_filterCut{this, "filterCut", 0.05};
    FloatProperty m_ccCut{this, "ccCut", 0.01};
    FloatProperty m_walkMin{this, "walkMin", 0.1};
    FloatProperty m_walkMax{this, "walkMax", 0.6};

    StringProperty m_embeddingFeatureNames{
      this, "EmbeddingFeatureNames",
      "r, phi, z, cluster_x_1, cluster_y_1, cluster_z_1, cluster_x_2, cluster_y_2, cluster_z_2, count_1, charge_count_1, loc_eta_1, loc_phi_1, localDir0_1, localDir1_1, localDir2_1, lengthDir0_1, lengthDir1_1, lengthDir2_1, glob_eta_1, glob_phi_1, eta_angle_1, phi_angle_1, count_2, charge_count_2, loc_eta_2, loc_phi_2, localDir0_2, localDir1_2, localDir2_2, lengthDir0_2, lengthDir1_2, lengthDir2_2, glob_eta_2, glob_phi_2, eta_angle_2, phi_angle_2",
      "Feature names for the Embedding model"};
    StringProperty m_embeddingFeatureScales{
      this, "EmbeddingFeatureScales",
      "1000, 3.14, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1, 1, 3.14, 3.14, 1, 1, 1, 1, 1, 1, 3.14, 3.14, 3.14, 3.14, 1, 1, 3.14, 3.14, 1, 1, 1, 1, 1, 1, 3.14, 3.14, 3.14, 3.14",
      "Feature scales for the Embedding model"};
  
    StringProperty m_filterFeatureNames{
      this, "FilterFeatureNames",
      "r, phi, z, cluster_x_1, cluster_y_1, cluster_z_1, cluster_x_2, cluster_y_2, cluster_z_2, count_1, charge_count_1, loc_eta_1, loc_phi_1, localDir0_1, localDir1_1, localDir2_1, lengthDir0_1, lengthDir1_1, lengthDir2_1, glob_eta_1, glob_phi_1, eta_angle_1, phi_angle_1, count_2, charge_count_2, loc_eta_2, loc_phi_2, localDir0_2, localDir1_2, localDir2_2, lengthDir0_2, lengthDir1_2, lengthDir2_2, glob_eta_2, glob_phi_2, eta_angle_2, phi_angle_2",
      "Feature names for the Filtering model"};
    StringProperty m_filterFeatureScales{
      this, "FilterFeatureScales",
      "1000, 3.14, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1, 1, 3.14, 3.14, 1, 1, 1, 1, 1, 1, 3.14, 3.14, 3.14, 3.14, 1, 1, 3.14, 3.14, 1, 1, 1, 1, 1, 1, 3.14, 3.14, 3.14, 3.14",
      "Feature scales for the Filtering model"};
    
    StringProperty m_gnnFeatureNames{
      this, "GNNFeatureNames",
      "r, phi, z, eta, cluster_r_1, cluster_phi_1, cluster_z_1, cluster_eta_1, cluster_r_2, cluster_phi_2, cluster_z_2, cluster_eta_2",
      "Feature names for the GNN model"};
    StringProperty m_gnnFeatureScales{
      this, "GNNFeatureScales",
      "1000.0, 3.14159265359, 1000.0, 1.0, 1000.0, 3.14159265359, 1000.0, 1.0, 1000.0, 3.14159265359, 1000.0, 1.0",
      "Feature scales for the GNN model"};

    void initTrainedModels();
    MsgStream&    dumpevent     (MsgStream&    out) const;

    private:
    ToolHandle< AthOnnx::IOnnxRuntimeInferenceTool > m_embedSessionTool {
      this, "Embedding", "AthOnnx::OnnxRuntimeInferenceTool"
    };
    ToolHandle< AthOnnx::IOnnxRuntimeInferenceTool > m_filterSessionTool {
      this, "Filtering", "AthOnnx::OnnxRuntimeInferenceTool"
    };
    ToolHandle< AthOnnx::IOnnxRuntimeInferenceTool > m_gnnSessionTool {
      this, "GNN", "AthOnnx::OnnxRuntimeInferenceTool"
    };
    ToolHandle<ISpacepointFeatureTool> m_spacepointFeatureTool{
      this, "SpacepointFeatureTool", "InDet::SpacepointFeatureTool"};

    std::vector<std::string> m_embeddingFeatureNamesVec;
    std::vector<float> m_embeddingFeatureScalesVec;
    std::vector<std::string> m_filterFeatureNamesVec;
    std::vector<float> m_filterFeatureScalesVec;
    std::vector<std::string> m_gnnFeatureNamesVec;
    std::vector<float> m_gnnFeatureScalesVec;

  };

  MsgStream&    operator << (MsgStream&   ,const SiGNNTrackFinderTool&);
  std::ostream& operator << (std::ostream&,const SiGNNTrackFinderTool&);

}

#endif
