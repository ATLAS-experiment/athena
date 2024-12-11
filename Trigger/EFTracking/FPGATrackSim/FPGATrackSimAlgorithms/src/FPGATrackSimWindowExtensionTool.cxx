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

#include "FPGATrackSimWindowExtensionTool.h"

#include <sstream>
#include <cmath>
#include <algorithm>

FPGATrackSimWindowExtensionTool::FPGATrackSimWindowExtensionTool(const std::string& algname, const std::string &name, const IInterface *ifc) :
  base_class(algname, name, ifc) {
  declareInterface<IFPGATrackSimTrackExtensionTool>(this);
}


StatusCode FPGATrackSimWindowExtensionTool::initialize() {

    // Retrieve the mapping service.
    ATH_CHECK(m_FPGATrackSimMapping.retrieve());
    if (m_idealGeoRoads) ATH_CHECK(m_FPGATrackSimBankSvc.retrieve());
    m_nLayers_1stStage = m_FPGATrackSimMapping->PlaneMap_1st(0)->getNLogiLayers();
    m_nLayers_2ndStage = m_FPGATrackSimMapping->PlaneMap_2nd(0)->getNLogiLayers() - m_nLayers_1stStage;

    // This now needs to be done once for each slice.
    for (size_t j=0; j<m_FPGATrackSimMapping->GetPlaneMap_2ndSliceSize(); j++){
        ATH_MSG_INFO("Processing second stage slice " << j);
        m_phits_atLayer[j] = std::map<unsigned, std::vector<std::shared_ptr<const FPGATrackSimHit>>>();
        for (unsigned i = m_nLayers_1stStage; i < m_nLayers_2ndStage +m_nLayers_1stStage ; i++) {
            ATH_MSG_INFO("Processing layer " << i);
            m_phits_atLayer[j][i] = std::vector<std::shared_ptr<const FPGATrackSimHit>>();
        }
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
            }
        }
    }
    // Now, loop over the tracks.
    for (std::shared_ptr<const FPGATrackSimTrack> track : tracks) {
        if (track->passedOR() == 0) {
            continue;
        }

        // Retrieve track parameters.
        double trackd0 = track->getD0();
        double trackphi = track->getPhi();
        double trackz0 = track->getZ0();
        double tracketa = track->getEta();
        double cottracktheta = 0.5*(exp(tracketa)-exp(-tracketa));
        double trackqoverpt = track->getQOverPt();

        // Copy over the existing hits. We require that the layer assignment in the first stage
        // is equal to the layer assignment in the second stage.
        std::vector<std::vector<std::shared_ptr<const FPGATrackSimHit>>> road_hits;
        road_hits.resize(m_nLayers_1stStage + m_nLayers_2ndStage);
        layer_bitmask_t hitLayers = 0;
        int nhit = 0;
        for (auto hit : track->getFPGATrackSimHits()) {
            road_hits[hit.getLayer()].push_back(std::make_shared<FPGATrackSimHit>(hit));
            if (hit.isReal()) {
                nhit += 1;
                hitLayers |= 1 << hit.getLayer();
            }
        }

        // At this point we should have copied over all the hits.
        std::vector<int> numHits(m_nLayers_2ndStage + m_nLayers_1stStage, 0);

        size_t slice = track->getSubRegion();
        pmap_2nd = m_FPGATrackSimMapping->PlaneMap_2nd(slice);
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

                double diff = abs(hitphi-pred_hitphi);
                double diffz = abs(hitz-pred_hitz);

                // Apply the actual layer check, only accept hits that fall into a track's window.
                ATH_MSG_DEBUG("Hit in region, comparing phi: " << diff << " to " << m_windows[layer] << " and z " << diffz << " to " << m_zwindows[layer]);
                if (diff < m_windows[layer] && diffz < m_zwindows[layer]) {
                    numHits[layer]++;
                    road_hits[layer].push_back(hit);
                    hitLayers |= 1 << hit->getLayer();
                }
            }
        }

        // now nhit will be equal to the number of layers with hits in the new array.
        for (auto num: numHits) {
            if(num > 0) nhit += 1;
        }

        // If we have enough hits, create a new road.
        if (nhit >= m_threshold) {
            ATH_MSG_DEBUG("Found new road with " << nhit << " hits relative to " << m_threshold);
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

    return StatusCode::SUCCESS;
}
