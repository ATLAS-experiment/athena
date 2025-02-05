// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#ifndef FPGATRACKSIMGNNGRAPHHITSELECTORTOOL_H
#define FPGATRACKSIMGNNGRAPHHITSELECTORTOOL_H

/**
 * @file FPGATrackSimGNNGraphHitSelectorTool.h
 * @author Jared Burleson - jared.dynes.burleson@cern.ch
 * @date November 27th, 2024
 * @brief Implements hit selection as a tool for graph construction for GNN pipeline 
 *
 * This class implements the selectHits() function which converts FPGATrackSimHit to FPGATrackSimGNNHit, a custom object used for GNN pattern recognition.
 * This is necessary because the GNN pipeline only intends to use pixel hits and spacepoints built from two clusters in the strip and it requires the cluster information for inference.
 * So we will use this code to provide the proper object creation useful for the remainder of our GNN pattern recognition pipeline.
 */

#include "GaudiKernel/ServiceHandle.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include "FPGATrackSimObjects/FPGATrackSimHit.h"
#include "FPGATrackSimObjects/FPGATrackSimGNNHit.h"

#include "TMath.h"

#include <string>
#include <vector>

class FPGATrackSimGNNGraphHitSelectorTool : public AthAlgTool
{
    public:

        ///////////////////////////////////////////////////////////////////////
        // AthAlgTool

        FPGATrackSimGNNGraphHitSelectorTool(const std::string&, const std::string&, const IInterface*);

        ///////////////////////////////////////////////////////////////////////
        // Functions

        virtual StatusCode selectHits(const std::vector<std::shared_ptr<const FPGATrackSimHit>> & hits, std::vector<std::shared_ptr<FPGATrackSimGNNHit>> & graph_hits);

    private:

        ///////////////////////////////////////////////////////////////////////
        // Helpers

        float getEta(const std::shared_ptr<const FPGATrackSimHit> & hit);

};


#endif // FPGATRACKSIMGNNGRAPHHITSELECTORTOOL_H