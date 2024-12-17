/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "SiGNNTrackFinderTool.h"
#include "ExaTrkXUtils.hpp"

// Framework include(s).
#include "PathResolver/PathResolver.h"
#include "AthOnnxUtils/OnnxUtils.h"

#include <cmath>
#include <random>      // std::random_device, std::mt19937, std::random_shuffle

InDet::SiGNNTrackFinderTool::SiGNNTrackFinderTool(
  const std::string& type, const std::string& name, const IInterface* parent):
    base_class(type, name, parent)
  {
    declareInterface<IGNNTrackFinder>(this);
  }

StatusCode InDet::SiGNNTrackFinderTool::initialize() {
  ATH_CHECK( m_embedSessionTool.retrieve() );
  m_embedSessionTool->printModelInfo();

  ATH_CHECK( m_filterSessionTool.retrieve() );
  m_filterSessionTool->printModelInfo();

  ATH_CHECK( m_gnnSessionTool.retrieve() );
  m_gnnSessionTool->printModelInfo();

  // tokenize the feature names by comma and push to the vector
  auto split_fn = [](const std::string& s, auto convert_fn) {
      using ReturnType = std::decay_t<decltype(convert_fn(std::declval<std::string>()))>;
      std::vector<ReturnType> tokens;
      std::string token;
      std::istringstream tokenStream(s);
      while (std::getline(tokenStream, token, ',')) {
          token = token.substr(token.find_first_not_of(" "), token.find_last_not_of(" ") + 1);
          tokens.push_back(convert_fn(token));
      }
      return tokens;
  };
  auto convert_to_float = [](const std::string& s) -> float { return std::stof(s); };
  auto convert_to_str = [](const std::string& s) -> std::string { return s; };

  m_embeddingFeatureNamesVec = split_fn(m_embeddingFeatureNames, convert_to_str);
  m_embeddingFeatureScalesVec = split_fn(m_embeddingFeatureScales, convert_to_float);
  assert(m_embeddingFeatureNamesVec.size() == m_embeddingFeatureScalesVec.size());

  m_filterFeatureNamesVec = split_fn(m_filterFeatureNames, convert_to_str);
  m_filterFeatureScalesVec = split_fn(m_filterFeatureScales, convert_to_float);
  assert(m_filterFeatureNamesVec.size() == m_filterFeatureScalesVec.size());

  m_gnnFeatureNamesVec = split_fn(m_gnnFeatureNames, convert_to_str);
  m_gnnFeatureScalesVec = split_fn(m_gnnFeatureScales, convert_to_float);
  assert(m_gnnFeatureNamesVec.size() == m_gnnFeatureScalesVec.size());

  return StatusCode::SUCCESS;
}

MsgStream&  InDet::SiGNNTrackFinderTool::dump( MsgStream& out ) const
{
  out<<std::endl;
  return dumpevent(out);
}

std::ostream& InDet::SiGNNTrackFinderTool::dump( std::ostream& out ) const
{
  return out;
}

MsgStream& InDet::SiGNNTrackFinderTool::dumpevent( MsgStream& out ) const
{
  out<<"|---------------------------------------------------------------------|"
       <<std::endl;
  out<<"| Number output tracks    | "<<std::setw(12)  
     <<"                              |"<<std::endl;
  out<<"|---------------------------------------------------------------------|"
     <<std::endl;
  return out;
}

StatusCode InDet::SiGNNTrackFinderTool::getTracks(
  const std::vector<const Trk::SpacePoint*>& spacepoints,
  std::vector<std::vector<uint32_t> >& tracks) const
{
  int64_t numSpacepoints = (int64_t)spacepoints.size();
  std::vector<float> eNodeFeatures;
  std::vector<float> fNodeFeatures;
  std::vector<float> gNodeFeatures;
  std::vector<uint32_t> spacepointIDs;
  std::vector<int> regions;

  int sp_idx = 0;
  for(const auto& sp: spacepoints){
    auto featureMap = m_spacepointFeatureTool->getFeatures(sp);
    regions.push_back(featureMap["region"]);
    // fill embedding node features.
    for(size_t i = 0; i < m_embeddingFeatureNamesVec.size(); i++){
      eNodeFeatures.push_back(
        featureMap[m_embeddingFeatureNamesVec[i]] / m_embeddingFeatureScalesVec[i]);
    }

    // fill filtering node features.
    for(size_t i = 0; i < m_filterFeatureNamesVec.size(); i++){
      fNodeFeatures.push_back(
        featureMap[m_filterFeatureNamesVec[i]] / m_filterFeatureScalesVec[i]);
    }

    // fill gnn node features.
    for(size_t i = 0; i < m_gnnFeatureNamesVec.size(); i++){
      gNodeFeatures.push_back(
        featureMap[m_gnnFeatureNamesVec[i]] / m_gnnFeatureScalesVec[i]);
    }

    spacepointIDs.push_back(sp_idx++);
  }
    // ************
    // Embedding
    // ************
    std::vector<int64_t> eInputShape{numSpacepoints, (long int) m_embeddingFeatureNamesVec.size()};
    std::vector<Ort::Value> eInputTensor;
    ATH_CHECK( m_embedSessionTool->addInput(eInputTensor, eNodeFeatures, 0, numSpacepoints) );

    std::vector<Ort::Value> eOutputTensor;
    std::vector<float> eOutputData;
    ATH_CHECK( m_embedSessionTool->addOutput(eOutputTensor, eOutputData, 0, numSpacepoints) );

    ATH_CHECK( m_embedSessionTool->inference(eInputTensor, eOutputTensor) );

    // ************
    // Building Edges
    // ************
    std::vector<int64_t> senders;
    std::vector<int64_t> receivers;
    ExaTrkXUtils::buildEdges(eOutputData, senders, receivers, numSpacepoints, m_embeddingDim, m_rVal, m_knnVal);
    int64_t numEdges = senders.size();

    // clean up embedding data.
    eNodeFeatures.clear();
    eInputTensor.clear();
    eOutputData.clear();
    eOutputTensor.clear();

    // sort the edge list and remove duplicate edges.
    std::vector<std::pair<int64_t, int64_t>> edgePairs;
    for(int64_t idx = 0; idx < numEdges; idx ++ ) {
        edgePairs.push_back({senders[idx], receivers[idx]});
    }
    std::sort(edgePairs.begin(), edgePairs.end());
    edgePairs.erase(std::unique(edgePairs.begin(), edgePairs.end()), edgePairs.end());
    
    // random shuffle the edge list.
    std::random_device rd;
    std::mt19937 rdm_gen(rd());
    std::random_shuffle(edgePairs.begin(), edgePairs.end());

    // sort the edge list by the sender * numSpacepoints + receiver.
    std::sort(edgePairs.begin(), edgePairs.end(), 
        [numSpacepoints](const std::pair<int64_t, int64_t>& a, const std::pair<int64_t, int64_t>& b){
            return a.first * numSpacepoints + a.second < b.first * numSpacepoints + b.second;
    });

    // convert the edge list to senders and receivers.
    senders.clear();
    receivers.clear();
    for(const auto& edge: edgePairs){
        senders.push_back(edge.first);
        receivers.push_back(edge.second);
    }

    edgePairs.clear();

    // ************
    // Filtering
    // ************
    std::vector<Ort::Value> fInputTensor;
    ATH_CHECK( m_filterSessionTool->addInput(fInputTensor, fNodeFeatures, 0, numSpacepoints) );

    std::vector<int64_t> edgeList(numEdges * 2);
    std::copy(senders.begin(), senders.end(), edgeList.begin());
    std::copy(receivers.begin(), receivers.end(), edgeList.begin() + senders.size());

    
    ATH_CHECK( m_filterSessionTool->addInput(fInputTensor, edgeList, 1, numEdges) );

    std::vector<float> fOutputData;
    std::vector<Ort::Value> fOutputTensor;
    ATH_CHECK( m_filterSessionTool->addOutput(fOutputTensor, fOutputData, 0, numEdges) );

    ATH_CHECK( m_filterSessionTool->inference(fInputTensor, fOutputTensor) );

    // apply sigmoid to the filtering output data
    // and remove edges with score < filterCut
    // and sort the edge list so that sender idx < receiver.
    std::vector<int64_t> rowIndices;
    std::vector<int64_t> colIndices;
    for (int64_t i = 0; i < numEdges; i++){
        float v = 1.f / (1.f + std::exp(-fOutputData[i]));  // sigmoid, float type
        if (v >= m_filterCut){
            auto src = edgeList[i];
            auto dst = edgeList[numEdges + i];
            if (src > dst) {
                std::swap(src, dst);
            }
            rowIndices.push_back(src);
            colIndices.push_back(dst);
        };
    };
    int64_t numEdgesAfterF = rowIndices.size();

    // clean up filtering data.
    fNodeFeatures.clear();
    fInputTensor.clear();
    fOutputData.clear();
    fOutputTensor.clear();
    // clean up sender and receiver list.
    senders.clear();
    receivers.clear();

    std::vector<int64_t> edgesAfterFiltering(numEdgesAfterF * 2);
    std::copy(rowIndices.begin(), rowIndices.end(), edgesAfterFiltering.begin());
    std::copy(colIndices.begin(), colIndices.end(), edgesAfterFiltering.begin() + senders.size());

    // ************
    // GNN
    // ************

    // use the same features for regions (2, 6)
    for(size_t idx = 0; idx < static_cast<size_t>(numSpacepoints); idx++){
        if (regions[idx] == 2 || regions[idx] == 6){
          for(size_t i = 4; i < m_gnnFeatureNamesVec.size(); i++){
              gNodeFeatures[idx * m_gnnFeatureNamesVec.size() + i] = gNodeFeatures[idx * m_gnnFeatureNamesVec.size() + i % 4];
          }
        }
    }
  
    std::vector<Ort::Value> gInputTensor;
    ATH_CHECK( m_gnnSessionTool->addInput(gInputTensor, gNodeFeatures, 0, numSpacepoints) );
    ATH_CHECK( m_gnnSessionTool->addInput(gInputTensor, edgesAfterFiltering, 1, numEdgesAfterF) );

    // calculate the edge features.
    std::vector<float> gnnEdgeFeatures;
    ExaTrkXUtils::calculateEdgeFeatures(gNodeFeatures, numSpacepoints, rowIndices, colIndices, gnnEdgeFeatures);
    ATH_CHECK( m_gnnSessionTool->addInput(gInputTensor, gnnEdgeFeatures, 2, numEdgesAfterF) );
    
    // gnn outputs
    std::vector<float> gOutputData;
    std::vector<Ort::Value> gOutputTensor;
    ATH_CHECK( m_gnnSessionTool->addOutput(gOutputTensor, gOutputData, 0, numEdgesAfterF) );

    ATH_CHECK( m_gnnSessionTool->inference(gInputTensor, gOutputTensor) );
    // apply sigmoid to the gnn output data
    for(auto& v : gOutputData){
        v = 1.f / (1.f + std::exp(-v));
    };

    // clean up GNN data.
    gNodeFeatures.clear();
    gInputTensor.clear();
    edgesAfterFiltering.clear();

    // ************
    // Track Labeling with cugraph::connected_components
    // ************
    tracks.clear();
    ExaTrkXUtils::CCandWalk(
      numSpacepoints,
      rowIndices, colIndices, gOutputData,
      tracks, m_ccCut, m_walkMin, m_walkMax
    );

    return StatusCode::SUCCESS;
}
