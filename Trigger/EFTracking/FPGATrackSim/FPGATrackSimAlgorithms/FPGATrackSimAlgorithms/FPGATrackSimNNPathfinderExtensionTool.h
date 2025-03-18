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
#include "GaudiKernel/ITHistSvc.h"

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
        ServiceHandle<ITHistSvc> m_tHistSvc {this, "THistSvc", "THistSvc"};

        // We'll definitely need properties, but I don't know which ones.
        Gaudi::Property<int> m_threshold  { this, "threshold", 10, "Minimum number of hits to fire a road"};

        // Options only needed for sector assignment.
        // The eta pattern option here should probably be dropped, because we're not using it
        // and supporting it requires having two sets of eta patterns (one for the first stage, one for the second)
        // and then running the eta pattern filter a second time.
        Gaudi::Property<std::vector<float>> m_windowR { this, "windowR", {20.0}, "Window Size to search in for r, either pass one value for all layers or use the number of layers"};
        Gaudi::Property<std::vector<float>> m_windowZ { this, "windowZ", {20.0}, "Window Size to search in for z, either pass one value for all layers or use the number of layers"};
        Gaudi::Property <float> m_lowPtValueForWindowRScaling { this, "lowPtValueWindowR", -1, "Value in MeV below which we scale the r window size"};
        Gaudi::Property <float> m_lowPtWindowRScaling {this, "lowPtRScaling", 1.0, "Scaling factor for low pt in R"};
        Gaudi::Property <float> m_lowPtValueForWindowZScaling { this, "lowPtValueWindowZ", -1, "Value in MeV below which we scale the r window size"};
        Gaudi::Property <float> m_lowPtWindowZScaling {this, "lowPtZScaling", 1.0, "Scaling factor for low pt in Z"};
        Gaudi::Property <float> m_missedHitRScaling {this, "missedHitRScaling", -1, "Amount to scale R window if previous hit was missed. Negative means this is disabled"};
        Gaudi::Property <float> m_missedHitZScaling {this, "missedHitZScaling", -1, "Amount to scale Z window if previous hit was missed. Negative means this is disabled"};  
        Gaudi::Property <int> m_maxBranches { this, "maxBranches", -1, "Max number of branches before we stop, if negative this is disabled"};
        Gaudi::Property <bool> m_doOutsideIn { this, "doOutsideIn", true, "Setup the tool so it's doing outside in extrap"};
        Gaudi::Property <int> m_predictionWindowLength { this, "predictionWindowLength", 3, "Length of hits needed for prediction"};

        StatusCode bookTree();
        TTree *m_tree = nullptr; // output tree
        std::vector<unsigned long> m_NcompletedRoads;
        std::vector<unsigned int> m_missingHitsOnRoad;
        std::vector<std::vector<unsigned long>> m_predictedHitsFineID;
        std::vector<std::vector<unsigned int>> m_foundHitITkLayer;
        std::vector<unsigned int> m_nHitsInSearchWindow;
        std::vector<std::vector<float>> m_distanceOfPredictedHitToFoundHit;
        std::vector<std::vector<bool>> m_foundHitIsSP;

        std::vector<FPGATrackSimRoad> m_roads;
        //This is a map(dict python equivalent) of slice IDs that have a map of layer IDs in it. That map has a vector of hits associated with it
        std::map<unsigned, std::map<unsigned, std::vector<std::shared_ptr<const FPGATrackSimHit>>>> m_phits_atLayer;
        unsigned m_nLayers_1stStage = 0;
        unsigned m_nLayers_2ndStage = 0;
        unsigned m_maxMiss = 0;

        static float getXScale() { return 1015.;};
        static float getYScale() { return 1015.;};
        static float getZScale() { return 3000.;};
        bool m_debugEvent = false;

        // Internal storage for the sliced hits (implemented as a LogicalEventInputHeader,
        // so we can easily copy to the output ROOT file).
        FPGATrackSimLogicalEventInputHeader*  m_slicedHitHeader = nullptr;
  
        OnnxRuntimeBase m_extensionVolNN;
        OnnxRuntimeBase m_extensionHitNN;

        StatusCode fillInputTensorForNN(FPGATrackSimRoad& thisRoad, std::vector<float>& inputTensorValues);
        StatusCode getPredictedHit(std::vector<float>& inputTensorValues, std::vector<float>& outputTensorValues, long& fineID);
        StatusCode addHitToRoad(FPGATrackSimRoad& newroad, FPGATrackSimRoad& currentRoad, const std::vector<std::shared_ptr<const FPGATrackSimHit>>& hits);
        StatusCode getFakeHit(FPGATrackSimRoad& currentRoad, size_t slice, std::vector<float>& predhit, const long& fineID, std::vector<std::shared_ptr<const FPGATrackSimHit>>& hits);
        StatusCode getLastLayer(FPGATrackSimRoad& currentRoad, unsigned& lastHitLayer, std::shared_ptr<const FPGATrackSimHit>& lastHit);

        StatusCode findHitinNextStripLayer(std::shared_ptr<const FPGATrackSimHit> hit, std::vector<std::shared_ptr<const FPGATrackSimHit>>& hitList, std::vector<std::shared_ptr<const FPGATrackSimHit>>& hits);

        void printRoad(FPGATrackSimRoad& currentRoad);

};

#endif
