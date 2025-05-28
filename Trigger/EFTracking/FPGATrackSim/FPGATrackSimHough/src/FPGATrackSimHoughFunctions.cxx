/*
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#include "FPGATrackSimHough/FPGATrackSimHoughFunctions.h"
#include "FPGATrackSimObjects/FPGATrackSimFunctions.h"
#include <stdexcept>

#include <AsgMessaging/MessageCheck.h>
using namespace asg::msgUserCode;

// EPSILON for hit position float comparisons
constexpr float EPSILON = 1e-5;

StatusCode runOverlapRemoval(std::vector<FPGATrackSimTrack>& tracks, const float minChi2, const int NumOfHitPerGrouping, ORAlgo orAlgo, ToolHandle<GenericMonitoringTool> & monTool)
{
  ANA_MSG_DEBUG("Beginning runOverlapRemoval()");
  ANA_MSG_DEBUG("Tracks in event: " << tracks.size());

  std::vector<int> flags_OR;
  flags_OR.clear();

  // Debug variables 
  int ntrack_passOR = 0;
  int ntrack = 0;
  std::vector<int> track_passOR_counter;
  std::vector<int> track_passOR_barcodefrac;
  track_passOR_counter.clear();
  track_passOR_barcodefrac.clear();
  int track_barcodefrac_num;
  int track_barcodefrac_den;
  float track_barcodefrac = -999;
  int ntrack_passOR_total = 0;
  int trackMuon_gt0pt5_passOR = 0;
  float tmp_TrueTrack_BCF = -999;

  // Create tracks to hold and compare
  for(unsigned int i=0; i<tracks.size();i++)
  {
    // Apply Chi2 cut
    if(tracks.at(i).getChi2ndof() > minChi2)
    {
      // Only consider track with chi2 smaller than minChi2
      tracks.at(i).setPassedOR(0);
      continue;
    }

    // Create vector for holding duplicate track list
    std::vector<int> duplicates(1,i);

    // Loop through the rest of the tracks
    for(unsigned int j=0; j<tracks.size(); j++)
    {
      if(i!=j)
      {
        // Apply Chi2 cut and potentially OR cut if so desired
        if(tracks.at(j).getChi2ndof()>minChi2)
        {
          tracks.at(j).setPassedOR(0);
          continue;
        }
        //  Based on the algorithm choose common hit of non-common hit
        if(orAlgo == ORAlgo::Normal)
        {
          // Find the number of common hits between two tracks
          int nOverlappingHits = 0;
          nOverlappingHits=findNCommonHits(tracks.at(i),tracks.at(j));

          // Group overlapping tracks into a vector for removal if at least [NumOfHitPerGrouping] hits are the same
          if(nOverlappingHits >= NumOfHitPerGrouping)
          {
            duplicates.push_back(j);
          }
        }
        else if(orAlgo == ORAlgo::InvertGrouping)
        {
          //  Find the number of non-common hits between two tracks
          int nNotOverlappingHits=0;
          nNotOverlappingHits=findNonOverlapHits(tracks.at(i), tracks.at(j));

          // If the number of non-overlapping hit is [NumOfHitPerGrouping] or less
          if(nNotOverlappingHits <= NumOfHitPerGrouping)
          {
            duplicates.push_back(j);
          }
        }
      }
    }
    findMinChi2MaxHit(duplicates, tracks, flags_OR, minChi2);

    // Monitoring 
    ntrack++;
    track_passOR_counter.push_back(ntrack);
    track_barcodefrac_num = 0;
    for(auto& hit : tracks.at(i).getFPGATrackSimHits())
      {
	      if(hit.getBarcode() == 10001) track_barcodefrac_num++;
      }
    track_barcodefrac_den = tracks.at(i).getFPGATrackSimHits().size();
    if (track_barcodefrac_den > 0){
      track_barcodefrac = (float)track_barcodefrac_num/(float)track_barcodefrac_den;
    }
    tracks.at(i).setBarcodeFrac(track_barcodefrac);
    track_passOR_barcodefrac.push_back(track_barcodefrac);
    if(tracks.at(i).getBarcodeFrac() > 0.5 && tracks.at(i).passedOR()) {
      trackMuon_gt0pt5_passOR++;
      if(trackMuon_gt0pt5_passOR == 1) { 
        tmp_TrueTrack_BCF = tracks.at(i).getBarcodeFrac(); 
      }
      if(trackMuon_gt0pt5_passOR > 1) { 
        if (tracks.at(i).getBarcodeFrac() > tmp_TrueTrack_BCF) {
          tmp_TrueTrack_BCF = tracks.at(i).getBarcodeFrac();
        }
      }
    }

  }

  // Monitoring histograms
  ANA_MSG_DEBUG("List of tracks passing OR:: ");
  for(unsigned int i=0; i<tracks.size();i++){
    if(tracks.at(i).passedOR()) {
        ntrack_passOR++;
        ANA_MSG_DEBUG("track# = " << track_passOR_counter[i] << ": chi2 = " << tracks.at(i).getChi2ndof() << " barcodefrac = " << track_passOR_barcodefrac[i]);
    }
  }
  ntrack_passOR_total += ntrack_passOR;
  auto mon_ntrack_passOR = Monitored::Scalar<int>("ntrack_passOR", ntrack_passOR);
  auto mon_barcodeFrac_passOR = Monitored::Scalar<int>("barcodeFrac_passOR", tmp_TrueTrack_BCF);
  Monitored::Group(monTool, mon_ntrack_passOR);
  Monitored::Group(monTool, mon_barcodeFrac_passOR);
  ANA_MSG_DEBUG("Number of tracks passing OR (total) = " << ntrack_passOR_total);

  
  return StatusCode::SUCCESS;
}

int findNonOverlapHits(const FPGATrackSimTrack& Track1, const FPGATrackSimTrack& Track2)
{
  int nonOverlapHits=0;

  // Loop through all layers
  for(unsigned int i = 0; i < Track1.getFPGATrackSimHits().size(); ++i)
  {
    const FPGATrackSimHit& hit1 = Track1.getFPGATrackSimHits().at(i);
    const FPGATrackSimHit& hit2 = Track2.getFPGATrackSimHits().at(i);
    //  First make sure we are looking at real hits
    if(!hit1.isReal() || !hit2.isReal())
    {
      continue;
    }
    //  Check if two hits are on the same plane
    else if(hit1.getLayer() != hit2.getLayer())
    {
      nonOverlapHits++;
    }
    // Check if two hits have the same hashID
    else if(hit1.getIdentifierHash() != hit2.getIdentifierHash())
    {
      nonOverlapHits++;
    }
    // Check if two hits have same coordinate. this is difficult due to spacepoints,
    // since the same hit can be used to make multiple spacepoints.
    else if (hit1.getHitType() == HitType::spacepoint && hit2.getHitType() == HitType::spacepoint) {
      if ((abs(hit1.getX() - hit2.getX()) > EPSILON) || (abs(hit1.getY() - hit2.getY()) < EPSILON) || (abs(hit1.getZ() - hit2.getZ()) < EPSILON)) {
        nonOverlapHits++;
      } else {
        continue;
      }
    }
    else if (std::abs(hit1.getGPhi()-hit2.getGPhi())>0.001 && std::abs(hit1.getZ()-hit2.getZ())>0.001 && std::abs(hit1.getR()-hit2.getR())>0.001) 
    {
      nonOverlapHits++;
    }
    else
    {
      continue;
    }
  }
  return nonOverlapHits;
}


void findMinChi2MaxHit(const std::vector<int>& duplicates, std::vector<FPGATrackSimTrack>& RMtracks, std::vector<int> flags_OR, const float minChi2)
{
  int ntr_belowMinChi2 = 0;
  std::vector<int> track_counter;

  for(unsigned int i=0; i<RMtracks.size();i++)
  {
    if(RMtracks.at(i).getChi2ndof() >  minChi2) {
      track_counter.push_back(0);
      flags_OR.push_back(-1);
      continue;
    }
    ntr_belowMinChi2++;    
    track_counter.push_back(ntr_belowMinChi2);
    flags_OR.push_back(1);
  }

  int dup_counter = 0;
  int head_track = 1;
  float head_chi2 = 0.;
  int head_nhits = 0;

  for(auto dup: duplicates)
  {
    float t_chi2 = RMtracks.at(dup).getChi2ndof();
    int t_nhitlayers = RMtracks.at(dup).getFPGATrackSimHits().size(); 
    for(auto& hit : RMtracks.at(dup).getFPGATrackSimHits())
    {
      ANA_MSG_DEBUG("Real hit info = " << hit);
      ANA_MSG_DEBUG("Real hit info (global) = Gphi= " << hit.getGPhi() << " Z=" << hit.getZ() << " R=" << hit.getR() << " chi2=" << t_chi2);

      if(!hit.isReal())
      {
        t_nhitlayers--;
      }
    }
    if (dup_counter == 0) {
      head_track = dup;
      head_chi2 = RMtracks.at(head_track).getChi2ndof();
      head_nhits = t_nhitlayers; 
    }
    if (dup_counter > 0){
     if(t_nhitlayers>head_nhits)
      {
        RMtracks.at(head_track).setPassedOR(0); 
      }
      else if(t_nhitlayers==head_nhits)
      {
        if((head_chi2-t_chi2)>0.000001)
          {
            RMtracks.at(head_track).setPassedOR(0);
          }
        if(std::abs(t_chi2-head_chi2)<0.000001)
          {
            if(track_counter[head_track] < track_counter[dup]) {
              RMtracks.at(dup).setPassedOR(0); 
            }
            if(track_counter[head_track] > track_counter[dup]) { 
              RMtracks.at(head_track).setPassedOR(0);
            }
          }
      }
    }
    
    if(!RMtracks.at(head_track).passedOR()) flags_OR[head_track] = 0;
    if(RMtracks.at(head_track).passedOR()) flags_OR[head_track] = 1;
    dup_counter++;
    
  }
}


int findNCommonHits(const FPGATrackSimTrack& Track1, const FPGATrackSimTrack& Track2)
{
  int nCommHits=0;

  // Loop through all layers
  for(unsigned int i = 0; i < Track1.getFPGATrackSimHits().size(); ++i)
  {
    const FPGATrackSimHit& hit1 = Track1.getFPGATrackSimHits().at(i);
    const FPGATrackSimHit& hit2 = Track2.getFPGATrackSimHits().at(i);

    // Check if hit is missing
    if(!hit1.isReal() || !hit2.isReal())
    {
      continue;
    }
    // Check if hit on the same plane
    else if(hit1.getLayer() != hit2.getLayer())
    {
      continue;
    }
    // Check if two hits have the same hashID
    else if(hit1.getIdentifierHash() != hit2.getIdentifierHash())
    {
      continue;
    }
    // Check if two hits have same coordinate. this is difficult due to spacepoints,
    // since the same hit can be used to make multiple spacepoints.
    else if (hit1.getHitType() == HitType::spacepoint && hit2.getHitType() == HitType::spacepoint) {

      if ((std::abs(hit1.getX() - hit2.getX()) < EPSILON) && (std::abs(hit1.getY() - hit2.getY()) < EPSILON) && (std::abs(hit1.getZ() - hit2.getZ()) < EPSILON)) {
        nCommHits++;
      } else {
        continue;
      }
    }
    // If both hits aren't spacepoints, we should be able to do this comparison.
    else if (std::abs(hit1.getGPhi()-hit2.getGPhi())<0.001 && std::abs(hit1.getZ()-hit2.getZ())<0.001 && std::abs(hit1.getR()-hit2.getR())<0.001) {
      nCommHits++;
    }
    else
    {
      continue;
    }
  }
  return nCommHits;
}


// Given road, populates the supplied variables with info on which layers missed hits
void getMissingInfo(const FPGATrackSimRoad & road, int & nMissing, bool & missPixel, bool & missStrip, layer_bitmask_t & missing_mask, layer_bitmask_t & norecovery_mask, const ServiceHandle<IFPGATrackSimMappingSvc> &FPGATrackSimMapping, const TrackCorrType idealCoordFitType)
{
    int subregion = road.getSubRegion();
    nMissing = FPGATrackSimMapping->PlaneMap_1st(subregion)->getNCoords(); // init with nCoords and decrement as we find misses
    missPixel = false;
    missStrip = false;
    missing_mask = 0;
    norecovery_mask = 0;
    unsigned int wclayers = road.getWCLayers();
    for (unsigned layer = 0; layer < FPGATrackSimMapping->PlaneMap_1st(subregion)->getNLogiLayers(); layer++)
    {
        int nHits = road.getHits(layer).size();
        if (nHits==0)
        {
            if (idealCoordFitType == TrackCorrType::None && ((wclayers >> layer) & 1))
            {
                int ix = FPGATrackSimMapping->PlaneMap_1st(subregion)->getCoordOffset(layer);
                int iy = ix + 1;
                if (FPGATrackSimMapping->PlaneMap_1st(subregion)->isSCT(layer))
                {
                    missing_mask |= 1 << ix;
                    nMissing -= 1;
                }
                else
                {
                    missing_mask |= (1<<ix) | (1<<iy);
                    nMissing -= 2;
                }
            }
            else
            {
                if (FPGATrackSimMapping->PlaneMap_1st(subregion)->isSCT(layer)) missStrip = true;
                else missPixel = true;
            }
        }
        else if (!((wclayers >> layer) & 1)) { // we have a hit
            int ix = FPGATrackSimMapping->PlaneMap_1st(subregion)->getCoordOffset(layer);
            int iy = ix + 1;
            if (FPGATrackSimMapping->PlaneMap_1st(subregion)->isSCT(layer))
	      {
                missing_mask |= 1 << ix;
                nMissing -= 1;
	      }
            else
	      {
                missing_mask |= (1<<ix) | (1<<iy);
                nMissing -= 2;
	      }

      }
    }
}


/**
 * Creates a list of track candidates by taking all possible combination of hits in road.
 * Sets basic ID info and hits.
 *
 * NB: If the number of combinations becomes large and memory is a concern,
 * it may be worth turning this function into a sort of iterator
 * over `combs`, return a single track each call.
 */
void makeTrackCandidates(const FPGATrackSimRoad & road, const FPGATrackSimTrack & temp, std::vector<FPGATrackSimTrack>& track_cands, const ServiceHandle<IFPGATrackSimMappingSvc> & FPGATrackSimMapping)
{
    int idbase = 0;           // offset for new track ids
    int subregion = road.getSubRegion();
    auto pmap = FPGATrackSimMapping->PlaneMap_2nd(subregion);

    if (temp.getTrackStage() == TrackStage::FIRST) {
      pmap = FPGATrackSimMapping->PlaneMap_1st(subregion);
    }

    std::vector<std::vector<int>> combs = ::getComboIndices(road.getNHits_layer());
    track_cands.resize(combs.size(), temp);

    const FPGATrackSimRegionMap* SUBREGIONMAP = FPGATrackSimMapping->SubRegionMap_2nd();
    //
    //get the WC hits:
    layer_bitmask_t wcbits= road.getWCLayers();
    // Add the hits from each combination to the track, and set ID
    for (size_t icomb = 0; icomb < combs.size(); icomb++)
    {
      //Need to set the ID and the hits size of this track
      track_cands[icomb].setTrackID(idbase + icomb);
      track_cands[icomb].setNLayers(pmap->getNLogiLayers());

      // If this is an idealized coordinate fit; keep references to the idealized radii.
      track_cands[icomb].setIdealRadii(SUBREGIONMAP->getAvgRadii(subregion));
      track_cands[icomb].setPassedOR(1);

        std::vector<int> const & hit_indices = combs[icomb]; // size nLayers
        for (unsigned layer = 0; layer < pmap->getNLogiLayers(); layer++)
        {
            if (hit_indices[layer] < 0) // Set a dummy hit if road has no hits in this layer
            {
                FPGATrackSimHit newhit=FPGATrackSimHit();
                newhit.setLayer(layer);
                newhit.setSection(0);
                if (pmap->getDim(layer) == 2) newhit.setDetType(SiliconTech::pixel);
	            else newhit.setDetType(SiliconTech::strip);

                if (wcbits & (1 << layer ) ) {
                    newhit.setHitType(HitType::wildcard);
                    newhit.setLayer(layer);
                }

                track_cands[icomb].setFPGATrackSimHit(layer, newhit);
            }
            else
            {
                const std::shared_ptr<const FPGATrackSimHit> hit = road.getHits(layer)[hit_indices[layer]];
                // If this is an outer spacepoint, and it is not the same as the inner spacepoint, reject it.
                // Here we "reject" it by marking the candidate as "invalid", to be rejected later.
                // That require another field on the track object, but it avoids having to change the sizes
                // of arrays computed above.
                if (hit->getHitType() == HitType::spacepoint && (hit->getPhysLayer(true) % 2) == 1) {
                    if (layer == 0) throw (std::out_of_range("makeTrackCandidates: Attempt to access vector at element -1"));
                    const FPGATrackSimHit & inner_hit = track_cands[icomb].getFPGATrackSimHits().at(layer - 1);
                    if ((abs(hit->getX() - inner_hit.getX()) > EPSILON) || (abs(hit->getY() - inner_hit.getY()) > EPSILON) || (abs(hit->getZ() - inner_hit.getZ()) > EPSILON)) {
                        track_cands[icomb].setValidCand(false);
                    }
                }
                track_cands[icomb].setFPGATrackSimHit(layer, *hit);
            }
        }
    }

    idbase += combs.size();
}


long getVolumeID(const FPGATrackSimHit & hit)
{
  // Custom labelling for the detector volumes
  // to be used in NN training.
  // Convention:
  //           barrel  |  else
  //-------------------------------------
  // Pixel |   0       |  2*sign(z)
  // Strip |   10      |  10 + 2*sign(z)
  //
  // Explicitly:
  // -2: C-side (-z) end cap pixel
  // 0: barrel pixel
  // 2: A-side (+z) end cap pixel
  // 8: C-side end cap strip
  // 10: barrel strip
  // 12: A-sde end cap strip
  // returns -999 if not a real hit


  long volumeID = -1;

  if (hit.getR() == 0.0) {
    return -999; //hit not in any physical layer
  }

  if(hit.isBarrel()) {
    if (hit.isPixel()) {
      volumeID = 0;
    }
    if (hit.isStrip()) {
      volumeID = 10;
    }
  }
  else {
    if (hit.isPixel()) {
      if (hit.getZ() >= 0.) {
        volumeID = 2;
      }
      else {
        volumeID = -2;
      }
    }
    else if (hit.isStrip()) {
      if (hit.getZ() >= 0.) {
        volumeID = 12;
      }
      else {
        volumeID = 8;
      }
    }
  }
  return volumeID;
}

long getCoarseID(const FPGATrackSimHit & hit)
{
  // Custom labelling for the detector layers
  // to be used in NN training.
  // This aims at providing a continuous numbering according to this convention:
  // strip barrel layers in [0,3]
  // strip C-side end cap layers in [4,9]
  // strip A-side end cap layers in [10,15]
  // pixel barrel layers in [16,20]
  // pixel C-side end cap layers in [21,29]
  // pixel A-side end cap layers > 30
  // returns large negative value if no layer

  long volumeID = getVolumeID(hit);
  unsigned layerID = hit.getLayerDisk(true);

  long offset = -10000;

  if(volumeID == 10)  offset = 0; //strip barrel
  if(volumeID == 8)   offset = 4; //strip -ve EC
  if(volumeID == 12)  offset = 10; //strip +ve EC
  if(volumeID == 0)   offset = 16; // pix barrel
  if(volumeID == -2)  offset = 21; //pix -ve EC
  if(volumeID == 2)   offset = 30; //pix +ve EC
  return offset + layerID;
}

bool isFineIDInStrip(long ID)
{
  return (ID < 16);
}

bool isFineIDInPixel(long ID)
{
  return !isFineIDInStrip(ID);
}

long getFineID(const FPGATrackSimHit & hit)
{
  // Custom labelling for the detector layers
  // to be used in NN training.
  // Returns a labelling of non-barrel pixel hits similar to a hit's eta index,
  // but with a continuous numbering.
  // Otherwise return convention defined in getCoarseID.

  long volumeID = getVolumeID(hit);
  unsigned layerID = hit.getLayerDisk(true);
  int etaID = hit.getEtaModule(true);

  long offset = -1000;

  if(volumeID == 10) return getCoarseID(hit);  //strip barrel
  if(volumeID == 8)  return getCoarseID(hit);  //strip -ve EC
  if(volumeID == 12) return getCoarseID(hit);  //strip +ve EC
  if(volumeID == 0)  return getCoarseID(hit);  //pix barrel
  if(volumeID == -999)  return -999;  //hit not in any physical layer
  if(volumeID == -2)
  {
    if(layerID == 0) offset = 21;
    if(layerID == 1) offset = 21+15;
    if(layerID == 2) offset = 21+15+6;
    if(layerID == 3) offset = 21+15+6+23;
    if(layerID == 4) offset = 21+15+6+23+6;
    if(layerID == 5) offset = 21+15+6+23+6+11;
    if(layerID == 6) offset = 21+15+6+23+6+11+8;
    if(layerID == 7) offset = 21+15+6+23+6+11+8+8;
    if(layerID == 8) offset = 21+15+6+23+6+11+8+8+9;
    return offset + etaID;
  }
  if(volumeID == 2)
  {
    if(layerID == 0) offset = 116;
    if(layerID == 1) offset = 116+15;
    if(layerID == 2) offset = 116+15+6;
    if(layerID == 3) offset = 116+15+6+23;
    if(layerID == 4) offset = 116+15+6+23+6;
    if(layerID == 5) offset = 116+15+6+23+6+11;
    if(layerID == 6) offset = 116+15+6+23+6+11+8;
    if(layerID == 7) offset = 116+15+6+23+6+11+8+8;
    if(layerID == 8) offset = 116+15+6+23+6+11+8+8+9;
    return offset + etaID;
  }

  return -1;
}

// Adapted from TrackFitter, but TrackFitter *depends* on fit constants and this algorithm
void roadsToTrack(std::vector<std::shared_ptr<const FPGATrackSimRoad>>& roads, std::vector<FPGATrackSimTrack>& track_cands, const FPGATrackSimPlaneMap *pmap)
{


    for (const std::shared_ptr<const FPGATrackSimRoad>& road : roads) {

      FPGATrackSimTrack temp;
      temp.setNLayers(pmap->getNLogiLayers());
      temp.setBankID(-1);
      temp.setPatternID(road->getPID());
      temp.setHoughX(road->getX());
      temp.setHoughY(road->getY());
      temp.setQOverPt(road->getY());

      temp.setSubRegion(road->getSubRegion());
      temp.setHoughXBin(road->getXBin());
      temp.setHoughYBin(road->getYBin());
      temp.setChi2(0);

      // This comes from FPGATrackSimFunctions
      std::vector<std::vector<int>> combs = getComboIndices(road->getNHits_layer());
      unsigned existing_size = track_cands.size();
      track_cands.resize(existing_size + combs.size(), temp);

      //get the WC hits:
      layer_bitmask_t wcbits= road->getWCLayers();
      // Add the hits from each combination to the track, and set ID
      for (size_t icomb = 0; icomb < combs.size(); icomb++)
      {
        if ((existing_size + icomb) >= track_cands.size()) continue;
        track_cands[existing_size + icomb].setNLayers(pmap->getNLogiLayers());
        std::vector<int> const & hit_indices = combs[icomb]; // size nLayers
        for (unsigned layer = 0; layer < pmap->getNLogiLayers(); layer++)
        {
            if (hit_indices[layer] < 0) // Set a dummy hit if road has no hits in this layer
            {
                FPGATrackSimHit newhit=FPGATrackSimHit();
                newhit.setLayer(layer);
                newhit.setSection(0);
                if (pmap->getDim(layer) == 2) newhit.setDetType(SiliconTech::pixel);
                    else newhit.setDetType(SiliconTech::strip);

                if (wcbits & (1 << layer ) ) {
                    newhit.setHitType(HitType::wildcard);
                    newhit.setLayer(layer);
                }

                track_cands[existing_size + icomb].setFPGATrackSimHit(layer, newhit);
            }
            else
            {
                const std::shared_ptr<const FPGATrackSimHit> hit = road->getHits(layer)[hit_indices[layer]];
                // If this is an outer spacepoint, and it is not the same as the inner spacepoint, reject it.
                // Here we "reject" it by marking the candidate as "invalid", to be rejected later.
                // That require another field on the track object, but it avoids having to change the sizes
                // of arrays computed above.
                if (hit->getHitType() == HitType::spacepoint && (hit->getPhysLayer() % 2) == 1 && (layer>0)) {
                    const FPGATrackSimHit inner_hit = track_cands[existing_size + icomb].getFPGATrackSimHits().at(layer - 1);
                    if ((hit->getX() != inner_hit.getX()) || (hit->getY() != inner_hit.getY()) || (hit->getZ() != inner_hit.getZ())) {
                        track_cands[existing_size + icomb].setValidCand(false);
                    }
                }
                track_cands[existing_size + icomb].setFPGATrackSimHit(layer, *hit);
            }
        }
      }
    }
}

