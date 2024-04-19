// Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

#ifndef FPGATRACKSIMETAPATTERNFILTERTOOL_H
#define FPGATRACKSIMETAPATTERNFILTERTOOL_H

/**
 * @file FPGATrackSimEtaPatternFilterTool.h
 * @author Elliot Lipeles - lipeles@cern.ch
 * @date March 25th, 2021
 * @brief Implements road filtering using eta module patterns
 *
 * Declarations in this file:
 *      class FPGATrackSimEtaPatternFilterTool : public AthAlgTool, virtual public IFPGATrackSimRoadFilterTool
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
#include "FPGATrackSimBanks/IFPGATrackSimBankSvc.h"
#include "FPGATrackSimMaps/IFPGATrackSimMappingSvc.h"

#include "TFile.h"

#include <string>
#include <vector>
#include <map>
#include <boost/dynamic_bitset_fwd.hpp>

class FPGATrackSimEtaPatternFilterTool : public extends<AthAlgTool, IFPGATrackSimRoadFilterTool>
{
    public:

        ///////////////////////////////////////////////////////////////////////
        // AthAlgTool

        FPGATrackSimEtaPatternFilterTool(const std::string&, const std::string&, const IInterface*);

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

	Gaudi::Property <std::string> m_pattern_file_path{this, "EtaPatterns", "", "path to pattern file"};
	Gaudi::Property <unsigned> m_threshold {this, "threshold", 0, "Minimum number of hit layers to fire a road"};
	Gaudi::Property <bool> m_isSecondStage {this, "isSecondStage", false, "Whether or not his this a second stage HT"};
	Gaudi::Property <bool> m_doEtaPatternConsts {this, "doEtaPatternConsts", false, "Use eta pattern IDs as part of sector definition"};
	Gaudi::Property <bool> m_dynamicThreshold { this, "dynamicThreshold", false, "Stop lowering the threshold once matches have been found"};

        ///////////////////////////////////////////////////////////////////////////////
        // Utility Structs

        struct ModuleId
        {
            SiliconTech siTech = SiliconTech::undefined;
            DetectorZone zone = DetectorZone::undefined;
            int etaModule = 0;
        };
        friend bool operator <(const ModuleId& lhs, const ModuleId& rhs);
        typedef std::vector<ModuleId> EtaPattern; // list of module ids in each layer


	// For a specific moduleId, stores pointers to the bitmasks of each pattern
	// that contains this module, set during initialize().
	// This class is also reused every input road, storing the hits in the module.
	
	struct ModulesToPattern
	{
	  std::vector<layer_bitmask_t*> m_pattern_bitmasks; // these point to the values in m_pattermap, created in initialize()
	  std::vector<const FPGATrackSimHit*> m_hits; // reset every input road
	  
	  void reset() { m_hits.clear(); }
	  void addPattern(layer_bitmask_t* counter) { m_pattern_bitmasks.push_back(counter); }
	  void addHit(const FPGATrackSimHit* hit)
	  {
	    if (hit->getHitType() != HitType::wildcard) {
	      m_hits.push_back(hit);
	      for (layer_bitmask_t* counter : m_pattern_bitmasks)
		(*counter) |= (1 << hit->getLayer());
	    }
	  }
	  const std::vector<const FPGATrackSimHit*> & getHits() const { return m_hits; }
	};
	
       	
        ///////////////////////////////////////////////////////////////////////
        // Event Storage
        std::vector<FPGATrackSimRoad> m_postfilter_roads;

        ///////////////////////////////////////////////////////////////////////
        // Convenience

        unsigned m_nLayers; // alias to m_FPGATrackSimMapping->PlaneMap1stStage()->getNLogiLayers();

        // The below maps are created in initialize, with fixed keys. But the counters (values)
        // are reset every input road.
        std::map<EtaPattern, layer_bitmask_t> m_patternmap;
            // keys initialized from file
            // for each input road, the bitmask is reset
        std::vector<std::map<ModuleId, ModulesToPattern>> m_moduleHits;
            // inverses the above map, mapping (layer, moduleId) to patterns
            // note this stores pointers to m_patternmap, and will modify it
            // also stores a list of hits for each input road

        ///////////////////////////////////////////////////////////////////////
        // Metadata and Monitoring

        ///////////////////////////////////////////////////////////////////////
        // Helpers
        void readPatterns(std::string const & filepath);
        void buildMap();
        void resetCounters();
        void addHitsToMap(FPGATrackSimRoad* r);
        void addRedundantPatterns(std::set<EtaPattern> & usedPatterns, EtaPattern const & currPatt, unsigned nExtra);
        FPGATrackSimRoad buildRoad(std::pair<EtaPattern, layer_bitmask_t> const & patt, FPGATrackSimRoad* origr, int etaPatternID) const;
        std::string to_string(const EtaPattern patt) const;
        std::string to_string(const FPGATrackSimRoad road) const;
        std::string to_string(const std::vector<unsigned> v) const;
};


#endif // FPGATRACKSIMETAPATTERNFILTERTOOL_H
