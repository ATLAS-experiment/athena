// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#ifndef FPGATrackSimSPACEPOINTROADFILTERTOOL_H
#define FPGATrackSimSPACEPOINTROADFILTERTOOL_H

/**
 * @file FPGATrackSimSpacepointRoadFilterTool.cxx
 * @author Ben Rosser - brosser@uchicago.edu
 * @date May 24th, 2022
 * @brief Split roads with mixed hits and spacepoints in the same layers.
 *
 * Declarations in this file:
 *      class FPGATrackSimSpacepointRoadFilterTool : public AthAlgTool, virtual public FPGATrackSimRoadFilterToolI
 *
 */

#include "GaudiKernel/ServiceHandle.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include "FPGATrackSimObjects/FPGATrackSimTypes.h"
#include "FPGATrackSimObjects/FPGATrackSimConstants.h"
#include "FPGATrackSimObjects/FPGATrackSimVectors.h"
#include "FPGATrackSimObjects/FPGATrackSimRoad.h"
#include "FPGATrackSimObjects/FPGATrackSimHit.h"
#include "FPGATrackSimObjects/FPGATrackSimTrackPars.h"
#include "FPGATrackSimHough/IFPGATrackSimRoadFilterTool.h"
#include "FPGATrackSimBanks/IFPGATrackSimBankSvc.h"
#include "FPGATrackSimMaps/IFPGATrackSimMappingSvc.h"


#include "TH1.h"
#include "TFile.h"

#include <string>
#include <vector>
#include <map>
#include <boost/dynamic_bitset_fwd.hpp>

class FPGATrackSimSpacepointRoadFilterTool : public extends<AthAlgTool, IFPGATrackSimRoadFilterTool>
{
    public:
        /// Constructor
        using base_class::base_class;

        virtual StatusCode initialize() override;
        virtual StatusCode finalize() override;

        ///////////////////////////////////////////////////////////////////////
        // FPGATrackSimRoadFilterToolI

        virtual StatusCode filterRoads(std::vector<std::shared_ptr<const FPGATrackSimRoad>> & prefilter_roads, std::vector<std::shared_ptr<const FPGATrackSimRoad>> & postfilter_roads) override;

    private:

        ///////////////////////////////////////////////////////////////////////
        // Handles
        ServiceHandle<IFPGATrackSimMappingSvc> m_FPGATrackSimMapping {this, "FPGATrackSimMappingSvc", ""};
        ServiceHandle<IFPGATrackSimBankSvc> m_FPGATrackSimBankSvc {this, "FPGATrackSimBankSvc", ""};

        ///////////////////////////////////////////////////////////////////////
        // Properties

        Gaudi::Property <unsigned> m_threshold {this, "threshold", 0, "Minimum number of hit layers to accept as a road (inclusive"};
        Gaudi::Property <unsigned> m_minSpacePlusPixel {this, "minSpacePlusPixel", 0, "Minimum number of '2D' hits to accept as a road"};
        Gaudi::Property <unsigned> m_minSpacePlusPixel2 {this, "minSpacePlusPixel2", 0, "Minimum number of '2D' hits to accept as a road for 2nd stage"};
        Gaudi::Property <bool> m_filtering {this, "filtering", 0, "Filter out unpaired strip hits"};
        Gaudi::Property <bool> m_setSectors {this, "setSectors", true, "Should the bank service be used to set sectors."};
        Gaudi::Property <bool> m_isSecondStage {this, "isSecondStage", false, "Is this the second stage?"};

        ///////////////////////////////////////////////////////////////////////
        // Event Storage
        std::vector<FPGATrackSimRoad> m_postfilter_roads;
        ///////////////////////////////////////////////////////////////////////
        // Convenience

        ///////////////////////////////////////////////////////////////////////
        // Metadata and Monitoring

        TH1I* m_inputRoads{nullptr};
        TH1I* m_badRoads{nullptr};
    
        ///////////////////////////////////////////////////////////////////////
        // Helpers

        bool splitRoad(FPGATrackSimRoad* initial_road);
        unsigned setSector(FPGATrackSimRoad& road);
        unsigned findUnique(std::vector<std::shared_ptr<const FPGATrackSimHit>>& sp_in, std::vector<std::shared_ptr<const FPGATrackSimHit>>& sp_out,
                            std::vector<std::shared_ptr<const FPGATrackSimHit>>& unique_in, std::vector<std::shared_ptr<const FPGATrackSimHit>>& unique_out,
                            std::vector<std::shared_ptr<const FPGATrackSimHit>>& new_sp_in, std::vector<std::shared_ptr<const FPGATrackSimHit>>& new_sp_out);

};


#endif // FPGATrackSimSPACEPOINTROADFILTERTOOL_H
