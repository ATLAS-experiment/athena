// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#ifndef FPGATrackSim_LAYERSTUDYALG_H
#define FPGATrackSim_LAYERSTUDYALG_H

/*
 * Layer study algorithm: runs data preparation and then dumps all the hits
 * into FPGATrackSimBinning, to produce a "layer study" tree.
 *
 * Pulled out of the genscan/inside out pattern recognition block, to facilitate
 * running layer studies with either the first stage or second stage hits (or both).
 */

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "FPGATrackSimMaps/FPGATrackSimSpacePointsToolI.h"
#include "FPGATrackSimBanks/IFPGATrackSimBankSvc.h"
#include "FPGATrackSimMaps/IFPGATrackSimMappingSvc.h"
#include "FPGATrackSimConfTools/IFPGATrackSimEventSelectionSvc.h"
#include "FPGATrackSimBinning/FPGATrackSimBinnedHits.h"
#include "FPGATrackSimBinning/FPGATrackSimLayerStudyTool.h"

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

class FPGATrackSimCluster;
class FPGATrackSimHit;
class FPGATrackSimRoad;
class FPGATrackSimTrack;

class FPGATrackSimLayerStudyAlg : public AthAlgorithm
{
    public:
        FPGATrackSimLayerStudyAlg(const std::string& name, ISvcLocator* pSvcLocator);
        virtual ~FPGATrackSimLayerStudyAlg() = default;

        virtual StatusCode initialize ATLAS_NOT_THREAD_SAFE() override;
        virtual StatusCode execute ATLAS_NOT_THREAD_SAFE() override;
        virtual StatusCode finalize() override;

    private:

        std::string m_description;

        // Handles
        ToolHandle<FPGATrackSimBinnedHits>      m_hitBinningTool {this, "BinningTool", "FPGATrackSimBinning/FPGATrackSimBinnedHits"};
        ToolHandle<FPGATrackSimLayerStudyTool>   m_binMonitoring  {this, "BinMonitoringTool", "FPGATrackSimBinning/FPGATrackSimLayerStudyTool"};

        ServiceHandle<IFPGATrackSimMappingSvc>           m_FPGATrackSimMapping {this, "FPGATrackSimMapping", "FPGATrackSimMappingSvc", "FPGATrackSimMappingSvc"};
        ServiceHandle<IFPGATrackSimEventSelectionSvc>    m_evtSel {this, "eventSelector", "FPGATrackSimEventSelectionSvc", "Event selection Svc"};

        // chrono service
        ServiceHandle<IChronoStatSvc> m_chrono{this,"ChronoStatSvc","ChronoStatSvc"};

        // Flags
        Gaudi::Property<int> m_stage {this, "stage", 0, "0 for all hits; 1 for pmap-indicated first stage; 2 for pmap-indicated second stage"};
        Gaudi::Property<int> m_threshold {this, "threshold", -1, "Threshold to apply for selecting bins with hits, defaults to not used (-1)"};

        // Event storage
        std::vector<FPGATrackSimTrack>   m_tracks_1st_guessedcheck, m_tracks_1st_nomiss, m_tracks_2nd_guessedcheck, m_tracks_2nd_nomiss;

        // internal counters
        double m_evt = 0; // number of events passing event selection, independent of truth
        double m_evt_truth = 0; // number of events passing event selection and having a truth object

        // Read hits from data prep algorithm-- note, not regonalized.
        SG::ReadHandleKey<FPGATrackSimHitCollection> m_FPGAHitKey {this, "FPGATrackSimHitKey","FPGAHits", "FPGATrackSim hits key"};

        // We do also need to pull truth tracks. I don't think we need offline tracks.
        SG::ReadHandleKey<FPGATrackSimTruthTrackCollection> m_FPGATruthTrackKey {this, "FPGATrackSimTruthTrackKey", "FPGATruthTracks", "FPGATrackSim truth tracks"};
};


#endif // FPGATrackSimLOGICALHITSTOALGORITHMS_h
