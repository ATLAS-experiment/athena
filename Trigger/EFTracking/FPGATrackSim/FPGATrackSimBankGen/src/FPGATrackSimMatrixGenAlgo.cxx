// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

/**
 * @file FPGATrackSimMatrixGenAlgo.cxx
 * @author Rewrite by Riley Xu - riley.xu@cern.ch after FTK code
 * @date May 8th, 2020
 * @brief See FPGATrackSimMatrixGenAlgo.h
 */


#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ITHistSvc.h"
#include "FPGATrackSimMatrixGenAlgo.h"
#include "FPGATrackSimMatrixAccumulator.h"
#include "FPGATrackSimConfTools/FPGATrackSimRegionSlices.h"
#include "FPGATrackSimObjects/FPGATrackSimConstants.h"
#include "FPGATrackSimObjects/FPGATrackSimFunctions.h"
#include "TruthUtils/MagicNumbers.h"

#include "TH1.h"
#include "TH2.h"
#include "TStyle.h"

#include <cassert>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cmath>
#include <utility>


///////////////////////////////////////////////////////////////////////////////
// Constructors
///////////////////////////////////////////////////////////////////////////////

FPGATrackSimMatrixGenAlgo::FPGATrackSimMatrixGenAlgo(const std::string& name, ISvcLocator* pSvcLocator) :
  AthAlgorithm(name, pSvcLocator)
{
}



///////////////////////////////////////////////////////////////////////////////
// Initialize
///////////////////////////////////////////////////////////////////////////////

StatusCode FPGATrackSimMatrixGenAlgo::initialize()
{
  ATH_MSG_DEBUG("initialize()");

  // set the slicing variables from inputs
  m_sliceMax.qOverPt = m_temp_c_max;
  m_sliceMax.phi = m_temp_phi_max;
  m_sliceMax.eta = m_temp_eta_max;
  m_sliceMax.d0 = m_temp_d0_max;
  m_sliceMax.z0 = m_temp_z0_max;

  m_sliceMin.qOverPt = m_temp_c_min;
  m_sliceMin.phi = m_temp_phi_min;
  m_sliceMin.eta = m_temp_eta_min;
  m_sliceMin.d0 = m_temp_d0_min;
  m_sliceMin.z0 = m_temp_z0_min;

  m_nBins.qOverPt = m_temp_c_slices;
  m_nBins.phi = m_temp_phi_slices;
  m_nBins.eta = m_temp_eta_slices;
  m_nBins.d0 = m_temp_d0_slices;
  m_nBins.z0 = m_temp_z0_slices;

  // Retrieve handles
  ATH_CHECK(m_tHistSvc.retrieve());
  ATH_CHECK(m_FPGATrackSimMapping.retrieve());
  ATH_CHECK(m_hitInputTool.retrieve());
  ATH_CHECK(m_hitMapTool.retrieve());
  ATH_CHECK(m_EvtSel.retrieve());
  ATH_CHECK(m_roadFinderTool.retrieve());
  if (m_doClustering) ATH_CHECK(m_clusteringTool.retrieve());
  if (m_doSpacePoints) ATH_CHECK(m_spacePointsTool.retrieve());

  if (m_doHoughConstants) {
    if (m_ideal_geom == 0) {
      ATH_MSG_INFO("Hough constants method needs idealized geometry > 0, aborting.");
      return StatusCode::FAILURE;
    }
    m_pmap = m_FPGATrackSimMapping->PlaneMap_1st(0);
    // Get detector configurations
    m_nLayers = m_FPGATrackSimMapping->PlaneMap_1st(0)->getNLogiLayers();
    m_nRegions = m_FPGATrackSimMapping->RegionMap_1st()->getNRegions();
    m_nDim = m_FPGATrackSimMapping->PlaneMap_1st(0)->getNCoords();

  }
  else {
    m_pmap = m_FPGATrackSimMapping->PlaneMap_2nd(0);
    // Get detector configurations
    m_nLayers = m_FPGATrackSimMapping->PlaneMap_2nd(0)->getNLogiLayers();
    m_nRegions = m_FPGATrackSimMapping->RegionMap_2nd()->getNRegions();
    m_nDim = m_FPGATrackSimMapping->PlaneMap_2nd(0)->getNCoords();
  }

  m_nDim2 = m_nDim * m_nDim;
  m_sector_cum.resize(m_nRegions);

  // Retrieve slice information
  m_sliceMin = m_EvtSel->getMin();
  m_sliceMax = m_EvtSel->getMax();

  // Check q/pt binning information
  if (m_qOverPtBins.size() == 0) {
    ATH_MSG_ERROR("q/pt bin information not set in matrix element job options!");
    return StatusCode::FAILURE;
  }


  // Histograms
  ATH_CHECK(bookHistograms());

  m_eventHeader            = new FPGATrackSimEventInputHeader();

  return StatusCode::SUCCESS;
}


StatusCode FPGATrackSimMatrixGenAlgo::bookHistograms()
{
  // Training tracks
  for (unsigned i = 0; i < FPGATrackSimTrackPars::NPARS; i++) {
    std::string name = "h_trainingTrack_" + FPGATrackSimTrackPars::parName(i);
    std::string title = FPGATrackSimTrackPars::parName(i) + " of tracks passing pt/barcode check";
    
    m_h_trainingTrack[i] = new TH1I(name.c_str(), title.c_str(), 100, m_sliceMin[i], m_sliceMax[i]);
    ATH_CHECK(m_tHistSvc->regHist("/TRIGFPGATrackSimMATRIXOUT/" + name, m_h_trainingTrack[i]));
  }

  // Sector pars
  for (unsigned i = 0; i < FPGATrackSimTrackPars::NPARS; i++) {
    
    std::string name = "h_sectorPars_" + FPGATrackSimTrackPars::parName(i);
    std::string title = "Average " + FPGATrackSimTrackPars::parName(i) + " in sector";
    
    m_h_sectorPars[i] = new TH1I(name.c_str(), title.c_str(), 100, m_sliceMin[i], m_sliceMax[i]);
    ATH_CHECK(m_tHistSvc->regHist("/TRIGFPGATrackSimMATRIXOUT/" + name, m_h_sectorPars[i]));
  }

  // Select hit failure
  for (unsigned i = 0; i < FPGATrackSimTrackPars::NPARS; i++) {
    
    std::string name = "h_SHfailure_" + FPGATrackSimTrackPars::parName(i);
    std::string title = FPGATrackSimTrackPars::parName(i) + " of tracks failing in selectHit()";
    
    m_h_SHfailure[i] = new TH1I(name.c_str(), title.c_str(), 100, m_sliceMin[i], m_sliceMax[i]);
    ATH_CHECK(m_tHistSvc->regHist("/TRIGFPGATrackSimMATRIXOUT/" + name, m_h_SHfailure[i]));
  }

  // 3 hits in layer
  for (unsigned i = 0; i < FPGATrackSimTrackPars::NPARS; i++) {

    std::string name = "h_3hitsInLayer_" + FPGATrackSimTrackPars::parName(i);
    std::string title = FPGATrackSimTrackPars::parName(i) + " of tracks containing 3+ hits in a single layer";
    
    m_h_3hitsInLayer[i] = new TH1I(name.c_str(), title.c_str(), 100, m_sliceMin[i], m_sliceMax[i]);
    ATH_CHECK(m_tHistSvc->regHist("/TRIGFPGATrackSimMATRIXOUT/" + name, m_h_3hitsInLayer[i]));
  }

  // Not enough hits
  for (unsigned i = 0; i < FPGATrackSimTrackPars::NPARS; i++) {
    std::string name = "h_notEnoughHits_" + FPGATrackSimTrackPars::parName(i);
    std::string title = FPGATrackSimTrackPars::parName(i) + " of tracks failing because it didn't have enough hits";
    
    m_h_notEnoughHits[i] = new TH1I(name.c_str(), title.c_str(), 100, m_sliceMin[i], m_sliceMax[i]);
    ATH_CHECK(m_tHistSvc->regHist("/TRIGFPGATrackSimMATRIXOUT/" + name, m_h_notEnoughHits[i]));
  }
  
  m_h_trackQoP_okHits = new TH1I("h_trackQoP_okHits", "half inverse pt of tracks passing hit check",
			       m_nBins.qOverPt, m_sliceMin.qOverPt, m_sliceMax.qOverPt);
  m_h_trackQoP_okRegion = new TH1I("h_trackQoP_okRegion", "half inverse pt of tracks passing region check",
				 m_nBins.qOverPt, m_sliceMin.qOverPt, m_sliceMax.qOverPt);
  m_h_nHit = new TH1I("h_nHit", "number of hits in sector", 100, 0, 100);

  ATH_CHECK(m_tHistSvc->regHist("/TRIGFPGATrackSimMATRIXOUT/h_trackQoP_okHits", m_h_trackQoP_okHits));
  ATH_CHECK(m_tHistSvc->regHist("/TRIGFPGATrackSimMATRIXOUT/h_trackQoP_okRegion", m_h_trackQoP_okRegion));
  ATH_CHECK(m_tHistSvc->regHist("/TRIGFPGATrackSimMATRIXOUT/h_nHit",m_h_nHit));

  return StatusCode::SUCCESS;
}

void fillTrackPars(TH1I* const hists[FPGATrackSimTrackPars::NPARS], FPGATrackSimTruthTrack const & track)
{
  FPGATrackSimTrackPars pars = track.getPars();
  for (unsigned i = 0; i < FPGATrackSimTrackPars::NPARS; i++)
    hists[i]->Fill(pars[i]);
}

///////////////////////////////////////////////////////////////////////////////
// Execute
///////////////////////////////////////////////////////////////////////////////


StatusCode FPGATrackSimMatrixGenAlgo::execute()
{
  m_eventHeader->clearHits();
  m_eventHeader->reset();

  // Get hits and training tracks from this event
  ATH_CHECK(m_hitInputTool->readData(m_eventHeader, Gaudi::Hive::currentContext()));

  std::vector<FPGATrackSimHit> hits = getLogicalHits();

  std::vector<FPGATrackSimTruthTrack> truth_tracks = m_eventHeader->optional().getTruthTracks();
  std::vector<FPGATrackSimTruthTrack> tracks = filterTrainingTracks(truth_tracks);

  m_nTracks += truth_tracks.size();
  if (tracks.empty()) {  
    ATH_MSG_DEBUG("Empty training tracks");
    return StatusCode::SUCCESS;
  }

  // Prepare a map of the hits according the barcode
  std::map<int, std::vector<FPGATrackSimHit>> barcode_hits = makeBarcodeMap(hits, tracks);

  // For each training track, find the sector it belongs to and accumulate the
  // hit coordinates and track parameters.
  for (FPGATrackSimTruthTrack const & track : tracks) {

    int nSlices = m_FPGATrackSimMapping->SubRegionMap()->getNRegions();
    // Get list of hits associated to the current truth track
    std::vector<FPGATrackSimHit> & track_hits = barcode_hits[track.getBarcode()];

    const FPGATrackSimPlaneMap *t_pmap = nullptr;

    // Get the hits that will form the actual sector

    for (int iSlice = 0; iSlice<nSlices; iSlice++){
      t_pmap = m_FPGATrackSimMapping->PlaneMap_1st(iSlice);
      for (auto & iHit : track_hits) {
        t_pmap->map(iHit);
      }
      std::vector<FPGATrackSimHit> sector_hits;
      bool success = filterSectorHits(track_hits, sector_hits, track, iSlice);
      if (!success) continue; // Skip this track if it has bad hits (not complete, etc.)
      m_h_trackQoP_okHits->Fill(track.getQOverPt());

      // Get the region of this sector
      int region = getRegion(sector_hits);
      if (region < 0 || region >= m_nRegions) continue;
      m_h_trackQoP_okRegion->Fill(track.getQOverPt());
        
      //For the Hough constants, find the Hough roads
      std::vector<std::shared_ptr<const FPGATrackSimRoad>> houghRoads;
      if (m_doHoughConstants) {
        std::vector<std::shared_ptr<const FPGATrackSimHit>> phits;

        for (const FPGATrackSimHit& hit : sector_hits) phits.emplace_back(std::make_shared<const FPGATrackSimHit>(hit));

        StatusCode sc = m_roadFinderTool->getRoads(phits, houghRoads);
        if (sc.isFailure()) ATH_MSG_WARNING("Hough Transform -> getRoads() failed");

        if (!houghRoads.empty()){
          double y = 0.0;
          double x = 0.0;

          //For each Hough road, make the accumulator
          for (auto const &hr : houghRoads){
            y = hr->getY();
            x = hr->getX();

            // Prepare the accumulator struct
            std::vector<module_t> modules(m_nLayers);
            FPGATrackSimMatrixAccumulator acc(m_nLayers, m_nDim);
            acc.pars.qOverPt = y;
            acc.pars.phi = x;
            std::pair<std::vector<module_t>, FPGATrackSimMatrixAccumulator> modules_acc = {modules, acc};
            ATH_CHECK(makeAccumulator(sector_hits, track, modules_acc));

            // Add the track to the accumulate map
            accumulate(m_sector_cum[region], modules_acc.first, modules_acc.second);
            m_nTracksUsed++;
          }
        } else {
          ATH_MSG_DEBUG("execute(): no hough roads?");
          return StatusCode::SUCCESS;
        }
      } else {
        // Prepare the accumulator struct
        std::vector<module_t> modules(m_nLayers);
        FPGATrackSimMatrixAccumulator acc(m_nLayers, m_nDim);
        std::pair<std::vector<module_t>, FPGATrackSimMatrixAccumulator> modules_acc = {modules, acc};
        ATH_CHECK(makeAccumulator(sector_hits, track, modules_acc));

        // Add the track to the accumulate map
        accumulate(m_sector_cum[region], modules_acc.first, modules_acc.second);
        m_nTracksUsed++;
      }
    }
  }
  
  return StatusCode::SUCCESS;
}


// Converts raw hits from header into logical hits, and filters those in FPGATrackSim layers
// Could replace this with the RawToLogical tool (but probably won't)
std::vector<FPGATrackSimHit> FPGATrackSimMatrixGenAlgo::getLogicalHits()
{
  std::vector<FPGATrackSimHit> hits;
  //Setup the logical header...
  FPGATrackSimLogicalEventInputHeader logicalHeader;
  //Map the hits to the logical header...
  unsigned stage = 0;
  if (m_doHoughConstants) stage = 1; // For now Hough constants only works on 1st stage
  else stage = 2;
  StatusCode sc = m_hitMapTool->convert(stage, *m_eventHeader, logicalHeader);
  if (sc.isFailure()) ATH_MSG_ERROR("Hit mapping failed");

  // Since the clustering tool modifies the logical towers-- refactored this
  // to only access the output hits from the towers.
  if (m_doClustering) {
    std::vector<FPGATrackSimCluster> clustered_hits;
    sc = m_clusteringTool->DoClustering(logicalHeader, clustered_hits);
    if (sc.isFailure()) ATH_MSG_ERROR("Clustering failed");
  }
  // Optionally do spacepoints (as well).
  if (m_doSpacePoints) {
    std::vector<FPGATrackSimCluster> spacepoints;
    sc = m_spacePointsTool->DoSpacePoints(logicalHeader, spacepoints);
    if (sc.isFailure()) ATH_MSG_ERROR("Spacepoints failed");
  }

  // It should now be safe to pull the towers, regardless.
  std::vector<FPGATrackSimTowerInputHeader> towers = logicalHeader.towers();
  for (auto &tower : towers) {
    std::vector<FPGATrackSimHit> const & towerHits = tower.hits();
    for (FPGATrackSimHit const & hit : towerHits) {
      hits.push_back(hit);
    }
  }
  return hits;
}


// Filters tracks based on m_PT_THRESHOLD and m_TRAING_PDG and D0_THRESHOLD
std::vector<FPGATrackSimTruthTrack> FPGATrackSimMatrixGenAlgo::filterTrainingTracks(std::vector<FPGATrackSimTruthTrack> const & truth_tracks) const
{
  std::vector<FPGATrackSimTruthTrack> training_tracks;

  for (FPGATrackSimTruthTrack const & track : truth_tracks) {
    if (HepMC::generations(&track) >= 1 || std::abs(track.getPDGCode()) != m_TRAIN_PDG) continue;
    if (std::abs(track.getD0()) > m_D0_THRESHOLD) continue;
    
    double pt = TMath::Sqrt(track.getPX()*track.getPX() + track.getPY()*track.getPY());
    double pt_GeV = pt / 1000;
    
    // Add the track to the list of good tracks
    if (pt_GeV > m_PT_THRESHOLD) training_tracks.push_back(track);
    
    // Debug
    if (msgLvl(MSG::DEBUG)) {
      double c = track.getQ() /(2 * pt);
      double eta = TMath::ASinH(track.getPZ() / pt);
      double phi = TMath::ATan2(track.getPY(), track.getPX());
      ATH_MSG_DEBUG("pt_GeV = "<< pt_GeV
		    << " c = " << c
		    << " eta = " << eta
		    << " phi = " << phi
		    << " pdgcode = " << track.getPDGCode());
    }
    fillTrackPars(m_h_trainingTrack, track);
  }
  
  return training_tracks;
}


// Sorts 'hits' by barcodes appearing in 'tracks', drops the rest.
std::map<int, std::vector<FPGATrackSimHit>> FPGATrackSimMatrixGenAlgo::makeBarcodeMap(std::vector<FPGATrackSimHit> const & hits, std::vector<FPGATrackSimTruthTrack> const & tracks) const
{
  std::map<int, std::vector<FPGATrackSimHit>> map;

  // Ready the barcodes
  for (const FPGATrackSimTruthTrack & track : tracks)
    map[track.getBarcode()] = std::vector<FPGATrackSimHit>();

  // Add the hits
  for (const FPGATrackSimHit & hit : hits) {
    // Get the predominant barcode for the current hit
    int barcode = hit.getTruth().best_barcode();
    
    // Add hit to the list; skip the hits if not associated to a good truth tracks
    auto it = map.find(barcode);
    if (it != map.end()) (*it).second.push_back(hit);
  }
  
  return map;
}


// Given two hits in the same layer, selects the better hit to use for sector
// generation based on multiple criteria.
//
// Returns:
//      0 - Failure, this track should be discarded
//      1 - Keep old hit
//      2 - Use new hit
//
// NB: sector overlap is a perfectly reasonable situation with forward disks
// eta and phi will differ in general in this case
// Take the lower-z hit preferentially (right thing to do? d0/pT tradeoff)
// But something fishy is going on if we've got two hits on the same disk.
FPGATrackSimMatrixGenAlgo::selectHit_returnCode FPGATrackSimMatrixGenAlgo::selectHit(FPGATrackSimHit const & old_hit, FPGATrackSimHit const & new_hit,
                                                                                     int subregion) const
{
  if ((new_hit.getSection() == old_hit.getSection()) && (new_hit.getLayer() == old_hit.getLayer())
      && (new_hit.getEtaModule() == old_hit.getEtaModule()) && (new_hit.getPhiModule() == old_hit.getPhiModule())) {
    ATH_MSG_DEBUG("Two hits on same module");
    return selectHit_returnCode::SH_FAILURE;
  }
    
  // Always prefer spacepoints, regardless of all other considerations.
  // This is necessary in part due to spacepoint duplication.
  if (old_hit.getHitType() == HitType::spacepoint && new_hit.getHitType() != HitType::spacepoint) {
    return selectHit_returnCode::SH_KEEP_OLD;
  } else if (old_hit.getHitType() != HitType::spacepoint && new_hit.getHitType() == HitType::spacepoint) {
    return selectHit_returnCode::SH_KEEP_NEW;
  }

  int new_section = new_hit.getSection();
  int old_section = old_hit.getSection();

  if (old_section == new_section) {

    if (old_hit.getEtaModule() == new_hit.getEtaModule()) {
      int rmax = 0;
      if (m_doHoughConstants) {
        // Use the subregion map, not the region map, for these checks.
        int phi_max = m_FPGATrackSimMapping->SubRegionMap()->getRegionBoundaries(subregion, new_hit.getLayer(), new_section).phi_max;
        int phi_min = m_FPGATrackSimMapping->SubRegionMap()->getRegionBoundaries(subregion, new_hit.getLayer(), new_section).phi_min;

        rmax = phi_max - phi_min;
      }
      else {
        // TODO: this needs updating, once we get the second stage going again.
        int reg = m_FPGATrackSimMapping->RegionMap_2nd()->getRegions(new_hit)[0]; // just take region with lowest index

        int phi_max = m_FPGATrackSimMapping->RegionMap_2nd()->getRegionBoundaries(reg, new_hit.getLayer(), new_section).phi_max;
        int phi_min = m_FPGATrackSimMapping->RegionMap_2nd()->getRegionBoundaries(reg, new_hit.getLayer(), new_section).phi_min;

        rmax = phi_max - phi_min;
      }
      
      int phi_diff = old_hit.getPhiModule() - new_hit.getPhiModule();
      
      if (phi_diff == 1 || phi_diff == -rmax) return selectHit_returnCode::SH_KEEP_OLD;
      else if (phi_diff == -1 || phi_diff == rmax) return selectHit_returnCode::SH_KEEP_NEW;
      else {
	ATH_MSG_DEBUG("Hits are too far away in phi");
	return selectHit_returnCode::SH_FAILURE;
      }
    }
    else { // Different eta is no good
      
      ATH_MSG_DEBUG("Hits are in different eta");
      return selectHit_returnCode::SH_FAILURE;
    }
  }
  else { // sections are different
    
    int  layer = old_hit.getLayer();
    bool old_isEC = 0;
    bool new_isEC = 0;
    int  old_disk = 0;
    int  new_disk = 0;
    if (m_doHoughConstants) {
      old_isEC = m_FPGATrackSimMapping->PlaneMap_1st(subregion)->isEC(layer, old_section);
      new_isEC = m_FPGATrackSimMapping->PlaneMap_1st(subregion)->isEC(layer, new_section);
      old_disk = m_FPGATrackSimMapping->PlaneMap_1st(subregion)->getLayerInfo(layer, old_section).physDisk;
      new_disk = m_FPGATrackSimMapping->PlaneMap_1st(subregion)->getLayerInfo(layer, new_section).physDisk;
    }
    else {
      // This will need updating, once we get to the second stage.
      old_isEC = m_FPGATrackSimMapping->PlaneMap_2nd(subregion)->isEC(layer, old_section);
      new_isEC = m_FPGATrackSimMapping->PlaneMap_2nd(subregion)->isEC(layer, new_section);
      old_disk = m_FPGATrackSimMapping->PlaneMap_2nd(subregion)->getLayerInfo(layer, old_section).physDisk;
      new_disk = m_FPGATrackSimMapping->PlaneMap_2nd(subregion)->getLayerInfo(layer, new_section).physDisk;
    }
    // If one is barrel and one endcap, it's definitely OK, take the barrel hit
    if (old_isEC != new_isEC) {
      
      if (old_isEC) return selectHit_returnCode::SH_KEEP_NEW;
      else return selectHit_returnCode::SH_KEEP_OLD;
    }
    // Two endcap hits : same disk: discard
    else if (old_disk == new_disk) {
      
      ATH_MSG_DEBUG("Two modules hit in same physical disk " << old_disk);
      return selectHit_returnCode::SH_FAILURE;
    }
    // Two endcap hits on same side: different disks: take the lower-z
    else {
      ATH_MSG_DEBUG("Keeping the lower-z of the two disks (" << old_disk << ", " << new_disk << ") hit");
      if (old_disk > new_disk) return selectHit_returnCode::SH_KEEP_NEW;
      else return selectHit_returnCode::SH_KEEP_OLD;
    }
  }
}


// A sector is created from 8 hits in 8 layers. Sometimes there will be extraneous hits
// that need to be filtered. This functions returns true on success, and by reference
// the filtered hit list with size m_nLayers.
//
// the truth track is "temp" and could be removed in the future?
// See selectHit() for details on which hit is chosen when there's more than 1 per layer.
bool FPGATrackSimMatrixGenAlgo::filterSectorHits(std::vector<FPGATrackSimHit> const & all_hits, std::vector<FPGATrackSimHit> & sector_hits,
                                                 FPGATrackSimTruthTrack const & t, int subregion) const
{
  FPGATrackSimHit nohit;
  nohit.setHitType(HitType::wildcard);
  sector_hits.resize(m_nLayers, nohit);
  std::vector<int> layer_count(m_nLayers); // count number of hits seen in each layer

  const FPGATrackSimRegionMap* rmap_1st = m_FPGATrackSimMapping->SubRegionMap();

  for (FPGATrackSimHit const & hit : all_hits) {
    if (!hit.isMapped()){
        continue;
    }
    // Sanity check. make sure the hit is actually in the first stage?
    // If the hit falls within the boundaries of ANY subregion in the first stage, it's 1st stage.
    if (rmap_1st->getRegions(hit).size() == 0) {
      continue;
    }
    int layer = hit.getLayer();

    if (layer_count[layer] == 0){
      layer_count[layer]++;
      sector_hits[layer] = hit;
    }
    else if (layer_count[layer] == 1) {
      layer_count[layer]++;
      
      // Already found a hit in this layer, so pick which hit to use.
      // This needs to be subregion aware, unfortunately, so we use the right subrmap to do the checks.
      selectHit_returnCode selected_hit = selectHit(sector_hits[layer], hit, subregion);
      
      if (selected_hit == selectHit_returnCode::SH_FAILURE) {
	fillTrackPars(m_h_SHfailure, t);
	return false;
      }
      else if (selected_hit == selectHit_returnCode::SH_KEEP_NEW) sector_hits[layer] = hit;
    }
    else {
      ATH_MSG_DEBUG("Too many hits on a plane, exiting filterHitsSec");
      fillTrackPars(m_h_3hitsInLayer, t);
      return false;
    }
  }
  
  // Count number of wildcards, spacepoints, and pixel hits.
  int nwc = 0;
  int num_sp = 0;
  int num_pixel = 0;

  // Check we have the right number of hits
  // Check we have the right number of hits.
  for (int i = 0; i < m_nLayers; ++i)
  {
    if (layer_count[i] == 0)
    {
      ATH_MSG_DEBUG("Layer " << i << " has no hits");
      nwc++;
    }

    // Now that we've decided which hit to use-- check their type.
    if (sector_hits[i].getHitType() == HitType::spacepoint) {
      num_sp += 1;
    }
    if (sector_hits[i].isPixel()) {
      num_pixel += 1;
    }
  }

  ATH_MSG_DEBUG("Found " << nwc << " wildcards compared to maximum: " << m_MaxWC);
  // Divide by 2 due to spacepoint duplication.
  num_sp /= 2;
  ATH_MSG_DEBUG("Found " << num_sp << " spacepoints after removing duplicates.");
  // Require we don't have too many wildcards.
  if (nwc > m_MaxWC)
  {
    fillTrackPars(m_h_notEnoughHits, t);
    return false;
  }
  // Require that we have a certain number of "2D" hits (i.e. pixels and spacepoints)
  // The effect of this is that we can ensure we have 4/5 2D hits but 7/9 hits total.
  // NOTE Again, uncomment logic below for second stage running.
  num_sp += num_pixel;
  int minSpacePlusPixel = /*m_isSecondStage ? m_minSpacePlusPixel2 :*/ m_minSpacePlusPixel;
  if (num_sp < minSpacePlusPixel) {
    ATH_MSG_DEBUG("Not enough pixel hits + spacepoints (" << num_sp << " < " << minSpacePlusPixel << ")");
    fillTrackPars(m_h_notEnoughHits, t);
    return false;
  }
  return true;
}


// Returns the lowest index region that contains all hits in 'hits'
int FPGATrackSimMatrixGenAlgo::getRegion(std::vector<FPGATrackSimHit> const & hits) const
{
  // Start with a bitmask, all true, and set a region to false if any mismatch is found
  std::vector<bool> region_mask(m_nRegions, true);

  for (FPGATrackSimHit const & hit : hits) {
    if (hit.getHitType() !=  HitType::wildcard){ // don't worry about hits that are WCs
      for (int region = 0; region < m_nRegions; region++) {
	if (m_doHoughConstants) {
	  if (!m_FPGATrackSimMapping->RegionMap_1st()->isInRegion(region, hit))
	    region_mask[region] = false;
	}
	else {
	  if (!m_FPGATrackSimMapping->RegionMap_2nd()->isInRegion(region, hit))
	    region_mask[region] = false;
	}
      }
    }
  }

  // For now just give preference to lowest region index for simplicity
  for (int region = 0; region < m_nRegions; region++)
    if (region_mask[region])
      return region;

  return -1;
}


// Given a track and corresponding hits, returns the sector (list of modules) and the accumulation
// struct.
StatusCode FPGATrackSimMatrixGenAlgo::makeAccumulator(std::vector<FPGATrackSimHit> const & sector_hits, FPGATrackSimTruthTrack const & track, std::pair<std::vector<module_t>, FPGATrackSimMatrixAccumulator> & accumulator) const
{
  std::vector<module_t> modules(m_nLayers);
  FPGATrackSimMatrixAccumulator acc(m_nLayers, m_nDim);

  //find the bin!
  // NOTE: this only implements q/pt binning, not the subregion / eta pattern-based constants for now.
  int sectorbin = 0;
  double qoverpt = track.getQ() / track.getPt();
  if (m_absQOverPtBinning) qoverpt = abs(qoverpt);
  for (unsigned bin = 0; bin < m_qOverPtBins.size()-1; bin++) {
    sectorbin = fpgatracksim::QPT_SECTOR_OFFSET * bin;
    if (qoverpt < m_qOverPtBins[bin+1]) break;
  }
  
  // Create sector definitions (list of modules)
  std::string module_printout = "";
  for (int i = 0; i < m_nLayers; i++)
    {
      if (sector_hits[i].getHitType() != HitType::wildcard) {
        if (m_single) modules[i] = sector_hits[i].getIdentifierHash();
        else {
          modules[i] = sectorbin;
          // Modify sectorbin by a "type" field, which for now means: 0 = not spacepoint, 1 = spacepoint.
          // This will fail if we have more than 99 q/pt bins!
          if (sector_hits[i].getHitType() == HitType::spacepoint) {
              modules[i] += fpgatracksim::SPACEPOINT_SECTOR_OFFSET;
          }
          module_printout += std::to_string(modules[i]) + ", ";
        }
      }
      else {
        modules[i] = -1; // WC
      }
    }

  ATH_MSG_DEBUG("Generating track in sectorbin = " << sectorbin << " with modules: " << module_printout);

  
  if (m_single) {
    const int ToKeep[13] = {2200,2564,2861,3831,5368,14169,14173,20442,20446,29625,29629,42176,42180};
    bool keepThis = true;
    for (int i = 0; i < 13; i++) {
      if (modules[i] != ToKeep[i] && modules[i] != -1) {
	keepThis = false;
      }
    }

    if (!keepThis) {
      for (int i = 0; i < 13; i++) modules[i] = -1;
    }
    else {
      for (int i = 0; i < 13; i++) {
      }
    }
  }


  // Hough Constants parameters
  double y = accumulator.second.pars.qOverPt;
  double x = accumulator.second.pars.phi;

  // Vectorize (flatten) coordinates
  std::vector<float> coords;

  for (int i = 0; i < m_nLayers; ++i) {
    if (sector_hits[i].getHitType() != HitType::wildcard) {

      double target_r = m_FPGATrackSimMapping->RegionMap_1st()->getAvgRadius(0, i);

      // If this is a spacepoint the target R should be the average of the two layers.
      // TODO, get this to be loaded in from a mean radii file into the mapping infrastructure.
      if (sector_hits[i].getHitType() == HitType::spacepoint) {
        int other_layer = (sector_hits[i].getSide() == 0) ? i + 1 : i - 1;
        target_r = (target_r + m_FPGATrackSimMapping->RegionMap_1st()->getAvgRadius(0, other_layer)) / 2.;
      }

      std::vector<float> coords_tmp;
      if ( m_ideal_geom > 1 ) {
        coords_tmp = computeIdealCoords(sector_hits[i], x, y, target_r, m_doDeltaPhiConsts, TrackCorrType::Second);
      }
      else {
        coords_tmp = computeIdealCoords(sector_hits[i], x, y, target_r, m_doDeltaPhiConsts, TrackCorrType::None);
      }

      // Create phi for any hits that are not spacepoints, as well as "inner" spacepoints.
      // but not outer spacepoints. this avoids duplicate coordinates.
      if (sector_hits[i].getHitType() != HitType::spacepoint || sector_hits[i].getSide() == 0) {
        // get idealized gPhi
        coords.push_back(coords_tmp[1]);
      }

       // Create a z coordinate for the "outer" layer of the spacepoint and for 2D pixel hits.
      // This means that a spacepoint will write out (phi, eta) pairs, but (0, phi) or (phi, 0) if it's missing.
      if (sector_hits[i].getDim() == 2 || (sector_hits[i].getHitType() == HitType::spacepoint && (sector_hits[i].getPhysLayer() % 2) == 1)) {
        // get idealized z
        coords.push_back(coords_tmp[0]);
      }
    }
    else {
      if (m_pmap->getDim(i) == 2) {
        coords.push_back(0);
      }
      coords.push_back(0);
    }
  }

  assert(coords.size() == (size_t)m_nDim);
  acc.hit_coords = coords;
  acc.hit_coordsG = coords;
  
  // Get the track parameters
  acc.pars = track.getPars();
  if (m_doHoughConstants && m_doDeltaPhiConsts) {
    acc.pars.qOverPt = (y / 1000.0) - track.getQOverPt(); // fit for delta q/pT
    acc.pars.phi = x - track.getPhi(); // fit for delta phi_0
  }
  
  FPGATrackSimTrackParsI bins;
  for (unsigned i = 0; i < FPGATrackSimTrackPars::NPARS; i++)
    bins[i] = (acc.pars[i] - m_sliceMin[i]) * m_nBins[i] / (m_sliceMax[i] - m_sliceMin[i]);
  acc.track_bins.push_back(bins);

  // Force phi to be in [0, 2pi] (post binning)
  if (!m_doDeltaPhiConsts) {
    while (acc.pars.phi < 0) acc.pars.phi += 2*M_PI;
    while (acc.pars.phi > 2*M_PI) acc.pars.phi -= 2*M_PI;
  }

  // Calculate the pre-multiplied elements
  for (int i = 0; i < m_nDim; i++)
    {
      acc.hit_x_QoP[i] = coords[i] * acc.pars.qOverPt;
      acc.hit_xG_HIP[i] = coords[i] * acc.pars.qOverPt;
      acc.hit_x_d0[i]  = coords[i] * acc.pars.d0;
      acc.hit_x_z0[i]  = coords[i] * acc.pars.z0;
      acc.hit_x_eta[i] = coords[i] * acc.pars.eta;
      acc.hit_xG_eta[i] = coords[i] * acc.pars.eta;
      acc.hit_x_phi[i] = coords[i] * acc.pars.phi;

      for (int j = i; j < m_nDim; j++)
	acc.covariance[i * m_nDim + j] = coords[i] * coords[j];

      for (int j = i; j < m_nDim; j++)
	acc.covarianceG[i * m_nDim + j] = coords[i] * coords[j];
    }

  accumulator = {modules, acc};
  return StatusCode::SUCCESS;
}



///////////////////////////////////////////////////////////////////////////////
// Finalize
///////////////////////////////////////////////////////////////////////////////


StatusCode FPGATrackSimMatrixGenAlgo::finalize()
{
  ATH_MSG_DEBUG("finalize()");
  ATH_MSG_INFO("Tracks used: " << m_nTracksUsed << "/" << m_nTracks);

  for (int region = 0; region < m_nRegions; region++) {
      // Create the tree
      std::stringstream name;
      std::stringstream title;
      name << "am" << region;
      title << "Ambank " << region << " parameters";
      TTree* tree = new TTree(name.str().c_str(), title.str().c_str());
      ATH_CHECK(m_tHistSvc->regTree(Form("/TRIGFPGATrackSimMATRIXOUT/%s",tree->GetName()), tree));

      // Fill the tree
      ::fillTree(m_sector_cum[region], tree, m_nLayers, m_nDim);
      // Monitoring
      ATH_MSG_INFO("Sectors found in region " << region << ": " << m_sector_cum[region].size());
      for (auto & sector_info : m_sector_cum[region]) {
	double coverage = sector_info.second.track_bins.size();
	m_h_nHit->Fill(coverage);
	for (unsigned i = 0; i < FPGATrackSimTrackPars::NPARS; i++)
	  m_h_sectorPars[i]->Fill(sector_info.second.pars[i] / coverage);
      }
  }
  
  writeSliceTree();
  ATH_CHECK(m_tHistSvc->finalize());
  return StatusCode::SUCCESS;
}


void FPGATrackSimMatrixGenAlgo::writeSliceTree()
{
  TTree* sliceTree = new TTree("slice", "Region slice boundaries"); // slice

  sliceTree->Branch("c_max",  &m_sliceMax.qOverPt);
  sliceTree->Branch("d0_max",         &m_sliceMax.d0);
  sliceTree->Branch("phi_max",        &m_sliceMax.phi);
  sliceTree->Branch("z0_max",         &m_sliceMax.z0);
  sliceTree->Branch("eta_max",        &m_sliceMax.eta);

  sliceTree->Branch("c_min",  &m_sliceMin.qOverPt);
  sliceTree->Branch("d0_min",         &m_sliceMin.d0);
  sliceTree->Branch("phi_min",        &m_sliceMin.phi);
  sliceTree->Branch("z0_min",         &m_sliceMin.z0);
  sliceTree->Branch("eta_min",        &m_sliceMin.eta);

  sliceTree->Branch("c_slices", &m_nBins.qOverPt);
  sliceTree->Branch("d0_slices",        &m_nBins.d0);
  sliceTree->Branch("phi_slices",       &m_nBins.phi);
  sliceTree->Branch("z0_slices",        &m_nBins.z0);
  sliceTree->Branch("eta_slices",       &m_nBins.eta);

  StatusCode sc = m_tHistSvc->regTree("/TRIGFPGATrackSimMATRIXOUT/slice",sliceTree);
  if (sc.isFailure()) ATH_MSG_ERROR("tHist failed");

  sliceTree->Fill();
}
