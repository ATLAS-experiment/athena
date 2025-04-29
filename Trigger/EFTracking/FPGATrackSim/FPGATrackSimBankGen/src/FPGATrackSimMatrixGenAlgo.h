// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#ifndef FPGATrackSimMatrixGenAlgo_h
#define FPGATrackSimMatrixGenAlgo_h

/**
 * @file FPGATrackSimMatrixGenAlgo.h
 * @author Rewrite by Riley Xu - riley.xu@cern.ch after FTK code
 * @date May 8th, 2020
 * @brief Algorithm to generate matrix files, to be used for sector and constant generation.
 *
 * This algorithm uses muon truth tracks to generate matrix files. The matrix files
 * contain the sector definitions (list of modules) and an accumulation of the tracks'
 * hits and track parameters. These are later used by ConstGenAlgo to generate fit constants.
 *
 * For each sector, we store the information in the MatrixAccumulator struct. Each track
 * found in the sector has its parameters and hit coordinates added/appended to the struct.
 */


#include "GaudiKernel/ITHistSvc.h"
#include "AthenaBaseComps/AthAlgorithm.h"
#include "FPGATrackSimObjects/FPGATrackSimTypes.h"
#include "FPGATrackSimConfTools/IFPGATrackSimEventSelectionSvc.h"
#include "FPGATrackSimMaps/IFPGATrackSimMappingSvc.h"
#include "FPGATrackSimAlgorithms/FPGATrackSimTrackFitterTool.h"
#include "FPGATrackSimAlgorithms/FPGATrackSimOverlapRemovalTool.h"
#include "FPGATrackSimAlgorithms/FPGATrackSimWindowExtensionTool.h"
#include "FPGATrackSimMaps/FPGATrackSimPlaneMap.h"
#include "FPGATrackSimMaps/FPGATrackSimRegionMap.h"
#include "FPGATrackSimSGInput/IFPGATrackSimInputTool.h"
#include "FPGATrackSimInput/IFPGATrackSimEventInputHeaderTool.h"
#include "FPGATrackSimInput/FPGATrackSimRawToLogicalHitsTool.h"
#include "FPGATrackSimMaps/FPGATrackSimClusteringToolI.h"
#include "FPGATrackSimMaps/FPGATrackSimSpacePointsToolI.h"
#include "FPGATrackSimObjects/FPGATrackSimEventInputHeader.h"
#include "FPGATrackSimObjects/FPGATrackSimTruthTrack.h"
#include "FPGATrackSimHough/FPGATrackSimRoadUnionTool.h"
#include "FPGATrackSimMatrixIO.h"

#include "TTree.h"
#include "TFile.h"
#include "TH1I.h"

#include <string>
#include <vector>


class ITHistSvc;
class TH1F;
class TH2F;
class IFPGATrackSimMappingSvc;

class FPGATrackSimMatrixGenAlgo : public AthAlgorithm
{
    public:
        FPGATrackSimMatrixGenAlgo(const std::string& name, ISvcLocator* pSvcLocator);
        virtual ~FPGATrackSimMatrixGenAlgo() = default;

        StatusCode initialize() override;
        StatusCode execute() override;
        StatusCode finalize() override;

    private:

        ///////////////////////////////////////////////////////////////////////
        // Objects

        // Main logic. For each sector, store a struct that accumulates the track parameters, hit coordinates, etc.
        std::vector<AccumulateMap> m_sector_cum; // Index by region

        ///////////////////////////////////////////////////////////////////////
        // Handles

        ServiceHandle<IFPGATrackSimMappingSvc>    m_FPGATrackSimMapping{this,"FPGATrackSimMappingSvc","FPGATrackSimMappingSvc"};
        ServiceHandle<IFPGATrackSimEventSelectionSvc> m_EvtSel{this,"FPGATrackSimEventSelectionSvc","FPGATrackSimEventSelectionSvc"};
        ServiceHandle<ITHistSvc>             m_tHistSvc{this,"THistSvc","THistSvc"};

        ToolHandle<IFPGATrackSimInputTool>       m_hitInputTool {this, "FPGATrackSimSGToRawHitsTool", "FPGATrackSimSGToRawHitsTool/FPGATrackSimSGToRawHits", "input handler"};
	ToolHandle<FPGATrackSimRawToLogicalHitsTool> m_hitMapTool {this, "FPGATrackSimRawToLogicalHitsTool", "FPGATrackSimRawToLogicalHitsTool/FPGATrackSim_RawToLogicalHitsTool", "FPGATrackSim_RawToLogicalHitsTool"};
        ToolHandle<FPGATrackSimClusteringToolI>       m_clusteringTool { this, "FPGATrackSimClusteringFTKTool", "FPGATrackSimClusteringFTKTool/FPGATrackSimClusteringFTKTool", "FPGATrackSimClusteringFTKTool" };
        ToolHandle<FPGATrackSimSpacePointsToolI>       m_spacePointsTool { this, "SpacePointTool", "FPGATrackSimSpacePointsTool/FPGATrackSimSpacePointsTool", "FPGATrackSimSpacePointsTool" };
	const FPGATrackSimPlaneMap* m_pmap = nullptr; // alias to m_FPGATrackSimMapping->PlaneMap();
	ToolHandle<FPGATrackSimRoadUnionTool>       m_roadFinderTool {this, "RoadFinder", "RoadFinder"};
        ToolHandle<FPGATrackSimTrackFitterTool>          m_trackFitterTool_1st {this, "TrackFitter_1st", "FPGATrackSimTrackFitterTool/FPGATrackSimTrackFitterTool_1st", "1st stage track fit tool"};

        ToolHandle<IFPGATrackSimTrackExtensionTool>      m_trackExtensionTool {this, "TrackExtensionTool", "FPGATrackSimTrackExtensionTool", "Track extensoin tool"};
        ToolHandle<FPGATrackSimOverlapRemovalTool>       m_overlapRemovalTool {this, "OverlapRemoval_1st", "FPGATrackSimOverlapRemovalTool/FPGATrackSimOverlapRemovalTool_1st", "1st stage overlap removal tool"};


	const FPGATrackSimPlaneMap* m_pmap_1st = nullptr; // alias to m_FPGATrackSimMapping->PlaneMap();
	const FPGATrackSimPlaneMap* m_pmap_2nd = nullptr; // alias to m_FPGATrackSimMapping->PlaneMap();  


        ///////////////////////////////////////////////////////////////////////
        // Configuration
	Gaudi::Property<int> m_nRegions {this, "NBanks", 0, "Number of banks to make"};
	Gaudi::Property<bool> m_doClustering {this, "Clustering", true, "Do cluster?"};
        Gaudi::Property<bool> m_doSecondStage {this, "SecondStage", false, "Run second stage?"};
	Gaudi::Property<bool> m_doSpacePoints {this, "SpacePoints", true, "Do spacepoints?"};
	Gaudi::Property<int> m_ideal_geom {this, "IdealiseGeometry", 0, "Ideal geo flag, 0 is non, 1 is 1st order, 2 is 2nd order"};
	Gaudi::Property<bool> m_single {this, "SingleSector", false, "Run single sector"};
	Gaudi::Property<bool> m_doHoughConstants {this, "HoughConstants", true, "If true will run Hough Transform to set q/pt and phi0"};
	Gaudi::Property<bool> m_doDeltaPhiConsts {this, "DeltaPhiConstants", false, "If true will generate delta phi constants"};
	Gaudi::Property<int> m_MaxWC {this, "WCmax", 0, "Max number of WCs"};
        Gaudi::Property<int> m_minSpacePlusPixel {this, "minSpacePlusPixel", 4, "Require that tracks in training have a certain number of pixel hits + spacepoints" };
	Gaudi::Property<float> m_PT_THRESHOLD {this, "PT_THRESHOLD", 0., "Min pt"};
	Gaudi::Property<float> m_D0_THRESHOLD {this, "D0_THRESHOLD", 1., "Max d0"};
	Gaudi::Property<int> m_TRAIN_PDG {this, "TRAIN_PDG", 0, "PDG of particles to train on"};
	Gaudi::Property<float> m_temp_c_min {this, "par_c_min", -1, "Min curvature"};
	Gaudi::Property<float> m_temp_c_max {this, "par_c_max", 1, "Max curvature"};
	Gaudi::Property<float> m_temp_phi_min {this, "par_phi_min", -TMath::Pi(), "Min phi"};
	Gaudi::Property<float> m_temp_phi_max {this, "par_phi_max", TMath::Pi(), "Max phi"};
	Gaudi::Property<float> m_temp_d0_min {this, "par_d0_min", -2, "Min d0"};
	Gaudi::Property<float> m_temp_d0_max {this, "par_d0_max", 2, "Max d0"};
	Gaudi::Property<float> m_temp_z0_min {this, "par_z0_min", -200, "Min z0"};
	Gaudi::Property<float> m_temp_z0_max {this, "par_z0_max", 200, "Max z0"};
	Gaudi::Property<float> m_temp_eta_min {this, "par_eta_min", -5, "Min eta"};
	Gaudi::Property<float> m_temp_eta_max {this, "par_eta_max", 5, "Max eta"};
	Gaudi::Property<int> m_temp_c_slices {this, "par_c_slices", 100, "Number of c slides"};
	Gaudi::Property<int> m_temp_phi_slices {this, "par_phi_slices", 100, "Number of phi slices"};
	Gaudi::Property<int> m_temp_d0_slices {this, "par_d0_slices", 100, "Number of d0 slices"};
	Gaudi::Property<int> m_temp_z0_slices {this, "par_z0_slices", 100, "Number of z0 slices"};
	Gaudi::Property<int> m_temp_eta_slices {this, "par_eta_slices", 100, "Number of eta slices"};
    Gaudi::Property<bool> m_absQOverPtBinning{this, "qptAbsBinning", false, "This property controls whether or not to interpret the bins as q/pt or |q/pt|"};
	Gaudi::Property<std::vector<double> > m_qOverPtBins{this, "sectorQPtBins", {}, "q/pt bins for sector definition"};
    Gaudi::Property<bool> m_dropHitsAndFill{this, "dropHitsAndFill", false, "If true, we can drop hits to fill the accumulator"};

	int m_nLayers_1st = 0;
	int m_nDim_1st = 0;
  
    int m_nLayers_2nd = 0;
	int m_nDim_2nd = 0;
	
  
        FPGATrackSimTrackPars m_sliceMin = 0;
        FPGATrackSimTrackPars m_sliceMax = 0;
        FPGATrackSimTrackParsI m_nBins;

        FPGATrackSimEventInputHeader*         m_eventHeader = nullptr;

        ///////////////////////////////////////////////////////////////////////
        // Meta Data

        size_t m_nTracks = 0; // Total number of truth tracks encountered
        size_t m_nTracksUsed = 0; // Total number of tracks after filtering, i.e. used to fit constants

        ///////////////////////////////////////////////////////////////////////
        // Helper Functions

        enum class selectHit_returnCode { SH_FAILURE, SH_KEEP_OLD, SH_KEEP_NEW };

        StatusCode bookHistograms();
        std::vector<FPGATrackSimHit> getLogicalHits() ;
        std::vector<FPGATrackSimTruthTrack> filterTrainingTracks(std::vector<FPGATrackSimTruthTrack> const & truth_tracks) const;
        std::map<int, std::vector<FPGATrackSimHit>> makeBarcodeMap(std::vector<FPGATrackSimHit> const & hits, std::vector<FPGATrackSimTruthTrack> const & tracks) const;
        selectHit_returnCode selectHit(FPGATrackSimHit const & old_hit, FPGATrackSimHit const & new_hit, bool is1ststage, int subregion) const;
        bool filterSectorHits(std::vector<FPGATrackSimHit> const & all_hits, std::vector<FPGATrackSimHit> & sector_hits, FPGATrackSimTruthTrack const & t, bool is1ststage, int subregion) const;
        int getRegion(std::vector<FPGATrackSimHit> const & hits, bool is1ststage) const;
        StatusCode makeAccumulator(std::vector<FPGATrackSimHit> const & sector_hits, FPGATrackSimTruthTrack const & track, std::pair<std::vector<module_t>, FPGATrackSimMatrixAccumulator> & accumulator) const;
        StatusCode fillAccumulatorByDropping(std::vector<FPGATrackSimHit> & sector_hits, bool is1ststage, double x, double y, std::vector<module_t> &modules, AccumulateMap &map, FPGATrackSimTruthTrack const & track, int subregion) const;
        void roadsToTrack(std::vector<std::shared_ptr<const FPGATrackSimRoad>>& houghRoads, std::vector<FPGATrackSimTrack>& track_cands, bool isSecond);
  
        std::vector<TTree*> createMatrixTrees();
        void fillMatrixTrees(std::vector<TTree*> const & matrixTrees);
        void writeSliceTree();

        ///////////////////////////////////////////////////////////////////////
        // Histograms

        // These are ordered as in FPGATrackSimTrackPars, phi, curvature, d0, z0, eta
        TH1I* m_h_trainingTrack[FPGATrackSimTrackPars::NPARS]{};
        TH1I* m_h_sectorPars[FPGATrackSimTrackPars::NPARS]{};
        TH1I* m_h_SHfailure[FPGATrackSimTrackPars::NPARS]{};
        TH1I* m_h_3hitsInLayer[FPGATrackSimTrackPars::NPARS]{};
        TH1I* m_h_notEnoughHits[FPGATrackSimTrackPars::NPARS]{};

        TH1I* m_h_trackQoP_okHits = nullptr;
        TH1I* m_h_trackQoP_okRegion = nullptr;
        TH1I* m_h_nHit = nullptr;
};

#endif // FPGATrackSimMatrixGenAlgo_h
