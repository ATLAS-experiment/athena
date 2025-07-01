// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#ifndef FPGATRACKSIMGNNROOTOUTPUTTOOL_H
#define FPGATRACKSIMGNNROOTOUTPUTTOOL_H

/**
 * @file FPGATrackSimGNNRootOutputTool.h
 * @author Jared Burleson - jared.dynes.burleson@cern.ch
 * @date November 27th, 2024
 * @brief Implements tool to output hit, edge and road information from GNN pattern recognition pipeline.
 *
 * This class creates a ROOT file using THistSvc with the following TTrees:
 *    - m_hit_tree     : Storing FPGATrackSimHit information by event
 *    - m_GNNHit_tree  : Storing FPGATrackSimGNNHit information by event
 *    - m_GNNEdge_tree : Storing FPGATrackSimGNNEdge information by event
 *    - m_road_tree    : Storing FPGATrackSimRoad information by event
 *
 * This is essential to providing an output comparison between offline algorithm in python and this implementation in athena.
 */

#include "GaudiKernel/ServiceHandle.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include "FPGATrackSimObjects/FPGATrackSimRoad.h"
#include "FPGATrackSimObjects/FPGATrackSimHit.h"
#include "FPGATrackSimObjects/FPGATrackSimGNNEdge.h"
#include "FPGATrackSimObjects/FPGATrackSimGNNHit.h"

#include "GaudiKernel/ITHistSvc.h"
#include "TTree.h"

class FPGATrackSimGNNRootOutputTool : public AthAlgTool
{
  public:

        ///////////////////////////////////////////////////////////////////////
        // AthAlgTool

        FPGATrackSimGNNRootOutputTool(const std::string&, const std::string&, const IInterface*);

        virtual StatusCode initialize() override;

        ///////////////////////////////////////////////////////////////////////
        // Functions

        StatusCode fillTree(const std::vector<std::shared_ptr<const FPGATrackSimHit>> & hits,
                            const std::vector<std::shared_ptr<FPGATrackSimGNNHit>> & gnn_hits,
                            const std::vector<std::shared_ptr<FPGATrackSimGNNEdge>> & edges,
                            const std::vector<std::shared_ptr<const FPGATrackSimRoad>> & roads);

    private:

        ///////////////////////////////////////////////////////////////////////
        // Helpers

        StatusCode bookTree();
        void resetVectors();

        ///////////////////////////////////////////////////////////////////////
        // Handles

        ServiceHandle<ITHistSvc> m_tHistSvc {this, "THistSvc", "THistSvc"};
        Gaudi::Property <std::string> m_region { this, "OutputRegion", "", "region ID"};

        ///////////////////////////////////////////////////////////////////////
        // Convenience

        TTree *m_hit_tree = nullptr; // output FPGATrackSimHit tree
        std::vector<unsigned int> m_hit_id{};
        std::vector<unsigned int> m_hit_module_id{};
        std::vector<float> m_hit_x{};
        std::vector<float> m_hit_y{};
        std::vector<float> m_hit_z{};
        std::vector<float> m_hit_r{};
        std::vector<float> m_hit_phi{};
        std::vector<bool> m_hit_isPixel{};
        std::vector<bool> m_hit_isStrip{};
        std::vector<int> m_hit_hitType{};
        std::vector<HepMcParticleLink::barcode_type> m_hit_uniqueID{};
        std::vector<long> m_hit_eventIndex{};
        std::vector<float> m_hit_cluster_x{};
        std::vector<float> m_hit_cluster_y{};
        std::vector<float> m_hit_cluster_z{};

        TTree *m_GNNHit_tree = nullptr; // output FPGATrackSimGNNHit tree
        std::vector<unsigned int> m_GNNHit_id{};
        std::vector<unsigned int> m_GNNHit_module_id{};
        std::vector<int> m_GNNHit_road_id{};
        std::vector<float> m_GNNHit_x{};
        std::vector<float> m_GNNHit_y{};
        std::vector<float> m_GNNHit_z{};
        std::vector<float> m_GNNHit_r{};
        std::vector<float> m_GNNHit_phi{};
        std::vector<float> m_GNNHit_eta{};
        std::vector<float> m_GNNHit_cluster_x_1{};
        std::vector<float> m_GNNHit_cluster_y_1{};
        std::vector<float> m_GNNHit_cluster_z_1{};
        std::vector<float> m_GNNHit_cluster_r_1{};
        std::vector<float> m_GNNHit_cluster_phi_1{};
        std::vector<float> m_GNNHit_cluster_eta_1{};
        std::vector<float> m_GNNHit_cluster_x_2{};
        std::vector<float> m_GNNHit_cluster_y_2{};
        std::vector<float> m_GNNHit_cluster_z_2{};
        std::vector<float> m_GNNHit_cluster_r_2{};
        std::vector<float> m_GNNHit_cluster_phi_2{};
        std::vector<float> m_GNNHit_cluster_eta_2{};

        TTree *m_GNNEdge_tree = nullptr; // output FPGATrackSimGNNEdge tree
        std::vector<unsigned int> m_GNNEdge_index_1{};
        std::vector<unsigned int> m_GNNEdge_index_2{};
        std::vector<float> m_GNNEdge_dR{};
        std::vector<float> m_GNNEdge_dPhi{};
        std::vector<float> m_GNNEdge_dZ{};
        std::vector<float> m_GNNEdge_dEta{};
        std::vector<float> m_GNNEdge_phiSlope{};
        std::vector<float> m_GNNEdge_rPhiSlope{};
        std::vector<float> m_GNNEdge_score{};

        TTree *m_road_tree = nullptr; // output FPGATrackSimRoad tree
        std::vector<int> m_road_id{};
        std::vector<size_t> m_road_nHits{};
        std::vector<std::vector<size_t>> m_road_nHits_layer{};
        std::vector<size_t> m_road_nLayers{};
        std::vector<std::vector<std::vector<HepMcParticleLink::barcode_type>>> m_road_hit_uniqueID{};
        std::vector<std::vector<std::vector<HepMcParticleLink::barcode_type>>> m_road_hit_barcode{};
        std::vector<std::vector<std::vector<float>>> m_road_hit_z{};
        std::vector<std::vector<std::vector<float>>> m_road_hit_r{};
};

#endif // FPGARACKSIMGNNROOTOUTPUTTOOL_H