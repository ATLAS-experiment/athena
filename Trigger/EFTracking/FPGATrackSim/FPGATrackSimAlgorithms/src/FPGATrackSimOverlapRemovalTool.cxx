// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#include "../FPGATrackSimAlgorithms/FPGATrackSimOverlapRemovalTool.h"
#include "FPGATrackSimMaps/FPGATrackSimPlaneMap.h"
#include "FPGATrackSimObjects/FPGATrackSimVectors.h"

#include "GaudiKernel/MsgStream.h"

#include <sstream>
#include <iostream>
#include <fstream>

/////////////////////////////////////////////////////////////////////////////
FPGATrackSimOverlapRemovalTool::FPGATrackSimOverlapRemovalTool(const std::string& algname, const std::string& name, const IInterface *ifc) :
    AthAlgTool(algname, name, ifc)
{
}

StatusCode FPGATrackSimOverlapRemovalTool::initialize()
{
  ATH_MSG_INFO( "FPGATrackSimOverlapRemovalTool::initialize()" );

  if (!m_monTool.empty()) ATH_CHECK(m_monTool.retrieve());

  // Check if this is 2nd stage
  if(m_do2ndStage)
  {
    m_totLayers = m_FPGATrackSimMapping->PlaneMap_2nd(0)->getNLogiLayers();
  }
  else
  {
    m_totLayers = m_FPGATrackSimMapping->PlaneMap_1st(0)->getNLogiLayers();
  }
  ATH_MSG_DEBUG("Total number of layer: " << m_totLayers);

  // Check road OR
  if (m_localMaxWindowSize && !m_roadSliceOR)
      ATH_MSG_WARNING("LocalMaxOR only being run per hough slice (i.e. this tool does nothing) since roadSliceOR is turned off");


  //  Setup OR algorithm
  if(m_algorithm == "Normal") m_algo=ORAlgo::Normal;
  else if(m_algorithm == "Invert") m_algo=ORAlgo::InvertGrouping;
  else
  {
    ATH_MSG_ERROR("initialize(): OR algorithm doesn't exist. ");
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Overlap removal algorithm is "<<m_algorithm.value());

  return StatusCode::SUCCESS;
}

bool isLocalMax(vector2D<FPGATrackSimRoad*> const & acc, unsigned x, unsigned y, int localMaxWindowSize)
{
    if (!localMaxWindowSize) return true;
    if (!acc(y, x)) return false;
    for (int j = -localMaxWindowSize; j <= localMaxWindowSize; j++)
        for (int i = -localMaxWindowSize; i <= localMaxWindowSize; i++)
        {
            if (i == 0 && j == 0) continue;
            if (y + j < acc.size(0) && x + i < acc.size(1))
            {
                if (!acc(y+j, x+i)) continue;
                if (acc(y+j, x+i)->getNHitLayers() > acc(y, x)->getNHitLayers()) return false;
                if (acc(y+j, x+i)->getNHitLayers() == acc(y, x)->getNHitLayers())
                {
                    if (acc(y+j, x+i)->getNHits() > acc(y, x)->getNHits()) return false;
                    if (acc(y+j, x+i)->getNHits() == acc(y, x)->getNHits() && j <= 0 && i <= 0) return false;
                }
            }
        }

    return true;
}

StatusCode FPGATrackSimOverlapRemovalTool::runOverlapRemoval(std::vector<std::shared_ptr<const FPGATrackSimRoad>>& roads)
{
    if (roads.empty()) return StatusCode::SUCCESS;

    if (!m_roadSliceOR) return StatusCode::SUCCESS;
    size_t in = roads.size();

    // Hough image
    vector2D<FPGATrackSimRoad*> acc(m_imageSize_y, m_imageSize_x);

    // Slice-wise duplicate removal: accept only one road (with most hits) per bin
    for (auto &r: roads)
    {
        FPGATrackSimRoad* & old = acc(r->getYBin(), r->getXBin());
        if (!old) old = new FPGATrackSimRoad (*r.get());
        else if (r->getNHitLayers() > old->getNHitLayers()) *old = *r.get();
        else if (r->getNHitLayers() == old->getNHitLayers() && r->getNHits() > old->getNHits()) *old = *r.get();
    }

    // Reformat to vector
    roads.clear();
    for (unsigned y = 0; y < m_imageSize_y; y++)
      for (unsigned x = 0; x < m_imageSize_x; x++)
        if (FPGATrackSimRoad *tempPtr = acc(y, x); tempPtr && isLocalMax(acc, x, y, m_localMaxWindowSize)/*All-slices local max*/) {
          roads.emplace_back(std::shared_ptr<const FPGATrackSimRoad>(tempPtr));
          acc(y, x) = nullptr;
        }
        else {
          delete acc(y,x);
          acc(y,x) = nullptr;
        }

    ATH_MSG_DEBUG("Input: " << in << " Output: " << roads.size());
    return StatusCode::SUCCESS;
}

StatusCode FPGATrackSimOverlapRemovalTool::runOverlapRemoval(std::vector<FPGATrackSimTrack>& tracks)
{

  // Do fast OR instead of requested
  if (m_doFastOR) return runOverlapRemoval_fast(tracks);

  // Otherwise, proceed
  ATH_MSG_DEBUG("Beginning runOverlapRemoval()");

  ATH_MSG_DEBUG("Tracks in event: " << tracks.size());

  return ::runOverlapRemoval(tracks, m_minChi2.value(), m_NumOfHitPerGrouping, getAlgorithm(), m_monTool, m_compareAllHits);
}


StatusCode FPGATrackSimOverlapRemovalTool::removeOverlapping(FPGATrackSimTrack & track1, FPGATrackSimTrack & track2) {

    // Hit comparison
    struct HitCompare {
        bool operator()(const FPGATrackSimHit* a, const FPGATrackSimHit* b) const { 
            auto hash_a = a->getIdentifierHash();
            auto hash_b = b->getIdentifierHash();
            if ( hash_a == hash_b ) {
                auto phi_a = a->getPhiIndex();
                auto phi_b = b->getPhiIndex();
                if ( phi_a == phi_b ) {
                    auto eta_a = a->getEtaIndex();
                    auto eta_b = b->getEtaIndex();
                    if ( eta_a == eta_b) {
                        auto layer_a = a->getPhysLayer();
                        auto layer_b = b->getPhysLayer();
                        return layer_a < layer_b;
                    }
                    return eta_a < eta_b;
                }
                return phi_a < phi_b;
            }
            return hash_a <  hash_b; 
        }
    };

    std::set<const FPGATrackSimHit*, HitCompare > hitsInTrack1;
    for ( auto& hit : track1.getFPGATrackSimHits()) {
        if (hit.isReal()) hitsInTrack1.insert(&hit);
    }

    std::set<const FPGATrackSimHit*, HitCompare> hitsInTrack2;
    for ( auto& hit: track2.getFPGATrackSimHits()){
        if (hit.isReal()) hitsInTrack2.insert(&hit);
    }

    std::vector<const FPGATrackSimHit*> sharedHits;    
    std::set_intersection( hitsInTrack1.begin(), hitsInTrack1.end(), 
                         hitsInTrack2.begin(), hitsInTrack2.end(), 
                         std::back_inserter(sharedHits), 
                         HitCompare() );

    // Number of real hits in track 1, number of real hits in track 2, number of shared hits, number of non-shared hits (?)
    int nHitsInTrack1 = hitsInTrack1.size();
    int nHitsInTrack2 = hitsInTrack2.size();
    int nSharedHits = sharedHits.size();
    // Original version seems to be only track 1; I want to compare each pair only once so
    // let's make this the most conservative option (i.e. smallest number) ?
    int nonOverlappingHits = std::min(nHitsInTrack1 - nSharedHits, nHitsInTrack2 - nSharedHits);

    // Now check if these pass our criteria for overlapping. If not, just return.
    if(getAlgorithm() == ORAlgo::Normal) {
      // Here decision is based on number of overlapping hits.
      // Consider these overlapping if they share >= m_NumOfHitPerGrouping
      if(nSharedHits < m_NumOfHitPerGrouping) return StatusCode::SUCCESS;

    } else if(getAlgorithm() == ORAlgo::InvertGrouping) {
      // This is the opposite: duplicates of number of unique hits is <= m_NumOfHitPerGrouping.
      if(nonOverlappingHits > m_NumOfHitPerGrouping) return StatusCode::SUCCESS;

    } else {
      // Unknown.
      return StatusCode::FAILURE;
    }

    // Made it here: these tracks are overlapping. 
    // But we already sorted them such that track 2 is the one
    // we want to keep, so we can just set track 1 to be removed.
    track1.setPassedOR(0);

    return StatusCode::SUCCESS;
}

bool FPGATrackSimOverlapRemovalTool::compareTrackQuality(const FPGATrackSimTrack & track1, const FPGATrackSimTrack & track2)
{
    std::vector<const FPGATrackSimHit*> hitsInTrack1;
    for ( auto& hit : track1.getFPGATrackSimHits()) {
        if (hit.isReal()) hitsInTrack1.push_back(&hit);
    }

    std::vector<const FPGATrackSimHit*> hitsInTrack2;
    for ( auto& hit: track2.getFPGATrackSimHits()){
        if (hit.isReal()) hitsInTrack2.push_back(&hit);
    }

    // If one track has more hits than the other, it's better.
    // Return true if track 2 is better than track 1.
    // Otherwise, decide based on chi2.
    bool goodOrder = true;
    if (hitsInTrack1.size() == hitsInTrack2.size()) {
      // Surprising number of cases where the chi2 is actually identical.
      // In these cases, let's default to the track ID number as the next most important property.
      // So put it higher since we are considering later tracks to be better.
      if (track1.getChi2ndof() == track2.getChi2ndof() && track1.getTrackID() < track2.getTrackID()) goodOrder = false; 
      // Now assuming they're different, we want them in decreasing chi2 order
      else if (track1.getChi2ndof() < track2.getChi2ndof()) goodOrder = false;
    } else if (hitsInTrack1.size() > hitsInTrack2.size()) {
      goodOrder = false;
    } 

    return goodOrder;

}

StatusCode FPGATrackSimOverlapRemovalTool::runOverlapRemoval_fast(std::vector<FPGATrackSimTrack>& tracks)
{
  ATH_MSG_DEBUG("Beginning fast overlap removal");

  // Sort tracks in order of increasing quality. 
  // This way, once a track has been eliminated in a comparison, we don't
  // need to check it against any other tracks - they will always be
  // better than it.
  std::sort(std::begin(tracks), 
            std::end(tracks), 
            compareTrackQuality); 

  // Now compare every pair of tracks.
  // Set passedOR to 0 for the worst one (first one)
  // if they are found to overlap.
  for (unsigned int i=0; i < tracks.size(); i++) {

    // Skip track i if bad chi2.
    if (tracks.at(i).getChi2ndof() > m_minChi2.value()) {
      tracks.at(i).setPassedOR(0);
      continue;
    }

    // Now check against all other tracks.
    for (unsigned int j=i+1; j< tracks.size(); j++) {

      // Uniquely comparing tracks i and j here.

      // If have set track i to 0 in a previous comparison, 
      // no need to look at it any more - we're done with it.
      if (!tracks.at(i).passedOR()) break;      

      // Ignore j if its chi2 is bad.
      if (tracks.at(j).getChi2ndof() > m_minChi2.value()) tracks.at(j).setPassedOR(0);

      // If we just set track j to zero for bad chi2,
      // no need to do the comparison.
      if (!tracks.at(j).passedOR()) continue;      

      // If we're still here, two at least semi-decent tracks.
      // Compare them and remove one if necessary.
      ATH_CHECK(removeOverlapping(tracks.at(i),tracks.at(j))); 

    }
  }

  return StatusCode::SUCCESS;
}


