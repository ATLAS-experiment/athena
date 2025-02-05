// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#include "FPGATrackSimGNNPatternRecoTool.h"

///////////////////////////////////////////////////////////////////////////////
// AthAlgTool

FPGATrackSimGNNPatternRecoTool::FPGATrackSimGNNPatternRecoTool(const std::string& algname, const std::string &name, const IInterface *ifc) :
  base_class(algname, name, ifc)
{
  declareInterface<IFPGATrackSimRoadFinderTool>(this);
}

StatusCode FPGATrackSimGNNPatternRecoTool::initialize()
{
    ATH_CHECK(m_GNNGraphHitSelectorTool.retrieve());

    return StatusCode::SUCCESS;
}

StatusCode FPGATrackSimGNNPatternRecoTool::getRoads(const std::vector<std::shared_ptr<const FPGATrackSimHit>> & hits, std::vector<std::shared_ptr<const FPGATrackSimRoad>> & roads)
{
    std::vector<std::shared_ptr<FPGATrackSimGNNHit>> graph_hits;
    std::vector<std::shared_ptr<FPGATrackSimGNNEdge>> graph_edges;
    
    ATH_CHECK(m_GNNGraphHitSelectorTool->selectHits(hits, graph_hits)); // Go from FPGATrackSimHits to FPGATrackSimGNNHit -> get information needed for GNNPipeline

    ATH_MSG_DEBUG("Size of graph_hits = " << graph_hits.size()); // Temporary to avoid warnings in build
    ATH_MSG_DEBUG("Size of graph_edges = " << graph_edges.size()); // Temporary to avoid warnings in build
    
    roads.clear(); // Temporary to avoid warnings in build
    return StatusCode::SUCCESS;
}

