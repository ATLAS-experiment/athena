// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration


/**
 * @file FPGATrackSimWindowExtensionTool.cxx
 * @author Ben Rosser - brosser@uchicago.edu
 * @date 2024/10/08
 * @brief Default track extension algorithm to produce "second stage" roads.
 * Much of this code originally written by Alec, ported/adapted to FPGATrackSim.
 */

#include "FPGATrackSimObjects/FPGATrackSimTypes.h"
#include "FPGATrackSimObjects/FPGATrackSimConstants.h"
#include "FPGATrackSimObjects/FPGATrackSimHit.h"
#include "FPGATrackSimBanks/FPGATrackSimSectorBank.h"

#include "FPGATrackSimAlgorithms/FPGATrackSimWindowExtensionTool.h"
#include "FPGATrackSimHough/FPGATrackSimHoughFunctions.h"

#include "FPGATrackSimBinning/IFPGATrackSimBinDesc.h"
#include "FPGATrackSimBinning/FPGATrackSimBinStep.h"
#include "FPGATrackSimBinning/FPGATrackSimBinUtil.h"

#include <sstream>
#include <cmath>
#include <algorithm>


StatusCode FPGATrackSimWindowExtensionTool::initialize() {

    // Retrieve the mapping service.
    ATH_CHECK(m_FPGATrackSimMapping.retrieve());
    if (m_idealGeoRoads) ATH_CHECK(m_FPGATrackSimBankSvc.retrieve());
    m_nLayers_1stStage = m_FPGATrackSimMapping->PlaneMap_1st(0)->getNLogiLayers();
    m_nLayers_2ndStage = m_FPGATrackSimMapping->PlaneMap_2nd(0)->getNLogiLayers() - m_nLayers_1stStage;

    m_threshold = (m_nLayers_1stStage + m_nLayers_2ndStage) - m_maxMiss;

    // This now needs to be done once for each slice.
    for (size_t j=0; j<m_FPGATrackSimMapping->GetPlaneMap_2ndSliceSize(); j++){
        ATH_MSG_DEBUG("Processing second stage slice " << j);
        m_phits_atLayer[j] = std::map<unsigned, std::vector<std::shared_ptr<const FPGATrackSimHit>>>();
        for (unsigned i = m_nLayers_1stStage; i < m_nLayers_2ndStage +m_nLayers_1stStage ; i++) {
            ATH_MSG_DEBUG("Processing layer " << i);
            m_phits_atLayer[j][i] = std::vector<std::shared_ptr<const FPGATrackSimHit>>();
        }
    }

    // Retrieve the hit binning tool, for use with inside out.
    ATH_CHECK(m_hitBinningTool.retrieve());
    // Setup layer configuration if not already set from layerMap
    // TODO: this may not be correct because of spacepoints. Do we *really* want to bin using 8 layers?
    if (m_hitBinningTool->getNLayers()==0) {
        m_hitBinningTool->setNLayers(m_nLayers_2ndStage);
    }

    // Probably need to do something here.
    return StatusCode::SUCCESS;
}

StatusCode FPGATrackSimWindowExtensionTool::extendTracks(const std::vector<std::shared_ptr<const FPGATrackSimHit>> & hits,
        const std::vector<std::shared_ptr<const FPGATrackSimTrack>> & tracks,
        std::vector<std::shared_ptr<const FPGATrackSimRoad>> & roads) {

    // Reset the internal second stage roads storage.
    roads.clear();
    m_roads.clear();
    for (auto& sliceEntry : m_phits_atLayer){
        for (auto& entry : sliceEntry.second) {
            entry.second.clear();
        }
    }
    const FPGATrackSimRegionMap* rmap_2nd = m_FPGATrackSimMapping->SubRegionMap_2nd();
    const FPGATrackSimPlaneMap *pmap_2nd = nullptr;

    // Create one "tower" per slice for this event.
    // Note that there now might be only one "slice", at least for the time being.
    if (m_slicedHitHeader) {
        for (int ireg = 0; ireg < rmap_2nd->getNRegions(); ireg++) {
            FPGATrackSimTowerInputHeader tower = FPGATrackSimTowerInputHeader(ireg);
            m_slicedHitHeader->addTower(tower);
        }
    }

    // Second stage hits may be unmapped, in which case map them.
    for (size_t i=0; i<m_FPGATrackSimMapping->GetPlaneMap_2ndSliceSize(); i++){
        pmap_2nd = m_FPGATrackSimMapping->PlaneMap_2nd(i);
        for (const std::shared_ptr<const FPGATrackSimHit>& hit : hits) {
            std::shared_ptr<FPGATrackSimHit> hitCopy = std::make_shared<FPGATrackSimHit>(*hit);
            pmap_2nd->map(*hitCopy);
            if (!hitCopy->isMapped()){
                continue;
            }
            if (rmap_2nd->isInRegion(i, *hitCopy)) {
                m_phits_atLayer[i][hitCopy->getLayer()].push_back(hitCopy);
                // Also store a copy of the hit object in the header class, for ROOT Output + TV creation.
                if (m_slicedHitHeader) m_slicedHitHeader->getTower(i)->addHit(*hitCopy);
            }
        }
    }

    // Unfortunately this is a bit different from the other code path
    // If we are using the binning tool here, we need to pass all of the hits to it, and then rely
    // on the binning tool to match first stage tracks to second stage bins.
    std::vector<std::shared_ptr<const FPGATrackSimHit>> allMappedHits;
    if (m_doBinning && m_FPGATrackSimMapping->GetPlaneMap_2ndSliceSize() == 1) {
        for (unsigned layer = 0; layer < m_nLayers_2ndStage + m_nLayers_1stStage; layer++) {
            allMappedHits.insert(allMappedHits.end(), m_phits_atLayer[0][layer].begin(), m_phits_atLayer[0][layer].end());
        }
        ATH_MSG_VERBOSE("Attempting to bin nhits = " << allMappedHits.size());
        ATH_CHECK(m_hitBinningTool->fill(allMappedHits));
    }

    // Now, loop over the tracks.
    for (std::shared_ptr<const FPGATrackSimTrack> track : tracks) {
        if (track->passedOR() == 0) {
            continue;
        }

        // Retrieve track parameters.
        double trackphi = track->getPhi();
        double trackqoverpt = track->getQOverPt();

        std::vector<int> numHits(m_nLayers_2ndStage + m_nLayers_1stStage, 0);

        // Copy over the existing hits. We require that the layer assignment in the first stage
        // is equal to the layer assignment in the second stage.
        std::vector<std::vector<std::shared_ptr<const FPGATrackSimHit>>> road_hits;
        road_hits.resize(m_nLayers_1stStage + m_nLayers_2ndStage);
        layer_bitmask_t hitLayers = 0;
        unsigned nhit = 0;
        // We can't just use the the iterator since hit.getLayer() isn't guaranteed to be right.
        for (unsigned layer = 0; layer < track->getFPGATrackSimHits().size(); layer++) {
            const FPGATrackSimHit& hit = track->getFPGATrackSimHits().at(layer);
            road_hits[layer].push_back(std::make_shared<FPGATrackSimHit>(hit));
            if (hit.isReal()) {
                hitLayers |= 1 << layer;
                numHits[layer]++;
            }
        }

        size_t slice = track->getSubRegion();
        pmap_2nd = m_FPGATrackSimMapping->PlaneMap_2nd(slice);

        // Extend the track using either slicing (F-200/300 legacy support) or binning (F-600)
        bool success = (!m_doBinning) ? extendTrackSliced(track, numHits, hitLayers, road_hits) : extendTrackBinned(track, numHits, hitLayers, road_hits);
        if (!success) continue;

        // now nhit will be equal to the number of layers with hits in the new array.
        for (auto num: numHits) {
            if(num > 0) nhit += 1;
        }

        // If we have enough hits, create a new road.
        ATH_MSG_DEBUG("Found potential new road with " << nhit << " hits relative to threshold of " << m_threshold);
        if (nhit >= m_threshold) {
            m_roads.emplace_back();
            FPGATrackSimRoad & road = m_roads.back();
            road.setRoadID(roads.size() - 1);

            // Set the "Hough x" and "Hough y" using the track parameters.
            road.setX(trackphi);
            road.setY(trackqoverpt);
            road.setXBin(track->getHoughXBin());
            road.setYBin(track->getHoughYBin());
            road.setSubRegion(track->getSubRegion());

            // figure out bit mask for wild card layers
            unsigned int wclayers = 0;
            for (unsigned i = 0; i < numHits.size(); i++){
                if(numHits[i]==0) wclayers |= (0x1 << i);
            }
            road.setWCLayers(wclayers);

            // set hit layers and hits
            road.setHitLayers(hitLayers);
            road.setHits(std::move(road_hits));
        }
    }

    // Copy the roads we found into the output argument and return success.
    roads.reserve(m_roads.size());
    for (FPGATrackSimRoad & r : m_roads) {
        roads.emplace_back(std::make_shared<const FPGATrackSimRoad>(r));
    }
    ATH_MSG_DEBUG("Found " << m_roads.size() << " new roads in second stage.");

    // Reset the hit binning tool.
    m_hitBinningTool->resetBins();

    return StatusCode::SUCCESS;
}

bool FPGATrackSimWindowExtensionTool::extendTrackSliced(std::shared_ptr<const FPGATrackSimTrack> track, std::vector<int>& numHits, layer_bitmask_t& hitLayers,
                                                        std::vector<std::vector<std::shared_ptr<const FPGATrackSimHit>>>& road_hits) {

    const FPGATrackSimRegionMap* rmap_2nd = m_FPGATrackSimMapping->SubRegionMap_2nd();

    // Retrieve track parameters.
    double trackd0 = track->getD0();
    double trackphi = track->getPhi();
    double trackz0 = track->getZ0();
    double tracketa = track->getEta();
    double trackqoverpt = track->getQOverPt();
    double cottracktheta = 0.5*(exp(tracketa)-exp(-tracketa));
    size_t slice = track->getSubRegion();

    for (unsigned layer = m_nLayers_1stStage; layer < m_nLayers_2ndStage + m_nLayers_1stStage; layer++) {
        ATH_MSG_DEBUG("Testing layer " << layer << " with " << m_phits_atLayer[slice][layer].size() << " hit");
        for (const std::shared_ptr<const FPGATrackSimHit>& hit: m_phits_atLayer[slice][layer]) {
            // Make sure this hit is in the same subregion as the track. TODO: mapping/slice changes.
            if (!rmap_2nd->isInRegion(track->getSubRegion(), *hit)) {
                continue;
            }

            double hitphi = hit->getGPhi();
            double hitr = hit->getR();
            double hitz = hit->getZ();
            double pred_hitphi = trackphi - asin(hitr * fpgatracksim::A * 1000 * trackqoverpt - trackd0/hitr);
            double pred_hitz = trackz0 + hitr*cottracktheta;

            // Field correction, now pulled from FPGATrackSimFunctions.
            if (m_fieldCorrection){
                double fieldCor = fieldCorrection(track->getRegion(), trackqoverpt, hitr);
                pred_hitphi += fieldCor;
            }

            double diffphi = abs(hitphi-pred_hitphi);
            double diffz = abs(hitz-pred_hitz);

            // Apply the actual layer check, only accept hits that fall into a track's window.
            ATH_MSG_DEBUG("Hit in region, comparing phi: " << diffphi << " to " << m_windows[layer] << " and z " << diffz << " to " << m_zwindows[layer]);
            if (diffphi < m_windows[layer] && diffz < m_zwindows[layer]) {
                numHits[layer]++;
                road_hits[layer].push_back(hit);
                hitLayers |= 1 << hit->getLayer();
            }
        }
    }

    return true;
}


bool FPGATrackSimWindowExtensionTool::extendTrackBinned(std::shared_ptr<const FPGATrackSimTrack> track, std::vector<int>& numHits, layer_bitmask_t& hitLayers,
                                                        std::vector<std::vector<std::shared_ptr<const FPGATrackSimHit>>>& road_hits) {

    // Retrieve track parameters.
    double trackd0 = track->getD0();
    double trackphi = track->getPhi();
    double trackz0 = track->getZ0();
    double tracketa = track->getEta();
    double trackqoverpt = track->getQOverPt();
    double cottracktheta = 0.5*(exp(tracketa)-exp(-tracketa));

    // If we're doing binning, match the track to a bin...
    const FPGATrackSimBinStep* binStep = m_hitBinningTool->getBinTool().lastStep();
    const IFPGATrackSimBinDesc* binDesc = m_hitBinningTool->getBinTool().binDesc();
    ATH_MSG_DEBUG("Attempting to look up binIdx for track with phi = " << track->getPhi() << ", chi2/DOF = " << track->getChi2ndof() << ", q/pt = " << track->getQOverPt() << ", eta = " << track->getEta() << ", d0 = " << track->getD0() << ", z0 = " << track->getZ0());
    FPGATrackSimTrackPars trackPars = track->getPars();

    // MeV/GeV conversion factor, needed when switching between track parametrizations.
    trackPars[FPGATrackSimTrackPars::IHIP] = trackPars[FPGATrackSimTrackPars::IHIP] * 1000;

    FPGATrackSimBinUtil::ParSet parSet = binDesc->trackParsToParSet(trackPars);
    if (!m_hitBinningTool->getBinTool().inRange(parSet)) {
        ATH_MSG_DEBUG("Track doesn't pass binning tool track parameter cuts");
        return false;
    }
    ATH_MSG_DEBUG("Found inside out parameters as " << parSet);
    FPGATrackSimBinUtil::IdxSet binPars = binStep->binIdx(parSet);
    ATH_MSG_DEBUG("Attempting to look up bin entry using binpars = " << binPars);
    FPGATrackSimBinnedHits::BinEntry entry = (m_hitBinningTool->lastStepBinnedHits())[binPars];

    ATH_MSG_DEBUG("Matched track to new bin containing " << entry.lyrCnt() << " layers with " << entry.hitCnt << " hits total");

    for (FPGATrackSimBinUtil::StoredHit& storedHit : entry.hits) {
        // These may need more adjustment if we try to bin deduplicated SPs in the future.
        unsigned layer = storedHit.layer + m_nLayers_1stStage;
        const std::shared_ptr<const FPGATrackSimHit>& hit = storedHit.hitptr;

        // The rest of this should be the same as the previous loop.
        double hitphi = hit->getGPhi();
        double hitr = hit->getR();
        double hitz = hit->getZ();
        double pred_hitphi = trackphi - asin(hitr * fpgatracksim::A * 1000 * trackqoverpt - trackd0/hitr);
        double pred_hitz = trackz0 + hitr*cottracktheta;

        // Field correction, now pulled from FPGATrackSimFunctions.
        if (m_fieldCorrection){
            double fieldCor = fieldCorrection(track->getRegion(), trackqoverpt, hitr);
            pred_hitphi += fieldCor;
        }

        double diffphi = abs(hitphi-pred_hitphi);
        double diffz = abs(hitz-pred_hitz);

        // Apply the actual layer check, only accept hits that fall into a track's window.
        ATH_MSG_DEBUG("Hit in region, comparing phi: " << diffphi << " to " << m_windows[layer] << " and z " << diffz << " to " << m_zwindows[layer]);
        if (diffphi < m_windows[layer] && diffz < m_zwindows[layer]) {
            numHits[layer]++;
            road_hits[layer].push_back(hit);
            hitLayers |= 1 << layer;
        }
    }

    return true;

}
