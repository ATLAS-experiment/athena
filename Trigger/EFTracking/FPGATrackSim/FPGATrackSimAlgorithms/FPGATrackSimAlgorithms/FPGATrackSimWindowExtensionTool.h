// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#ifndef FPGATrackSimWINDOWEXTENSION_H
#define FPGATrackSimWINDOWEXTENSION_H

/**
 * @file FPGATrackSimWindowExtensionTool.h
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
#include "FPGATrackSimBinning/FPGATrackSimBinnedHits.h"
#include "FPGATrackSimNNTrackTool.h"

#include <vector>

class FPGATrackSimWindowExtensionTool : public extends <AthAlgTool, IFPGATrackSimTrackExtensionTool>
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

        // Helper functions to extend a single track using different methods.
        bool extendTrackSliced(std::shared_ptr<const FPGATrackSimTrack> track, std::vector<int>& numHits, layer_bitmask_t& hitLayers,
                          std::vector<std::vector<std::shared_ptr<const FPGATrackSimHit>>>& road_hits);
        bool extendTrackBinned(std::shared_ptr<const FPGATrackSimTrack> track, std::vector<int>& numHits, layer_bitmask_t& hitLayers,
                          std::vector<std::vector<std::shared_ptr<const FPGATrackSimHit>>>& road_hits);

    private:

        ToolHandle<FPGATrackSimBinnedHits> m_hitBinningTool {this, "BinningTool", "FPGATrackSimBinning/FPGATrackSimBinnedHits"};

        ServiceHandle<IFPGATrackSimBankSvc> m_FPGATrackSimBankSvc {this, "FPGATrackSimBankSvc", "FPGATrackSimBankSvc"};
        ServiceHandle<IFPGATrackSimMappingSvc> m_FPGATrackSimMapping {this, "FPGATrackSimMappingSvc", "FPGATrackSimMappingSvc"};

        // We'll definitely need properties, but I don't know which ones.
        Gaudi::Property<int> m_threshold  { this, "threshold", 11, "Minimum number of hits to fire a road"};
        Gaudi::Property<std::vector<float>> m_windows {this, "phiWindow", {}, "Default window settings for phi, must be size nlayers."};
        Gaudi::Property<std::vector<float>> m_zwindows {this, "zWindow", {}, "Default window settings for z, must be size nlayers."};
        Gaudi::Property<bool> m_fieldCorrection {this, "fieldCorrection", true, "Use magnetic field correction for Hough transform"};
        Gaudi::Property<bool> m_idealGeoRoads {this, "IdealGeoRoads", true, "Do sector assignment of second stage roads"};
        Gaudi::Property<bool> m_doBinning {this, "doBinning", false, "Use second stage binning to sort hits, not the plane map"};

        // Options only needed for sector assignment.
        // The eta pattern option here should probably be dropped, because we're not using it
        // and supporting it requires having two sets of eta patterns (one for the first stage, one for the second)
        // and then running the eta pattern filter a second time.
        Gaudi::Property <bool> m_doRegionalMapping { this, "RegionalMapping", false,  "Use the sub-region maps to define the sector"};
        Gaudi::Property <bool> m_doEtaPatternConsts { this, "doEtaPatternConsts", false, "Whether to use the eta pattern tool for constant generation"};
        Gaudi::Property <bool> m_useSpacePoints { this, "useSpacePoints", false, "Whether we are using spacepoints."};

        std::vector<FPGATrackSimRoad> m_roads;
        //This is a map(dict python equivalent) of slice IDs that have a map of layer IDs in it. That map has a vector of hits associated with it
        std::map<unsigned, std::map<unsigned, std::vector<std::shared_ptr<const FPGATrackSimHit>>>> m_phits_atLayer;
        unsigned m_nLayers_1stStage = 0;
        unsigned m_nLayers_2ndStage = 0;
        unsigned m_maxMiss = 0;

        // Internal storage for the sliced hits (implemented as a LogicalEventInputHeader,
        // so we can easily copy to the output ROOT file).
        FPGATrackSimLogicalEventInputHeader*  m_slicedHitHeader = nullptr;

};

#endif
