/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef SiGNNTrackFinder_H
#define SiGNNTrackFinder_H

// System include(s).
#include <list>
#include <iostream>
#include <memory>

#include "AthenaBaseComps/AthAlgTool.h"
#include "InDetRecToolInterfaces/IGNNTrackFinder.h"

// ONNX Runtime include(s).
#include "AthOnnxInterfaces/IOnnxRuntimeInferenceTool.h"
#include <core/session/onnxruntime_cxx_api.h>

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
    UnsignedIntegerProperty m_embeddingDim{this, "embeddingDim", 8};
    FloatProperty m_rVal{this, "rVal", 1.7};
    UnsignedIntegerProperty m_knnVal{this, "knnVal", 500};
    FloatProperty m_filterCut{this, "filterCut", 0.21};
    StringProperty m_inputMLModuleDir{this, "inputMLModelDir", ""};
    BooleanProperty m_useCUDA {this, "UseCUDA", false, "Use CUDA"};

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

  };

  MsgStream&    operator << (MsgStream&   ,const SiGNNTrackFinderTool&);
  std::ostream& operator << (std::ostream&,const SiGNNTrackFinderTool&); 

}

#endif
