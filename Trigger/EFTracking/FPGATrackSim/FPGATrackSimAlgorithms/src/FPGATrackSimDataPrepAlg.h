// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#ifndef FPGATrackSim_DATAPREPALG_H
#define FPGATrackSim_DATAPREPALG_H

/*
 * Please put a description on what this class does
 */

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "FPGATrackSimInput/FPGATrackSimOutputHeaderTool.h"
#include "FPGATrackSimObjects/FPGATrackSimEventInfo.h"
#include "FPGATrackSimInput/IFPGATrackSimEventInputHeaderTool.h"
#include "FPGATrackSimMaps/IFPGATrackSimHitFilteringTool.h"
#include "FPGATrackSimMaps/FPGATrackSimClusteringToolI.h"
#include "FPGATrackSimMaps/IFPGATrackSimMappingSvc.h"
#include "FPGATrackSimConfTools/FPGATrackSimEventSelectionTool.h"
#include "FPGATrackSimInput/FPGATrackSimRawToLogicalHitsTool.h"
#include "FPGATrackSimInput/FPGATrackSimReadRawRandomHitsTool.h"
#include "FPGATrackSimHough/FPGATrackSimHoughRootOutputTool.h"
#include "FPGATrackSimLRT/FPGATrackSimLLPRoadFilterTool.h"
#include "FPGATrackSimObjects/FPGATrackSimEventInputHeader.h"
#include "FPGATrackSimSGInput/IFPGATrackSimInputTool.h"

#include "AthenaMonitoringKernel/Monitored.h"

#include <fstream>

#include "StoreGate/StoreGateSvc.h"
#include "FPGATrackSimObjects/FPGATrackSimEventInfoCollection.h"
#include "FPGATrackSimObjects/FPGATrackSimClusterCollection.h"
#include "FPGATrackSimObjects/FPGATrackSimHitCollection.h"
#include "FPGATrackSimObjects/FPGATrackSimHitContainer.h"
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
class FPGATrackSimLogicalEventInputHeader;
class FPGATrackSimLogicalEventOutputHeader;

class FPGATrackSimDataPrepAlg : public ::AthReentrantAlgorithm
{
    public:
        FPGATrackSimDataPrepAlg(const std::string& name, ISvcLocator* pSvcLocator);
        virtual ~FPGATrackSimDataPrepAlg() = default;

        virtual StatusCode initialize() override;
        virtual StatusCode execute(const EventContext& ctx) const override final;
        virtual StatusCode finalize() override;

    private:

        std::string m_description;
        mutable std::atomic<unsigned> m_ev{0};

        // Handles
        ToolHandle<IFPGATrackSimInputTool>               m_hitSGInputTool {this, "SGInputTool", "", "Input tool from SG"};
        ToolHandle<IFPGATrackSimEventInputHeaderTool>    m_hitInputTool {this, "InputTool", "FPGATrackSimSGToRawHitsTool/FPGATrackSimSGToRawHitsTool", "Input Tool"};
        ToolHandle<FPGATrackSimReadRawRandomHitsTool>    m_hitInputTool2 {this, "InputTool2", "FPGATrackSimReadRawRandomHitsTool/FPGATrackSimReadRawRandomHitsTool", "Potential 2nd input Tool to load data from more than one source"};
        ToolHandleArray<FPGATrackSimRawToLogicalHitsTool>     m_hitMapTools {this, "RawToLogicalHitsTools", {}, "Raw To Logical Tools"};
        ToolHandle<IFPGATrackSimHitFilteringTool>        m_hitFilteringTool {this, "HitFilteringTool", "FPGATrackSimHitFilteringTool/FPGATrackSimHitFilteringTool", "Hit Filtering Tool"};
        ToolHandle<FPGATrackSimClusteringToolI>          m_clusteringTool {this, "ClusteringTool", "FPGATrackSimClusteringTool/FPGATrackSimClusteringTool", "Hit Clustering Tool"};
        ToolHandle<FPGATrackSimOutputHeaderTool>         m_writeOutputTool {this, "OutputTool", "FPGATrackSimOutputHeaderTool/FPGATrackSimOutputHeaderTool", "Output tool"};
        ToolHandleArray<FPGATrackSim::FPGATrackSimEventSelectionTool> m_eventSelectionTools {this, "eventSelectors", {}, "Event selection Tools"};
        // chrono service
        ServiceHandle<IChronoStatSvc> m_chrono{this,"ChronoStatSvc","ChronoStatSvc"};

        // Flags
        Gaudi::Property<bool> m_isDataPrepPipeline {this, "isDataPrepPipeline", false, "If True, this is for data prep pipeline only, thus skipping unecessary steps"};
        Gaudi::Property<int> m_firstInputToolN {this, "FirstInputToolN", 1, "number of times to use event from first input tool"};
        Gaudi::Property<int> m_secondInputToolN {this, "SecondInputToolN", 0, "number of times to use event from second input tool"};
        Gaudi::Property<bool> m_doHitFiltering {this, "HitFiltering", false, "flag to enable hit/cluster filtering"};
        Gaudi::Property<int> m_clustering {this, "Clustering", 0, "int to enable the clustering and say how many times to run it"};
        Gaudi::Property<bool> m_writeOutputData  {this, "writeOutputData", true,"write the output TTree"};
        Gaudi::Property<bool> m_writePreClusterBranch {this, "writePreClusterBranch", true, "If set to false, never write precluster branches"};
        // Properties for the output header tool.
  
        Gaudi::Property<bool> m_doEvtSel {this, "doEvtSel", false, "do event selection"};
        Gaudi::Property<bool> m_useInternalTruthTracks {this,"useInternalTruthTracks", false, "case when runnin on RDO file (and not or wrapper)"};
        Gaudi::Property<bool> m_recordHits {this,"recordHits", true, "For F-100 this is not needed"};
        Gaudi::Property<int> m_writeRegion {this,"writeRegion", -1, "Only output selected region, default is -1 which means not requirement"};

        // Properties for the output header tool.
        Gaudi::Property<std::string> m_preClusterBranch      {this, "preClusterBranch", "LogicalEventInputHeader_PreCluster", "Name of the branch for pre-cluster input data in output ROOT file." };
        Gaudi::Property<std::string> m_postClusterBranch     {this, "postClusterBranch", "LogicalEventInputHeader_PostCluster", "Name of the branch for post-cluster input data in output ROOT file." };

        // ROOT pointers
        FPGATrackSimLogicalEventInputHeader*  m_logicEventHeader_precluster = nullptr;
        FPGATrackSimLogicalEventInputHeader*  m_logicEventHeader = nullptr;

        // internal counters
        mutable std::atomic<size_t> m_evt = 0; // number of events passing event selection, independent of truth

        mutable std::atomic<unsigned long> m_nPixClusters = 0; // number of clusters for pix, total
        mutable std::atomic<unsigned> m_nMaxPixClusters = 0; // max number of pixel clusters in an event
        mutable std::atomic<unsigned long> m_nStripClusters = 0; // number of clusters for strip, total
        mutable std::atomic<unsigned> m_nMaxStripClusters = 0; // max number of strip clusters in an event
        mutable std::atomic<unsigned> m_nMaxClusters = 0; // max number of total clusters in an event


        StatusCode readInputs(const EventContext& ctx,
                     FPGATrackSimEventInputHeader& eventHeader,
                     FPGATrackSimEventInputHeader& firstInputHeader,
                     bool& done) const;
                     
        StatusCode processInputs(const FPGATrackSimEventInputHeader& eventHeader,
                        FPGATrackSimLogicalEventInputHeader& logicEventHeader,
                        FPGATrackSimLogicalEventInputHeader& logicEventHeader_precluster,
                        std::vector<std::unique_ptr<FPGATrackSimHit>>& hits_miss,
                        FPGATrackSimClusterCollection& clusters,
                        SG::WriteHandle<FPGATrackSimHitCollection> &FPGAHitUnmapped,
                        SG::WriteHandle<FPGATrackSimClusterCollection> &FPGAClusters) const;

        ToolHandle<GenericMonitoringTool> m_monTool{this,"MonTool", "", "Monitoring tool"};

        // NOTE: the clusters collection(s) contain ALL Clusters, not just "first stage" clusters.
        SG::WriteHandleKeyArray<FPGATrackSimClusterCollection> m_FPGAClusterKey{this, "FPGATrackSimClusterKey",{"FPGAClusters_1st"},"FPGATrackSim Clusters key"};
        SG::WriteHandleKey<FPGATrackSimHitCollection> m_FPGAHitKey{this, "FPGATrackSimHitKey","FPGAHits", "FPGATrackSim Hits key"};
        SG::WriteHandleKey<FPGATrackSimHitCollection> m_FPGAHitUnmappedKey{this, "FPGATrackSimHitUnmappedKey","FPGAHitsUnmapped_1st","FPGATrackSim Unmapped Hits 1st stage key"};

        SG::ReadHandleKey<xAOD::TruthParticleContainer> m_inputTruthParticleContainerKey{this, "TruthTrackContainer", "TruthParticles", "Truth Particle Container"};
        SG::WriteHandleKey<xAODTruthParticleLinkVector> m_truthLinkContainerKey{this, "TruthLinks", "xAODFPGATruthLinks", "Output EF xAODTruthLinks container"};

        SG::WriteHandleKey<FPGATrackSimTruthTrackCollection> m_FPGATruthTrackKey {this, "FPGATrackSimTruthTrackKey", "FPGATruthTracks", "FPGATrackSim truth tracks"};
        SG::WriteHandleKey<FPGATrackSimOfflineTrackCollection> m_FPGAOfflineTrackKey {this, "FPGATrackSimOfflineTrackKey", "FPGAOfflineTracks", "FPGATrackSim offline tracks"};
        SG::WriteHandleKey<FPGATrackSimEventInfo> m_FPGAEventInfoKey{this, "FPGATrackSimEventInfoKey", "FPGAEventInfo", "FPGATrackSim event info"};

        mutable std::mutex m_rootWriteMutex; // Protect ROOT writes in const execute()

};


#endif // FPGATrackSimLOGICALHITSTOALGORITHMS_h
