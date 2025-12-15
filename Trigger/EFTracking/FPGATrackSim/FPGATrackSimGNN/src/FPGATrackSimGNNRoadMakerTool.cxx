
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
    ATH_CHECK(m_layerNumberTool.retrieve());
    m_pix_h2l = m_layerNumberTool->pixelLayers();
    m_layerGeometry = m_layerNumberTool->layerGeometry();

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
    } else if (m_roadMakerTool == "JunctionAwareCC"){
        doJunctionAwareCC();
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
    reorderIndices();
    boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS> g(m_unique_nodes.size());

    for (size_t i = 0; i < m_pass_edge_index_1.size(); i++) {
        int u = m_node_index_map[m_pass_edge_index_1[i]];
        int v = m_node_index_map[m_pass_edge_index_2[i]];
        add_edge(u, v, g);  // Add the edge between the mapped node indices
    }

    std::vector<int> components(num_vertices(g), -1);
    m_component.resize(num_vertices(g), std::vector<int>(1,-1));
    m_num_components = boost::connected_components(g, &components[0]);

    for (size_t i = 0; i != components.size(); ++i){
        m_component[i][0] = components[i];
    }

    m_labels.resize(m_num_nodes, std::vector<int>(1,-1));

    for (size_t i = 0; i < m_unique_indices.size(); i++) {
        m_labels[m_unique_indices[i]][0] = m_component[i][0];
    }

}

void FPGATrackSimGNNRoadMakerTool::doJunctionAwareCC()
{
    reorderIndices();
    boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS> g(m_unique_nodes.size());

    for (size_t i = 0; i < m_pass_edge_index_1.size(); i++) {
        int u = m_node_index_map[m_pass_edge_index_1[i]];
        int v = m_node_index_map[m_pass_edge_index_2[i]];
        add_edge(u, v, g);  // Add the edge between the mapped node indices
    }
    // Predecessor map, allowing to keep track of the previous hits in a component
    std::unordered_map<Vertex, std::vector<Vertex>> pred_map;
    // Visited vertices map
    std::vector<bool> visited(num_vertices(g), false);

    m_control_var.resize(num_vertices(g), -1);
    m_component.resize(num_vertices(g), std::vector<int>(1, -1));
    int comp_id = 0;
    
    for (Vertex v = 0; v < num_vertices(g); ++v){
        if(!visited[v] && boost::in_degree(v,g) == 0){

            // Create a color map to store state of the nodes during the BFS
            std::vector<boost::default_color_type> color_storage(num_vertices(g)); 
            auto color_map = boost::make_iterator_property_map(color_storage.begin(), get(boost::vertex_index,g));

            JunctionAwareVisitor JA_vis(comp_id, m_control_var, m_component, pred_map, color_map);
            breadth_first_search(g, v, visitor(JA_vis).color_map(color_map));
            // Mark visited vertices and reset control variables
            for (unsigned long u = 0; u != num_vertices(g); ++u){
                if (m_control_var[u] != -1){
                    visited[u] = true;
                    m_control_var[u] = -1;
                }
            }
            // Clear predecessor map
            pred_map.clear();
            ++comp_id;
        }
    }
    m_num_components = comp_id;

    m_labels.resize(m_num_nodes, std::vector<int>(1,-1));

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
        for (auto& current_label : m_labels[i]){
            if(current_label != -1) {
                m_road_hit_list[current_label].push_back(gnn_hits[i]->getHitID());
            }
        }
    }
    for (const auto& hit_list : m_road_hit_list) {
        if(m_doGNNPixelSeeding) { addRoadForPixelSeed(hits, hit_list); }
        else { addRoad(hits, hit_list); }
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

void FPGATrackSimGNNRoadMakerTool::addRoadForPixelSeed(const std::vector<std::shared_ptr<const FPGATrackSimHit>> & hits, const std::vector<int>& road_hitIDs) 
{
    std::vector<std::shared_ptr<const FPGATrackSimHit>> all_hits;
    std::vector<std::shared_ptr<const FPGATrackSimHit>> track_hit_list;

    for (const auto& hitID : road_hitIDs) {
        auto &hit = hits[hitID]; //Hit object inside the road candidate
        all_hits.push_back(hit);
    }

    // Sort hits in CC list by rho
    std::sort(all_hits.begin(), all_hits.end(),
             [](const std::shared_ptr<const FPGATrackSimHit>& hit1,
                const std::shared_ptr<const FPGATrackSimHit>& hit2) {
                double rho1 = std::hypot(hit1->getX(), hit1->getY());
                double rho2 = std::hypot(hit2->getX(), hit2->getY());
                return rho1 < rho2;
            });

    // From all the hits in the CC list, only accept one per global layer ID, defined by the layer configuration from GBTS
    std::unordered_set<int> seenLayers;
    for (const auto &hit : all_hits) {
        short layer = m_pix_h2l->at(static_cast<int>(hit->getIdentifierHash()));
        TrigInDetSiLayer layerGeometry = m_layerGeometry->at(layer);
        int combinedId = layerGeometry.m_subdet;

        if (seenLayers.count(combinedId) == 0) {  // unique layer
            track_hit_list.push_back(hit);
            seenLayers.insert(combinedId);
        } 
    }

    std::size_t nSp = track_hit_list.size();
    std::vector<std::vector<std::shared_ptr<const FPGATrackSimHit>>> seed_hit_list;

    // Use a method to choose which spacepoints to keep for the road, which becomes a seed
    // Need at least 3 spacepoints to form a seed, so only consider 3 spacepoint lists of hits-per-layer
    // For >=4 spacepoints, consider a evenly-spaced method instead of first N spacepoints
    // So the road candidate can have anywhere from 3-5 spacepoints
    auto spacePointIndicesFun = [](std::size_t nSp, std::size_t nSeeds) -> std::vector<std::size_t> {
        std::vector<std::size_t> idx;
    
        for (std::size_t i = 0; i < nSeeds; ++i) {
            std::size_t pos = (i * (nSp - 1)) / (nSeeds - 1); // evenly spaced
            if (idx.empty() || idx.back() != pos) { // avoid duplicates
                idx.push_back(pos);
            }
        }   
        return idx;
    };

    // Only keep road candidates with at >= 3 spacepoints in unique layers
    if (nSp >= 3) {
        std::size_t nSeeds = std::min<std::size_t>(5, nSp);
        auto indices = spacePointIndicesFun(nSp, nSeeds);

        for (auto idx : indices) {
            seed_hit_list.push_back({track_hit_list[idx]});
        }

        m_roads.emplace_back();
        FPGATrackSimRoad & r = m_roads.back();
        r.setHits(std::move(seed_hit_list));
        r.setRoadID(m_roads.size() - 1);
    }
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

void FPGATrackSimGNNRoadMakerTool::reorderIndices()
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
}

JunctionAwareVisitor::JunctionAwareVisitor(int& in_current, std::vector<int>& in_control_vars, std::vector<std::vector<int>>& in_comps,
                                           std::unordered_map<Vertex, std::vector<Vertex>>& in_pred_map, ColorMap in_cmap) :
  m_current_comp(in_current), m_control_vars(in_control_vars), m_components(in_comps),
  m_pred_map(in_pred_map), m_initial_comp(in_current), m_color_map(in_cmap) {}

template <typename VertexT, typename GraphT>
void JunctionAwareVisitor::discover_vertex(VertexT v, const GraphT& g)
{
    if (m_control_vars[v] == -1){
        m_control_vars[v] = boost:: out_degree(v,g) < 2 ? 1 : -2; // -2 labels junctions before they are transversed
        m_components[v].push_back(m_current_comp);
    }
}

template <typename EdgeT, typename GraphT>
void JunctionAwareVisitor::examine_edge(EdgeT e, const GraphT& g)
{
    auto src_node = source(e,g);
    auto tar_node = target(e,g);
    m_pred_map[tar_node].push_back(src_node);
    process_edges(src_node, tar_node, g);

    // If target node was already visited, propagate the component also to the nodes it is connected to
    if ((get(m_color_map, tar_node) == boost::black_color) && (boost::out_degree(tar_node, g) > 0)){
        m_control_vars[tar_node] = boost:: out_degree(tar_node,g) < 2 ? 1 : -2;
        auto out_edges = boost::out_edges(tar_node, g);
        for (auto it = out_edges.first; it != out_edges.second; ++ it){
            Vertex next_dst = target(*it, g);
            process_edges(tar_node, next_dst, g);
        }
    }
}

template <typename VertexT, typename GraphT>
void JunctionAwareVisitor::process_edges(VertexT src_node, VertexT tar_node, const GraphT& g){
    std::vector<int> src_comp; // Store the components of the source that need to be tracked

    for(int comp : m_components[src_node]){
        if(comp >= m_initial_comp && 
           std::find(m_components[tar_node].begin(), m_components[tar_node].end(), comp) == m_components[tar_node].end()){
            src_comp.push_back(comp); // Components of source node not in target
        }
    }

    // No junction case
    if (m_control_vars[src_node] == 1){
        m_components[tar_node].insert(m_components[tar_node].end(), src_comp.begin(), src_comp.end());
        m_control_vars[tar_node] = boost::out_degree(tar_node, g) < 2 ? 1 : -2;
    }

    // First time visiting a junction
    else if (m_control_vars[src_node] == -2){
        m_components[tar_node].insert(m_components[tar_node].end(), src_comp.begin(), src_comp.end());
        m_control_vars[tar_node] = boost::out_degree(tar_node, g) < 2 ? 1 : -2;
        m_control_vars[src_node] = 2; // 2 labels a junction after the first visit to it
        m_n_iter = src_comp.size(); // Number of indices to add to each component going out of this junction
    }

    // Going through a junction already visited
    else if (m_control_vars[src_node] == 2){
        for(int i = 0; i != m_n_iter; ++i){
            ++m_current_comp;
            m_components[tar_node].push_back(m_current_comp);
            m_control_vars[tar_node] = boost::out_degree(tar_node, g) < 2 ? 1 : -2;
            // Backtrace the node's predecessors to propagate the new label
            std::unordered_set<Vertex> visited;
            std::function<void(Vertex)> backtrace = [&](Vertex node){
                if(visited.count(node)) return;
                visited.insert(node);
                const auto& node_comps = m_components[node];
                if((std::find(node_comps.begin(), node_comps.end(), m_current_comp) == node_comps.end()) &&
                   (src_comp.empty() || std::find(node_comps.begin(), node_comps.end(), src_comp[i]) != node_comps.end())){ // Backpropagate with junction awareness
                    m_components[node].push_back(m_current_comp);
                }
                for (const auto& pred : m_pred_map[node]){
                    backtrace(pred);
                }
            };
            backtrace(tar_node);
        }
    }
}
