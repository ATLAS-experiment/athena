// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#ifndef FPGATrackSimGenScanTool_H
#define FPGATrackSimGenScanTool_H

/**
 * @file FPGATrackSimGenScanTool.h
 * @author Elliot Lipeles
 * @date Sept 6th, 2024
 * @brief Implements a generalized 5d scan which filters pair of hits in bins linearized about nominal trajectory
 *
 * Declarations in this file:
 *      class FPGATrackSimGenScanTool : public AthAlgTool, virtual public IFPGATrackSimRoadFinderTool
 *
 * Overview of structure:
 *      - This tool generalized a Hough transform to scan in up to 5 track paramters
 *        (e.g. pt,eta,phi,d0,z0). Slides documenting algorithm and performance can be found here:
 *        https://its.cern.ch/jira/browse/EFTRACK-195
 *      - The 5 parameters are defined by a abstract base class "FPGATrackSimGenScanBinningBase"
 *        which is in seperate corresponing .h and .cxx files. Included there are the base class
 *        and implementations for the "StdBinning" = (pt,eta,phi,d0,z0) but also a KeyLyr based
 *        version which bins tracks by the phi and z values at two radii and a sagitta parameter
 *        (called xm here). See implementation details in that header.
 *      - FPGATrackSimGenScanBinning can define some of the 5 parameters to be "slice", "scan", and "row".
 *        E.g. a classic 2d-Hough could be implemented by defining slice parameters to be eta,z0
 *        scan to be pT, and row to be phi (with d0 unused)
 *      - In order to efficiently navigate and iterate over the 5d bins, a dedicated array class
 *        has been developed in FPGATrackSimGenScanArray.
 *      - After binning it creates pairs of hits in sequential layers and applies pair
 *        filter cuts to those to enforce consistency with the bin they are found in.  The cuts
 *        are based on internal "phiShift" and "etaShift" values that are calcualted as deviations the
 *        bins nominal track trajectory. The exact meaning of the phi and eta parameter can
 *        be changed in the FPGATrackSimGenScanBinning implementations. There is an internal subclass
 *        FPGATrackSimGenScanTool::HitPair to store hit pairs and calculates relevant variables.
 *           - There is internal subclass FPGATrackSimGenScanTool::StoredHit to store a hits with 
 *             their bin specific phiShifts/etaShifts
 *           - There is internal subclass FPGATrackSimGenScanTool::BinEntry to store set of StoredHits 
 *             for a bin and calculates some relevant local quantities
 *      - The tool then groups the pairs of hits into pairsets that are consistent (e.g. match
 *        translationally and in slope). 
 *           - There is an internal subclass FPGATrackSimGenScanTool::HitPairSet to store sets of 
 *             hit pairs and calculate relevan variables.
 *      - This code has detailed monitoring implemented in FPGATrackSimGenScanMonitoring to plot 
 *        all the variables that are cut on and uses truth information to make histograms for these 
 *        variables only in the bin which the track should be reconstructed. This allows for: 
 *        semi-automatic cut tuning, detailed internal dataflow numbers for firmware planning,
 *        and produces event displays using internal values for low-level debugging and investigations.
 *
 * References:
 *
 */

#include "GaudiKernel/ServiceHandle.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include "FPGATrackSimObjects/FPGATrackSimTypes.h"
#include "FPGATrackSimObjects/FPGATrackSimRoad.h"
#include "FPGATrackSimObjects/FPGATrackSimHit.h"
#include "FPGATrackSimHough/IFPGATrackSimRoadFinderTool.h"
#include "FPGATrackSimBanks/IFPGATrackSimBankSvc.h"
#include "FPGATrackSimMaps/IFPGATrackSimMappingSvc.h"
#include "FPGATrackSimConfTools/IFPGATrackSimEventSelectionSvc.h"

#include "FPGATrackSimGenScanBinning.h"
#include "FPGATrackSimGenScanArray.h"

#include <string>
#include <vector>
#include <utility>

class FPGATrackSimGenScanMonitoring;

class FPGATrackSimGenScanTool : public extends<AthAlgTool, IFPGATrackSimRoadFinderTool>
{
public:
  ///////////////////////////////////////////////////////////////////////
  // AthAlgTool
  
    FPGATrackSimGenScanTool(const std::string &, const std::string &, const IInterface *);

    virtual StatusCode initialize() override;

    ///////////////////////////////////////////////////////////////////////
    // IFPGATrackSimRoadFinderTool virtual method for interface
    virtual StatusCode getRoads(const std::vector<std::shared_ptr<const FPGATrackSimHit>> &hits,
                                std::vector<std::shared_ptr<const FPGATrackSimRoad>> &road) override;
    virtual int getSubRegion() const override {return 0;}

    friend class FPGATrackSimGenScanMonitoring;
    
protected:
    ///////////////////////////////////////////////////////////////////////
    // Handles

    ServiceHandle<IFPGATrackSimEventSelectionSvc> m_EvtSel{this, "FPGATrackSimEventSelectionSvc", "FPGATrackSimEventSelectionSvc"};
    ServiceHandle<IFPGATrackSimBankSvc> m_FPGATrackSimBankSvc{this, "FPGATrackSimBankSvc", "FPGATrackSimBankSvc"};
    ServiceHandle<IFPGATrackSimMappingSvc> m_FPGATrackSimMapping{this, "FPGATrackSimMappingSvc", "FPGATrackSimMappingSvc"};
    ToolHandle<FPGATrackSimGenScanMonitoring> m_monitoring {this, "Monitoring", "FPGATrackSimGenScanMonitoring", "Monitoring Tool"};
    ToolHandle<FPGATrackSimGenScanBinningBase> m_binning {this, "Binning", "FPGATrackSimGenScanBinningBase", "Gen Scan Binning Tool"};

    ///////////////////////////////////////////////////////////////////////
    // Properties
    Gaudi::Property<std::string> m_parSet{this, "parSet", {}, "String name of parameter set"};
    Gaudi::Property<std::vector<float>> m_parMin{this, "parMin", {}, "Vector of minimum bounds of parameters (expect 5"};
    Gaudi::Property<std::vector<float>> m_parMax{this, "parMax", {}, "Vector of maximum bounds of parameters (expect 5"};
    Gaudi::Property<std::vector<unsigned>> m_parBins{this, "parBins", {}, "Vector of number of bins for each parameter (expect 5)"};

    Gaudi::Property<double> m_rin{this, "rin", {-1.0}, "Radius of inner layer for extrapolations and keylayer definition"};
    Gaudi::Property<double> m_rout{this, "rout", {-1.0}, "Radius of outer layer for extrapolations and keylayer definition"};

    Gaudi::Property<std::string> m_lyrmapFile{this, "layerMapFile",{""}, "use externally defined layer map"};

    Gaudi::Property<double> m_d0FractionalPadding{this, "d0FractionalPadding", {}, "Fractional padding used when calculating the valid range of bins"};
    Gaudi::Property<double> m_z0FractionalPadding{this, "z0FractionalPadding", {}, "Fractional padding used when calculating the valid range of bins"};
    Gaudi::Property<double> m_etaFractionalPadding{this, "etaFractionalPadding", {}, "Fractional padding used when calculating the valid range of bins"};
    Gaudi::Property<double> m_phiFractionalPadding{this, "phiFractionalPadding", {}, "Fractional padding used when calculating the valid range of bins"};
    Gaudi::Property<double> m_qOverPtFractionalPadding{this, "qOverPtFractionalPadding", {}, "Fractional padding used when calculating the valid range of bins"};


    
    Gaudi::Property<unsigned> m_threshold{this, "threshold", {}, "Minimum value to accept as a road (inclusive)"};

    Gaudi::Property<std::string> m_binFilter{this, "binFilter", {"PairThenGroup"}, "which bin filter to run, current options: PairThenGroup, IncrementalBuild"};

    Gaudi::Property<bool> m_binningOnly{this, "binningOnly", {false}, "Turn off road building to test the binning only"};
    Gaudi::Property<bool> m_applyPairFilter{this, "applyPairFilter", {}, "Apply Pair Filter"};
    Gaudi::Property<bool> m_reversePairDir{this, "reversePairDir", {}, "Build Pairs starting at last layer and work in"};
    Gaudi::Property<std::vector<double>> m_pairFilterDeltaPhiCut{this, "pairFilterDeltaPhiCut", {}, "Pair Filter Delta Phi Cut Value (list one per layer)"};
    Gaudi::Property<std::vector<double>> m_pairFilterDeltaEtaCut{this, "pairFilterDeltaEtaCut", {}, "Pair Filter Delta Eta Cut Value (list one per layer)"};
    Gaudi::Property<std::vector<double>> m_pairFilterPhiExtrapCut{this, "pairFilterPhiExtrapCut", {}, "Pair Filter Phi Extrap Cut Value (in/out pair)"};
    Gaudi::Property<std::vector<double>> m_pairFilterEtaExtrapCut{this, "pairFilterEtaExtrapCut", {}, "Pair Filter Eta Extrap Cut Value(in/out pair)"};

    Gaudi::Property<bool> m_applyPairSetFilter{this, "applyPairSetFilter", {}, "Apply PairSet Filter"};
    Gaudi::Property<double> m_pairSetMatchPhiCut{this, "pairSetMatchPhiCut", {}, "Pair Set Match Phi Cut Value"};
    Gaudi::Property<double> m_pairSetMatchEtaCut{this, "pairSetMatchEtaCut", {}, "Pair Set Match Eta Cut Value"};
    Gaudi::Property<double> m_pairSetDeltaDeltaPhiCut{this, "pairSetDeltaDeltaPhiCut", {}, "Pair Set Delta Delta Phi Cut Value"};
    Gaudi::Property<double> m_pairSetDeltaDeltaEtaCut{this, "pairSetDeltaDeltaEtaCut", {}, "Pair Set Delta Eta Cut Value"};
    Gaudi::Property<double> m_pairSetPhiCurvatureCut{this, "pairSetPhiCurvatureCut", {}, "Pair Set Phi Cut Value"};
    Gaudi::Property<double> m_pairSetEtaCurvatureCut{this, "pairSetEtaCurvatureCut", {}, "Pair Set Eta Cut Value"};
    Gaudi::Property<double> m_pairSetDeltaPhiCurvatureCut{this, "pairSetDeltaPhiCurvatureCut", {}, "Pair Set Delta Phi Curvature Cut Value"};
    Gaudi::Property<double> m_pairSetDeltaEtaCurvatureCut{this, "pairSetDeltaEtaCurvatureCut", {}, "Pair Set Delta Eta Curvature Cut Value"};
    Gaudi::Property<std::vector<double>> m_pairSetPhiExtrapCurvedCut{this, "pairSetPhiExtrapCurvedCut", {}, "Pair Set Phi Extrap Curved Cut Value(in/out pair)"};

    ///////////////////////////////////////////////////////////////////////
    // Core

    // These are forwards for the internal data storage so you can read
    // the core elements before getting the clutter of details
    struct StoredHit;  // stores hit, plus offsets from nominal bin trajectory
    struct BinEntry; // stores list of StoredHit for a bin
    class HitPair; // pair of StoredHit with methods to make variables to cut on
    struct HitPairSet; // group of HitPair with methods to make variables to cut on

    // Which bins are consistent with the (pT, eta, pho, d0, z0)
    // ranges computed from region definition defined in the eventselection
    // service or set by the layer map
    void initValidBins();
    void computeValidBins();
    void setValidBin(std::vector<unsigned> idx);// reuse setting all the different idx types
    void printValidBin();// dump an output to log for x-checks

    // Put hits in all track parameter bins they could be a part of (binning is defined
    // by m_binning object)
    StatusCode fillImage(const std::vector<std::shared_ptr<const FPGATrackSimHit>> &hits);

    // Filter the bins above threshold into pairsets which output roads (2 options)
    //
    // Option 1) Originl version described here: https://indico.cern.ch/event/1469103/contributions/6187259/attachments/2952665/5190800/InsideOut_241010.pdf
    StatusCode pairThenGroupFilter(const BinEntry &bindata, std::vector<HitPairSet> &output_pairset);
    // Option 2) New version (no external documentation as of now)
    struct IntermediateState;
    void updateState(const IntermediateState &inputstate,
                     IntermediateState &outputstate,
                     unsigned lyridx, const std::vector<const StoredHit *>& newhits);
    StatusCode incrementalBuildFilter(const BinEntry &bindata, std::vector<HitPairSet> &output_pairset);
    
    // 1st step of filter: sort hits by layer
    StatusCode sortHitsByLayer(const BinEntry &bindata, std::vector<std::vector<const StoredHit *>> &hitsByLayer);

    // 2nd step of filter: make pairs of hits from adjacent and next-to-adjacent layers
    StatusCode makePairs(const std::vector<std::vector<const StoredHit *>>& hitsByLayer, HitPairSet &pairs);

    // 3rd step of filter: make cuts on the pairs to ensure that are consisten
    // with the bin they are in
    bool pairPassesFilter(const HitPair &pair);
    StatusCode filterPairs(HitPairSet &pairs, HitPairSet &filteredpairs);

    // 4th step of filter: group pairs into sets where they are all consistent with being from the same track
    StatusCode groupPairs(HitPairSet &filteredpairs, std::vector<HitPairSet> &clusters, bool verbose);

    // used to determine if a pair is consistend with a pairset
    bool pairMatchesPairSet(const HitPairSet &pairset, const HitPair &pair, bool verbose);

    // format final pairsets into expected output of getRoads
    void addRoad(std::vector<const StoredHit *> const &hits, const FPGATrackSimGenScanBinningBase::IdxSet &idx);

    ///////////////////////////////////////////////////////////////////////
    // Internal Storage Classes

    // Stores hit plus the phi/etashift from the nominal bin center
    struct StoredHit
    {
        std::shared_ptr<const FPGATrackSimHit> hitptr;
        double phiShift;
        double etaShift; // note this might be eta or z depending on m_binning
        int layer;
        double rzrad() const { return sqrt(hitptr->getR()*hitptr->getR()+hitptr->getZ()*hitptr->getZ());}
    };
    friend std::ostream &operator<<(std::ostream &os, const StoredHit &hit);

    // each bin contains a list of StoredHit objects 
    struct BinEntry
    {
        BinEntry() {}
        void reset();
        void addHit(StoredHit hit);
        unsigned int lyrCnt() { return __builtin_popcount(lyrhit); };
        unsigned int hitCnt = 0;
        layer_bitmask_t lyrhit = 0;
        std::vector<StoredHit> hits{};
    };

    ///////////////////////////////////////////////////////////////////////
    // HitPair and HitPairSet Storage Classes

    // Pair of hits, methods gives variable you might want to cut on
    class HitPair : public std::pair<const StoredHit *, const StoredHit *>
    {
    public:
        HitPair(const StoredHit *first, const StoredHit *second, bool reverse) : std::pair<const StoredHit *, const StoredHit *>(first, second), m_reverse(reverse) {}
        double dPhi() const { return this->second->phiShift - this->first->phiShift; }
        double dEta() const { return this->second->etaShift - this->first->etaShift; }
        double dR() const { return this->second->hitptr->getR() - this->first->hitptr->getR(); }

        // Eta/PhiExtrapolation methods below assume pair is radially ordered
        // this enables flipping them if working in reverse
        const HitPair Reversed() const {
            return HitPair(this->second, this->first, false);
        }

        double PhiOutExtrap(double r_out) const {
          if (m_reverse)
            return Reversed().PhiOutExtrap(r_out);
          return this->second->phiShift + this->dPhi() / this->dR() * (r_out - this->second->hitptr->getR());  }
        double PhiInExtrap(double r_in) const {
          if (m_reverse) return Reversed().PhiInExtrap(r_in);
          return this->first->phiShift + this->dPhi() / this->dR() * (r_in - this->first->hitptr->getR()); }
        double EtaOutExtrap(double r_out) const {
          if (m_reverse) return Reversed().EtaOutExtrap(r_out);
          return this->second->etaShift + this->dEta() / this->dR() * (r_out - this->second->hitptr->getR()); }
        double EtaInExtrap(double r_in) const {
          if (m_reverse) return Reversed().EtaInExtrap(r_in);
          return this->first->etaShift + this->dEta() / this->dR() * (r_in - this->first->hitptr->getR()); }

      private:
        bool m_reverse;
    };

    // Pair of hits, methods gives variable you might want to cut on
    struct HitPairSet
    {
        ////////////////////
        // Data

        // list of hit pairs
        std::vector<HitPair> pairList{};

        // bit mask of layers hit
        layer_bitmask_t hitLayers = 0;

        // list of hits in all included pairs
        std::vector<const StoredHit *> hitlist{};

        // saves curvatures from last pair added so change in curvature can be calculated
        double LastPhiCurvature = 0.;
        double LastEtaCurvature = 0.;

        ////////////////////
        // Methods - name are hopefully clear
        int addPair(const HitPair &pair);
        const HitPair &lastpair() const { return pairList.back(); }
        const HitPair &secondtolastpair() const { return *std::prev(pairList.end(), 2); }

        bool hasHit(const StoredHit *hit) const;
        bool hasLayer(int layer) const { return ((hitLayers & (0x1 << layer)) != 0); }
        unsigned int lyrCnt() const { return __builtin_popcount(hitLayers); }
        
        // variable to check if new pair is consistent with last pair added to pairset
        double MatchPhi(const HitPair &pair) const;
        double MatchEta(const HitPair &pair) const;

        double DeltaDeltaPhi(const HitPair &pair) const;

        double DeltaDeltaEta(const HitPair &pair) const;

        double PhiCurvature(const HitPair &pair) const;
        double EtaCurvature(const HitPair &pair) const;

        double DeltaPhiCurvature(const HitPair &pair) const;
        double DeltaEtaCurvature(const HitPair &pair) const;
        double PhiInExtrapCurved(const HitPair &pair, double r_in) const;
        double PhiOutExtrapCurved(const HitPair &pair, double r_out) const;
    };

    // state structure used by buildGroupsWithPairs method
    struct IntermediateState {
      std::vector<const StoredHit *> unpairedHits{};
      std::vector<HitPairSet> pairsets{};
    };
    
    ///////////////////////////////////////////////////////////////////////
    // Event Storage
    int m_evtsProcessed = 0;
    unsigned m_nLayers = 0; // copy of m_FPGATrackSimMapping->PlaneMap1stStage()->getNLogiLayers();
    std::vector<unsigned int> m_pairingLayers; 
    
    // The implementation of the binning base class that defines the binning to be used
    // FPGATrackSimGenScanBinningBase *m_binning{nullptr};

    // Main image (up to 5d) with the hits binned according to m_binning
    FPGATrackSimGenScanArray<BinEntry> m_image;

    // Tells which bins/slices/scans actually correspond to track parameters in the specified region
    FPGATrackSimGenScanArray<int> m_validBin;
    FPGATrackSimGenScanArray<int> m_validSlice;
    FPGATrackSimGenScanArray<int> m_validScan;
    FPGATrackSimGenScanArray<int> m_validSliceAndScan;

    // structure is indexed on bin, then layer, then a set of modules
    void readLayerMap(const std::string & filename);
    FPGATrackSimGenScanArray< std::vector <std::set<unsigned> > > m_lyr_to_mod_map;
    FPGATrackSimGenScanArray< std::map<unsigned,unsigned> > m_mod_to_lyr_map;

    // output roads
    std::vector<FPGATrackSimRoad> m_roads{};
};

#endif // FPGATrackSimGenScanTool_H
