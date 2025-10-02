// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#include "FPGATrackSimGNNPatternRecoTool.h"


StatusCode FPGATrackSimGNNPatternRecoTool::initialize()
{
    ATH_CHECK(m_GNNGraphHitSelectorTool.retrieve());
    ATH_CHECK(m_GNNGraphConstructionTool.retrieve());
    ATH_CHECK(m_GNNEdgeClassifierTools.retrieve());
    ATH_CHECK(m_GNNRoadMakerTool.retrieve());
    ATH_CHECK(m_GNNRootOutputTool.retrieve());

    // Need to selected the right tool based on region
    for (auto& tool : m_GNNEdgeClassifierTools) {
        if (tool->regionNum() == m_regionNum) {
            m_selectedGNNEdgeClassifierTool = tool;
            break;
        }
    }
    if (!m_selectedGNNEdgeClassifierTool.isValid()) {
        ATH_MSG_ERROR("No GNNEdgeClassifierTool found for region " << m_regionNum);
        return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
}

StatusCode FPGATrackSimGNNPatternRecoTool::getRoads(const std::vector<std::shared_ptr<const FPGATrackSimHit>> & hits, std::vector<std::shared_ptr<const FPGATrackSimRoad>> & roads)
{
    std::vector<std::shared_ptr<FPGATrackSimGNNHit>> graph_hits;
    std::vector<std::shared_ptr<FPGATrackSimGNNEdge>> graph_edges;
    
    ATH_CHECK(m_GNNGraphHitSelectorTool->selectHits(hits, graph_hits)); // Go from FPGATrackSimHits to FPGATrackSimGNNHit -> get information needed for GNNPipeline
    ATH_CHECK(m_GNNGraphConstructionTool->getEdges(graph_hits, graph_edges)); // Build edges using module map (or metric learning)
    ATH_CHECK(m_selectedGNNEdgeClassifierTool->scoreEdges(graph_hits, graph_edges)); // Score edges using IN GNN
    ATH_CHECK(m_GNNRoadMakerTool->makeRoads(hits, graph_hits, graph_edges, roads)); // Build road candidates using connected components (need to make a C++ version of it)
    if(m_doGNNRootOutput) ATH_CHECK(m_GNNRootOutputTool->fillTree(hits, graph_hits, graph_edges, roads)); // Output the hit/edge/road information into a ROOT file
    
    graph_hits.clear();
    graph_edges.clear();
    return StatusCode::SUCCESS;
}

