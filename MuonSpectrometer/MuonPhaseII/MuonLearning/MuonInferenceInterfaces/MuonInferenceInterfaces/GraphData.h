/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONINFERENCEINTERACES_GRAPHNODE_H
#define MUONINFERENCEINTERACES_GRAPHNODE_H

#include <onnxruntime_cxx_api.h>
#include <vector>
#include <memory>

namespace MuonML{
    class NodeFeatureList;
        
    /** @brief Helper struct containing all the information needed to process  */
    struct InferenceGraph {
        /** @brief Vector of the inference input tensors */
        std::vector<Ort::Value> dataTensor{};
        /** @brief Vector of the input names. The raw char data
         *         is neither owned or managed by the object */
        std::vector<const char*> nameTensor{};
     };
    
    /** @brief Helper struct to ship the Graph from the space point buckets 
     *         to ONNX */
    struct GraphRawData {
        using FeatureVec_t = std::vector<float>;
        using NodeConnectVec_t = std::vector<int64_t>;
        using EdgeCounterVec_t = std::vector<int64_t>;
        /** @brief Vector containing all features */
        FeatureVec_t featureLeaves{};
        /** @brief Vector encoding the source index of the */
        EdgeCounterVec_t srcEdges{};
        /** @brief Vect  */
        EdgeCounterVec_t desEdges{};
        /** @brief  Vector keeping track of how many space points are in each parsed bucket */
        NodeConnectVec_t spacePointsInBucket{};
        /** @brief Packed edge index buffer (kept alive for ONNX tensors that reference it)
         *  This stores [srcEdges, dstEdges] packed together and is used as backing storage
         *  for the ONNX 'edge_index' input tensor so that the Ort::Value does not point
         *  to a local (stack) buffer that would be freed at function exit.
         */
        EdgeCounterVec_t edgeIndexPacked{};
        /** @brief Pointer to the latest parsed NodeFeatureList */
        const NodeFeatureList* previousList{};
        /** @brief Pointer to the graph to be parsed to ONNX */
        std::unique_ptr<InferenceGraph> graph{};
        
        /** @brief The following variables are needed to fill the consistently the raw data 
         *         for the Graph Building*/
        std::vector<float>::iterator currLeave{featureLeaves.begin()};
        /** @brief Number of the already filled nodes */
        unsigned int nodeIndex{0};
    };
}

#endif