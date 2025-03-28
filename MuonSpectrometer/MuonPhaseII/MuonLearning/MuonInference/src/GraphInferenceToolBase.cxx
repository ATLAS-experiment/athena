/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "GraphInferenceToolBase.h"

#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "MuonInferenceInterfaces/GraphData.h"
#include "MuonInferenceInterfaces/NodeFeatureList.h"
#include "MuonPatternHelpers/MatrixUtils.h"
#include "PathResolver/PathResolver.h"

#include <span>

namespace {
    template <typename T>
        std::ostream& operator<<(std::ostream& ostr, const std::vector<T>& vec) {
            ostr << "[";
            for (const T& val : vec) {
                ostr << val << ", ";
            }
            ostr << "]";
            return ostr;
        }
    template <typename T1 , typename T2>
        std::ostream& operator<<(std::ostream& ostr, const std::pair<T1, T2>& pair) {
            ostr << "(" << pair.first << ", " << pair.second << ")";
            return ostr;
        }

    std::vector<std::pair<int64_t, int64_t>> makeIndexPairs(const std::vector<int64_t>& edges) {
        std::vector<std::pair<int64_t, int64_t>> indexPairs;
        const size_t nEdges = edges.size() / 2;
        indexPairs.reserve(nEdges);
        for (size_t i = 0; i < nEdges; ++i) {
            indexPairs.emplace_back(std::make_pair(edges[i], edges[i+nEdges]));
        }
        return indexPairs;
    }

    std::vector<int64_t> makeSortedEdges(const std::vector<int64_t>& edges) {
        const size_t nEdges = edges.size() / 2;
        std::vector<std::pair<int64_t, int64_t>> indexPairs;
        indexPairs.reserve(nEdges);
    
        // Create (src, dst) pairs
        for (size_t i = 0; i < nEdges; ++i) {
            indexPairs.emplace_back(edges[i], edges[i + nEdges]);
        }
    
        // Sort by src, then dst
        std::sort(indexPairs.begin(), indexPairs.end(), [](const auto& a, const auto& b) {
            return (a.first < b.first) || (a.first == b.first && a.second < b.second);
        });
    
        // Reconstruct sorted flat edges: [sorted_src..., sorted_dst...]
        std::vector<int64_t> sortedEdges;
        sortedEdges.reserve(2 * nEdges);
        for (const auto& pair : indexPairs) {
            sortedEdges.push_back(pair.first);
        }
        for (const auto& pair : indexPairs) {
            sortedEdges.push_back(pair.second);
        }
    
        return sortedEdges;
    }

    std::string formatNodeFeatures(const std::vector<float>& featureLeaves, size_t numFeaturesPerNode) {
        std::ostringstream oss;
    
        const size_t numNodes = featureLeaves.size() / numFeaturesPerNode;
        oss << "Number of nodes: " << numNodes << "\n";
        oss << "Features per node: " << numFeaturesPerNode << "\n";
    
        for (size_t nodeIdx = 0; nodeIdx < numNodes; ++nodeIdx) {
            oss << "Node[" << nodeIdx << "]: [";
            for (size_t f = 0; f < numFeaturesPerNode; ++f) {
                if (f > 0) oss << ", ";
                oss << featureLeaves[nodeIdx * numFeaturesPerNode + f];
            }
            oss << "]\n";
        }
    
        return oss.str();
    }
}
namespace MuonML{
    Ort::Session& GraphInferenceToolBase::model() const {
        return m_onnxSessionTool->session();
    }
    StatusCode GraphInferenceToolBase::setupModel() {
        ATH_CHECK(m_onnxSessionTool.retrieve());
        ATH_CHECK(m_readKey.initialize());
            
        Ort::ModelMetadata metadata = model().GetModelMetadata();
        Ort::AllocatorWithDefaultOptions allocator;
        Ort::AllocatedStringPtr feature_json_ptr = metadata.LookupCustomMetadataMapAllocated("feature_names", allocator);
            
        if (feature_json_ptr) {
            std::string feature_json = feature_json_ptr.get();
            nlohmann::json json_obj = nlohmann::json::parse(feature_json);
            for (const auto& feature : json_obj) {
                m_graphFeatures.addFeature(feature.get<std::string>(), msgStream());
            }
        }
        m_graphFeatures.setConnector("fullyConnected", msgStream());

        if (!m_graphFeatures.isValid()) {
            ATH_MSG_FATAL("No graph features have been parsed. Please check the model: "<<m_graphFeatures.featureNames());
            return StatusCode::FAILURE;
        }
        return StatusCode::SUCCESS;
    }

    StatusCode GraphInferenceToolBase::buildGraph(const EventContext& ctx,
                                                  GraphRawData& graphData) const {

        /** Check whether the graph needs a rebuild */
        if (graphData.previousList && (*graphData.previousList) != m_graphFeatures) {
            graphData.graph.reset();
        }
        /** Don't launch the rebuild of the graph */
        if (graphData.graph) {
            return StatusCode::SUCCESS;
        }        
        if (!m_graphFeatures.isValid()) {
            ATH_MSG_ERROR("The feature list is in complete. Either it has no features or no node connector set");
            return StatusCode::FAILURE;
        }
    
        graphData.graph = std::make_unique<InferenceGraph>();
    
        SG::ReadHandle spacePoints{m_readKey, ctx};
        ATH_CHECK(spacePoints.isPresent());
    
        int64_t nNodes{0}, possConn{0};
        graphData.spacePointsInBucket.clear();
        graphData.spacePointsInBucket.reserve(spacePoints->size());

        for (const MuonR4::SpacePointBucket* bucket : *spacePoints) {
            nNodes += graphData.spacePointsInBucket.emplace_back(bucket->size());
            possConn += MuonR4::sumUp(graphData.spacePointsInBucket.back());
        }

        graphData.nodeIndex = 0;
        graphData.featureLeaves.resize(nNodes * m_graphFeatures.numFeatures());
        graphData.currLeave = graphData.featureLeaves.begin();
        
        graphData.srcEdges.reserve(possConn);
        graphData.desEdges.reserve(possConn);

        /** Fill the graph edge features and all their respective connections */
        for (const MuonR4::SpacePointBucket* bucket : *spacePoints) {
            const LayerSpBucket mlBucket{*bucket};
            m_graphFeatures.fillInData(mlBucket, graphData);
        }

        graphData.srcEdges.insert(graphData.srcEdges.end(), std::make_move_iterator(graphData.desEdges.begin()),
                                    std::make_move_iterator(graphData.desEdges.end()));

        ATH_MSG_DEBUG("Features:"<<m_graphFeatures.featureNames());
        ATH_MSG_DEBUG(formatNodeFeatures(graphData.featureLeaves, m_graphFeatures.numFeatures()));
        ATH_MSG_DEBUG("Edge indices:"<<makeIndexPairs(makeSortedEdges(graphData.srcEdges)));

        std::vector<int64_t> featShape{nNodes, static_cast<int64_t>(m_graphFeatures.numFeatures())};    // (N, nFeatures)
        std::vector<int64_t> edgeShape{2, static_cast<int64_t>(graphData.srcEdges.size() / 2)};         // (2, E)

        
        Ort::MemoryInfo memInfo = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
        graphData.graph->dataTensor.emplace_back(Ort::Value::CreateTensor<float>(memInfo, 
                                                    graphData.featureLeaves.data(), graphData.featureLeaves.size(), 
                                                    featShape.data(), featShape.size()));


        Ort::Value edge_tensor = Ort::Value::CreateTensor<int64_t>(memInfo, graphData.srcEdges.data(), 
            graphData.srcEdges.size(), edgeShape.data(), edgeShape.size());

        graphData.graph->dataTensor.emplace_back(std::move(edge_tensor));

        graphData.previousList = &m_graphFeatures;

        graphData.srcEdges.clear();
        graphData.desEdges.clear();
        graphData.featureLeaves.clear();
        ATH_MSG_DEBUG("Graph data built successfully.");
        return StatusCode::SUCCESS;
    }

    StatusCode GraphInferenceToolBase::runInference(GraphRawData& graphData) const {
        if (!m_graphFeatures.isValid()) {
            ATH_MSG_ERROR("ONNX model is not loaded. Please call setupModel()");
            return StatusCode::FAILURE;
        }
        if (!graphData.graph) {
            ATH_MSG_ERROR("Graph data is not built.");
            return StatusCode::FAILURE;
        }

        if (graphData.graph->dataTensor.size() < 2) {
            ATH_MSG_ERROR("Data tensor does not contain both feature and edge tensors.");
            return StatusCode::FAILURE;
        }

        std::vector<const char*> inputNames  = {"features", "edge_index"};
        std::vector<const char*> outputNames = {"output"};

        Ort::RunOptions run_options;
        run_options.SetRunLogSeverityLevel(ORT_LOGGING_LEVEL_WARNING);

        std::vector<Ort::Value> outputTensors = model().Run(run_options, 
                                                inputNames.data(),                    // input tensor names
                                                graphData.graph->dataTensor.data(),  // pointer to the tensor vector
                                                graphData.graph->dataTensor.size(),   // size of the tensor vector
                                                outputNames.data(),                  // output tensor names
                                                outputNames.size());                 // number of output tensors
    
        if (outputTensors.empty()) {
            ATH_MSG_ERROR("Inference returned empty output.");
            return StatusCode::FAILURE;
        }     

        float* output_data = outputTensors[0].GetTensorMutableData<float>();
        size_t output_size = outputTensors[0].GetTensorTypeAndShapeInfo().GetElementCount();

        std::span<float> predictions(output_data, output_data + output_size);
        
        for (size_t i = 0; i < output_size; i++) {
            if (!std::isfinite(predictions[i])) {
                ATH_MSG_WARNING("Non-finite prediction detected! Setting to -100..");
                predictions[i] = -100.0f;
            }
        }

        graphData.graph->dataTensor.emplace_back(std::move(outputTensors[0]));

        return StatusCode::SUCCESS;
    }
    
}
