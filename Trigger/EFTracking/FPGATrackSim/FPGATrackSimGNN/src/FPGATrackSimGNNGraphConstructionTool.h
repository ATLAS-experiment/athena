// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#ifndef FPGATRACKSIMGNNGRAPHCONSTRUCTIONTOOL_H
#define FPGATRACKSIMGNNGRAPHCONSTRUCTIONTOOL_H

/**
 * @file FPGATrackSimGNNGraphConstructionTool.h
 * @author Jared Burleson - jared.dynes.burleson@cern.ch
 * @date November 27th, 2024
 * @brief Implements graph construction tool to build edges (connections) between hits 
 *
 * This class implements the getEdges() function which uses a graph construction tool to algorithmically connect hits together by an edge.
 * There are multiple possible algorithms capable of performing edge construction, which can be configured by the graphTool from the python script.
 * The default algorithm is the doublet Module Map, but future implementations will be added to allow for different options for graph construction.
 * The FPGATrackSimGNNEdge objects will be created and passed alongside the FPGATrackSimGNNHit objects through the rest of the GNN pipeline. 
 */

#include "AthenaBaseComps/AthAlgTool.h"

#include "FPGATrackSimObjects/FPGATrackSimGNNEdge.h"
#include "FPGATrackSimObjects/FPGATrackSimGNNHit.h"

#include "AthOnnxInterfaces/IOnnxRuntimeInferenceTool.h"
#include "AthOnnxUtils/OnnxUtils.h"
#include <onnxruntime_cxx_api.h>

class FPGATrackSimGNNGraphConstructionTool : public AthAlgTool
{
    public:

        ///////////////////////////////////////////////////////////////////////
        // AthAlgTool

        FPGATrackSimGNNGraphConstructionTool(const std::string&, const std::string&, const IInterface*);

        virtual StatusCode initialize() override;

        ///////////////////////////////////////////////////////////////////////
        // Functions

        virtual StatusCode getEdges(const std::vector<std::shared_ptr<FPGATrackSimGNNHit>> & hits,
                                    std::vector<std::shared_ptr<FPGATrackSimGNNEdge>> & edges);

    private:
        
        ///////////////////////////////////////////////////////////////////////
        // Handles

        ToolHandle<AthOnnx::IOnnxRuntimeInferenceTool> m_MLInferenceTool {this, "MLInferenceTool", "AthOnnx::OnnxRuntimeInferenceTool"};

        ///////////////////////////////////////////////////////////////////////
        // Properties

        Gaudi::Property<std::string> m_graphTool { this, "graphTool", "", "Tool for graph construction" };
        Gaudi::Property<std::string> m_moduleMapType { this, "moduleMapType", "", "Type for Module Map for graph construction" };
        Gaudi::Property<std::string> m_moduleMapFunc { this, "moduleMapFunc", "", "Function for Module Map for graph construction" };
        Gaudi::Property<float> m_moduleMapTol { this, "moduleMapTol", 0.0, "Tolerance value for Module Map cut calculations" };
        Gaudi::Property<std::string> m_moduleMapPath { this, "moduleMapPath", "", "Location of Module Map ROOT file" };
        Gaudi::Property<float> m_metricLearningR { this, "metricLearningR", 0.0, "Clustering radius for Metric Learning"};
        Gaudi::Property<int> m_metricLearningMaxN { this, "metricLearningMaxN", 1, "Max number of neighbours for Metric Learning"};

        ///////////////////////////////////////////////////////////////////////
        // Convenience

        // Module Map Information
        std::vector<unsigned int> m_mid1{};
        std::vector<unsigned int> m_mid2{};
        std::vector<float> m_z0min_12{};
        std::vector<float> m_dphimin_12{};
        std::vector<float> m_phislopemin_12{};
        std::vector<float> m_detamin_12{};
        std::vector<float> m_z0max_12{};
        std::vector<float> m_dphimax_12{};
        std::vector<float> m_phislopemax_12{};
        std::vector<float> m_detamax_12{};

        ///////////////////////////////////////////////////////////////////////
        // Helpers

        void loadDoubletModuleMap();
        void doModuleMap(const std::vector<std::shared_ptr<FPGATrackSimGNNHit>> & hits,
                               std::vector<std::shared_ptr<FPGATrackSimGNNEdge>> & edges);
        void getDoubletEdges(const std::vector<std::shared_ptr<FPGATrackSimGNNHit>> & hits,
                             std::vector<std::shared_ptr<FPGATrackSimGNNEdge>> & edges);
        void applyDoubletCuts(const std::shared_ptr<FPGATrackSimGNNHit> & hit1, const std::shared_ptr<FPGATrackSimGNNHit> & hit2,
                              std::vector<std::shared_ptr<FPGATrackSimGNNEdge>> & edges,
                              int hit1_index, int hit2_index, unsigned int modulemap_id);
        bool doMask(float val, float min, float max);
        bool doMinMaxMask(float val, float min, float max);
        float featureSign(float feature);
        void doMetricLearning(const std::vector<std::shared_ptr<FPGATrackSimGNNHit>> & hits, std::vector<std::shared_ptr<FPGATrackSimGNNEdge>> & edges);
        std::vector<float> getNodeFeatures(const std::vector<std::shared_ptr<FPGATrackSimGNNHit>> & hits);
        std::vector<float> embed(const std::vector<std::shared_ptr<FPGATrackSimGNNHit>> & hits);
        void doClustering(const std::vector<std::shared_ptr<FPGATrackSimGNNHit>> & hits, std::vector<std::shared_ptr<FPGATrackSimGNNEdge>> & edges, std::vector<float> & gEmbedded);
        void computeEdgeFeatures(std::shared_ptr<FPGATrackSimGNNEdge>& edge, const std::shared_ptr<FPGATrackSimGNNHit> & hit1, const std::shared_ptr<FPGATrackSimGNNHit> & hit2);
        // Metric Learning Properties
        StringArrayProperty m_MLFeatureNamesVec{
            this, "MLFeatureNames",
            {"r", "phi", "z"},
            "Feature names for the Metric Learning model"};
        FloatArrayProperty m_MLFeatureScalesVec{
            this, "MLFeatureScales",
            {1000.0, 3.14159265359, 1000.0},
            "Feature scales for the Metric Learning model"};
};

#endif // FPGATRACKSIMGNNGRAPHCONSTRUCTIONTOOL_H