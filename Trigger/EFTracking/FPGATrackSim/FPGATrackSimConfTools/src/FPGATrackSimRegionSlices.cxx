// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

/**
 * @file FPGATrackSimRegionSlices.h
 * @author Riley Xu - riley.xu@cern.ch
 * @date Janurary 7th, 2021
 * @brief Stores slice definitions for FPGATrackSim regions
 *
 * See header.
 */

#include "FPGATrackSimConfTools/FPGATrackSimRegionSlices.h"

#include <AsgMessaging/MessageCheck.h>

#include <string>
#include <iostream>
#include <fstream>

using namespace asg::msgUserCode;

///////////////////////////////////////////////////////////////////////////////
// Constructor/Desctructor
///////////////////////////////////////////////////////////////////////////////


FPGATrackSimRegionSlices::FPGATrackSimRegionSlices(std::string const & filepath)  // old constructor, we read in a file
{
  // Open the file
  std::ifstream fin(filepath);
  if (!fin.is_open())
    {
      ANA_MSG_ERROR("Couldn't open " << filepath);
      throw ("FPGATrackSimRegionSlices couldn't open " + filepath);
    }

  // Variables to fill
  unsigned region;
  std::string line, key;
  FPGATrackSimTrackPars min, max;

  // Parse the file
  bool ok = true;
  while (getline(fin, line))
    {
      if (line.empty() || line[0] == '#') continue;
      std::istringstream sline(line);
      
      ok = ok && (sline >> key);
      if (!ok) break;
      if (key == "region")
	{
	  ok = ok && (sline >> region);
	  if (ok && region > 0) m_regions.push_back({ min, max });
	  ok = ok && (region == m_regions.size());
	  min = FPGATrackSimTrackPars(); // reset
	  max = FPGATrackSimTrackPars(); // reset
	}
      else if (key == "phi") ok = ok && (sline >> min.phi >> max.phi);
      else if (key == "eta") ok = ok && (sline >> min.eta >> max.eta);
      else if (key == "qpt") ok = ok && (sline >> min.qOverPt >> max.qOverPt);
      else if (key == "d0") ok = ok && (sline >> min.d0 >> max.d0);
      else if (key == "z0") ok = ok && (sline >> min.z0 >> max.z0);
      else ok = false;
      
      if (!ok) break;
    }
  
  if (!ok)
    {
      ANA_MSG_ERROR("Found error reading file at line: " << line);
      throw "FPGATrackSimRegionSlices read error";
    }
  
  m_regions.push_back({ min, max }); // last region still needs to be added
}


FPGATrackSimRegionSlices::FPGATrackSimRegionSlices(float mind0, float minz0, float minqQverPt,
						   float maxd0, float maxz0, float maxQoverPt) // new constructor, we pass d0,z0,qoverpt and calculate the rest
{
  FPGATrackSimTrackPars min, max;
  min.d0 = mind0;
  min.z0 = minz0;
  min.qOverPt = minqQverPt;
  max.d0 = maxd0;
  max.z0 = maxz0;
  max.qOverPt = maxQoverPt;

  
  for (unsigned i=0; i < 1280; i++) {
    double phibinSize = M_PI/16;
    double etabinSize = 0.2;
    int phibin = i & 0x1f;
    int etabin = (i >> 6) & 0x1f;
    int etaside = (i >> 5) & 0x1; // 1 is positive side, 0 negative side
    min.phi = phibinSize*phibin;
    max.phi = phibinSize*(phibin+1);

    // Force these to fall within [-pi, pi].
    while (min.phi < -M_PI) min.phi += 2*M_PI;
    while (min.phi >= M_PI) min.phi -= 2*M_PI;
    while (max.phi <= -M_PI) max.phi += 2*M_PI;
    while (max.phi > M_PI) max.phi -= 2*M_PI;

    if (etaside > 0) {
      min.eta = etabinSize * etabin;
      max.eta = etabinSize * (etabin+1);	
    }
    else {
      min.eta = -etabinSize * (etabin+1);
      max.eta = -etabinSize * etabin;	
    }
    m_regions.push_back({ min, max });    
  }
}




///////////////////////////////////////////////////////////////////////////////
// Interface Functions
///////////////////////////////////////////////////////////////////////////////


bool FPGATrackSimRegionSlices::inRegion(unsigned region, FPGATrackSimTruthTrack const & t) const
{
  if (region >= m_regions.size())
    {
      ANA_MSG_WARNING("inRegion() region " << region << " out-of-bounds " << m_regions.size());
      return false;
    }
  FPGATrackSimTrackPars min = m_regions[region].first;
  FPGATrackSimTrackPars max = m_regions[region].second;
  FPGATrackSimTrackPars cur = t.getPars();

  for (unsigned i = 0; i < FPGATrackSimTrackPars::NPARS; i++)
    {
      if (cur[i] < min[i]) return false;
      if (cur[i] > max[i]) return false;
    }

  return true;
}
