// Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

/**
 * @file FPGATrackSimEtaPatternFilterTool.cxx
 * @author Elliot Lipeles - lipeles@cern.ch
 * @date March 25th, 2021
 * @brief Implements road filtering using eta module patterns
 */

#include "FPGATrackSimMaps/IFPGATrackSimMappingSvc.h"
#include "FPGATrackSimMaps/FPGATrackSimPlaneMap.h"
#include "FPGATrackSimObjects/FPGATrackSimTypes.h"
#include "FPGATrackSimObjects/FPGATrackSimHit.h"
#include "FPGATrackSimObjects/FPGATrackSimConstants.h"
#include "FPGATrackSimBanks/IFPGATrackSimBankSvc.h"
#include "FPGATrackSimBanks/FPGATrackSimSectorBank.h"
#include "FPGATrackSimEtaPatternFilterTool.h"

#include "TH2.h"

#include <sstream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <boost/dynamic_bitset.hpp>
#include <iostream>



inline bool operator <(const FPGATrackSimEtaPatternFilterTool::ModuleId& lhs, const FPGATrackSimEtaPatternFilterTool::ModuleId& rhs)
{
    if (lhs.siTech != rhs.siTech) return lhs.siTech < rhs.siTech;
    if (lhs.zone != rhs.zone) return lhs.zone < rhs.zone;
    return lhs.etaModule < rhs.etaModule;
}


///////////////////////////////////////////////////////////////////////////////
// AthAlgTool

FPGATrackSimEtaPatternFilterTool::FPGATrackSimEtaPatternFilterTool(const std::string& algname, const std::string &name, const IInterface *ifc) :
  base_class(algname, name, ifc)
{
    declareInterface<IFPGATrackSimRoadFilterTool>(this);
}


StatusCode FPGATrackSimEtaPatternFilterTool::initialize()
{
    // Retrieve info
    ATH_CHECK(m_FPGATrackSimMapping.retrieve());
    ATH_CHECK(m_FPGATrackSimBankSvc.retrieve());
    m_nLayers = m_FPGATrackSimMapping->PlaneMap_1st()->getNLogiLayers();

    // Check inputs
    if (m_pattern_file_path.empty())
    {
        ATH_MSG_FATAL("No File Specified");
        return StatusCode::FAILURE;
    }

    // Read pattern file, populate m_patternmap keys
    readPatterns(m_pattern_file_path);

    // Create inverse map from module to pattern counter, m_moduleHits
    buildMap();

    return StatusCode::SUCCESS;
}


void FPGATrackSimEtaPatternFilterTool::readPatterns(std::string const & filepath)
{
    // Open the file
    std::ifstream fin(filepath);
    if (!fin.is_open())
    {
        ATH_MSG_FATAL("Couldn't open " << filepath);
        throw ("FPGATrackSimEtaPatternFilterTool couldn't open " + filepath);
    }

    // Parse the file
    bool ok = true;
    std::string line;
    while (getline(fin, line))
    {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream sline(line);

        // Read Pattern
        EtaPattern pattern;
        pattern.resize(m_nLayers);
        int tech, zone, mod;
        for (unsigned layer = 0; layer < m_nLayers; layer++)
        {
            ok = ok && (sline >> tech >> zone >> mod);
            if (!ok) break;
            pattern[layer] = { static_cast<SiliconTech>(tech), static_cast<DetectorZone>(zone), mod };
        }
        if (!ok) break;

        m_patternmap.emplace(pattern, 0);
    }

    if (!ok)
    {
        ATH_MSG_FATAL("Found error reading file at line: " << line);
        throw "FPGATrackSimEtaPatternFilterTool read error";
    }

    ATH_MSG_INFO("Read " << m_patternmap.size() << " patterns from " << filepath);
}


void FPGATrackSimEtaPatternFilterTool::buildMap()
{
    ATH_MSG_DEBUG("Building Map ");
    m_moduleHits.resize(m_nLayers);

    for (auto & entry : m_patternmap)
    {
        EtaPattern const & patt = entry.first;
        layer_bitmask_t & counter = entry.second;
        for (unsigned lyr = 0; lyr < m_nLayers; lyr++)
            m_moduleHits[lyr][patt[lyr]].addPattern(&counter);
    }

    ATH_MSG_DEBUG("Done Building Map ");
}


StatusCode FPGATrackSimEtaPatternFilterTool::finalize()
{
    return StatusCode::SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
// Main Algorithm

StatusCode FPGATrackSimEtaPatternFilterTool::filterRoads(const std::vector<FPGATrackSimRoad*> & prefilter_roads, std::vector<FPGATrackSimRoad*> & postfilter_roads)
{
    m_postfilter_roads.clear();
    postfilter_roads.clear();

    for (auto & road : prefilter_roads)
    {
        // reset all maps
        resetCounters();

        // put hits in module objects
        addHitsToMap(road);

        // keep track of used patterns, don't reuse
        std::set<EtaPattern> usedPatterns;

        // Count the number of matches from each individual road. This allows a dynamic threshold
        // during constant generation (where we don't want to discard training tracks).
        // At least, where we *might* not want to discard training tracks?
        int numMatches = 0;

        // check what is above threshold, moving down from 8/8 hits
        for (unsigned working_threshold = m_nLayers; working_threshold >= m_threshold; working_threshold--)
        {
            for (auto & patt_bitmask : m_patternmap)
            {
                unsigned nLayers = __builtin_popcount(patt_bitmask.second);
                if (nLayers >= working_threshold)
                {
                    // create subpattern from layers that actually have hits
                    EtaPattern subpatt(patt_bitmask.first);
                    for (unsigned lyr = 0; lyr < m_nLayers; lyr++)
                        if (!(patt_bitmask.second & (1 << lyr)))
                            subpatt[lyr] = {}; // defaults to undefined enums = missed

                    // check if this pattern was used already
                    if (usedPatterns.count(subpatt) > 0) continue;
                    addRedundantPatterns(usedPatterns, subpatt, nLayers - m_threshold);

                    // Get an "ID" for the pattern. This is a crude hack but it sort of works
                    // since the keys aren't being changed after the map is created. And provided the
                    // map is created in a consistent order this SHOULD be driven by insertion order.
                    // but this assumption may be fragile.
                    int index = std::distance(m_patternmap.begin(), m_patternmap.find(patt_bitmask.first));
                    if ((uint)index == m_patternmap.size() || index < 0) {
                        ATH_MSG_ERROR("Invalid pattern ID: " << index);
                    }
                    numMatches += 1;

                    // make the road
                    m_postfilter_roads.push_back(buildRoad(patt_bitmask, road, index));
                }
            }

            // If using a "dynamic" threshold-- once matches have been found at a given level, exit.
            if (numMatches > 0 && m_dynamicThreshold) {
                break;
            }
        }
    }

    // copy roads to outputs
    postfilter_roads.reserve(m_postfilter_roads.size());
    for (FPGATrackSimRoad & r : m_postfilter_roads)
        postfilter_roads.push_back(&r);

    return StatusCode::SUCCESS;
}

void FPGATrackSimEtaPatternFilterTool::resetCounters()
{
    for (auto & entry : m_patternmap)
        entry.second = 0;

    for (unsigned lyr = 0; lyr < m_nLayers; lyr++)
        for (auto & entry : m_moduleHits[lyr])
            entry.second.reset();
}

void FPGATrackSimEtaPatternFilterTool::addHitsToMap(FPGATrackSimRoad* r)
{
    for (unsigned lyr = 0; lyr < m_nLayers; lyr++)
        for (auto & hit : r->getHits(lyr))
            if (hit->getHitType() != HitType::wildcard)
            {
                ModuleId mod = { hit->getDetType(), hit->getDetectorZone(), (int)(hit->getEtaModule()) };
                auto itr = m_moduleHits[lyr].find(mod);
                if (itr != m_moduleHits[lyr].end())
                    itr->second.addHit(hit);
                else {
                    ATH_MSG_ERROR("Module not in map: " << hit->getDetType() << " " << hit->getDetectorZone() << " " <<  static_cast<int>(hit->getEtaModule()) );
                    for (auto & itr2 : m_moduleHits[lyr])
                    {
                        ATH_MSG_ERROR("   " << itr2.first.siTech << " " << itr2.first.zone << " " << itr2.first.etaModule);
                    }
                }
            }
}

// Dropping hits from currPattern can still result in valid (duplicated) patterns above threshold.
// This functions adds all of those duplicates to a blacklist in "usedPatterns".
void FPGATrackSimEtaPatternFilterTool::addRedundantPatterns(std::set<EtaPattern> & usedPatterns, EtaPattern const & currPatt, unsigned nExtra)
{
    usedPatterns.insert(currPatt);

    std::vector<unsigned> allowmissing(nExtra, 0);
    bool done = false;
    while (!done)
    {
        EtaPattern subpatt(currPatt);
        for (auto h : allowmissing)
            subpatt[h] = {};
        usedPatterns.insert(subpatt);

        // increment allowmissing with rollover
        done = (nExtra == 0);
        for (unsigned i = 0; i < nExtra; i++)
        {
            allowmissing[i]++;
            if (allowmissing[i] == m_nLayers)
            {
                allowmissing[i] = 0;
                done = (i==nExtra-1); // incremented last counter so were done
            }
            else
            {
                break;
            }
        }
    }
}

FPGATrackSimRoad FPGATrackSimEtaPatternFilterTool::buildRoad(std::pair<EtaPattern, layer_bitmask_t> const & patt, FPGATrackSimRoad* origr, int etaPatternID) const
{
    // All roads are now Hough roads.
    FPGATrackSimRoad r(*origr);

    r.setHitLayers(patt.second);
    r.setEtaPatternID(etaPatternID);
    for (unsigned lyr = 0; lyr < m_nLayers; lyr++)
        r.setHits(lyr, m_moduleHits[lyr].find(patt.first[lyr])->second.getHits());

    // Here, if told to do so, we'll reset the road's sector using the eta pattern ID.
    // Perhaps this should also be AND'ed with the "match ideal geometry" setting?
    // This code is adapted from FPGATrackSimHoughTransformTool::matchIdealGeoSectors, which perhaps
    // calls for moving this into the road class itself.
    if (m_doEtaPatternConsts) {
        int sectorbin = fpgatracksim::ETA_SECTOR_OFFSET*etaPatternID + r.getSectorBin();
        r.setSectorBin(sectorbin);
        std::vector<module_t> modules;
        for (unsigned int il = 0; il < r.getNLayers(); il++) {
            if (r.getNHits_layer()[il] == 0) {
                modules.push_back(-1);
            } else {
                modules.push_back(sectorbin);
            }
        }
        const FPGATrackSimSectorBank* sectorbank = !m_isSecondStage ? m_FPGATrackSimBankSvc->SectorBank_1st() : m_FPGATrackSimBankSvc->SectorBank_2nd();
        r.setSector(sectorbank->findSector(modules));
    }

    return r;
}


///////////////////////////////////////////////////////////////////////////////
// Prints

std::string FPGATrackSimEtaPatternFilterTool::to_string(const std::vector<unsigned> v) const
{
    std::ostringstream oss;
    oss << "[";
    if (!v.empty())
    {
        std::copy(v.begin(), v.end()-1, std::ostream_iterator<unsigned>(oss, ", "));
        oss << v.back();
    }
    oss << "]";
    return oss.str();
}

std::string FPGATrackSimEtaPatternFilterTool::to_string(const EtaPattern patt) const
{
    std::ostringstream oss;
    oss << "[";
    for (auto mod : patt) {
        oss << "[";
        oss << mod.siTech << ", " << mod.zone << ", " << mod.etaModule;
        oss << "],";
    }
    oss << "]";
    return oss.str();
}

std::string FPGATrackSimEtaPatternFilterTool::to_string(const FPGATrackSimRoad road) const
{
    std::ostringstream oss;
    oss << road.getNHits()  << " : [";
    for (unsigned layer = 0; layer < m_nLayers; layer++) {
        oss << road.getNHits_layer()[layer] << ",";
    }
    oss << "]";
    return oss.str();
}
