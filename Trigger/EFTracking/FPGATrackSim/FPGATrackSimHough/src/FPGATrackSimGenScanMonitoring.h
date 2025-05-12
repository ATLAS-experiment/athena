// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#ifndef FPGATrackSimGenScanMonitoring_H
#define FPGATrackSimGenScanMonitoring_H

/**
 * @file FPGATrackSimGenScanMonitoring.h
 * @author Elliot Lipeles
 * @date Sept 6th, 2024
 * @brief This is the monitoring for the FPGATrackSimGenScanTool 
 * 
 * Description: See the FPGATrackSimGenScanTool.h for overview. This class holds 
 *    all the historgrams and graphs output and controls the filling intereface 
 *    for all of them. The histogram outputs are enormously important for choosing 
 *    and optimizing the cuts and validating the code.
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

#include "FPGATrackSimGenScanTool.h"


 class FPGATrackSimGenScanMonitoring : public AthAlgTool
 {
    public:

    ///////////////////////////////////////////////////////////////////////
    // AthAlgTool

    FPGATrackSimGenScanMonitoring(const std::string &, const std::string &, const IInterface *);

    virtual StatusCode initialize() override;

    // This is done at the start of execution to create all the graphs
    // ... also stores some of the configuration parameters for later use
    StatusCode registerHistograms(const FPGATrackSimBinnedHits* binnedhits);
    void allocateDataFlowCounters();
    void resetDataFlowCounters();

    // This is done at the end of event execution to store any graphs that were created
    StatusCode registerGraphs();

    // Simple accessors for directory
    const std::string dir() const {return m_dir;}

    // Takes the truthtracks as input and parses it into a useful form for later use
    // (e.g. stores which bin the true track is in)
    void parseTruthInfo(std::vector<FPGATrackSimTruthTrack> const * truthtracks, bool isSingleParticle);

    // Fill methods
    void fillHitLevelInput(const FPGATrackSimHit* hit);
    void fillBinLevelOutput(
        const FPGATrackSimBinUtil::IdxSet &idx,
        const FPGATrackSimBinnedHits::BinEntry &data);
    void fillHitsByLayer(const std::vector<
            std::vector<const FPGATrackSimBinUtil::StoredHit *>>
            &hitsByLayer);
    void fillPairingHits(std::vector<const FPGATrackSimBinUtil::StoredHit *> const *lastlyr,
                         std::vector<const FPGATrackSimBinUtil::StoredHit *> const *lastlastlyr);
    void fillPairFilterCuts(const FPGATrackSimGenScanTool::HitPair &pair, double r_in, double r_out);
    void fillPairSetFilterCut(std::vector<TH1D *> &histset, double val,
                                    const FPGATrackSimGenScanTool::HitPair &pair,
                                    const FPGATrackSimGenScanTool::HitPair &lastpair, 
                                    bool nminus1);

    
    void fillBinningSummary(const std::vector<std::shared_ptr<const FPGATrackSimHit>> &hits);

    void fillBuildGroupsWithPairs(const std::vector<FPGATrackSimGenScanTool::IntermediateState>& states, unsigned allowed_misses);
    
    // Error Checks
    void sliceCheck();
    void pairFilterCheck(const FPGATrackSimGenScanTool::HitPairSet& pairs, 
                               const FPGATrackSimGenScanTool::HitPairSet& filteredpairs, 
                               bool passedPairFilter);
    void pairSetFilterCheck(
        const FPGATrackSimGenScanTool::HitPairSet &filteredpairs,
        const std::vector<FPGATrackSimGenScanTool::HitPairSet> &pairsets,
        unsigned threshold);

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
    std::vector<FPGATrackSimTruthTrack> const *m_truthtracks{nullptr};
    bool m_isSingleParticle = false;
    bool m_truthIsValid = false;
    FPGATrackSimTrackPars m_truthpars;
    std::vector<FPGATrackSimBinUtil::IdxSet> m_truthbin;
    FPGATrackSimBinUtil::ParSet m_truthparset;
    
    // plots are only filled for the truth bin if single particle sample
    // this gives the distributions of the cut variables when they are 
    // reconstructed in the right bin
    void setBinPlotsActive(const FPGATrackSimBinUtil::IdxSet &idx) { 
        m_binPlotsActive = ((m_truthbin.back() == idx) || (!m_isSingleParticle));}        
    // this flag governs if pair filter and pairset filter plots filled
    bool m_binPlotsActive = false;
    
    ///////////////////////////////////////////////////////////////////////
    // Data Flow Counters
    std::vector<unsigned> m_hitsCntByLayer;

    ///////////////////////////////////////////////////////////////////////
    // Histograms
    std::vector<TH2D *> m_rZ_allhits;
    TH1D *m_truthpars_hists[5] = {0, 0, 0, 0, 0};
    std::vector<TH1D *> m_phiResidual;
    std::vector<TH1D *> m_etaResidual;
    std::vector<TH1D *> m_phiTrueBinShift;
    std::vector<TH1D *> m_etaTrueBinShift;

    TH1D *m_inputHits = 0;

    // step-by-step plot
    std::vector<TH1D *> m_hitsPerStepBin;    
    TH1D * m_hitsPerLayer = 0;    
    TH2D * m_hitsPerLayer2D = 0;    
    TH1D * m_numLyrsPerBin = 0;

    TH2D *m_hitsPerLayer_bin = 0;

    TH1D *m_phiShift_road = 0;
    TH1D *m_etaShift_road = 0;
    TH2D *m_phiShift2D_road = 0;
    TH2D *m_etaShift2D_road = 0;
    

    TH1D *m_pairs = 0;
    TH1D *m_filteredpairs = 0;
    TH1D *m_pairsets = 0;

    // Build Pairs with Groups
    TH2D* m_unpairedHits = 0;
    TH2D* m_pairsetsIncr = 0;
    TH2D* m_pairsetsHits = 0;
    TH2D* m_binStagesIncr = 0;
    TH2D* m_totalInputIncr = 0;

    TH1D *m_pairinghits = 0;

    TH1D *m_roadFilterFlow = 0;

    TH1D *m_deltaPhi = 0;
    TH1D *m_deltaEta = 0;
    TH1D *m_deltaPhiDR = 0;
    TH1D *m_deltaEtaDR = 0;
    TH1D *m_phiOutExtrap = 0;
    TH1D *m_phiInExtrap = 0;
    TH1D *m_phiOutExtrapCurveLimit =0;
    TH1D *m_phiInExtrapCurveLimit =0;
    TH1D *m_etaOutExtrap = 0;
    TH1D *m_etaInExtrap = 0;
    std::vector<TH1D *> m_deltaPhiByLyr;
    std::vector<TH1D *> m_deltaEtaByLyr;
    
    // This classifies a pair of pairs into categories (described by m_twoPairClasses) 
    // for plotting the pair matching variables
    // an extra category is tacked on the end to make n-1 plots
    unsigned pairpairCategory(
        const FPGATrackSimGenScanTool::HitPair &pair,
        const FPGATrackSimGenScanTool::HitPair &lastpair) const;
    const static inline std::vector<std::string> m_twoPairClasses = {
        "", "_same", "_sequential", "_sameend", "_skip1", "_skip2", "_nm1"};
    unsigned m_nminus1_idx = 6;

    public:
    // These are exposed because of the fill mechanism
    // FGPATrackSimGenScanTool needs to be pointers to all
    // of them (not worth writing a bunch of accessors)
    std::vector<TH1D *> m_deltaDeltaPhi;
    std::vector<TH1D *> m_deltaDeltaEta;
    std::vector<TH1D *> m_pairSetMatchPhi;
    std::vector<TH1D *> m_pairSetMatchEta;
    std::vector<TH1D *> m_phiCurvature;
    std::vector<TH1D *> m_etaCurvature;
    std::vector<TH1D *> m_deltaPhiCurvature;
    std::vector<TH1D *> m_deltaEtaCurvature;
    std::vector<TH1D *> m_phiCurvatureMax;
    std::vector<TH1D *> m_etaCurvatureMax;
    std::vector<TH1D *> m_phiInExtrapCurved;
    std::vector<TH1D *> m_phiOutExtrapCurved;

    private:
    TH2D *m_pairMatchPhi2D = 0;
    TH2D *m_pairMatchEta2D = 0;

    //////////////////////////////////////////////////////////////////////
    // Event Displays

    // this struct creates, stores, and fills a set of TGraphs
    // that act as event displays saving hits in 2-d plane variables
    // both the standard FPGATrackSimHit R-Z and X-Y are saved,
    // and for StoredHits in bins, R-Phi and R-Eta are stored using
    // the FPGATrackSimGenScanTool phiShift and etaShift variables
    // to show the distance from the bin's center to the hit
    struct eventDispSet {
      eventDispSet(std::string name, int maxevts)
          : m_name(name), m_maxEvts(maxevts) {}

      TGraph *initGraph(const std::string &name);
      void AddPoint(TGraph *g, double x, double y);

      void addEvent(const std::vector<std::shared_ptr<const FPGATrackSimHit>> &hits);
      void addEvent(const std::vector<FPGATrackSimBinUtil::StoredHit> &hits);
      void addEvent(const std::vector<const FPGATrackSimBinUtil::StoredHit *> &hits);

      StatusCode registerGraphs(FPGATrackSimGenScanMonitoring* parent);

      private:
      const std::string m_name;
      const unsigned m_maxEvts;

      std::vector<TGraph *> m_rZ;    // detector coordinates
      std::vector<TGraph *> m_xY;    // detector coordinates
      std::vector<TGraph *> m_rEta;  // road coordinates
      std::vector<TGraph *> m_rPhi;  // road coordinates
    };

    // Instantiate event displays for various points in the logical flow
    eventDispSet m_allHitsGraph{std::string("allHits"), 10};
    eventDispSet m_roadGraph{std::string("road"), 10};
    eventDispSet m_lostPairFilterGraph{std::string("lostpairfilter"), 10};
    eventDispSet m_passPairFilterGraph{std::string("passpairfilter"), 10};
    eventDispSet m_lostPairSetFilterGraph{std::string("lostpairsetfilter"), 10};
    eventDispSet m_passPairSetFilterGraph{std::string("passpairsetfilter"), 10};

    // TTree for layer definitions studies
    StatusCode bookTree();
    void ClearTreeVectors();
    TTree *m_bin_module_tree = nullptr; // output tree
    std::vector<unsigned> m_tree_bin; // 5 tracks parameter bin
    std::vector<float> m_tree_r;
    std::vector<float> m_tree_z;
    std::vector<int> m_tree_id;
    std::vector<int> m_tree_hash;
    std::vector<int> m_tree_layer;
    std::vector<int> m_tree_side;
    std::vector<int> m_tree_etamod;
    std::vector<int> m_tree_phimod;
    std::vector<int> m_tree_dettype;
    std::vector<int> m_tree_detzone;

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

#endif // FPGATrackSimGenScanMonitoring_H
