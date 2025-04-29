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

#include <memory>
#include <vector>
#include <map>
#include <set>

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
        
        ///////////////////////////////////////////////////////////////////////
        // Properties

        Gaudi::Property<float> m_edgeScoreCut { this, "edgeScoreCut", 0.0, "Cut value for edge scores to pass for road making algorithm" };
        Gaudi::Property<std::string> m_roadMakerTool { this, "roadMakerTool", "", "Algorithm to perform graph segmentation into roads"};

        ///////////////////////////////////////////////////////////////////////
        // Convenience

        int m_num_nodes = 0;
        std::vector<int> m_pass_edge_index_1{};
        std::vector<int> m_pass_edge_index_2{};
        unsigned m_nLayers = 0;
        std::set<int> m_unique_nodes{};
        std::map<int, int> m_node_index_map{};
        std::vector<int> m_unique_indices{};
        typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS> m_Graph;
        std::vector<int> m_component{};
        int m_num_components = 0;
        std::vector<int> m_labels{};
        std::vector<std::vector<int>> m_road_hit_list{};

        ///////////////////////////////////////////////////////////////////////
        // Helpers

        void doScoreCut(const std::vector<std::shared_ptr<FPGATrackSimGNNEdge>> & edges);
        void doConnectedComponents();
        void addRoads(const std::vector<std::shared_ptr<const FPGATrackSimHit>> & hits, 
                      const std::vector<std::shared_ptr<FPGATrackSimGNNHit>> & gnn_hits, 
                      std::vector<std::shared_ptr<const FPGATrackSimRoad>> & roads);
        void addRoad(const std::vector<std::shared_ptr<const FPGATrackSimHit>> & hits, const std::vector<int>& road_hitIDs);
        void resetVectors();

        ///////////////////////////////////////////////////////////////////////
        // Event Storage

        std::vector<FPGATrackSimRoad> m_roads{};
};


#endif // FPGATrackSimGNNRoadMakerTool_H