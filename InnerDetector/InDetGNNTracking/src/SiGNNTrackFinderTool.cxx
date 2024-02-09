/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "SiGNNTrackFinderTool.h"
#include "ExaTrkXUtils.hpp"

// Framework include(s).
#include "PathResolver/PathResolver.h"
#include "AthOnnxUtils/OnnxUtils.h"
#include <cmath>

InDet::SiGNNTrackFinderTool::SiGNNTrackFinderTool(
  const std::string& type, const std::string& name, const IInterface* parent):
    base_class(type, name, parent)
  {
    declareInterface<IGNNTrackFinder>(this);
  }

StatusCode InDet::SiGNNTrackFinderTool::initialize() {
  ATH_CHECK( m_embedSessionTool.retrieve() );
  ATH_CHECK( m_filterSessionTool.retrieve() );
  ATH_CHECK( m_gnnSessionTool.retrieve() );
  return StatusCode::SUCCESS;
}

StatusCode InDet::SiGNNTrackFinderTool::finalize() {
  StatusCode sc = AlgTool::finalize();
  return sc;
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
  std::vector<float> inputValues;
  std::vector<uint32_t> spacepointIDs;

  int64_t spacepointFeatures = 3;
  int sp_idx = 0;
  for(const auto& sp: spacepoints){
    // depending on the trained embedding and GNN models, the input features
    // may need to be updated.

    float z = sp->globalPosition().z() / 1000.;
    float r = sp->r() / 1000.;
    float phi = sp->phi() / M_PI;
    inputValues.push_back(r);
    inputValues.push_back(phi);
    inputValues.push_back(z);


    spacepointIDs.push_back(sp_idx++);
  }

    // ************
    // Embedding
    // ************

    std::vector<int64_t> eInputShape{numSpacepoints, spacepointFeatures};
    std::vector<Ort::Value> eInputTensor;
    ATH_CHECK( m_embedSessionTool->addInput(eInputTensor, inputValues, 0, numSpacepoints) );

    std::vector<Ort::Value> eOutputTensor;
    std::vector<float> eOutputData;
    ATH_CHECK( m_embedSessionTool->addOutput(eOutputTensor, eOutputData, 0, numSpacepoints) );

    ATH_CHECK( m_embedSessionTool->inference(eInputTensor, eOutputTensor) );

    // ************
    // Building Edges
    // ************
    std::vector<int64_t> edgeList;
    buildEdges(eOutputData, edgeList, numSpacepoints, m_embeddingDim, m_rVal, m_knnVal);
    int64_t numEdges = edgeList.size() / 2;

    // ************
    // Filtering
    // ************
    std::vector<Ort::Value> fInputTensor;
    fInputTensor.push_back(
        std::move(eInputTensor[0])
    );
    ATH_CHECK( m_filterSessionTool->addInput(fInputTensor, edgeList, 1, numEdges) );
    std::vector<int64_t> fEdgeShape{2, numEdges};

    std::vector<float> fOutputData;
    std::vector<Ort::Value> fOutputTensor;
    ATH_CHECK( m_filterSessionTool->addOutput(fOutputTensor, fOutputData, 0, numEdges) );

    ATH_CHECK( m_filterSessionTool->inference(fInputTensor, fOutputTensor) );

    // apply sigmoid to the filtering output data
    // and remove edges with score < filterCut
    std::vector<int64_t> rowIndices;
    std::vector<int64_t> colIndices;
    for (int64_t i = 0; i < numEdges; i++){
        float v = 1.f / (1.f + std::exp(-fOutputData[i]));  // sigmoid, float type
        if (v > m_filterCut){
            rowIndices.push_back(edgeList[i]);
            colIndices.push_back(edgeList[numEdges + i]);
        };
    };
    std::vector<int64_t> edgesAfterFiltering;
    edgesAfterFiltering.insert(edgesAfterFiltering.end(), rowIndices.begin(), rowIndices.end());
    edgesAfterFiltering.insert(edgesAfterFiltering.end(), colIndices.begin(), colIndices.end());

    int64_t numEdgesAfterF = edgesAfterFiltering.size() / 2;

    // ************
    // GNN
    // ************
    std::vector<Ort::Value> gInputTensor;
    gInputTensor.push_back(
        std::move(fInputTensor[0])
    );
    ATH_CHECK( m_gnnSessionTool->addInput(gInputTensor, edgesAfterFiltering, 1, numEdgesAfterF) );
    
    // gnn outputs
    std::vector<float> gOutputData;
    std::vector<Ort::Value> gOutputTensor;
    ATH_CHECK( m_gnnSessionTool->addOutput(gOutputTensor, gOutputData, 0, numEdgesAfterF) );

    ATH_CHECK( m_gnnSessionTool->inference(gInputTensor, gOutputTensor) );
    // apply sigmoid to the gnn output data
    for(auto& v : gOutputData){
        v = 1.f / (1.f + std::exp(-v));
    };

    // ************
    // Track Labeling with cugraph::connected_components
    // ************
    std::vector<int32_t> trackLabels(numSpacepoints);
    weaklyConnectedComponents<int64_t,float,int32_t>(numSpacepoints, rowIndices, colIndices, gOutputData, trackLabels);

    if (trackLabels.size() == 0)  return StatusCode::SUCCESS;

    tracks.clear();

    int existTrkIdx = 0;
    // map labeling from MCC to customized track id.
    std::map<int32_t, int32_t> trackLableToIds;

    for(int32_t idx=0; idx < numSpacepoints; ++idx) {
        int32_t trackLabel = trackLabels[idx];
        uint32_t spacepointID = spacepointIDs[idx];

        int trkId;
        if(trackLableToIds.find(trackLabel) != trackLableToIds.end()) {
            trkId = trackLableToIds[trackLabel];
            tracks[trkId].push_back(spacepointID);
        } else {
            // a new track, assign the track id
            // and create a vector
            trkId = existTrkIdx;
            tracks.push_back(std::vector<uint32_t>{spacepointID});
            trackLableToIds[trackLabel] = trkId;
            existTrkIdx++;
        }
    }
    return StatusCode::SUCCESS;
}

