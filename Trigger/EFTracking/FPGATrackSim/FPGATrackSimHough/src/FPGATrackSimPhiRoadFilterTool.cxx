// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

/**
 * @file FPGATrackSimPhiRoadFilterTool.cxx
 * @author Elliot Lipeles - lipeles@cern.ch
 * @date March 25th, 2021
 * @brief Implements road filtering using eta module patterns
 */

#include "FPGATrackSimObjects/FPGATrackSimTypes.h"
#include "FPGATrackSimObjects/FPGATrackSimHit.h"
#include "FPGATrackSimObjects/FPGATrackSimConstants.h"
#include "FPGATrackSimMaps/IFPGATrackSimMappingSvc.h"
#include "FPGATrackSimMaps/FPGATrackSimPlaneMap.h"
#include "FPGATrackSimPhiRoadFilterTool.h"

#include "TH2.h"

#include <sstream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <boost/dynamic_bitset.hpp>
#include <iostream>

static inline std::string instance_name(std::string const & s);


///////////////////////////////////////////////////////////////////////////////
// AthAlgTool

FPGATrackSimPhiRoadFilterTool::FPGATrackSimPhiRoadFilterTool(const std::string& algname, const std::string &name, const IInterface *ifc) :
    base_class(algname, name, ifc),
    m_name(instance_name(name))
{
    declareInterface<IFPGATrackSimRoadFilterTool>(this);
}


StatusCode FPGATrackSimPhiRoadFilterTool::initialize()
{
  // Retrieve info
  ATH_CHECK(m_FPGATrackSimMapping.retrieve());
  m_nLayers = m_FPGATrackSimMapping->PlaneMap_1st()->getNLogiLayers();
  
  return StatusCode::SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
// Main Algorithm

StatusCode FPGATrackSimPhiRoadFilterTool::filterRoads(const std::vector<FPGATrackSimRoad*> & prefilter_roads, std::vector<FPGATrackSimRoad*> & postfilter_roads) 
{
    ATH_MSG_INFO("Start Phi Road Filter"); 
    
    m_postfilter_roads.clear();
    postfilter_roads.clear();

    // Filter roads
    for (auto & road : prefilter_roads) {
      FPGATrackSimRoad newroad = buildRoad(road);
      unsigned hit_layers = newroad.getHitLayers();
      
      unsigned layer_cnt=0;
      for (unsigned lyr = 0; lyr < m_nLayers; lyr++) {
	if (hit_layers & (1<<lyr)) layer_cnt++;
      }
      
      if (layer_cnt >= m_threshold) {
	m_postfilter_roads.push_back(newroad);
      }
	
    }

    // copy roads to outputs
    postfilter_roads.reserve(m_postfilter_roads.size());
    for (FPGATrackSimRoad & r : m_postfilter_roads) postfilter_roads.push_back(&r);
    
    ATH_MSG_INFO("Event Done");
    
    m_event++;
    return StatusCode::SUCCESS;
}

FPGATrackSimRoad FPGATrackSimPhiRoadFilterTool::buildRoad(FPGATrackSimRoad* origr) const
{
  ATH_MSG_DEBUG("PhiRoad Build Road");
  float phi = origr->getX();
  float qPt  = origr->getY();

  // make new road -- main alg doesn't keep it if not needed
  FPGATrackSimRoad r(*origr); // only works with Hough roads!
  r.setNLayers(m_nLayers);
  layer_bitmask_t hitLayers = 0;
  
  // add hits
  for (unsigned lyr = 0; lyr < m_nLayers; lyr++) {
    std::vector<const FPGATrackSimHit*> road_hits;
    for (auto hit : origr->getHits(lyr)) {
      float phi_expected = -1.0*asin(fpgatracksim::A * hit->getR() * qPt) + phi;
      if (abs(hit->getGPhi()-phi_expected)< (m_window.value()[lyr]+qPt*m_ptscaling)) {
	road_hits.push_back(hit);
	hitLayers |= 1 << hit->getLayer();
      }
    }
    ATH_MSG_DEBUG("PhiRoad Hits " << lyr << " " << road_hits.size() << " " << origr->getHits(lyr).size());
    r.setHits(lyr,road_hits);
  }

  r.setHitLayers(hitLayers);
  return r;
}

static inline std::string instance_name(std::string const & s)
{
    size_t pos = s.find_last_of(".");
    if (pos != std::string::npos)
        return s.substr(pos + 1);
    return s;
}

