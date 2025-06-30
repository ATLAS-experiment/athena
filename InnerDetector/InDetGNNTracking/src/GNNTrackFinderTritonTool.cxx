/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "GNNTrackFinderTritonTool.h"

#include "ExaTrkXUtils.hpp"
#include "CxxUtils/StringUtils.h"

// Framework include(s).
#include <cmath>

#include "PathResolver/PathResolver.h"

StatusCode InDet::GNNTrackFinderTritonTool::initialize() {
  ATH_CHECK(m_gnnTrackingTritonTool.retrieve());
  ATH_CHECK(m_spacepointFeatureTool.retrieve());

  // tokenize the feature names by comma and push to the vector
  m_featureNamesVec = CxxUtils::tokenize(m_featureNames, ",");
  return StatusCode::SUCCESS;
}

StatusCode InDet::GNNTrackFinderTritonTool::getTracks(
    const std::vector<const Trk::SpacePoint*>& spacepoints,
    std::vector<std::vector<uint32_t> >& tracks) const {
  int64_t numSpacepoints = (int64_t)spacepoints.size();
  std::vector<float> inputValues;
  std::vector<uint32_t> spacepointIDs;

  int64_t spacepointFeatures = m_featureNamesVec.size();
  int sp_idx = 0;
  for (const auto& sp : spacepoints) {
    // depending on the trained embedding and GNN models, the input features
    // may need to be updated.
    auto featureMap = m_spacepointFeatureTool->getFeatures(sp);
    for (int i = 0; i < spacepointFeatures; i++){
      inputValues.push_back(featureMap[m_featureNamesVec[i]]);
    }

    spacepointIDs.push_back(sp_idx++);
  }

  AthInfer::InputDataMap inputData;
  inputData["FEATURES"] = std::make_pair(
      std::vector<int64_t>{numSpacepoints, spacepointFeatures}, std::move(inputValues));

  AthInfer::OutputDataMap outputData;
  outputData["LABELS"] = std::make_pair(std::vector<int64_t>{numSpacepoints, 1}, std::vector<int64_t>{});

  ATH_CHECK(m_gnnTrackingTritonTool->inference(inputData, outputData));

  auto& trackLabels = std::get<std::vector<int64_t>>(outputData["LABELS"].second);
  if (trackLabels.size() == 0){
    ATH_MSG_DEBUG("No tracks found in the event.");
    return StatusCode::SUCCESS;
  }

  tracks.clear();
  std::vector<uint32_t> this_track;
  for (auto label : trackLabels) {
    if (label == -1) {
      if (this_track.size() > 0) {
        tracks.push_back(this_track);
        this_track.clear();
      }
    } else {
      this_track.push_back(label);
    }
  }

  return StatusCode::SUCCESS;
}

MsgStream&  InDet::GNNTrackFinderTritonTool::dump( MsgStream& out ) const
{
  out<<std::endl;
  return dumpevent(out);
}

std::ostream& InDet::GNNTrackFinderTritonTool::dump( std::ostream& out ) const
{
  return out;
}

MsgStream& InDet::GNNTrackFinderTritonTool::dumpevent( MsgStream& out ) const
{
  out<<"|---------------------------------------------------------------------|"
       <<std::endl;
  out<<"| Number output tracks    | "<<std::setw(12)
     <<"                              |"<<std::endl;
  out<<"|---------------------------------------------------------------------|"
     <<std::endl;
  return out;
}
