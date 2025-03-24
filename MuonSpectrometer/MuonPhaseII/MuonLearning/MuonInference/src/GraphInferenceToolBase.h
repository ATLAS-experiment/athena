/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONINFERENCETOOLS_GRAPHINFERENCETOOL_H
#define MUONINFERENCETOOLS_GRAPHINFERENCETOOL_H

#include "MuonInferenceInterfaces/IGraphInferenceTool.h"
#include "MuonInferenceInterfaces/NodeFeatureList.h"
#include "MuonInferenceInterfaces/GraphData.h"


#include "MuonSpacePoint/SpacePointContainer.h"

#include "AthenaBaseComps/AthAlgTool.h"
#include "StoreGate/ReadHandleKey.h"
#include <onnxruntime_cxx_api.h> // is this somewhere else?
#include "nlohmann/json.hpp"

namespace MuonML{
    /** @brief Baseline tool to handle the  */
    class GraphInferenceToolBase : public extends<AthAlgTool, IGraphInferenceTool> {
        public:
            /** @brief Keep the constructor of the parent class */
            using base_class::base_class;
            /** @brief Fill up the GraphRawData and construct the graph for the ML inference with
             *         ONNX. If the graph has been built by another inference tool and would be the 
             *         same than this one the rebuild is skipped 
             *  @param ctx: EventContext to access the space ponit container from StoreGate
             *  @param graphData: Rerference to the data object to be filled. */

            StatusCode buildGraph(const EventContext& ctx,
                                    GraphRawData& graphData) const;

            StatusCode runInference(GraphRawData& graphData) const;
        

        protected:
            StatusCode setupModel();
    
            const Ort::Session* model() const;
            /** @brief Input space points to filter  */
            SG::ReadHandleKey<MuonR4::SpacePointContainer> m_readKey{this, "ReadSpacePoints", "MuonSpacePoints"};
        private:
            /** @brief Location of the model file */
            Gaudi::Property<std::string> m_modelPath{this, "ModelPath", ""};
            /** @brief List of features to be used for the inference */
            NodeFeatureList m_graphFeatures{};
            /** @brief Pointer to the ONNX runtime environment */
            std::unique_ptr<Ort::Env> m_env{};
            /** @brief Pointer to the ONNX session options */
            std::unique_ptr<Ort::SessionOptions> m_session_options{};
            /** @brief Pointer to the ONNX model session */
            std::unique_ptr<Ort::Session> m_model{};

    }; 

}

#endif