// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#ifndef FPGATrackSim_LOGICALHITSPROCESSALG_H
#define FPGATrackSim_LOGICALHITSPROCESSALG_H

/*
 * Please put a description on what this class does
 */

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "FPGATrackSimInput/FPGATrackSimOutputHeaderTool.h"
#include "FPGATrackSimInput/IFPGATrackSimEventInputHeaderTool.h"
#include "FPGATrackSimHough/IFPGATrackSimRoadFilterTool.h"
#include "FPGATrackSimHough/IFPGATrackSimRoadFinderTool.h"
#include "FPGATrackSimHough/FPGATrackSimRoadUnionTool.h"
#include "FPGATrackSimMaps/FPGATrackSimSpacePointsToolI.h"
#include "FPGATrackSimMaps/IFPGATrackSimHitFilteringTool.h"
#include "FPGATrackSimBanks/IFPGATrackSimBankSvc.h"
#include "FPGATrackSimMaps/FPGATrackSimClusteringToolI.h"
#include "FPGATrackSimMaps/IFPGATrackSimMappingSvc.h"
#include "FPGATrackSimConfTools/IFPGATrackSimEventSelectionSvc.h"
#include "FPGATrackSimInput/FPGATrackSimRawToLogicalHitsTool.h"
#include "FPGATrackSimInput/FPGATrackSimReadRawRandomHitsTool.h"
#include "FPGATrackSimHough/FPGATrackSimHoughRootOutputTool.h"
#include "FPGATrackSimLRT/FPGATrackSimLLPRoadFilterTool.h"
#include "FPGATrackSimObjects/FPGATrackSimEventInputHeader.h"
#include "FPGATrackSimSGInput/IFPGATrackSimInputTool.h"

#include "AthenaMonitoringKernel/Monitored.h"

#include <fstream>

#include "StoreGate/StoreGateSvc.h"
#include "FPGATrackSimObjects/FPGATrackSimClusterCollection.h"
#include "FPGATrackSimObjects/FPGATrackSimHitCollection.h"
#include "FPGATrackSimObjects/FPGATrackSimHitContainer.h"
#include "FPGATrackSimObjects/FPGATrackSimRoadCollection.h"
#include "FPGATrackSimObjects/FPGATrackSimTrackCollection.h"
#include "FPGATrackSimObjects/FPGATrackSimTruthTrackCollection.h"
#include "FPGATrackSimObjects/FPGATrackSimOfflineTrackCollection.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/WriteHandleKeyArray.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadHandleKeyArray.h"

#include "GeneratorObjects/xAODTruthParticleLink.h"
#include "xAODTruth/TruthParticleContainer.h"

class FPGATrackSimHoughRootOutputTool;
class FPGATrackSimLLPRoadFilterTool;
class FPGATrackSimNNTrackTool;
class FPGATrackSimOverlapRemovalTool;
class FPGATrackSimTrackFitterTool;
class FPGATrackSimEtaPatternFilterTool;

class FPGATrackSimCluster;
class FPGATrackSimHit;
class FPGATrackSimRoad;
class FPGATrackSimLogicalEventInputHeader;
class FPGATrackSimLogicalEventOutputHeader;
class FPGATrackSimTrack;
class FPGATrackSimDataFlowInfo;

class FPGATrackSimLogicalHitsProcessAlg : public AthAlgorithm
{
    public:
        FPGATrackSimLogicalHitsProcessAlg(const std::string& name, ISvcLocator* pSvcLocator);
        virtual ~FPGATrackSimLogicalHitsProcessAlg() = default;

        virtual StatusCode initialize() override;
        virtual StatusCode execute() override;
        virtual StatusCode finalize() override;

    private:

        std::string m_description;

        // Handles
        ToolHandle<FPGATrackSimRoadUnionTool>            m_roadFinderTool {this, "RoadFinder", "FPGATrackSimPatternMatchTool", "Road Finder Tool"};
        ToolHandle<FPGATrackSimLLPRoadFilterTool>        m_LRTRoadFilterTool {this, "LRTRoadFilter", "FPGATrackSimLLPRoadFilterTool/FPGATrackSimLLPRoadFilterTool", "LRT Road Filter Tool"};
        ToolHandle<IFPGATrackSimRoadFinderTool>          m_LRTRoadFinderTool {this, "LRTRoadFinder", "FPGATrackSimHoughTransform_d0phi0_Tool/FPGATrackSimHoughTransform_d0phi0_Tool", "LRT Road Finder Tool"};
        ToolHandle<IFPGATrackSimRoadFilterTool>          m_roadFilterTool {this, "RoadFilter", "FPGATrackSimEtaPatternFilterTool", "Road Filter Tool"};
        ToolHandle<IFPGATrackSimRoadFilterTool>          m_roadFilterTool2 {this, "RoadFilter2", "FPGATrackSimPhiRoadFilterTool", "Road Filter2 Tool"};
        ToolHandle<IFPGATrackSimRoadFilterTool>          m_spRoadFilterTool {this, "SPRoadFilterTool", "FPGATrackSimSpacepointRoadFilterTool", "Spacepoint Road Filter Tool"};
        ToolHandle<FPGATrackSimNNTrackTool>              m_NNTrackTool {this, "NNTrackTool", "FPGATrackSimNNTrackTool/FPGATrackSimNNTrackTool", "NN Track Tool"};
        ToolHandle<FPGATrackSimHoughRootOutputTool>      m_houghRootOutputTool {this, "HoughRootOutputTool", "FPGATrackSimHoughRootOutputTool/FPGATrackSimHoughRootOutputTool", "Hough ROOT Output Tool"};
        ToolHandle<FPGATrackSimTrackFitterTool>          m_trackFitterTool_1st {this, "TrackFitter_1st", "FPGATrackSimTrackFitterTool/FPGATrackSimTrackFitterTool_1st", "1st stage track fit tool"};
        ToolHandle<FPGATrackSimOverlapRemovalTool>       m_overlapRemovalTool_1st {this, "OverlapRemoval_1st", "FPGATrackSimOverlapRemovalTool/FPGATrackSimOverlapRemovalTool_1st", "1st stage overlap removal tool"};
        ToolHandle<FPGATrackSimOutputHeaderTool>         m_writeOutputTool {this, "OutputTool", "FPGATrackSimOutputHeaderTool/FPGATrackSimOutputHeaderTool", "Output tool"};
        ServiceHandle<IFPGATrackSimMappingSvc>           m_FPGATrackSimMapping {this, "FPGATrackSimMapping", "FPGATrackSimMappingSvc", "FPGATrackSimMappingSvc"};
        ServiceHandle<IFPGATrackSimEventSelectionSvc>    m_evtSel {this, "eventSelector", "FPGATrackSimEventSelectionSvc", "Event selection Svc"};
        
        // Flags
        Gaudi::Property<bool> m_doSpacepoints {this, "Spacepoints", false, "flag to enable the spacepoint formation"};
        Gaudi::Property<bool> m_doTracking {this, "tracking", false, "flag to enable the tracking"};
        Gaudi::Property<bool> m_doOverlapRemoval {this, "doOverlapRemoval", true , "flag to enable the overlap removal"}; // defaul true to not change functionality
        Gaudi::Property<bool> m_doMissingHitsChecks {this, "DoMissingHitsChecks", false};
        Gaudi::Property<bool> m_filterRoads  {this, "FilterRoads", false, "enable first road filter"};
        Gaudi::Property<bool> m_filterRoads2  {this, "FilterRoads2", false,  "enable second road filter"};
        Gaudi::Property<bool> m_doHoughRootOutput {this, "DoHoughRootOutput", false, "Dump output from the Hough Transform to flat ntuples"};
        Gaudi::Property<bool> m_doNNTrack  {this, "DoNNTrack", false, "Run NN track filtering"};
        Gaudi::Property<bool> m_doLRT {this, "doLRT", false, "Enable Large Radius Tracking"};
        Gaudi::Property<bool> m_doLRTHitFiltering {this, "LRTHitFiltering", false, "flag to enable hit/cluster filtering for LRT"};
        Gaudi::Property<bool> m_writeOutputData  {this, "writeOutputData", true,"write the output TTree"};
        Gaudi::Property<bool> m_runOnRDO {this,"runOnRDO", false, "case when runnin on RDO file (and not or wrapper)"};
        Gaudi::Property<float> m_trackScoreCut {this, "TrackScoreCut", 25.0, "Minimum track score (e.g. chi2 or NN)." };
        Gaudi::Property<bool> m_writeOutNonSPStripHits {this, "writeOutNonSPStripHits", true, "Write tracks to RootOutput if they have strip hits which are not SPs"};
        Gaudi::Property <int> m_NumOfHitPerGrouping { this, "NumOfHitPerGrouping", 5, "Number of minimum overlapping hits for a track candidate to be removed in the HoughRootOutputTool"};
        Gaudi::Property<bool> m_passLowestChi2TrackOnly {this,"passLowestChi2TrackOnly", false, "case when passing only lowest chi2 track per road"};

        // Properties for the output header tool.
        Gaudi::Property<std::string> m_sliceBranch  {this, "SliceBranchName", "LogicalEventSlicedHeader", "Name of the branch for slied hits in output ROOT file." };
        Gaudi::Property<std::string> m_outputBranch {this, "outputBranchName", "LogicalEventOutputHeader", "Name of the branch for output data in output ROOT file." };

        // ROOT pointers.
        FPGATrackSimLogicalEventInputHeader*  m_slicedHitHeader = nullptr;
        FPGATrackSimLogicalEventOutputHeader* m_logicEventOutputHeader = nullptr;

        // Event storage
        std::vector<FPGATrackSimTrack>   m_tracks_1st_guessedcheck, m_tracks_1st_nomiss, m_tracks_2nd_guessedcheck, m_tracks_2nd_nomiss;

        // internal counters
        double m_evt = 0; // number of events passing event selection, independent of truth
        long m_nRoadsTot = 0; // total number of roads in those events
        long m_nTracksTot = 0; // total number of tracks in those events
        long m_nTracksChi2Tot = 0; // total number of tracks passing chi2 in those events
        long m_nTracksChi2OLRTot = 0; // total number of tracks passing chi2 and OLR in those events

        double m_evt_truth = 0; // number of events passing event selection and having a truth object
        long m_nRoadsFound = 0; // total number of those events with at least one road
        long m_nTracksFound = 0; // total number of those events with at least one track
        long m_nTracksChi2Found = 0; // total number of those events with at least one track passing chi2
        long m_nTracksChi2OLRFound = 0; // total number of those events with at least one track passing chi2 and OLR


        StatusCode writeOutputData(const std::vector<std::shared_ptr<const FPGATrackSimRoad>> & roads_1st, std::vector<FPGATrackSimTrack> const & tracks_1st,
                                   FPGATrackSimDataFlowInfo const * dataFlowInfo);

        void printHitSubregions(std::vector<FPGATrackSimHit> const & hits);

        ToolHandle<GenericMonitoringTool> m_monTool{this,"MonTool", "", "Monitoring tool"};

        // Read hits from data prep algorithm. TODO: regionalize.
        SG::ReadHandleKeyArray<FPGATrackSimHitCollection> m_FPGAHitKey{this, "FPGATrackSimHitKey",{"FPGAHits_1st"},"FPGATrackSim Hits key"};
        SG::ReadHandleKeyArray<FPGATrackSimHitCollection> m_FPGAHitKey_2nd{this, "FPGATrackSimHitKey_2nd",{"FPGAHits_2nd"},"FPGATrackSim 2nd stage hits key"};

        // Write out roads, hits in roads, and tracks.
        SG::WriteHandleKey<FPGATrackSimHitCollection> m_FPGAHitFilteredKey{this, "FPGATrackSimHitFiltered1stKey","FPGAHitsFiltered_1st","FPGATrackSim Filtered Hits 1st stage key"};
        SG::WriteHandleKey<FPGATrackSimHitContainer> m_FPGAHitInRoadsKey{this, "FPGATrackSimHitInRoads1stKey","FPGAHitsInRoads_1st","FPGATrackSim Hits in 1st stage roads key"};
        SG::WriteHandleKey<FPGATrackSimRoadCollection> m_FPGARoadKey{this, "FPGATrackSimRoad1stKey","FPGARoads_1st","FPGATrackSim Roads 1st stage key"};
        SG::WriteHandleKey<FPGATrackSimTrackCollection> m_FPGATrackKey{this, "FPGATrackSimTrack1stKey","FPGATracks_1st","FPGATrackSim Tracks 1st stage key"};

        SG::ReadHandleKey<xAOD::TruthParticleContainer> m_inputTruthParticleContainerKey{this, "TruthTrackContainer", "TruthParticles", "Truth Particle Container"};
        SG::ReadHandleKey<FPGATrackSimTruthTrackCollection> m_FPGATruthTrackKey {this, "FPGATrackSimTruthTrackKey", "FPGATruthTracks", "FPGATrackSim truth tracks"};
        SG::ReadHandleKey<FPGATrackSimOfflineTrackCollection> m_FPGAOfflineTrackKey {this, "FPGATrackSimOfflineTrackKey", "FPGAOfflineTracks", "FPGATrackSim offline tracks"};
};


#endif // FPGATrackSimLOGICALHITSTOALGORITHMS_h
