// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#ifndef FPGATrackSimGNNRoadMakerTool_H
#define FPGATrackSimGNNRoadMakerTool_H

/**
 * @file FPGATrackSimGNNRoadMakerTool.h
 * @author Jared Burleson - jared.dynes.burleson@cern.ch
 * @date November 27th, 2024
 * @brief Implements algorithm to construct a road from a list of hits using edge scores. 
 *
 * This class implements the makeRoads() function which use FPGATrackSimGNNHits and FPGATrackSimGNNEdges that are scored by the GNN to build FPGATrackSimRoad objects.
 * The roads are built using a C++ implementation of the SciPy sparse CSGraph's Connected Components algorithm to identify logical connections of hits using edges that pass a score cut threshold.
 * Using the output of our connected components algorithm, road objects are created as a vector of FPGATrackSimHits, which are derived from the corresponding FPGATrackSimGNNHits.
 */

#include "GaudiKernel/ServiceHandle.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include "FPGATrackSimMaps/IFPGATrackSimMappingSvc.h"
#include "FPGATrackSimObjects/FPGATrackSimRoad.h"
#include "FPGATrackSimObjects/FPGATrackSimGNNHit.h"
#include "FPGATrackSimObjects/FPGATrackSimGNNEdge.h"
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include "TrigInDetToolInterfaces/ITrigL2LayerNumberTool.h"

#include <memory>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>

typedef boost::graph_traits<boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS>>::vertex_descriptor Vertex;

class FPGATrackSimGNNRoadMakerTool : public AthAlgTool
{
    public:

        ///////////////////////////////////////////////////////////////////////
        // AthAlgTool

        FPGATrackSimGNNRoadMakerTool(const std::string&, const std::string&, const IInterface*);

        virtual StatusCode initialize() override;

        ///////////////////////////////////////////////////////////////////////
        // Functions

        virtual StatusCode makeRoads(const std::vector<std::shared_ptr<const FPGATrackSimHit>> & hits, 
                                     const std::vector<std::shared_ptr<FPGATrackSimGNNHit>> & gnn_hits, 
                                     const std::vector<std::shared_ptr<FPGATrackSimGNNEdge>> & edges, 
                                     std::vector<std::shared_ptr<const FPGATrackSimRoad>> & roads);

    private:
        
        ///////////////////////////////////////////////////////////////////////
        // Handles
        
        ServiceHandle<IFPGATrackSimMappingSvc> m_FPGATrackSimMapping {this, "FPGATrackSimMappingSvc", "FPGATrackSimMappingSvc"};
        ToolHandle<ITrigL2LayerNumberTool> m_layerNumberTool{this, "LayerNumberTool", "TrigL2LayerNumberToolITk"};
        
        ///////////////////////////////////////////////////////////////////////
        // Properties

        Gaudi::Property<float> m_edgeScoreCut { this, "edgeScoreCut", 0.0, "Cut value for edge scores to pass for road making algorithm" };
        Gaudi::Property<std::string> m_roadMakerTool { this, "roadMakerTool", "", "Algorithm to perform graph segmentation into roads"};
        Gaudi::Property<bool> m_doGNNPixelSeeding { this, "doGNNPixelSeeding", false, "Flag to configure for GNN Pixel Seeding" };

        ///////////////////////////////////////////////////////////////////////
        // Convenience

        int m_num_nodes = 0;
        std::vector<int> m_pass_edge_index_1{};
        std::vector<int> m_pass_edge_index_2{};
        unsigned m_nLayers = 0;
        std::set<int> m_unique_nodes{};
        std::map<int, int> m_node_index_map{};
        std::vector<int> m_unique_indices{};
        std::vector<int> m_control_var{};
        std::vector<std::vector<int>> m_component{};
        int m_num_components = 0;
        std::vector<std::vector<int>> m_labels{};
        std::vector<std::vector<int>> m_road_hit_list{};
        const std::vector<short>* m_pix_h2l{nullptr};
        const std::vector<TrigInDetSiLayer>* m_layerGeometry{nullptr};

        ///////////////////////////////////////////////////////////////////////
        // Helpers

        void doScoreCut(const std::vector<std::shared_ptr<FPGATrackSimGNNEdge>> & edges);
        void doConnectedComponents();
        void doJunctionAwareCC();
        void addRoads(const std::vector<std::shared_ptr<const FPGATrackSimHit>> & hits, 
                      const std::vector<std::shared_ptr<FPGATrackSimGNNHit>> & gnn_hits, 
                      std::vector<std::shared_ptr<const FPGATrackSimRoad>> & roads);
        void addRoad(const std::vector<std::shared_ptr<const FPGATrackSimHit>> & hits, const std::vector<int>& road_hitIDs);
        void addRoadForPixelSeed(const std::vector<std::shared_ptr<const FPGATrackSimHit>> & hits, const std::vector<int>& road_hitIDs);
        void resetVectors();
        void reorderIndices();

        ///////////////////////////////////////////////////////////////////////
        // Event Storage

        std::vector<FPGATrackSimRoad> m_roads{};
};

///////////////////////////////////////////////////////////////////////
// Helper: Junctoin Aware BFS visitor
class JunctionAwareVisitor : public boost::default_bfs_visitor
{
    public:
        JunctionAwareVisitor(int& current, std::vector<int>& in_control_vars, std::vector<std::vector<int>>& in_comps,
                             const boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS>& in_graph,
                             std::unordered_map<Vertex, std::vector<Vertex>>& in_pred_map);

        template <typename VertexT, typename GraphT>
            void discover_vertex(VertexT v, const GraphT& g);

        template <typename EdgeT, typename GraphT>
            void examine_edge(EdgeT e, const GraphT& g);

    private:
        int& m_current_comp;
        std::vector<int>& m_control_vars;
        std::vector<std::vector<int>>& m_components;
        const boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS>& m_graph;
        std::unordered_map<Vertex, std::vector<Vertex>>& m_pred_map;
        int m_initial_comp;
        int m_n_iter = 1;
};

#endif // FPGATrackSimGNNRoadMakerTool_H
