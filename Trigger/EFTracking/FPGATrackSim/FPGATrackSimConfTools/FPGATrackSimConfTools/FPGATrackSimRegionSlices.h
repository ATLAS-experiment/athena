// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

/**
 * @file FPGATrackSimRegionSlices.h
 * @author Riley Xu - riley.xu@cern.ch
 * @date Janurary 7th, 2021
 * @brief Stores slice definitions for FPGATrackSim regions
 *
 * This class reads the slice description file to get the region definition in
 * terms of track parameters. This is complementary to FPGATrackSimRegionMap, which defines
 * the modules that belong in each region.
 */

#ifndef FPGATrackSimCONFTOOLS_TTREGIONSLICES_H
#define FPGATrackSimCONFTOOLS_TTREGIONSLICES_H

#include "FPGATrackSimObjects/FPGATrackSimTrackPars.h"
#include "FPGATrackSimObjects/FPGATrackSimTruthTrack.h"

#include <vector>
#include <utility>


class FPGATrackSimRegionSlices
{
 public:
  FPGATrackSimRegionSlices(std::string const & filepath);
  FPGATrackSimRegionSlices(float mind0, float minz0, float minqOverPt,
			  float maxd0, float maxz0, float maxqOverPt);  
  
  unsigned nRegions() const { return m_regions.size(); }

  std::pair<FPGATrackSimTrackPars, FPGATrackSimTrackPars> const & getRegion(unsigned region) const { return m_regions.at(region); }
  FPGATrackSimTrackPars const & getMin(unsigned region) const { return m_regions.at(region).first; }
  FPGATrackSimTrackPars const & getMax(unsigned region) const { return m_regions.at(region).second; }

  bool inRegion(unsigned region, FPGATrackSimTruthTrack const & t) const;

 private:

  std::vector<std::pair<FPGATrackSimTrackPars, FPGATrackSimTrackPars>> m_regions; // index by region, min/max

};

#endif // FPGATrackSimCONFTOOLS_TTREGIONSLICES_H
