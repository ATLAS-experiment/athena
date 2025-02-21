
// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#ifndef FPGATrackSim_SECONDSTAGEALG_H
#define FPGATrackSim_SECONDSTAGEALG_H

/*
 * Please put a description on what this class does
 */

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "FPGATrackSimInput/FPGATrackSimOutputHeaderTool.h"
#include "FPGATrackSimHough/IFPGATrackSimRoadFilterTool.h"
#include "FPGATrackSimMaps/IFPGATrackSimMappingSvc.h"
#include "FPGATrackSimConfTools/IFPGATrackSimEventSelectionSvc.h"
#include "FPGATrackSimHough/FPGATrackSimHoughRootOutputTool.h"

#include "FPGATrackSimAlgorithms/IFPGATrackSimTrackExtensionTool.h"

#include "AthenaMonitoringKernel/Monitored.h"

#include <fstream>
#include "StoreGate/StoreGateSvc.h"
#include "FPGATrackSimObjects/FPGATrackSimHitCollection.h"
#include "FPGATrackSimObjects/FPGATrackSimHitContainer.h"
#include "FPGATrackSimObjects/FPGATrackSimRoadCollection.h"
#include "FPGATrackSimObjects/FPGATrackSimTrackCollection.h"
#include "FPGATrackSimObjects/FPGATrackSimTruthTrackCollection.h"
#include "FPGATrackSimObjects/FPGATrackSimOfflineTrackCollection.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadHandleKey.h"

class FPGATrackSimNNTrackTool;
class FPGATrackSimOverlapRemovalTool;
class FPGATrackSimTrackFitterTool;

class FPGATrackSimRoad;
class FPGATrackSimLogicalEventOutputHeader;
class FPGATrackSimLogicalEventInputHeader;
class FPGATrackSimTrack;
class FPGATrackSimDataFlowInfo;

class FPGATrackSimSecondStageAlg : public AthAlgorithm
{
    public:
        FPGATrackSimSecondStageAlg(const std::string& name, ISvcLocator* pSvcLocator);
        virtual ~FPGATrackSimSecondStageAlg() = default;

        virtual StatusCode initialize() override;
        virtual StatusCode execute() override;
        virtual StatusCode finalize() override;

    private:

        std::string m_description;

        // Handles
        ToolHandle<IFPGATrackSimTrackExtensionTool>      m_trackExtensionTool {this, "TrackExtensionTool", "FPGATrackSimTrackExtensionTool", "Track extensoin tool"};

        // We definitely need this one, in case we do Elliot's inside in -> extrapolate to spacepoints.
        ToolHandle<IFPGATrackSimRoadFilterTool>          m_spRoadFilterTool {this, "SPRoadFilterTool", "FPGATrackSimSpacepointRoadFilterTool", "Spacepoint Road Filter Tool"};

        // Hough ROOT output.
        ToolHandle<FPGATrackSimHoughRootOutputTool>      m_houghRootOutputTool {this, "HoughRootOutputTool", "FPGATrackSimHoughRootOutputTool/FPGATrackSimHoughRootOutputTool", "Hough ROOT Output Tool"};

        // Scoring and duplicate removal. We need second stage versions of these.
        ToolHandle<FPGATrackSimNNTrackTool>              m_NNTrackTool {this, "NNTrackTool", "FPGATrackSimNNTrackTool/FPGATrackSimNNTrackTool", "NN Track Tool"};
        ToolHandle<FPGATrackSimTrackFitterTool>          m_trackFitterTool {this, "TrackFitter_2nd", "FPGATrackSimTrackFitterTool/FPGATrackSimTrackFitterTool_2nd", "2nd stage track fit tool"};
        ToolHandle<FPGATrackSimOverlapRemovalTool>       m_overlapRemovalTool {this, "OverlapRemoval_2nd", "FPGATrackSimOverlapRemovalTool/FPGATrackSimOverlapRemovalTool_2nd", "2nd stage overlap removal tool"};

        // Miscellaneous infrastructure.
        ToolHandle<FPGATrackSimOutputHeaderTool>         m_writeOutputTool {this, "OutputTool", "FPGATrackSimOutputHeaderTool/FPGATrackSimOutputHeaderTool", "Output tool"};
        ServiceHandle<IFPGATrackSimMappingSvc>           m_FPGATrackSimMapping {this, "FPGATrackSimMapping", "FPGATrackSimMappingSvc", "FPGATrackSimMappingSvc"};
        ServiceHandle<IFPGATrackSimEventSelectionSvc>    m_evtSel {this, "eventSelector", "FPGATrackSimEventSelectionSvc", "Event selection Svc"};

        // Flags
        Gaudi::Property<bool> m_doSpacepoints {this, "Spacepoints", false, "flag to enable the spacepoint formation"};
        Gaudi::Property<bool> m_doTracking {this, "tracking", false, "flag to enable the tracking"};
        Gaudi::Property<bool> m_doMissingHitsChecks {this, "DoMissingHitsChecks", false};
        Gaudi::Property<bool> m_doHoughRootOutput {this, "DoHoughRootOutput", false, "Dump output from the Hough Transform to flat ntuples"};
        Gaudi::Property<bool> m_doNNTrack  {this, "DoNNTrack", false, "Run NN track filtering"};
        Gaudi::Property<bool> m_writeOutputData  {this, "writeOutputData", true,"write the output TTree"};
        Gaudi::Property<float> m_trackScoreCut {this, "TrackScoreCut", 25.0, "Minimum track score (e.g. chi2 or NN)." };
        Gaudi::Property<bool> m_writeOutNonSPStripHits {this, "writeOutNonSPStripHits", true, "Write tracks to RootOutput if they have strip hits which are not SPs"};
        Gaudi::Property<int> m_NumOfHitPerGrouping { this, "NumOfHitPerGrouping", 5, "Number of minimum overlapping hits for a track candidate to be removed in the HoughRootOutputTool"};
        Gaudi::Property<bool> m_passLowestChi2TrackOnly {this, "passLowestChi2TrackOnly", false};




        // Road filtering configuration, it isn't obvious to me if we want to allow these.
        Gaudi::Property<bool> m_filterRoads  {this, "FilterRoads", false, "enable first road filter"};
        Gaudi::Property<bool> m_filterRoads2  {this, "FilterRoads2", false,  "enable second road filter"};

        // Properties for the output header tool.
        Gaudi::Property<std::string> m_sliceBranch  {this, "SliceBranchName", "LogicalEventSlicedHeader", "Name of the branch for slied hits in output ROOT file." };
        Gaudi::Property<std::string> m_outputBranch     {this, "outputBranchName", "LogicalEventOutputHeader", "Name of the branch for output data in output ROOT file." };

        // ROOT pointers
        FPGATrackSimLogicalEventInputHeader*  m_slicedHitHeader = nullptr;
        FPGATrackSimLogicalEventOutputHeader* m_logicEventOutputHeader = nullptr;

        // Event storage. ??? do we need anything?

        // internal counters
        long m_evt = 0; // number of events passing event selection, independent of truth
        long m_nRoadsTot = 0; // total number of roads in those events
        long m_nTracksTot = 0; // total number of tracks in those events
        long m_nTracksChi2Tot = 0; // total number of tracks passing chi2 in those events
        long m_nTracksChi2OLRTot = 0; // total number of tracks passing chi2 and OLR in those events

        long m_evt_truth = 0; // number of events passing event selection and having a truth object
        long m_nRoadsFound = 0; // total number of those events with at least one road
        long m_nTracksFound = 0; // total number of those events with at least one track
        long m_nTracksChi2Found = 0; // total number of those events with at least one track passing chi2
        long m_nTracksChi2OLRFound = 0; // total number of those events with at least one track passing chi2 and OLR

        unsigned long m_maxNRoadsFound = 0; // max number of roads in an event
        unsigned long m_maxNTracksTot = 0; // max number of tracks in an event
        unsigned long m_maxNTracksChi2Tot = 0; // max number of tracks passing chi2 in an event
        unsigned long m_maxNTracksChi2OLRTot = 0; // max number of tracks passing chi2 and OLR in an events
  
        // TODO: what functions should we have here?
        StatusCode writeOutputData(const std::vector<std::shared_ptr<const FPGATrackSimRoad>> & roads_2nd,
                                   std::vector<FPGATrackSimTrack> const & tracks_2nd,
                                   FPGATrackSimDataFlowInfo const * dataFlowInfo);

        ToolHandle<GenericMonitoringTool> m_monTool{this,"MonTool", "", "Monitoring tool"};

        // Input: SECOND stage hits, FIRST stage tracks.
        SG::ReadHandleKey<FPGATrackSimHitCollection> m_FPGAHitKey{this, "FPGATrackSimHitKey", "FPGAHits_2nd", "FPGATrackSim Hits key"};
        SG::ReadHandleKey<FPGATrackSimTrackCollection> m_FPGAInputTrackKey {this, "FPGATrackSimTrack1stKey", "FPGATracks_1st", "FPGATrackSim tracks 1st stage key"};

        // Write out SECOND STAGE roads, hits in roads, and tracks.
        SG::WriteHandleKey<FPGATrackSimHitContainer> m_FPGAHitInRoadsKey{this, "FPGATrackSimHitInRoads2ndKey","FPGAHitsInRoads_2nd","FPGATrackSim Hits in 1st stage roads key"};
        SG::WriteHandleKey<FPGATrackSimRoadCollection> m_FPGARoadKey{this, "FPGATrackSimRoad2ndKey","FPGARoads_2nd","FPGATrackSim Roads 2nd stage key"};
        SG::WriteHandleKey<FPGATrackSimTrackCollection> m_FPGATrackKey{this, "FPGATrackSimTrack2ndKey","FPGATracks_2nd","FPGATrackSim Tracks 2nd stage key"};

        // Not sure if this algorithm also needs these.
        SG::ReadHandleKey<FPGATrackSimTruthTrackCollection> m_FPGATruthTrackKey {this, "FPGATrackSimTruthTrackKey", "FPGATruthTracks", "FPGATrackSim truth tracks"};
        SG::ReadHandleKey<FPGATrackSimOfflineTrackCollection> m_FPGAOfflineTrackKey {this, "FPGATrackSimOfflineTrackKey", "FPGAOfflineTracks", "FPGATrackSim offline tracks"};
};


#endif // FPGATrackSimLOGICALHITSTOALGORITHMS_h
