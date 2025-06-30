// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#ifndef FPGATrackPATHFINDEREXTENSION_H
#define FPGATrackPATHFINDEREXTENSION_H

/**
 * @file FPGATrackSimNNPathfinderExtensionTool.h
 * @author Ben Rosser - brosser@uchicago.edu
 * @date 2024/10/08
 * @brief Default track extension algorithm to produce "second stage" roads.
 */

#include "GaudiKernel/ServiceHandle.h"
#include "AthenaBaseComps/AthAlgTool.h"

// add public header directory to algorithms for this?
#include "IFPGATrackSimTrackExtensionTool.h"

#include "FPGATrackSimObjects/FPGATrackSimTypes.h"
#include "FPGATrackSimObjects/FPGATrackSimFunctions.h"
#include "FPGATrackSimObjects/FPGATrackSimHit.h"
#include "FPGATrackSimObjects/FPGATrackSimRoad.h"
#include "FPGATrackSimObjects/FPGATrackSimTrack.h"
#include "FPGATrackSimMaps/IFPGATrackSimMappingSvc.h"
#include "FPGATrackSimBanks/IFPGATrackSimBankSvc.h"
#include "FPGATrackSimMaps/FPGATrackSimPlaneMap.h"
#include "FPGATrackSimMaps/FPGATrackSimRegionMap.h"
#include "FPGATrackSimObjects/FPGATrackSimTowerInputHeader.h"
#include "FPGATrackSimNNTrackTool.h"
#include "GaudiKernel/ITHistSvc.h"

#include <vector>

  // internal object for book-keeping during the tree branching, basically just a vector of hits with helper functions - NOTHING else
  struct miniRoad {

    miniRoad() {}
    miniRoad(unsigned nLayers) {m_hits.resize(nLayers);}

    // only one objects that we storek a vector of the hits 
    std::vector<std::shared_ptr<const FPGATrackSimHit>> m_hits;

    // getter functions
    std::shared_ptr<const FPGATrackSimHit> getHit(size_t layer) const {
      if (layer < m_hits.size()) return m_hits[layer];
      const FPGATrackSimHit dummyHit;
      return std::make_shared<const FPGATrackSimHit>(dummyHit);
    }

    std::vector<std::shared_ptr<const FPGATrackSimHit>>& getHits() {
      return m_hits;
    }

    std::vector<std::vector<std::shared_ptr<const FPGATrackSimHit>>> getVecHits() const {
      std::vector<std::vector<std::shared_ptr<const FPGATrackSimHit>>> vecHits;
      vecHits.resize(m_hits.size());
      for (unsigned layer = 0; layer < m_hits.size(); layer++) {
	std::vector<std::shared_ptr<const FPGATrackSimHit>> thislayerVec;
	thislayerVec.push_back(m_hits[layer]);
	vecHits[layer] = std::move(thislayerVec);
      }
      return vecHits;
    }

    unsigned getNHits() const {
      return m_hits.size();
    }
    
    size_t getNLayers() const { return m_hits.size(); }
    size_t getNHitLayers() const {
      return getNLayers() - getNWCLayers();
    }
    size_t getNWCLayers() const {
      size_t nwc = 0;
      for (auto hit : m_hits) {
	if (!hit->isReal()) nwc++;
      }
      return nwc;
    }
    
    layer_bitmask_t getWCLayers() const {
      layer_bitmask_t wcLayers = 0;
      for (unsigned layer = 0; layer < m_hits.size(); layer++)
	if (!((*m_hits[layer]).isReal()))
	  wcLayers |= (0x1 << layer);
      return wcLayers;
    }

    layer_bitmask_t getHitLayers() const {
      layer_bitmask_t hitLayers = 0;
      for (unsigned layer = 0; layer < m_hits.size(); layer++)
	if ((*m_hits[layer]).isReal())
	  hitLayers |= (0x1 << layer);
      return hitLayers;
    }

    
    // setters
    void setNLayers(unsigned layers) { m_hits.resize(layers); }    
    void setHits(std::vector<std::shared_ptr<const FPGATrackSimHit>> &hits) {
      m_hits = hits;
    }
    void addHits(const std::vector<std::shared_ptr<const FPGATrackSimHit>> &hits) {
      m_hits.insert(m_hits.end(), hits.begin(), hits.end());
    }
    
    void setHit(unsigned layer, const std::shared_ptr<const FPGATrackSimHit> & hit) {
      if (layer >= m_hits.size()) m_hits.resize(layer + 1);
      m_hits[layer] = hit;
    }

    void addHit(const std::shared_ptr<const FPGATrackSimHit> & hit) {
      m_hits.push_back(hit);
    }

    
    ///////////////////////////////////////////////////////////////////////
    // Utility


  };



class FPGATrackSimNNPathfinderExtensionTool   : public extends <AthAlgTool, IFPGATrackSimTrackExtensionTool>
{
    public:
        /// Constructor
        using base_class::base_class;

        virtual StatusCode initialize() override;

        virtual StatusCode extendTracks(const std::vector<std::shared_ptr<const FPGATrackSimHit>> & hits,
                                        const std::vector<std::shared_ptr<const FPGATrackSimTrack>> & tracks,
                                        std::vector<std::shared_ptr<const FPGATrackSimRoad>> & roads) override;

        // We don't have a "union" tool that sits in front of the extension tool, so this is needed here.
        virtual StatusCode setupSlices(FPGATrackSimLogicalEventInputHeader *slicedHitHeader) override {
          m_slicedHitHeader = slicedHitHeader;
          return StatusCode::SUCCESS;
        };


    private:
        ServiceHandle<IFPGATrackSimMappingSvc> m_FPGATrackSimMapping {this, "FPGATrackSimMappingSvc", "FPGATrackSimMappingSvc"};
        ServiceHandle<ITHistSvc> m_tHistSvc {this, "THistSvc", "THistSvc"};

        Gaudi::Property<unsigned> m_maxMiss { this, "threshold", 2, "Maximum number of missing hits to reject a road"};

        // Options only needed for sector assignment.
        // The eta pattern option here should probably be dropped, because we're not using it
        // and supporting it requires having two sets of eta patterns (one for the first stage, one for the second)
        // and then running the eta pattern filter a second time.
        Gaudi::Property<std::vector<float>> m_windowR { this, "windowR", {20.0}, "Window Size to search in for r, either pass one value for all layers or use the number of layers"};
        Gaudi::Property<std::vector<float>> m_windowZ { this, "windowZ", {20.0}, "Window Size to search in for z, either pass one value for all layers or use the number of layers"};
        Gaudi::Property<std::vector<float>> m_windowPhi { this, "windowPhi", {0.4}, "Window Size to search in for phi, either pass one value for all layers or use the number of layers"};  
        Gaudi::Property <float> m_lowPtValueForWindowRScaling { this, "lowPtValueWindowR", -1, "Value in MeV below which we scale the r window size"};
        Gaudi::Property <float> m_lowPtWindowRScaling {this, "lowPtRScaling", 1.0, "Scaling factor for low pt in R"};
        Gaudi::Property <float> m_lowPtValueForWindowZScaling { this, "lowPtValueWindowZ", -1, "Value in MeV below which we scale the r window size"};
        Gaudi::Property <float> m_lowPtWindowZScaling {this, "lowPtZScaling", 1.0, "Scaling factor for low pt in Z"};
        Gaudi::Property <float> m_lowPtValueForWindowPhiScaling { this, "lowPtValueWindowPhi", -1, "Value in MeV below which we scale the phi window size"};
        Gaudi::Property <float> m_lowPtWindowPhiScaling {this, "lowPtPhiScaling", 1.0, "Scaling factor for low pt in Phi"};

        Gaudi::Property <float> m_missedHitRScaling {this, "missedHitRScaling", -1, "Amount to scale R window if previous hit was missed. Negative means this is disabled"};
        Gaudi::Property <float> m_missedHitZScaling {this, "missedHitZScaling", -1, "Amount to scale Z window if previous hit was missed. Negative means this is disabled"};
        Gaudi::Property <float> m_missedHitPhiScaling {this, "missedHitPhiScaling", -1, "Amount to scale Phi window if previous hit was missed. Negative means this is disabled"};    
        Gaudi::Property <int> m_maxBranches { this, "maxBranches", -1, "Max number of branches before we stop, if negative this is disabled"};
        Gaudi::Property <bool> m_doOutsideIn { this, "doOutsideIn", true, "Setup the tool so it's doing outside in extrap"};
        Gaudi::Property <int> m_predictionWindowLength { this, "predictionWindowLength", 3, "Length of hits needed for prediction"};
        Gaudi::Property <bool> m_useCartesian { this, "useCartesian", true, "If true, NNs use Cartestian coordinates. If false,they use cylindrical coordiantes"};
  
        StatusCode bookTree();
        TTree *m_tree = nullptr; // output tree
        std::vector<unsigned long> m_NcompletedRoads;
        std::vector<unsigned int> m_missingHitsOnRoad;
        std::vector<std::vector<unsigned long>> m_predictedHitsFineID;
        std::vector<std::vector<unsigned int>> m_foundHitITkLayer;
        std::vector<unsigned int> m_nHitsInSearchWindow;
        std::vector<std::vector<float>> m_distanceOfPredictedHitToFoundHit;
        std::vector<std::vector<bool>> m_foundHitIsSP;

        std::vector<FPGATrackSimRoad> m_roads;
        unsigned m_nLayers_1stStage = 0;
        unsigned m_nLayers_2ndStage = 0;

        static float getXScale() { return 1015.;};
        static float getYScale() { return 1015.;};
        static float getZScale() { return 3000.;};
        static float getRScale() {return 1015.;};
        static float getPhiScale() {return 3.15;};
  
        bool m_debugEvent = false;

        // Internal storage for the sliced hits (implemented as a LogicalEventInputHeader,
        // so we can easily copy to the output ROOT file).
        FPGATrackSimLogicalEventInputHeader*  m_slicedHitHeader = nullptr;
  
        OnnxRuntimeBase m_extensionVolNN;
        OnnxRuntimeBase m_extensionHitNN;

        StatusCode fillInputTensorForNN(miniRoad& thisRoad, std::vector<float>& inputTensorValues);
        StatusCode getPredictedHit(std::vector<float>& inputTensorValues, std::vector<float>& outputTensorValues, long& fineID);
        StatusCode addHitToRoad(miniRoad& newroad, miniRoad& currentRoad, const std::vector<std::shared_ptr<const FPGATrackSimHit>>& hits);
        StatusCode getFakeHit(miniRoad& currentRoad, std::vector<float>& predhit, const long& fineID, std::vector<std::shared_ptr<const FPGATrackSimHit>>& hits);
        StatusCode getLastLayer(miniRoad& currentRoad, unsigned& lastHitLayer, std::shared_ptr<const FPGATrackSimHit>& lastHit);

        StatusCode findHitinNextStripLayer(std::shared_ptr<const FPGATrackSimHit> hit, const std::vector<std::shared_ptr<const FPGATrackSimHit>>& hitList, std::vector<std::shared_ptr<const FPGATrackSimHit>>& hits);

        void printRoad(miniRoad& currentRoad);

  
};

#endif
