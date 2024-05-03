// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#ifndef FPGATrackSimPHIROADFILTERTOOL_H
#define FPGATrackSimPHIROADFILTERTOOL_H

/**
 * @file FPGATrackSimPhiRoadFilterTool.h
 * @author Elliot Lipeles - lipeles@cern.ch
 * @date March 28th, 2021
 * @brief Implements road filtering using eta module patterns
 *
 * Declarations in this file:
 *      class FPGATrackSimPhiRoadFilterTool : public AthAlgTool, virtual public IFPGATrackSimRoadFilterTool
 *
 */

#include "GaudiKernel/ServiceHandle.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include "FPGATrackSimObjects/FPGATrackSimTypes.h"
#include "FPGATrackSimObjects/FPGATrackSimVectors.h"
#include "FPGATrackSimObjects/FPGATrackSimRoad.h"
#include "FPGATrackSimObjects/FPGATrackSimHit.h"
#include "FPGATrackSimObjects/FPGATrackSimTrackPars.h"
#include "FPGATrackSimHough/IFPGATrackSimRoadFilterTool.h"

#include "TFile.h"

#include <string>
#include <vector>
#include <map>

class IFPGATrackSimBankSvc;
class IFPGATrackSimMappingSvc;


class FPGATrackSimPhiRoadFilterTool : public extends <AthAlgTool, IFPGATrackSimRoadFilterTool>
{
    public:

        ///////////////////////////////////////////////////////////////////////
        // AthAlgTool

        FPGATrackSimPhiRoadFilterTool(const std::string&, const std::string&, const IInterface*);

        virtual StatusCode initialize() override;

        ///////////////////////////////////////////////////////////////////////
        // IFPGATrackSimRoadFilterTool

        virtual StatusCode filterRoads(const std::vector<FPGATrackSimRoad*> & prefilter_roads, std::vector<FPGATrackSimRoad*> & postfilter_roads) override;

    private:

        ///////////////////////////////////////////////////////////////////////
        // Handles
        ServiceHandle<IFPGATrackSimMappingSvc> m_FPGATrackSimMapping {this, "FPGATrackSimMappingSvc", "FPGATrackSimMappingSvc"};

        ///////////////////////////////////////////////////////////////////////
        // Properties

	Gaudi::Property <unsigned> m_threshold {this, "threshold", 0, "Minimum number of hit layers to fire a road"};
	Gaudi::Property <std::vector<float> > m_window {this, "window", {}, "Distance from nominal path to keep hit, list of length nLayers"};
	Gaudi::Property <float> m_ptscaling {this, "ptscaling", 0.0, "Add a pT dependent resolution to each resolution in window"};

  
        //////////////////////////////////////////////////////////////////////
        // Event Storage
        std::vector<FPGATrackSimRoad> m_postfilter_roads;
  
        ///////////////////////////////////////////////////////////////////////
        // Convenience
        unsigned m_nLayers = 0U; // alias to m_FPGATrackSimMapping->PlaneMap1stStage()->getNLogiLayers();
  
        ///////////////////////////////////////////////////////////////////////
        // Metadata and Monitoring

        unsigned m_event = 0;
        std::string m_name; // Gets the instance name from the full gaudi name

        ///////////////////////////////////////////////////////////////////////
        // Helpers
        FPGATrackSimRoad buildRoad(FPGATrackSimRoad* origr) const;
};


#endif // FPGATrackSimPHIROADFILTERTOOL_H
