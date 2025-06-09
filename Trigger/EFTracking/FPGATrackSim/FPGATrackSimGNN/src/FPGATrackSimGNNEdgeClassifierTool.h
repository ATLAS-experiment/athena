// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#ifndef FPGATRACKSIMGNNEDGECLASSIFIERTOOL_H
#define FPGATRACKSIMGNNEDGECLASSIFIERTOOL_H

/**
 * @file FPGATrackSimGNNEdgeClassifierTool.h
 * @author Jared Burleson - jared.dynes.burleson@cern.ch
 * @date November 27th, 2024
 * @brief Implements edge classification by inferencing on an Interaction Network GNN. 
 *
 * This class implements the scoreEdges() function which will calculate a score for each FPGATrackSimGNNEdge using an Interaction Network GNN.
 * Each edge will receive a score between 0 and 1 that indicates how likely the edge is to connect two hits that belong to the same particle/track.
 * The default value of the edge score is -1, indicating that an edge has not been scored by the GNN yet.
 */

#include "AthenaBaseComps/AthAlgTool.h"

#include "FPGATrackSimObjects/FPGATrackSimGNNHit.h"
#include "FPGATrackSimObjects/FPGATrackSimGNNEdge.h"

#include "AthOnnxInterfaces/IOnnxRuntimeInferenceTool.h"
#include <onnxruntime_cxx_api.h>

class FPGATrackSimGNNEdgeClassifierTool : public AthAlgTool
{
    public:

        ///////////////////////////////////////////////////////////////////////
        // AthAlgTool

        FPGATrackSimGNNEdgeClassifierTool(const std::string&, const std::string&, const IInterface*);

        virtual StatusCode initialize() override;

        ///////////////////////////////////////////////////////////////////////
        // Functions

        virtual StatusCode scoreEdges(const std::vector<std::shared_ptr<FPGATrackSimGNNHit>> & hits, std::vector<std::shared_ptr<FPGATrackSimGNNEdge>> & edges);

    private:

        ///////////////////////////////////////////////////////////////////////
        // Handles

        ToolHandle<AthOnnx::IOnnxRuntimeInferenceTool> m_GNNInferenceTool {this, "GNNInferenceTool", "AthOnnx::OnnxRuntimeInferenceTool"};

        ///////////////////////////////////////////////////////////////////////
        // Helpers
        
        std::vector<float> getNodeFeatures(const std::vector<std::shared_ptr<FPGATrackSimGNNHit>> & hits);
        std::vector<int64_t> getEdgeList(const std::vector<std::shared_ptr<FPGATrackSimGNNEdge>> & edges);
        std::vector<float> getEdgeFeatures(std::vector<std::shared_ptr<FPGATrackSimGNNEdge>> & edges, const std::vector<float> & gNodeFeatures);
        void computeEdgeFeatures(std::shared_ptr<FPGATrackSimGNNEdge>& edge, const int& hit1_index, const int& hit2_index, const std::vector<float> & gNodeFeatures);

        StringArrayProperty m_gnnFeatureNamesVec{
            this, "GNNFeatureNames",
            {"r", "phi", "z", "eta", "cluster_r_1", "cluster_phi_1", "cluster_z_1", "cluster_eta_1", "cluster_r_2", "cluster_phi_2", "cluster_z_2", "cluster_eta_2"},
            "Feature names for the GNN model"};
        FloatArrayProperty m_gnnFeatureScalesVec{
            this, "GNNFeatureScales",
            {1000.0, 3.14159265359, 1000.0, 1.0, 1000.0, 3.14159265359, 1000.0, 1.0, 1000.0, 3.14159265359, 1000.0, 1.0},
            "Feature scales for the GNN model"};

};      


#endif // FPGATRACKSIMGNNEDGECLASSIFIERTOOL_H