// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#ifndef FPGATrackSim_LOGICALHITSPROCESSALG_H
#define FPGATrackSim_LOGICALHITSPROCESSALG_H

/*
 * Please put a description on what this class does
 */

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "FPGATrackSimInput/IFPGATrackSimEventOutputHeaderTool.h"
#include "FPGATrackSimInput/IFPGATrackSimEventInputHeaderTool.h"
#include "FPGATrackSimHough/IFPGATrackSimRoadFilterTool.h"
#include "FPGATrackSimHough/IFPGATrackSimRoadFinderTool.h"
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

class FPGATrackSimDataFlowTool;
class FPGATrackSimHoughRootOutputTool;
class FPGATrackSimLLPRoadFilterTool;
class FPGATrackSimNNTrackTool;
class FPGATrackSimOverlapRemovalTool;
class FPGATrackSimTrackFitterTool;
class FPGATrackSimEtaPatternFilterTool;

class FPGATrackSimCluster;
class FPGATrackSimHit;
class FPGATrackSimLogicalEventInputHeader;
class FPGATrackSimLogicalEventOutputHeader;
class FPGATrackSimRoad;
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
        int m_ev = 0;

        // Handles
        ToolHandle<IFPGATrackSimInputTool>               m_hitSGInputTool {this, "SGInputTool", "", "Input tool from SG"};
        ToolHandle<IFPGATrackSimEventInputHeaderTool>    m_hitInputTool {this, "InputTool", "FPGATrackSimSGToRawHitsTool/FPGATrackSimSGToRawHitsTool", "Input Tool"};
        ToolHandle<FPGATrackSimReadRawRandomHitsTool>    m_hitInputTool2 {this, "InputTool2", "FPGATrackSimReadRawRandomHitsTool/FPGATrackSimReadRawRandomHitsTool", "Potential 2nd input Tool to load data from more than one source"};
        ToolHandle<FPGATrackSimRawToLogicalHitsTool>     m_hitMapTool {this, "RawToLogicalHitsTool", "FPGATrackSim_RawToLogicalHitsTool/FPGATrackSim_RawToLogicalHitsTool", "Raw To Logical Tool"};
        ToolHandle<IFPGATrackSimHitFilteringTool>        m_hitFilteringTool {this, "HitFilteringTool", "FPGATrackSimHitFilteringTool/FPGATrackSimHitFilteringTool", "Hit Filtering Tool"};
        ToolHandle<FPGATrackSimClusteringToolI>          m_clusteringTool {this, "ClusteringTool", "FPGATrackSimClusteringTool/FPGATrackSimClusteringTool", "Hit Clustering Tool"};
        ToolHandle<FPGATrackSimSpacePointsToolI>         m_spacepointsTool {this, "SpacePointTool", "FPGATrackSimSpacePointsTool/FPGATrackSimSpacePointsTool", "Space Points Tool"};
        ToolHandle<IFPGATrackSimRoadFinderTool>          m_roadFinderTool {this, "RoadFinder", "FPGATrackSimPatternMatchTool", "Road Finder Tool"};
        ToolHandle<FPGATrackSimLLPRoadFilterTool>        m_LRTRoadFilterTool {this, "LRTRoadFilter", "FPGATrackSimLLPRoadFilterTool/FPGATrackSimLLPRoadFilterTool", "LRT Road Filter Tool"};
        ToolHandle<IFPGATrackSimRoadFinderTool>          m_LRTRoadFinderTool {this, "LRTRoadFinder", "FPGATrackSimHoughTransform_d0phi0_Tool/FPGATrackSimHoughTransform_d0phi0_Tool", "LRT Road Finder Tool"};
        ToolHandle<IFPGATrackSimRoadFilterTool>          m_roadFilterTool {this, "RoadFilter", "FPGATrackSimEtaPatternFilterTool", "Road Filter Tool"};
        ToolHandle<IFPGATrackSimRoadFilterTool>          m_roadFilterTool2 {this, "RoadFilter2", "FPGATrackSimPhiRoadFilterTool", "Road Filter2 Tool"};
        ToolHandle<IFPGATrackSimRoadFilterTool>          m_spRoadFilterTool {this, "SPRoadFilterTool", "FPGATrackSimSpacepointRoadFilterTool", "Spacepoint Road Filter Tool"};
        ToolHandle<FPGATrackSimNNTrackTool>              m_NNTrackTool {this, "NNTrackTool", "FPGATrackSimNNTrackTool/FPGATrackSimNNTrackTool", "NN Track Tool"};
        ToolHandle<FPGATrackSimHoughRootOutputTool>      m_houghRootOutputTool {this, "HoughRootOutputTool", "FPGATrackSimHoughRootOutputTool/FPGATrackSimHoughRootOutputTool", "Hough ROOT Output Tool"};
        ToolHandle<FPGATrackSimTrackFitterTool>          m_trackFitterTool_1st {this, "TrackFitter_1st", "FPGATrackSimTrackFitterTool/FPGATrackSimTrackFitterTool_1st", "1st stage track fit tool"};
        ToolHandle<FPGATrackSimTrackFitterTool>          m_trackFitterTool_2nd {this, "TrackFitter_2nd", "FPGATrackSimTrackFitterTool/FPGATrackSimTrackFitterTool_2nd", "2nd stage track fit tool"};
        ToolHandle<FPGATrackSimOverlapRemovalTool>       m_overlapRemovalTool_1st {this, "OverlapRemoval_1st", "FPGATrackSimOverlapRemovalTool/FPGATrackSimOverlapRemovalTool_1st", "1st stage overlap removal tool"};
        ToolHandle<FPGATrackSimOverlapRemovalTool>       m_overlapRemovalTool_2nd {this, "OverlapRemoval_2nd", "FPGATrackSimOverlapRemovalTool/FPGATrackSimOverlapRemovalTool_2nd", "2nd stage overlap removal tool"};
        ToolHandle<FPGATrackSimDataFlowTool>             m_dataFlowTool {this, "DataFlowTool", "FPGATrackSimDataFlowTool/FPGATrackSimDataFlowTool", "Data Flow Tool"};
        ToolHandle<IFPGATrackSimEventOutputHeaderTool>   m_writeOutputTool {this, "OutputTool", "FPGATrackSimOutputHeaderTool/FPGATrackSimOutputHeaderTool", "Output tool"};
        ServiceHandle<IFPGATrackSimMappingSvc>           m_FPGATrackSimMapping {this, "FPGATrackSimMapping", "FPGATrackSimMappingSvc", "FPGATrackSimMappingSvc"};
        ServiceHandle<IFPGATrackSimEventSelectionSvc>    m_evtSel {this, "eventSelector", "FPGATrackSimEventSelectionSvc", "Event selection Svc"};
        
        // Flags
	Gaudi::Property<int> m_firstInputToolN {this, "FirstInputToolN", 1, "number of times to use event from first input tool"};
	Gaudi::Property<int> m_secondInputToolN {this, "SecondInputToolN", 0, "number of times to use event from second input tool"};
        Gaudi::Property<bool> m_doHitFiltering {this, "HitFiltering", false, "flag to enable hit/cluster filtering"};
	Gaudi::Property<bool> m_clustering {this, "Clustering", false, "flag to enable the clustering"};
	Gaudi::Property<bool> m_doSpacepoints {this, "Spacepoints", false, "flag to enable the spacepoint formation"};
	Gaudi::Property<bool> m_doTracking {this, "tracking", false, "flag to enable the tracking"};
	Gaudi::Property<bool> m_doMissingHitsChecks {this, "DoMissingHitsChecks", false};
	Gaudi::Property<bool> m_filterRoads  {this, "FilterRoads", false, "enable first road filter"};
	Gaudi::Property<bool> m_filterRoads2  {this, "FilterRoads2", false,  "enable second road filter"};
	Gaudi::Property<bool> m_runSecondStage {this, "RunSecondStage", false,  "flag to enable running the second stage fitting"};
	Gaudi::Property<bool> m_doHoughRootOutput {this, "DoHoughRootOutput", false, "Dump output from the Hough Transform to flat ntuples"};
	Gaudi::Property<bool> m_doNNTrack  {this, "DoNNTrack", false, "Run NN track filtering"};
	Gaudi::Property<bool> m_doLRT {this, "doLRT", false, "Enable Large Radius Tracking"};
	Gaudi::Property<bool> m_doLRTHitFiltering {this, "LRTHitFiltering", false, "flag to enable hit/cluster filtering for LRT"};
        Gaudi::Property<bool> m_writeOutputData  {this, "writeOutputData", true,"write the output TTree"};
	Gaudi::Property<bool> m_outputHitTxt  {this, "outputHitTxt", false, "write out road hits to text file"};
	Gaudi::Property<bool> m_doEvtSel {this, "doEvtSel", true, "do event selection"};
	
        std::string m_outputHitTxtName = "outputRoadHits.txt";
        std::ofstream m_outputHitTxtStream;

        // ROOT pointers 
        FPGATrackSimEventInputHeader          m_eventHeader;
        FPGATrackSimEventInputHeader          m_firstInputHeader;
        FPGATrackSimLogicalEventInputHeader*  m_logicEventHeader_1st = nullptr;
        FPGATrackSimLogicalEventInputHeader*  m_logicEventHeader_2nd = nullptr;
        FPGATrackSimLogicalEventOutputHeader* m_logicEventOutputHeader = nullptr;

        // Event storage
        std::vector<FPGATrackSimCluster> m_clusters_1st, m_clusters_1st_original, m_clusters_2nd;
        std::vector<FPGATrackSimCluster> m_spacepoints_1st, m_spacepoints_2nd;
        std::vector<FPGATrackSimHit>     m_hits_1st_miss, m_hits_2nd_miss;
        std::vector<FPGATrackSimTrack>   m_tracks_1st_guessedcheck, m_tracks_1st_nomiss, m_tracks_2nd_guessedcheck, m_tracks_2nd_nomiss;


        StatusCode readInputs(bool & done);
        StatusCode processInputs();
        StatusCode secondStageProcessing(std::vector<FPGATrackSimTrack> const & tracks_1st,
                                         std::vector<FPGATrackSimRoad*> & roads_2nd, std::vector<FPGATrackSimTrack> & tracks_2nd);


        StatusCode writeOutputData(std::vector<FPGATrackSimRoad*> const & roads_1st, std::vector<FPGATrackSimTrack> const & tracks_1st,
                                   std::vector<FPGATrackSimRoad*> const & roads_2nd, std::vector<FPGATrackSimTrack> const & tracks_2nd,
                                   FPGATrackSimDataFlowInfo const * dataFlowInfo);

        void printHitSubregions(std::vector<FPGATrackSimHit> const & hits);

        ToolHandle<GenericMonitoringTool> m_monTool{this,"MonTool", "", "Monitoring tool"};
};

#endif // FPGATrackSimLOGICALHITSTOALGORITHMS_h
