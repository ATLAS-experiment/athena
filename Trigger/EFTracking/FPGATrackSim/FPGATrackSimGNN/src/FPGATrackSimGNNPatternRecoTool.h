// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#ifndef FPGATRACKSIMGNNPATTERNRECOTOOL_H
#define FPGATRACKSIMGNNPATTERNRECOTOOL_H

/**
 * @file FPGATrackSimGNNPatternRecoTool.h
 * @author Jared Burleson - jared.dynes.burleson@cern.ch
 * @date November 27th, 2024
 * @brief Implements road finding using GNNs for pattern recognition 
 *
 * This class serves as the overhead for using GNNs for pattern recognition in FPGATrackSim.
 * The getRoads() function from IFPGATrackSimRoadFinderTool is declared here and it will use the tools that are included in this file.
 * Each tool will be part of the pipeline necessary to complete getRoads().
 * 
 * The GNN Pattern Recognition Tool has four phases:
 *    1. Graph Hit Selector Tool  : select hits needed for pattern recognition and record additional information
 *    2. Graph Construction Tool  : use hits to build edges necessary for the GNN, which are spatial connections between hits
 *    3. Edge Classifier Tool     : run inference of Interaction Network GNN model to assign a score to each edge 
 *    4. Road Maker Tool          : use edge scores to select hits that can be algorithmically identified in a path, called a road
 * 
 * The output of this tool will feed back into the FPGATrackSim Analysis pipeline.
 * Currently this code does not operate with subRegions(), but only using the region defined by the map defined in the input python script for hit selection.
 * This is handled in the data preparation algorithm before this code.
 */

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"

#include "FPGATrackSimHough/IFPGATrackSimRoadFinderTool.h"

#include "FPGATrackSimGNNGraphHitSelectorTool.h"
#include "FPGATrackSimGNNGraphConstructionTool.h"
#include "FPGATrackSimGNNEdgeClassifierTool.h"
#include "FPGATrackSimGNNRoadMakerTool.h"
#include "FPGATrackSimGNNRootOutputTool.h"

class FPGATrackSimGNNPatternRecoTool : public extends <AthAlgTool, IFPGATrackSimRoadFinderTool>
{
    public:
        /// Constructor
        using base_class::base_class;

        virtual StatusCode initialize() override;

        ///////////////////////////////////////////////////////////////////////
        // FPGATrackSimRoadFinderToolI

        virtual StatusCode getRoads(const std::vector<std::shared_ptr<const FPGATrackSimHit>> & hits, std::vector<FPGATrackSimRoad> & roads) override;
        virtual int getSubRegion() const override{ return 0; }

    private: 

        ///////////////////////////////////////////////////////////////////////
        // Properties

        Gaudi::Property<bool> m_doGNNRootOutput { this, "doGNNRootOutput", false, "Flag for GNN Root Output Tool" };
        Gaudi::Property<int> m_regionNum { this, "regionNum", -1, "region number"};
        
        ///////////////////////////////////////////////////////////////////////
        // Handles
        ToolHandle<FPGATrackSimGNNGraphHitSelectorTool>    m_GNNGraphHitSelectorTool {this, "GNNGraphHitSelector", "FPGATrackSimGNNGraphHitSelectorTool", "Graph HitSelector Tool"};
        ToolHandle<FPGATrackSimGNNGraphConstructionTool>   m_GNNGraphConstructionTool {this, "GNNGraphConstruction", "FPGATrackSimGNNGraphConstructionTool", "Graph Construction Tool"};
        ToolHandleArray<FPGATrackSimGNNEdgeClassifierTool> m_GNNEdgeClassifierTools {this, "GNNEdgeClassifiers", {}, "One GNNEdgeClassifierTool per region"};
        ToolHandle<FPGATrackSimGNNEdgeClassifierTool>      m_selectedGNNEdgeClassifierTool;
        ToolHandle<FPGATrackSimGNNRoadMakerTool>           m_GNNRoadMakerTool {this, "GNNRoadMaker", "FPGATrackSimGNNRoadMakerTool", "Road Maker Tool"};
        ToolHandle<FPGATrackSimGNNRootOutputTool>          m_GNNRootOutputTool {this, "GNNRootOutput", "FPGATrackSimGNNRootOutputTool", "GNN ROOT Output Tool"};
};


#endif // FPGATRACKSIMGNNPATTERNRECOTOOL_H
