// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

/**
 * @file FPGATrackSimRegionMap.h
 * @author Riley Xu - riley.xu@cern.ch (rewrite from FTK)
 * @date Janurary 7th, 2020
 * @brief Maps ITK module indices to FPGATrackSim regions.
 *
 * See header.
 */

#include "FPGATrackSimMaps/FPGATrackSimRegionMap.h"
#include <AsgMessaging/MessageCheck.h>

#include <cstdlib>
#include <string>
#include <iostream>
#include <vector>

using namespace std;
using namespace asg::msgUserCode;


///////////////////////////////////////////////////////////////////////////////
// Constructor/Desctructor
///////////////////////////////////////////////////////////////////////////////


FPGATrackSimRegionMap::FPGATrackSimRegionMap(const std::vector<std::unique_ptr<FPGATrackSimPlaneMap>> & pmaps, std::string const & filepath ) :
    m_pmaps(pmaps)
{
    // Open the file
    ifstream fin(filepath);
    if (!fin.is_open())
    {
        ANA_MSG_FATAL("Couldn't open " << filepath);
        throw ("FPGATrackSimRegionMap Couldn't open " + filepath);
    }
    m_filepath=filepath;

    // Reads the header of the file to resize all the vector members
    allocateMap(fin);

    // Read all the region data
    for (int region = 0; region < m_nregions; region++){
        readRegion(fin, region);
    }

    // Resize the radius structure  appropriately.
    m_radii_map.clear();
    m_radii_map.resize(m_nregions, std::vector<double>(m_pmaps.at(0)->getNLogiLayers()));
}

// Reads the header of the file to resize all the vector members
void FPGATrackSimRegionMap::allocateMap(ifstream & fin)
{
    string line, towerKey;
    bool ok = true;

    ok = ok && getline(fin, line);
    ANA_MSG_DEBUG(line << " <  " << ok);

    istringstream sline(line);
    ok = ok && (sline >> towerKey >> m_nregions);
    ok = ok && (towerKey == "towers");
    if((m_filepath.size()-7)==m_filepath.find("subrmap")){
        if(int(m_pmaps.size())!= m_nregions){
            ANA_MSG_FATAL("Error Pmap slice size does not match Rmap: PMAP_SIZE:"<<m_pmaps.size()<<"  RMAP_SIZE:"<<m_nregions);
            throw ("Pmap slice size does not match Rmap:" );
        }
    }
    

    if (!ok) ANA_MSG_FATAL("Error reading header");

    m_map.resize(m_nregions);
    
    for (int iRegion=0; iRegion<int(m_map.size()); iRegion++)
    {
        m_map.at(iRegion).resize(m_pmaps.at(0)->getNLogiLayers());
        for (size_t l = 0; l < m_map.at(iRegion).size(); l++) m_map.at(iRegion).at(l).resize(m_pmaps.at(iRegion)->getNSections(l));
    }
}


// Reads one region from file.
void FPGATrackSimRegionMap::readRegion(ifstream & fin, int expected_region)
{

    string line, dummy;
    bool ok = true;
    int region = -1;
    uint32_t linesRead = 0; // detLayer lines read

    while (getline(fin, line))
    {
        if (line.empty() || line[0] == '#') continue;
        istringstream sline(line);

        if (region < 0) // Find the starting header of the next region
        {
            ok = ok && (sline >> region);// should check this is a sensible number
            ok = ok && !(sline >> dummy); // No keyword to check that we're not reading a detector line, so make sure rest of string is empty
            ok = ok && (region == expected_region);
            if (!ok) break;
        }
        else // Detector layer line
        {
            int isPix{}, BEC{}, physLayer{}, phi_min{}, phi_max{}, phi_tot{}, eta_min{}, eta_max{}, eta_tot{};
            //should check these are within sensible limits after they are read
            ok = ok && (sline >> isPix >> BEC >> physLayer >> phi_min >> phi_max >> phi_tot >> eta_min >> eta_max >> eta_tot);
            if (!ok) break;
            //region WW
            int logiLayer = m_pmaps.at(region)->getLayerSection(static_cast<SiliconTech>(isPix), static_cast<DetectorZone>(BEC), physLayer).layer;
            int section   = m_pmaps.at(region)->getLayerSection(static_cast<SiliconTech>(isPix), static_cast<DetectorZone>(BEC), physLayer).section;

            if (logiLayer > -1)
                m_map[region][logiLayer][section] = { phi_min, phi_max, eta_min, eta_max };

            if (++linesRead == m_pmaps.at(region)->getNDetLayers()) break;
        }
    }

    if (!ok)
    {
        ANA_MSG_FATAL("Found error reading file at line: " << line);
        throw "FPGATrackSimRegionMap read error";
    }
}


// Read module id LUT (defining global -> tower-local module IDs)
void FPGATrackSimRegionMap::loadModuleIDLUT(std::string const & filepath)
{
    ANA_MSG_INFO("Reading module LUT" << filepath);
    ifstream fin(filepath);
    if (!fin.is_open())
    {
        ANA_MSG_ERROR("Couldn't open " << filepath);
        throw ("FPGATrackSimRegionMap Couldn't open " + filepath);
    }

    m_global_local_map.clear();
    m_global_local_map.resize(m_nregions, vector<map<uint32_t, uint32_t>>(m_pmaps.at(0)->getNLogiLayers()));

    string line;
    while (getline(fin, line))
    {
        uint32_t region, layer, globalID, localID;
        istringstream sline(line);

        if (!(sline >> region >> layer >> globalID >> localID))
            ANA_MSG_WARNING("Error reading module LUT");
        else if (region >= m_global_local_map.size() || layer >= m_pmaps.at(0)->getNLogiLayers())
            ANA_MSG_WARNING("loadModuleIDLUT() bad region=" << region << " or layer=" << layer);
        else
            m_global_local_map[region][layer][globalID] = localID;
    }
}

// Copied from the 1D Hough bitstream tool.
void FPGATrackSimRegionMap::loadRadiiFile(std::string const & filepath, unsigned layer_offset = 0, unsigned layer_max = 0)
{
    // If layer_max is 0, then set it equal to the number of layers in the configured plane map, minus the offset.
    layer_max = (layer_max == 0) ? m_pmaps.at(0)->getNLogiLayers() - layer_offset: layer_max - layer_offset;

    // Open the file
    std::ifstream fin(filepath);
    if (!fin.is_open())
    {
        ANA_MSG_FATAL("Couldn't open radius file " << filepath);
    }

    // Variables to fill
    std::string line;
    bool ok = true;
    double r = 0.0;

    // Parse the file
    while (getline(fin, line))
    {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream sline(line);
        std::vector<int> shifts;

        int subregion{-1};
        ok = ok && (sline >> subregion);

        // The radii file contains an "inclusive" line and then one for each subregion.
        // If we only have one region (because we are the rmap or because we are a subrmap
        // with one z-slice) then we only want to read the inclusive line.
        // Otherwise we want to read everything BUT the inclusive line.
        if (m_nregions == 1 && subregion != -1) {
            continue;
        }
        if (m_nregions > 1 && subregion == -1) {
            continue;
        }

        // Read up to layer_max layers out of the radii file. This is set by the mapping service when loading these files.
        for (unsigned layer = 0; layer < layer_max; layer++) {
            ok = ok && (sline >> r);
            if (!ok) break;
            unsigned eff_layer = layer + layer_offset;
            ANA_MSG_DEBUG("Reading average radius at effective (actual) layer = " << eff_layer << " (" << layer << ") = " << r);
            if (r<=0) {
                ANA_MSG_WARNING("Radius in radiiFile is "<< r <<" for layer: " << eff_layer << " setting to dummy value!");
                r = 500.0; // dummy value that won't cause a crash, but won't work anywhere.
            }
            if (subregion == -1) {
                m_radii_map[0][eff_layer] = r;
            } else {
                m_radii_map[subregion][eff_layer] = r;
            }
        }

        if (!ok) break;
    }

    if (!ok)
    {
        ANA_MSG_FATAL("Found error reading file at line: " << line);
    }
}


///////////////////////////////////////////////////////////////////////////////
// Interface Functions
///////////////////////////////////////////////////////////////////////////////

bool FPGATrackSimRegionMap::isInRegion(uint32_t region, const FPGATrackSimHit &hit) const
{
    // Always assume that the hit's "layer" might not correspond to what's in the pmap
    // Also, to avoid confusion and double-counting, by convention, always use the coordinates of the inner hit
    // when testing if a spacepoint is in a (sub)region.
    uint32_t layer;
    uint32_t section;

    LayerSection ls;
    if (hit.getHitType() == HitType::spacepoint) {
        ls = m_pmaps.at(region)->getLayerSection(hit.getPairedDetType(), hit.getPairedDetZone(), hit.getPairedPhysLayer());
    } else {
        ls = m_pmaps.at(region)->getLayerSection(hit.getDetType(), hit.getDetectorZone(), hit.getPhysLayer());
    }
    layer = ls.layer;
    section = ls.section;

    int etamod = (hit.getHitType() == HitType::spacepoint) ? hit.getPairedEtaModule() : hit.getEtaModule();
    unsigned phimod = (hit.getHitType() == HitType::spacepoint) ? hit.getPairedPhiModule() : hit.getPhiModule();
    return isInRegion(region, layer, section, etamod, phimod);
}


bool FPGATrackSimRegionMap::isInRegion(uint32_t region, uint32_t layer, uint32_t section, int eta, int phi) const
{
    if (    region  >= m_map.size()
         || layer   >= m_map[region].size()
         || section >= m_map[region][layer].size() )
    {
        return false;
    }

    int eta_min = m_map[region][layer][section].eta_min;
    int eta_max = m_map[region][layer][section].eta_max;

    if (eta < eta_min || eta > eta_max) return false;

    int phi_min = m_map[region][layer][section].phi_min;
    int phi_max = m_map[region][layer][section].phi_max;

    // Need special cases for phi because it can go from 2pi to 0.
    if (phi_min <= phi_max) // Region does not cross phi = 0
    {
        if (phi < phi_min || phi > phi_max) return false;
    }
    else // Region crosses phi = 0
    {
        if (phi < phi_min && phi > phi_max) return false;
    }

    return true;
}

std::vector<uint32_t> FPGATrackSimRegionMap::getRegions(const FPGATrackSimHit &hit) const
{
    std::vector<uint32_t> regions;
    for (uint32_t region = 0; region < m_map.size(); region++) {
        if (isInRegion(region, hit))
            regions.push_back(region);
    }
    return regions;
}

uint32_t FPGATrackSimRegionMap::getUnmappedID(uint32_t region, const FPGATrackSimHit &hit) const
{
    /*
    Todo: Does this handle EC hits correctly?

    error code key:
    6 digit number. 1 = ok. 2 = not ok

    1st digit - Endcap Check
    2nd - region
    3rd - layer
    4th - section
    5th - eta
    6th - phi
    */

  uint32_t layer   = hit.getLayer();
  uint32_t section = hit.getSection();
  int eta          = hit.getEtaModule();
  int phi          = hit.getPhiModule();

    int anyerr = 0;
    int err[] = {1,1,1,1,1,1};

    if (region >= m_map.size()) anyerr = err[1] = 2;

    if (!anyerr && layer >= m_map[region].size()) anyerr = err[2] = 2;

    if (!anyerr && section >= m_map[region][layer].size()) anyerr = err[3] = 2;

    if (!anyerr) {
      int eta_min = m_map[region][layer][section].eta_min;
      int eta_max = m_map[region][layer][section].eta_max;

      if (eta < eta_min) err[4] = 3;
      if (eta > eta_max) err[4] = 2;

      int phi_min = m_map[region][layer][section].phi_min;
      int phi_max = m_map[region][layer][section].phi_max;

      // Need special cases for phi berrause it can go from 2pi to 0.
      if (phi_min <= phi_max) // Region does not cross phi = 0
      {
        if (phi < phi_min || phi > phi_max) err[5] = 2;
      }
      else // Region crosses phi = 0
      {
        if (phi < phi_min && phi > phi_max) err[5] = 3;
      }
    }

    int error_code = 100000*err[0] + 10000*err[1] + 1000*err[2] + 100*err[3] + 10*err[4] + err[5];

    return error_code;
}


uint32_t FPGATrackSimRegionMap::getLocalID(uint32_t region, uint32_t layer, uint32_t globalModuleID) const
{
    // TEMPORARY UNTIL WE HAVE A MODULE LUT
    (void) region;
    (void) layer;
    return globalModuleID & 0x3ff;
}


uint32_t FPGATrackSimRegionMap::getGlobalID(uint32_t region, uint32_t layer, uint32_t localModuleID) const
{
    if (region >= m_global_local_map.size() || layer >= m_pmaps.at(0)->getNLogiLayers())
    {
        ANA_MSG_ERROR("getGlobalID() bad region=" << region << " or layer=" << layer);
        return -1;
    }

    for (auto const & g_l : m_global_local_map[region][layer])
        if (g_l.second == localModuleID) return g_l.first;

    ANA_MSG_ERROR("getGlobalID() Did not find global id for region " << region << ", layer " << layer << ", localID " << localModuleID);
    return -1;
}

double FPGATrackSimRegionMap::getAvgRadius(unsigned region, unsigned layer) const {

    if (region >= m_radii_map.size() || layer >= m_pmaps.at(0)->getNLogiLayers())
    {
        ANA_MSG_ERROR("getAvgRadius() bad region=" << region << " or layer=" << layer);
        return -1;
    }

    // Return the radius we loaded for this region.
    return m_radii_map[region][layer];
}
