// Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

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

        ///////////////////////////////////////////////////////////////////////
        // AthAlgTool

        FPGATrackSimSpacepointRoadFilterTool(const std::string&, const std::string&, const IInterface*);

        virtual StatusCode initialize() override;
        virtual StatusCode finalize() override;

        ///////////////////////////////////////////////////////////////////////
        // FPGATrackSimRoadFilterToolI

        virtual StatusCode filterRoads(const std::vector<FPGATrackSimRoad*> & prefilter_roads, std::vector<FPGATrackSimRoad*> & postfilter_roads) override;

    private:

        ///////////////////////////////////////////////////////////////////////
        // Handles
        ServiceHandle<IFPGATrackSimMappingSvc> m_FPGATrackSimMapping {this, "FPGATrackSimMappingSvc", "FPGATrackSimMappingSvc"};
        ServiceHandle<IFPGATrackSimBankSvc> m_FPGATrackSimBankSvc {this, "FPGATrackSimBankSvc", "FPGATrackSimBankSvc"};

        ///////////////////////////////////////////////////////////////////////
        // Properties

        Gaudi::Property <unsigned> m_threshold {this, "threshold", 0, "Minimum number of hit layers to accept as a road (inclusive"};
        Gaudi::Property <unsigned> m_minSpacePlusPixel {this, "minSpacePlusPixel", 0, "Minimum number of '2D' hits to accept as a road"};
        Gaudi::Property <unsigned> m_minSpacePlusPixel2 {this, "minSpacePlusPixel2", 0, "Minimum number of '2D' hits to accept as a road for 2nd stage"};
        Gaudi::Property <bool> m_filtering {this, "filtering", 0, "Filter out unpaired strip hits"};

        ///////////////////////////////////////////////////////////////////////
        // Event Storage
        std::vector<FPGATrackSimRoad> m_postfilter_roads;
        std::vector<FPGATrackSimRoad> m_postfilter_roads_2nd;
        ///////////////////////////////////////////////////////////////////////
        // Convenience

        ///////////////////////////////////////////////////////////////////////
        // Metadata and Monitoring

        TH1I* m_inputRoads;
        TH1I* m_badRoads;
        TH1I* m_inputRoads_2nd;
        TH1I* m_badRoads_2nd;
    
        ///////////////////////////////////////////////////////////////////////
        // Helpers

        bool splitRoad(FPGATrackSimRoad* initial_road);
        unsigned setSector(FPGATrackSimRoad& road);
        unsigned findUnique(std::vector<const FPGATrackSimHit*>& sp_in, std::vector<const FPGATrackSimHit*>& sp_out,
                            std::vector<const FPGATrackSimHit*>& unique_in, std::vector<const FPGATrackSimHit*>& unique_out,
                            std::vector<const FPGATrackSimHit*>& new_sp_in, std::vector<const FPGATrackSimHit*>& new_sp_out);

};


#endif // FPGATrackSimSPACEPOINTROADFILTERTOOL_H
