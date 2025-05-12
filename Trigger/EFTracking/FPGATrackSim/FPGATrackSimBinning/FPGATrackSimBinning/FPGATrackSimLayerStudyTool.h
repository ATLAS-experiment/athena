// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#ifndef FPGATrackSimLayerStudyTool_H
#define FPGATrackSimLayerStudyTool_H

/**
 * @file FPGATrackSimGenScanMonitoring.h
 * @author Elliot Lipeles, Ben Rosser
 * @date Sept 6th, 2024
 * @brief This is the monitoring for the FPGATrackSimGenScanTool
 *
 * Description: See the FPGATrackSimGenScanTool.h for overview. This class holds
 *    all the historgrams and graphs output and controls the filling intereface
 *    for all of them. The histogram outputs are enormously important for choosing
 *    and optimizing the cuts and validating the code.
 *
 * This is a (currently stripped down) version of GenScanMonitoring that ONLY contains
 * what's necessary to produce the "layer study" outputs-- dependence on internal
 * GenScan classes has been removed.
 *
 **/

#include "FPGATrackSimBinning/FPGATrackSimBinnedHits.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ITHistSvc.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include "TGraph.h"
class TH1D;
class TH2D;

#include "FPGATrackSimObjects/FPGATrackSimHit.h"
#include "FPGATrackSimObjects/FPGATrackSimTrackPars.h"
#include "FPGATrackSimObjects/FPGATrackSimTruthTrack.h"

#include "FPGATrackSimBinning/FPGATrackSimBinUtil.h"


 class FPGATrackSimLayerStudyTool : public AthAlgTool
 {
    public:

    ///////////////////////////////////////////////////////////////////////
    // AthAlgTool

    FPGATrackSimLayerStudyTool(const std::string &, const std::string &, const IInterface *);

    virtual StatusCode initialize() override;

    // This is done at the start of execution to create all the graphs
    // ... also stores some of the configuration parameters for later use
    StatusCode registerHistograms(const FPGATrackSimBinnedHits* binnedhits, bool skipTruth);
    void allocateDataFlowCounters();
    void resetDataFlowCounters();

    // Simple accessors for directory
    const std::string dir() const {return m_dir;}

    // Takes the truthtracks as input and parses it into a useful form for later use
    // (e.g. stores which bin the true track is in)
    void parseTruthInfo ATLAS_NOT_THREAD_SAFE(std::vector<FPGATrackSimTruthTrack> const & truthtracks);

    // Fill methods
    void fillHitLevelInput(const FPGATrackSimHit* hit);
    void fillBinLevelOutput ATLAS_NOT_THREAD_SAFE(const FPGATrackSimBinUtil::IdxSet &idx, const FPGATrackSimBinnedHits::BinEntry &data);
    void fillBinningSummary ATLAS_NOT_THREAD_SAFE(const std::vector<std::shared_ptr<const FPGATrackSimHit>> &hits);

    // Error Checks
    void sliceCheck();

   private:
    ///////////////////////////////////////////////////////////////////////
    // Handles
    ServiceHandle<ITHistSvc> m_tHistSvc{this, "THistSvc", "THistSvc"};

    ///////////////////////////////////////////////////////////////////////
    // Properties
    Gaudi::Property<std::string> m_dir{this, "dir", {"/GENSCAN/"}, "String name of output directory"};
    Gaudi::Property<double> m_phiScale{this, "phiScale", {}, "Scale for Delta Phi variable"};
    Gaudi::Property<double> m_etaScale{this, "etaScale", {}, "Scale for Delta Eta variable"};
    Gaudi::Property<double> m_drScale{this, "drScale", {}, "Scale for radius differences"};

    ///////////////////////////////////////////////////////////////////////
    // Pointer to binned hits
    const FPGATrackSimBinnedHits *m_binnedhits{nullptr};

    ///////////////////////////////////////////////////////////////////////
    // Parsed truth/info
    bool m_isSingleParticle = false;
    bool m_truthIsValid = false;
    FPGATrackSimTrackPars m_truthpars;
    std::vector<FPGATrackSimBinUtil::IdxSet> m_truthbin;
    FPGATrackSimBinUtil::ParSet m_truthparset;

    // plots are only filled for the truth bin if single particle sample
    // this gives the distributions of the cut variables when they are
    // reconstructed in the right bin
    void setBinPlotsActive(const FPGATrackSimBinUtil::IdxSet &idx) {m_binPlotsActive = ((m_truthbin.back() == idx) || (!m_isSingleParticle));}
    // this flag governs if pair filter and pairset filter plots filled
    bool m_binPlotsActive = false;

    ///////////////////////////////////////////////////////////////////////
    // Data Flow Counters

    ///////////////////////////////////////////////////////////////////////
    // Histograms
    std::vector<TH2D *> m_rZ_allhits;
    TH1D *m_truthpars_hists[5] = {0, 0, 0, 0, 0};

    TH1D *m_inputHits = 0;

    // residuals and shifts from truth bin center
    std::vector<TH1D *> m_phiResidual;
    std::vector<TH1D *> m_etaResidual;
    std::vector<TH1D *> m_phiTrueBinShift;
    std::vector<TH1D *> m_etaTrueBinShift;
    int m_N_ptplot{5}; // for 2-d plots in pt slices
    std::vector<TH2D *> m_phiResidual_v_r;
    std::vector<TH2D *> m_etaResidual_v_r;
    std::vector<TH2D *> m_phiScale_v_r;
    std::vector<TH2D *> m_etaScale_v_r;

    TH1D *m_phiShift_road = 0;
    TH1D *m_etaShift_road = 0;
    TH2D *m_phiShift2D_road = 0;
    TH2D *m_etaShift2D_road = 0;

    // step-by-step plot
    std::vector<TH1D *> m_hitsPerStepBin;
    TH1D * m_hitsPerLayer = 0;
    TH2D * m_hitsPerLayer2D = 0;
    TH2D *m_hitsPerLayer_bin = 0;

    private:

    // TTree for layer definitions studies
    StatusCode bookTrees();
    void ClearTreeVectors();

    TTree *m_bin_tree = nullptr; // output tree
    std::vector<unsigned> m_bin_tree_bin; // 5 tracks parameter bin
    std::vector<float> m_bin_tree_r;
    std::vector<float> m_bin_tree_z;
    std::vector<int> m_bin_tree_id;
    std::vector<int> m_bin_tree_hash;
    std::vector<int> m_bin_tree_layer;
    std::vector<int> m_bin_tree_side;
    std::vector<int> m_bin_tree_etamod;
    std::vector<int> m_bin_tree_phimod;
    std::vector<int> m_bin_tree_dettype;
    std::vector<int> m_bin_tree_detzone;

    TTree *m_truth_tree = nullptr; // output tree
    double m_truth_tree_phi{0.0};
    double m_truth_tree_qOverPt{0.0};
    double m_truth_tree_d0{0.0};
    double m_truth_tree_z0{0.0};
    double m_truth_tree_eta{0.0};
    std::vector<double> m_truth_tree_parset;

    //////////////////////////////////////////////////////////////////////
    // make and register histogram or vector of histograms in one line...
    template <typename HistType, typename... HistDef>
    StatusCode makeAndRegHist(HistType *&ptr, HistDef... histargs)
    {
        ptr = new HistType(histargs...);
        ATH_CHECK(m_tHistSvc->regHist(m_dir + ptr->GetName(), ptr));
        return StatusCode::SUCCESS;
    }


    template <typename HistType, typename... HistDef>
    StatusCode makeAndRegHistVector(std::vector<HistType*>& vec, unsigned len, const std::vector<std::string>* namevec, const char* namebase, HistDef... histargs)
    {
        if (vec.size()==0){
            for (unsigned i = 0; i < len; i++) {
                HistType *ptr = 0;
                std::string name = std::string(namebase);
                if (!namevec) {
                    name += std::to_string(i);
                } else {
                    if (namevec->size()==len) {
                        name += (*namevec)[i];
                    } else {
                        return StatusCode::FAILURE;
                    }
                }
                ATH_CHECK(makeAndRegHist(ptr, name.c_str(), histargs...));
                vec.push_back(ptr);
            }
        }
        return StatusCode::SUCCESS;
    }

    // Register a graph
    StatusCode regGraph(TGraph *g) const { return m_tHistSvc->regGraph(m_dir + g->GetName(), g);}

 };

#endif // FPGATrackSimLayerStudyTool_H
