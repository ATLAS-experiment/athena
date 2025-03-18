// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#ifndef FPGATrackSimHOUGHROOTOUTPUTTOOL_H
#define FPGATrackSimHOUGHROOTOUTPUTTOOL_H

/**
 * @file FPGATrackSimHoughRootOutputTool.h
 * @author Jahred Adelman
 * @date Jan 30 2021
 * @brief Output roads into a ROOT file
 *
 */

#include "GaudiKernel/ServiceHandle.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include "FPGATrackSimObjects/FPGATrackSimRoad.h"
#include "FPGATrackSimObjects/FPGATrackSimHit.h"
#include "FPGATrackSimObjects/FPGATrackSimTrack.h"
#include "FPGATrackSimObjects/FPGATrackSimMultiTruth.h"
#include "FPGATrackSimObjects/FPGATrackSimTruthTrack.h"
#include "FPGATrackSimObjects/FPGATrackSimOfflineTrack.h"

#include "GaudiKernel/ITHistSvc.h"

#include "FPGATrackSimMaps/IFPGATrackSimMappingSvc.h"
#include "FPGATrackSimMaps/FPGATrackSimPlaneMap.h"
#include "FPGATrackSimMaps/FPGATrackSimRegionMap.h"
#include "FPGATrackSimHough/FPGATrackSimHoughFunctions.h"

#include "TTree.h"


class IFPGATrackSimMappingSvc;
class IFPGATrackSimEventSelectionSvc;



class FPGATrackSimHoughRootOutputTool : public AthAlgTool
{
  public:

        ///////////////////////////////////////////////////////////////////////
        // AthAlgTool

        FPGATrackSimHoughRootOutputTool(const std::string&, const std::string&, const IInterface*);

        virtual StatusCode initialize() override;
        StatusCode fillTree(const std::vector<std::shared_ptr<const FPGATrackSimRoad>> &roads, const std::vector<FPGATrackSimTruthTrack> &truthTracks, const std::vector<FPGATrackSimOfflineTrack> &offlineTracks, const std::vector<std::shared_ptr<const FPGATrackSimHit>> &hits_2nd, const bool writeOutNonSPStripHits, const float minChi2, const int maxOverlappingHits, const bool roadsAreSecondStage);
        StatusCode fillTree(const std::vector<FPGATrackSimTrack> &track_cands, const std::vector<FPGATrackSimTruthTrack> &truthTracks, const std::vector<FPGATrackSimOfflineTrack> &offlineTracks, const std::vector<std::shared_ptr<const FPGATrackSimHit>> &hits_2nd, const bool writeOutNonSPStripHits, const bool roadsAreSecondStage);



    private:

        StatusCode bookTree();

        void ResetVectors();

	ServiceHandle<IFPGATrackSimEventSelectionSvc> m_EvtSel {this, "FPGATrackSimEventSelectionSvc", "FPGATrackSimEventSelectionSvc"};
        ServiceHandle<IFPGATrackSimMappingSvc> m_FPGATrackSimMapping {this, "FPGATrackSimMappingSvc", "FPGATrackSimMappingSvc"};
        ServiceHandle<ITHistSvc> m_tHistSvc {this, "THistSvc", "THistSvc"};
        Gaudi::Property <std::string> m_algorithm { this, "ORAlgo", "Normal", "Overlap removal algorithm"};

        ORAlgo m_algo{ORAlgo::Normal};       //  Internal ORAlgo enum for faster compare

        const FPGATrackSimRegionMap* m_SUBREGIONMAP = m_FPGATrackSimMapping->SubRegionMap();
        TrackCorrType m_IdealCoordFitType = TrackCorrType::None;


        TTree *m_tree = nullptr; // output tree
        std::vector<float> m_x; // x position of hit in road
        std::vector<float> m_y; // y pos
        std::vector<float> m_z; // z pos
        std::vector<int> m_volumeID; // custom volume ID
        std::vector<int> m_custom_layerID; // custom layer ID
        std::vector<int> m_layerID;
        std::vector<int> m_etaID;
        std::vector<float> m_gphi;
        std::vector<float> m_zIdeal; // idealized z
        std::vector<float> m_gphiIdeal; // idealized gphi
        std::vector<float> m_barcodefrac; // truth barcode fraction for the hit
        std::vector<unsigned long> m_barcode; // truth barcode for the hit
        std::vector<int> m_eventindex; // event index for the hit
        std::vector<unsigned int> m_isPixel; // is hit pixel? if 0 it is strip
        std::vector<unsigned int> m_layer; // layer ID
        std::vector<unsigned int> m_isBarrel; // is hit in barrel? if 0 it is endcap
        std::vector<unsigned int> m_etawidth;
        std::vector<unsigned int> m_phiwidth;
        std::vector<unsigned int> m_etamodule;
        std::vector<unsigned int> m_phimodule;
        std::vector<unsigned int> m_ID; // ID hash for hit
        std::vector<unsigned int> m_diskLayer;
        std::vector<unsigned int> m_passesOR;
        std::vector<float> m_roadChi2;
        std::vector<float> m_nMissingHits;
        std::vector<bool> m_mapped;
        std::vector<bool> m_realHit;

        TrackCorrType m_idealCoordFitType = TrackCorrType::None;


        float m_phi = 0.0F; // phi pre-estimate from the 2d hough
        float m_invpt = 0.0F; // invpt pre-estimate from the 2d hough
        unsigned int m_subregion = 0; // subregion of road
        long m_NTracksORMinusRoads = 0;

        // quantities for the track matched to truth, not per hit
        float m_candidate_barcodefrac = 0.0F;
        float m_candidate_barcode = 0.0F;
        float m_candidate_eventindex = 0.0F;
        unsigned int m_fakelabel = 0; // label for fake tracks. 1 if track candidate's barcodefrac is < 0.5, 0 else

        // track number in the event, since the request is to store this per road
        // naively vectors of vectors and one entry per event makes more sense but this was the
        // request from the ML people
        int m_tracknumber = 0;

        // this is the tree index used to connect to the truth information
        int m_treeindex = 0;

        // road number separates information from each road;
        int m_roadnumber = 0;

        // Separate tree with truth track information
        TTree *m_truthtree = nullptr; // output tree

        std::vector<float> m_truth_d0;
        std::vector<float> m_truth_z0;
        std::vector<float> m_truth_pt;
        std::vector<float> m_truth_eta;
        std::vector<float> m_truth_phi;
        std::vector<float> m_truth_pdg;
        std::vector<int> m_truth_q;
        std::vector<int> m_truth_barcode;
        std::vector<int> m_truth_eventindex;
        std::vector<bool> m_has_strip_nonspacepoint;
        std::vector<std::vector<float>> m_track_hit_x;
        std::vector<std::vector<float>> m_track_hit_y;
        std::vector<std::vector<float>> m_track_hit_z;
        std::vector<std::vector<float>> m_track_hit_R;
        std::vector<std::vector<float>> m_track_hit_eta;
        std::vector<std::vector<float>> m_track_hit_phi;
        std::vector<std::vector<int>> m_track_hit_layer_disk;
        std::vector<std::vector<bool>> m_track_hit_isPixel;
        std::vector<std::vector<bool>> m_track_hit_isStrip;
        std::vector<std::vector<bool>> m_track_hit_isClustered;
        std::vector<std::vector<bool>> m_track_hit_isSpacepoint;
        std::vector<std::vector<bool>> m_track_hit_isMapped;
        std::vector<std::vector<int>> m_track_hit_barcode;
        std::vector<std::vector<float>> m_track_hit_barcodefrac;
        std::vector<std::vector<float>> m_track_hit_zIdeal;
        std::vector<std::vector<float>> m_track_hit_gphiIdeal;
        std::vector<std::vector<long>> m_track_hit_fineID;
        std::vector<std::vector<int>> m_track_hit_volumeID;

        // And now the offline information
        TTree *m_offlinetree = nullptr;
        std::vector<int> m_offline_n_holes;
        std::vector<int> m_offline_n_measurement;
        std::vector<int> m_offline_n_inertmaterial;
        std::vector<int> m_offline_n_brempoint;
        std::vector<int> m_offline_n_scatterer;
        std::vector<int> m_offline_n_perigee;
        std::vector<int> m_offline_n_outlier;
        std::vector<int> m_offline_n_other;

        std::vector<float> m_offline_d0;
        std::vector<float> m_offline_z0;
        std::vector<float> m_offline_pt;
        std::vector<float> m_offline_eta;
        std::vector<float> m_offline_phi;
        std::vector<int> m_offline_q;
        std::vector<int> m_offline_barcode;
        std::vector<float> m_offline_barcodefrac;
};


#endif // FPGATrackSimHOUGHROOTOUTPUTTOOL_H
