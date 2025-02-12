// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#ifndef FPGATrackPATHFINDEREXTENSION_H
#define FPGATrackPATHFINDEREXTENSION_H

/**
 * @file FPGATrackSimNNPathfinderExtensionTool.h
 * @author Ben Rosser - brosser@uchicago.edu
 * @date 2024/10/08
 * @brief Default track extension algorithm to produce "second stage" roads.
 */

#include "GaudiKernel/ServiceHandle.h"
#include "AthenaBaseComps/AthAlgTool.h"

// add public header directory to algorithms for this?
#include "IFPGATrackSimTrackExtensionTool.h"

#include "FPGATrackSimObjects/FPGATrackSimTypes.h"
#include "FPGATrackSimObjects/FPGATrackSimFunctions.h"
#include "FPGATrackSimObjects/FPGATrackSimHit.h"
#include "FPGATrackSimObjects/FPGATrackSimRoad.h"
#include "FPGATrackSimObjects/FPGATrackSimTrack.h"
#include "FPGATrackSimMaps/IFPGATrackSimMappingSvc.h"
#include "FPGATrackSimBanks/IFPGATrackSimBankSvc.h"
#include "FPGATrackSimMaps/FPGATrackSimPlaneMap.h"
#include "FPGATrackSimMaps/FPGATrackSimRegionMap.h"
#include "FPGATrackSimObjects/FPGATrackSimTowerInputHeader.h"
#include "FPGATrackSimNNTrackTool.h"

#include <vector>

class FPGATrackSimNNPathfinderExtensionTool   : public extends <AthAlgTool, IFPGATrackSimTrackExtensionTool>
{
    public:

        FPGATrackSimNNPathfinderExtensionTool(const std::string&, const std::string&, const IInterface*);

        virtual StatusCode initialize() override;

        virtual StatusCode extendTracks(const std::vector<std::shared_ptr<const FPGATrackSimHit>> & hits,
                                        const std::vector<std::shared_ptr<const FPGATrackSimTrack>> & tracks,
                                        std::vector<std::shared_ptr<const FPGATrackSimRoad>> & roads) override;

        // We don't have a "union" tool that sits in front of the extension tool, so this is needed here.
        virtual StatusCode setupSlices(FPGATrackSimLogicalEventInputHeader *slicedHitHeader) override {
          m_slicedHitHeader = slicedHitHeader;
          return StatusCode::SUCCESS;
        };


    private:
        ServiceHandle<IFPGATrackSimMappingSvc> m_FPGATrackSimMapping {this, "FPGATrackSimMappingSvc", "FPGATrackSimMappingSvc"};

        // We'll definitely need properties, but I don't know which ones.
        Gaudi::Property<int> m_threshold  { this, "threshold", 11, "Minimum number of hits to fire a road"};

        // Options only needed for sector assignment.
        // The eta pattern option here should probably be dropped, because we're not using it
        // and supporting it requires having two sets of eta patterns (one for the first stage, one for the second)
        // and then running the eta pattern filter a second time.
        Gaudi::Property <float> m_windowR { this, "windowR", 20.0, "Window Size to search in for r, right now one value for all layers"};
        Gaudi::Property <float> m_windowZ { this, "windowZ", 20.0, "Window Size to search in for z, right now one value for all layers"};
        Gaudi::Property <int> m_maxBranches { this, "maxBranches", -1, "Max number of branches before we stop, if negative this is disabled"};
        Gaudi::Property <bool> m_doOutsideIn { this, "doOutsideIn", true, "Setup the tool so it's doing outside in extrap"};
        Gaudi::Property <int> m_predictionWindowLength { this, "predictionWindowLength", 3, "Length of hits needed for prediction"};

        std::vector<FPGATrackSimRoad> m_roads;
        //This is a map(dict python equivalent) of slice IDs that have a map of layer IDs in it. That map has a vector of hits associated with it
        std::map<unsigned, std::map<unsigned, std::vector<std::shared_ptr<const FPGATrackSimHit>>>> m_phits_atLayer;
        unsigned m_nLayers_1stStage = 0;
        unsigned m_nLayers_2ndStage = 0;
        unsigned m_maxMiss = 0;

        static float getXScale() { return 1015.;};
        static float getYScale() { return 1015.;};
        static float getZScale() { return 3000.;};

        // Internal storage for the sliced hits (implemented as a LogicalEventInputHeader,
        // so we can easily copy to the output ROOT file).
        FPGATrackSimLogicalEventInputHeader*  m_slicedHitHeader = nullptr;
  
        OnnxRuntimeBase m_extensionVolNN;
        OnnxRuntimeBase m_extensionHitNN;

        StatusCode fillInputTensorForNN(FPGATrackSimRoad& thisRoad, std::vector<float>& inputTensorValues);
        StatusCode getPredictedHit(std::vector<float>& inputTensorValues, std::vector<float>& outputTensorValues, long& fineID);
        StatusCode addHitToRoad(FPGATrackSimRoad& newroad, FPGATrackSimRoad& currentRoad, const std::shared_ptr<const FPGATrackSimHit>&hit);
        StatusCode getFakeHit(FPGATrackSimRoad& currentRoad, size_t slice, std::vector<float>& predhit, std::shared_ptr<FPGATrackSimHit> &guessedHitPtr);
};

#endif
