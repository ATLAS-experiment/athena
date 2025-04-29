
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#include "FPGATrackSimGNNRoadMakerTool.h"
#include "FPGATrackSimMaps/FPGATrackSimPlaneMap.h"

///////////////////////////////////////////////////////////////////////////////
// AthAlgTool

FPGATrackSimGNNRoadMakerTool::FPGATrackSimGNNRoadMakerTool(const std::string& algname, const std::string &name, const IInterface *ifc) 
    : AthAlgTool(algname, name, ifc) {}

StatusCode FPGATrackSimGNNRoadMakerTool::initialize()
{
    ATH_CHECK(m_FPGATrackSimMapping.retrieve());
    m_nLayers = m_FPGATrackSimMapping->PlaneMap_1st(0)->getNLogiLayers();
    
    return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////////////
// Functions

StatusCode FPGATrackSimGNNRoadMakerTool::makeRoads(const std::vector<std::shared_ptr<const FPGATrackSimHit>> & hits, const std::vector<std::shared_ptr<FPGATrackSimGNNHit>> & gnn_hits, const std::vector<std::shared_ptr<FPGATrackSimGNNEdge>> & edges, std::vector<std::shared_ptr<const FPGATrackSimRoad>> & roads)
{
    m_num_nodes = gnn_hits.size();
    doScoreCut(edges);

    if(m_roadMakerTool == "ConnectedComponents") {
        doConnectedComponents();
        addRoads(hits, gnn_hits, roads);
    }

    resetVectors();

    return StatusCode::SUCCESS;
}

void FPGATrackSimGNNRoadMakerTool::doScoreCut(const std::vector<std::shared_ptr<FPGATrackSimGNNEdge>> & edges)
{
    for (const auto& edge : edges) {
        if(edge->getEdgeScore() > m_edgeScoreCut) {
            m_pass_edge_index_1.push_back(edge->getEdgeIndex1());
            m_pass_edge_index_2.push_back(edge->getEdgeIndex2());
        }
    }
}

void FPGATrackSimGNNRoadMakerTool::doConnectedComponents()
{
    // Remove isolated nodes from list of nodes using list of edges and indices
    m_unique_nodes.insert(m_pass_edge_index_1.begin(), m_pass_edge_index_1.end());
    m_unique_nodes.insert(m_pass_edge_index_2.begin(), m_pass_edge_index_2.end());

    int index = 0;
    for (int node : m_unique_nodes) {
        m_node_index_map[node] = index++;  // Mapping original node index to new graph index
    }

    for (const auto& entry : m_node_index_map) {
        m_unique_indices.push_back(entry.first);  // Push the original node index into unique_indices
    }

    m_Graph g(m_unique_nodes.size());

    for (size_t i = 0; i < m_pass_edge_index_1.size(); i++) {
        int u = m_node_index_map[m_pass_edge_index_1[i]];
        int v = m_node_index_map[m_pass_edge_index_2[i]];
        add_edge(u, v, g);  // Add the edge between the mapped node indices
    }
    
    m_component.resize(num_vertices(g), -1);
    m_num_components = boost::connected_components(g, &m_component[0]);

    m_labels.resize(m_num_nodes,-1);

    for (size_t i = 0; i < m_unique_indices.size(); i++) {
        m_labels[m_unique_indices[i]] = m_component[i];
    }
}

void FPGATrackSimGNNRoadMakerTool::addRoads(const std::vector<std::shared_ptr<const FPGATrackSimHit>> & hits, 
                                            const std::vector<std::shared_ptr<FPGATrackSimGNNHit>> & gnn_hits, 
                                            std::vector<std::shared_ptr<const FPGATrackSimRoad>> & roads)
{
    roads.clear();
    m_roads.clear();

    m_road_hit_list.resize(m_num_components); 
    for (size_t i = 0; i < m_labels.size(); i++) {
        if(m_labels[i] != -1) {
            m_road_hit_list[m_labels[i]].push_back(gnn_hits[i]->getHitID());
        }
    }
    
    for (const auto& hit_list : m_road_hit_list) {
        // Temporarily do not make a road if it has more than 20 hits, there is an issue with some big roads which we do not want to work with right now.
        if (hit_list.size() < 20) {
            addRoad(hits, hit_list);
        }
    }

    roads.reserve(m_roads.size());
    for (const FPGATrackSimRoad & r : m_roads) roads.emplace_back(std::make_shared<const FPGATrackSimRoad>(r));
}

void FPGATrackSimGNNRoadMakerTool::addRoad(const std::vector<std::shared_ptr<const FPGATrackSimHit>> & hits, const std::vector<int>& road_hitIDs)
{
    // Take the HitID values and find the correct hit from FPGATrackSimHit, then insert it into a vector of mapped FPGATrackSimHit which are needed for the FPGATrackSimRoad
    std::vector<std::shared_ptr<const FPGATrackSimHit>> mapped_road_hits;
    const FPGATrackSimPlaneMap *pmap = m_FPGATrackSimMapping->PlaneMap_1st(0);
    layer_bitmask_t hitLayers = 0;

    for (const auto& hitID : road_hitIDs) {
        if(hitID+1 < static_cast<int>(hits.size()) && hits[hitID]->isStrip() && hits[hitID+1]->isStrip() &&
           to_string(hits[hitID]->getHitType()) == "spacepoint" && to_string(hits[hitID+1]->getHitType()) == "spacepoint" &&
           hits[hitID]->getX() == hits[hitID+1]->getX()) {
            auto &hit1 = hits[hitID];
            std::shared_ptr<FPGATrackSimHit> hitCopy1 = std::make_shared<FPGATrackSimHit>(*hit1);
            pmap->map(*hitCopy1);
            hitLayers |= 1 << hitCopy1->getLayer();
            mapped_road_hits.push_back(std::move(hitCopy1));
            //
            auto &hit2 = hits[hitID+1];
            std::shared_ptr<FPGATrackSimHit> hitCopy2 = std::make_shared<FPGATrackSimHit>(*hit2);
            pmap->map(*hitCopy2);
            hitLayers |= 1 << hitCopy2->getLayer();
            mapped_road_hits.push_back(std::move(hitCopy2));
        }
        else {
            auto &hit = hits[hitID];
            std::shared_ptr<FPGATrackSimHit> hitCopy = std::make_shared<FPGATrackSimHit>(*hit);
            pmap->map(*hitCopy);
            hitLayers |= 1 << hitCopy->getLayer();
            mapped_road_hits.push_back(std::move(hitCopy));
        }
    }
    
    m_roads.emplace_back();
    FPGATrackSimRoad & r = m_roads.back();
    auto sorted_hits = ::sortByLayer(mapped_road_hits);
    sorted_hits.resize(m_nLayers);
    r.setRoadID(m_roads.size() - 1);
    r.setHitLayers(hitLayers);
    r.setHits(std::vector<std::vector<std::shared_ptr<const FPGATrackSimHit>>>(std::move(sorted_hits)));
    r.setSubRegion(0);
}

void FPGATrackSimGNNRoadMakerTool::resetVectors()
{
    m_pass_edge_index_1.clear();
    m_pass_edge_index_2.clear();
    m_unique_nodes.clear();
    m_node_index_map.clear();
    m_unique_indices.clear();
    m_component.clear();
    m_labels.clear();
    m_road_hit_list.clear();
}