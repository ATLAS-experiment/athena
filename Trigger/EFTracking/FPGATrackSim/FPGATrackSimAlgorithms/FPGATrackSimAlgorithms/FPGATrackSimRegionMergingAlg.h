// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#ifndef FPGATrackSim_REGIONMERGING_H
#define FPGATrackSim_REGIONMERGING_H

// Athena libraries
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/StoreGateSvc.h"

// FPGATrackSim libraries
#include "FPGATrackSimObjects/FPGATrackSimTrackCollection.h"
#include "FPGATrackSimObjects/FPGATrackSimHitCollection.h"
#include "FPGATrackSimObjects/FPGATrackSimHitContainer.h"
#include "FPGATrackSimObjects/FPGATrackSimRoadCollection.h"
#include "FPGATrackSimAlgorithms/FPGATrackSimOverlapRemovalTool.h"


/**
 * @file FPGATrackSimRegionMergingAlg.h
 * 
 * @brief This algorithm is used to merge track or road collections from multiple regions. It also provides functionality for overlap removal using the FPGATrackSimOverlapRemovalTool. The algorithm supports both track and road merging, depending on the configuration.
 */
namespace FPGATrackSim {
    class FPGATrackSimRegionMergingAlg : public ::AthReentrantAlgorithm {
    public:
        FPGATrackSimRegionMergingAlg(const std::string& name, ISvcLocator* pSvcLocator);
        virtual ~FPGATrackSimRegionMergingAlg() = default;

        virtual StatusCode initialize() override final;
        virtual StatusCode execute(const EventContext& ctx) const override final;
        virtual StatusCode finalize() override final;

    private:
        // Gaudi Properties
        Gaudi::Property<bool> m_doOverlapRemoval {this, "doOverlapRemoval", true , "flag to enable the overlap removal"}; 
        Gaudi::Property<bool> m_useRoads {this, "useRoads", false, "If set to truth it merges roads instead of tracks (tracking is set to False)"};

        // Tools
        mutable ToolHandle<FPGATrackSimOverlapRemovalTool> m_overlapRemovalTool ATLAS_THREAD_SAFE{ this, "OverlapRemovalTool", "FPGATrackSimOverlapRemovalTool/FPGATrackSimOverlapRemovalTool", "Overla removal tool to run on overlapping regions" };
        
        // Handles
        SG::ReadHandleKeyArray<FPGATrackSimTrackCollection> m_FPGATrackCollectionKeys {this, "FPGATrackSimTrackCollections",{},"List of FPGA track collections from different regions"};
        SG::ReadHandleKeyArray<FPGATrackSimRoadCollection> m_FPGARoadCollectionKeys {this, "FPGATrackSimRoadCollections",{},"List of FPGA road collections from different regions"};
        SG::ReadHandleKeyArray<FPGATrackSimHitContainer> m_FPGAHitsInRoadsCollectionKeys {this, "FPGATrackSimHitsInRoadsCollections",{},"List of FPGA hits roads"};

        SG::WriteHandleKey<FPGATrackSimTrackCollection> m_FinalFPGATrackCollectionKey {this, "FinalFPGATrackCollection","FPGATracks","Outgoing FPGA track collection containing tracks after region merging and OR"};
        SG::WriteHandleKey<FPGATrackSimRoadCollection> m_FinalFPGARoadkCollectionKey {this, "FinalFPGARoadCollection","FPGARoads","Outgoing FPGA road collection containing roads after region merging"};
        SG::WriteHandleKey<FPGATrackSimHitContainer> m_FinalFPGAHitsInRoadsCollectionKey {this, "FinalFPGAHitsInRoadsCollection","FPGAHitsInRoads","Outgoing FPGA hits-in-roads collection after region merging"};

        // chrono service
        ServiceHandle<IChronoStatSvc> m_chrono{this,"ChronoStatSvc","ChronoStatSvc"};
        

        // internal members
        StatusCode mergeTracks(const std::vector<const FPGATrackSimTrackCollection*>& inputTracksPtrs,
                               std::unique_ptr<FPGATrackSimTrackCollection>& outputTracks) const;
        StatusCode mergeRoads(const std::vector<const FPGATrackSimRoadCollection*>& inputRoads,
                              const std::vector<const FPGATrackSimHitContainer*>& inputHitsInRoads,
                              std::unique_ptr<FPGATrackSimRoadCollection>& outputRoads,
                              std::unique_ptr<FPGATrackSimHitContainer>& outputHitsInRoads) const;

        mutable std::atomic<size_t> m_allIncomingTracks{0}, m_nPreORTracks{0}, m_nPostORTracks{0};
    };
}

#endif