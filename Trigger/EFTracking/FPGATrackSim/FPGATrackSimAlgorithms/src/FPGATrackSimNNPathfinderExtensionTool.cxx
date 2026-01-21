// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration


/**
 * @file FPGATrackSimNNPathfinderExtensionTool.cxx
 * @author Ben Rosser - brosser@uchicago.edu
 * @date 2024/10/08
 * @brief Default track extension algorithm to produce "second stage" roads.
 * Much of this code originally written by Alec, ported/adapted to FPGATrackSim.
 */
#include "FPGATrackSimAlgorithms/FPGATrackSimNNPathfinderExtensionTool.h"
#include "FPGATrackSimBanks/FPGATrackSimSectorBank.h"
#include "FPGATrackSimHough/FPGATrackSimHoughFunctions.h"

#include "AthenaKernel/Chrono.h"

#include <cmath>
#include <algorithm>
#include <deque>
#include <unordered_map>
#include "CLHEP/Units/SystemOfUnits.h"

using CLHEP::pi;

namespace {
    // Helper to wrap a raw pointer as a non-owning shared_ptr<const FPGATrackSimHit>
    inline std::shared_ptr<const FPGATrackSimHit> makeNonOwningHitPtr(const FPGATrackSimHit* hit) {
        return std::shared_ptr<const FPGATrackSimHit>(hit, [](const FPGATrackSimHit*){});
    }

    // Helper to create spatial hash key from coordinates for strip matching
    inline long makeCoordinatesKey(float x, float y, float z, float gridSize = 1.0f) {
        int ix = static_cast<int>(std::floor(x / gridSize));
        int iy = static_cast<int>(std::floor(y / gridSize));
        int iz = static_cast<int>(std::floor(z / gridSize));
        return (static_cast<long>(ix + 10000) << 40) |
                     (static_cast<long>(iy + 10000) << 20) |
                     static_cast<long>(iz + 10000);
    }

    // Comparator for Cartesian sorting
    template<typename HitVec>
    bool cartesianComparator(const HitVec& a, const HitVec& b, double predr, double predz) {
        const auto& hitA = *(a[0]);
        const auto& hitB = *(b[0]);
        double hitz_a = hitA.getZ();
        double hitr_a = hitA.getR();
        double hitz_b = hitB.getZ();
        double hitr_b = hitB.getR();
        float distance_a = (hitr_a - predr)*(hitr_a - predr) + (hitz_a - predz)*(hitz_a - predz);
        float distance_b = (hitr_b - predr)*(hitr_b - predr) + (hitz_b - predz)*(hitz_b - predz);
        return distance_a < distance_b;
    }

    // Comparator for polar sorting
    template<typename HitVec>
    bool polarComparator(const HitVec& a, const HitVec& b, double predr, double predphi, double predz, double zScale2, double phiScale2, double rScale2) {
        const auto& hitA = *(a[0]);
        const auto& hitB = *(b[0]);
        
        double hitr_a = hitA.getR();
        double hitphi_a = hitA.getGPhi();
        double hitz_a = hitA.getZ();
        double dz_a = abs(hitz_a - predz);
        double dr_a = abs(hitr_a - predr);
        double dphi_a = abs(hitphi_a - predphi);
        while (dphi_a > pi) dphi_a -= pi;
        // scaled distance because z and phi and r are not in same units
        float distance_a = dz_a*dz_a/zScale2 + dphi_a*dphi_a/phiScale2 + dr_a*dr_a/rScale2;

        double hitr_b = hitB.getR();
        double hitphi_b = hitB.getGPhi();
        double hitz_b = hitB.getZ();
        double dz_b = abs(hitz_b - predz);
        double dr_b = abs(hitr_b - predr);
        double dphi_b = abs(hitphi_b - predphi);
        while (dphi_b > pi) dphi_b -= pi;
        // scaled distance because z and phi and r are not in same units
        float distance_b = dz_b*dz_b/zScale2 + dphi_b*dphi_b/phiScale2 + dr_b*dr_b/rScale2;

        return distance_a < distance_b;
    }

    // Spatial index for fast hit lookup
    struct HitSpatialIndex {
        std::unordered_map<long, std::vector<const FPGATrackSimHit*>> fineIDToHits;
        std::unordered_map<long, std::vector<const FPGATrackSimHit*>> coordToHits;

        void build(const std::vector<std::shared_ptr<const FPGATrackSimHit>>& hits) {
            fineIDToHits.clear();
            coordToHits.clear();
            fineIDToHits.reserve(50000);
            coordToHits.reserve(hits.size());

            for (const auto& hitPtr : hits) {
                if (!hitPtr->isReal()) continue;
                
                // For fineID index we can just skip second half of strip SPs since they get added via the first half
                if (!(hitPtr->getHitType() == HitType::spacepoint && (hitPtr->getPhysLayer(true) % 2 == 1))) {
                    long fineID = getFineID(*hitPtr);
                    fineIDToHits[fineID].push_back(hitPtr.get());
                }

                // For coordinate index we include all hits since we need to find strip SP pairs
                long coordKey = makeCoordinatesKey(hitPtr->getX(), hitPtr->getY(), hitPtr->getZ());
                coordToHits[coordKey].push_back(hitPtr.get());
            }
        }

        const std::vector<const FPGATrackSimHit*>* getHits(long fineID) const {
            auto it = fineIDToHits.find(fineID);
            return (it != fineIDToHits.end()) ? &(it->second) : nullptr;
        }

        const std::vector<const FPGATrackSimHit*>* getHitsByCoord(float x, float y, float z) const {
            long coordKey = makeCoordinatesKey(x, y, z);
            auto it = coordToHits.find(coordKey);
            return (it != coordToHits.end()) ? &(it->second) : nullptr;
        }
    };
}

StatusCode FPGATrackSimNNPathfinderExtensionTool::initialize() {

    // Retrieve the mapping service.
    ATH_CHECK(m_FPGATrackSimMapping.retrieve());

    // hard code this for now but we may chance in the future
    m_nLayers_1stStage = 5;
    m_nLayers_2ndStage = 8;

    if (m_windowR.size() != 1 && m_windowR.size() != m_windowFineID.size()) {
      ATH_MSG_ERROR("Window r size = " << m_windowR << " is not equal to 1 (for all layers) and not equal to " << m_windowFineID.size());
      return StatusCode::FAILURE;
    }

    if (m_windowPhi.size() != 1 && m_windowPhi.size() != m_windowFineID.size()) {
      ATH_MSG_ERROR("Window phi size = " << m_windowPhi << " is not equal to 1 (for all layers) and not equal to " << m_windowFineID.size());
      return StatusCode::FAILURE;
    }    

    if (m_windowZ.size() != 1 && m_windowZ.size() != m_windowFineID.size()) {
      ATH_MSG_ERROR("Window z size = " << m_windowZ << " is not equal to 1 (for all layers) and not equal to " << m_windowFineID.size());
      return StatusCode::FAILURE;
    }

    if (m_FPGATrackSimMapping->getExtensionNNVolMapString() != "" && m_FPGATrackSimMapping->getExtensionNNHitMapString() != "") {
        ATH_MSG_INFO("Initializing extension hit NN with string = " << m_FPGATrackSimMapping->getExtensionNNHitMapString());
        m_extensionHitNN.initialize(m_FPGATrackSimMapping->getExtensionNNHitMapString());
        ATH_MSG_INFO("Initializing volume NN with string = " << m_FPGATrackSimMapping->getExtensionNNVolMapString());
        m_extensionVolNN.initialize(m_FPGATrackSimMapping->getExtensionNNVolMapString());
    }
    else {
        ATH_MSG_ERROR("Path to NN-based track extension ONNX file is empty! If you want to run this pipeline, you need to provide an input file.");
        return StatusCode::FAILURE;
    }

    ATH_CHECK(m_tHistSvc.retrieve());
    ATH_CHECK(m_chronoSvc.retrieve());

    return StatusCode::SUCCESS;
}


StatusCode FPGATrackSimNNPathfinderExtensionTool::extendTracks(const std::vector<std::shared_ptr<const FPGATrackSimHit>> & hits,
        const std::vector<std::shared_ptr<const FPGATrackSimTrack>> & tracks,
        std::vector<FPGATrackSimRoad> & roads) {

    // Reset the internal second stage roads storage.
    roads.clear();
    m_roads.clear();
    const FPGATrackSimRegionMap* rmap_2nd = m_FPGATrackSimMapping->SubRegionMap_2nd();

    // Create one "tower" per slice for this event.
    // Note that there now might be only one "slice", at least for the time being.
    if (m_slicedHitHeader) {
      for (int ireg = 0; ireg < rmap_2nd->getNRegions(); ireg++) {
        FPGATrackSimTowerInputHeader tower = FPGATrackSimTowerInputHeader(ireg);
        m_slicedHitHeader->addTower(tower);
      }
    }
    
    // Build spatial index (maps) once per event
    HitSpatialIndex hitIndex;
    {
        Athena::Chrono chronoBuildIndex("NNPathfinder:BuildSpatialIndex", m_chronoSvc.get());
        hitIndex.build(hits);
    }
    
    if(m_debugEvent) ATH_MSG_DEBUG("Got: "<<tracks.size()<<" tracks to extrapolate");
    // Now, loop over the tracks.
    for (const auto& track : tracks) {
        Athena::Chrono chronoTrackLoop("NNPathfinder:TrackLoop", m_chronoSvc.get());
        if(m_debugEvent) ATH_MSG_DEBUG("\033[1;31m-------------------------- extraploating Track ------------------ \033[0m");
        if (track->passedOR() == 0) {
            continue;
        }
        const std::vector<FPGATrackSimHit> hitsOnTrack = track->getFPGATrackSimHits();
        miniRoad road;
        float pt = track->getPt();

        for (const auto &thit : hitsOnTrack) {
            road.addHit(std::make_shared<const FPGATrackSimHit>(thit)); // add all hits, we check if WC later
        }

        if (m_debugEvent) {
            ATH_MSG_DEBUG("-----------------Hits in event");
            for (const auto& hit : hits) {
                ATH_MSG_DEBUG("Hit " << " X: " << hit->getX() << " Y: " << hit->getY() << " Z: " << hit->getZ() << " R: " << hit->getR() << "phi = " << hit->getGPhi() << " hitType: " << hit->getHitType() << " getDetType: " << hit->getDetType());
            }
        }
        // Using deque instead of vector for O(1) pop_front since we need operate only the first road each iteration
        std::deque<miniRoad> roadsToExtrapolate;
        roadsToExtrapolate.push_back(std::move(road));

        std::vector<miniRoad> completedRoads;

        int count = 0;
	// FIXED: Add maximum iteration limit to prevent infinite loops
        const int MAX_ROADS = 10000;
	while(!roadsToExtrapolate.empty() && count < MAX_ROADS ) {
	  miniRoad currentRoad = std::move(roadsToExtrapolate.front());

            // Erase this road from the deque
            roadsToExtrapolate.pop_front();
            count ++;
            if(m_debugEvent) {
                ATH_MSG_DEBUG("\033[1;31m-------------------------- extraploating road "<< count << "------------------ \033[0m");
                printRoad(currentRoad);
            }
            // Check exit condition
            if (currentRoad.getNHits() >= (m_nLayers_1stStage+m_nLayers_2ndStage))
            {
                completedRoads.push_back(std::move(currentRoad));
                continue; // this one is done
            }
            // Other try to find the next hit in this road
            std::vector<float> inputTensorValues;
            std::vector<float> predhit;
            long fineID;
            {
                Athena::Chrono chronoNN("NNPathfinder:NNInference", m_chronoSvc.get());
                if (!fillInputTensorForNN(currentRoad, inputTensorValues)) {
                    ATH_MSG_WARNING("Failed to create input tensor for this road");
                    continue;
                }
                if (!getPredictedHit(inputTensorValues, predhit, fineID)) {
                    ATH_MSG_WARNING("Failed to predict hit for this road");
                    continue;
                }
            }
            // Check if exist conditions are there
            if (m_doOutsideIn) {
                // Make sure we are not predicting inside the inner most layer (x and y < 25, or r < 25)
                // If we are, road is done
	      if ((m_useCartesian && (abs(predhit[0]) < 25 && abs(predhit[1]) < 25)) ||
		  (!m_useCartesian && abs(predhit[0]) < 25))
		{
                    completedRoads.push_back(std::move(currentRoad));
                    continue;
                }
            }
            else {
                // Make sure we are not predicting outside the outer most layer	      
                // if we are, road is done
                double radius = std::sqrt(predhit[0] * predhit[0] + predhit[1] * predhit[1]);
                if ((m_useCartesian && (abs(predhit[0]) > 1024 || abs(predhit[1]) > 1024 || radius > 1024 || abs(predhit[2]) > 3000)) ||
		    (!m_useCartesian && (abs(predhit[0]) > 1024 || abs(predhit[2]) > 3000))) {
                    completedRoads.push_back(std::move(currentRoad));
                    continue;
                }
            }
            if(m_debugEvent) {
                ATH_MSG_DEBUG("Predicted hit at: " << predhit[0] << " " << predhit[1] << " " << predhit[2]);
            }

            // Now search for the hits
            bool foundhitForRoad = false;
            if(fineID == 215){
                ATH_MSG_DEBUG("Stopping condition reached");
                completedRoads.push_back(std::move(currentRoad));
                continue;
            }
            // Get the last layer and hit in the road
            unsigned lastLayerInRoad = 0;
            std::shared_ptr<const FPGATrackSimHit> lastHit;
            if(!getLastLayer(currentRoad, lastLayerInRoad, lastHit) or !lastHit) {
                ATH_MSG_WARNING("Failed to find last layer this road");
                continue;
            }
            unsigned layer = lastLayerInRoad+1; // the layer we're looking to find
            bool lastHitWasReal = lastHit->isReal();
            float lastHitR = lastHit->getR();
            if(layer >= (m_nLayers_1stStage + m_nLayers_2ndStage)) {
                completedRoads.push_back(std::move(currentRoad));
                continue;
            }
            unsigned int hitsInWindow = 0;

            // Cache predicted values and window parameters once per road, not per hit
            const double predr = (m_useCartesian ? sqrt(predhit[0] * predhit[0] + predhit[1] * predhit[1]) : predhit[0]);
            const double predphi = predhit[1];
            const double predz = predhit[2];
            
            // Calculate window parameters once
            double windowR = m_windowR[0];
            double windowPhi = m_windowPhi[0];
            double windowZ = m_windowZ[0];
            int fineID_index = 0;
            
            if (m_windowZ.size() > 1 || m_windowR.size() > 1 || m_windowPhi.size() > 1) {
                auto fineID_it = std::find(m_windowFineID.begin(), m_windowFineID.end(), fineID);
                if (fineID_it == m_windowFineID.end()){
                    ATH_MSG_DEBUG("No windows for predicted fineID " << fineID << ", using maximum in provided list instead!");
                    fineID_index = -1;
                }
                else {
                    fineID_index = fineID_it - m_windowFineID.begin();
                }
            }
            if (m_windowR.size() > 1) {
                windowR = (fineID_index == -1) ? 
                    *std::max_element(m_windowR.begin(), m_windowR.end()) : 
                    m_windowR[fineID_index];
            }
            if (m_windowZ.size() > 1) {
                windowZ = (fineID_index == -1) ? 
                    *std::max_element(m_windowZ.begin(), m_windowZ.end()) : 
                    m_windowZ[fineID_index];
            }
            if (m_windowPhi.size() > 1) {
                windowPhi = (fineID_index == -1) ? 
                    *std::max_element(m_windowPhi.begin(), m_windowPhi.end()) : 
                    m_windowPhi[fineID_index];
            }
            
            // Apply scaling factors
            if (m_missedHitRScaling > 0 && !lastHitWasReal) windowR *= m_missedHitRScaling;
            if (m_missedHitZScaling > 0 && !lastHitWasReal) windowZ *= m_missedHitZScaling;
            if (m_missedHitPhiScaling > 0 && !lastHitWasReal) windowPhi *= m_missedHitPhiScaling;
            if (pt < m_lowPtValueForWindowRScaling.value()) windowR *= m_lowPtWindowRScaling.value();
            if (pt < m_lowPtValueForWindowZScaling.value()) windowZ *= m_lowPtWindowZScaling.value();
            if (pt < m_lowPtValueForWindowPhiScaling.value()) windowPhi *= m_lowPtWindowPhiScaling.value();

            // List of all the hits, with their distances to the predicted point
            std::vector<std::vector<std::shared_ptr<const FPGATrackSimHit>>> listofHitsFound;

            {
            Athena::Chrono chronoHitSearch("NNPathfinder:HitSearchLoop", m_chronoSvc.get());
            
            // Use spatial index (HitSpatialIndex) to get only candidate hits with matching fineID
            const auto* candidateHits = hitIndex.getHits(fineID);
            if (candidateHits) {
                listofHitsFound.reserve(candidateHits->size());

                for (const FPGATrackSimHit* hit : *candidateHits) {
                    // Apply direction filter
                    const float hitr = hit->getR();
                    if (m_doOutsideIn && hitr > lastHitR) continue;
                    if (!m_doOutsideIn && hitr < lastHitR) continue;

                    if(m_debugEvent) {
                        ATH_MSG_DEBUG("In the hit loop hit at x: " << hit->getX() << " y " << hit->getY() << " z " << hit->getZ() << " phi " << hit->getGPhi());
                    }

                    // Check if hit is within window
                    const double hitz = hit->getZ();
                    const double hitphi = hit->getGPhi();

                    const double dr = abs(hitr - predr);
                    const double dz = abs(hitz - predz);
                    double dphi = abs(hitphi - predphi);
                    while (dphi > pi) dphi -= pi;

                    const bool inWindow = (m_useCartesian && dr < windowR && dz < windowZ) ||
                                          (!m_useCartesian && dphi < windowPhi && dz < windowZ && dr < windowR);

                    if (!inWindow) continue;

                    // Only create shared_ptr when hit passes all filters
                    std::vector<std::shared_ptr<const FPGATrackSimHit>> theseHits{ makeNonOwningHitPtr(hit) };
                    hitsInWindow = hitsInWindow + 1;

                    // Handle strip space points
                    if(hit->isStrip()) {
                        if (hit->getHitType() == HitType::spacepoint) {
                            // Use spatial index for strip matching
                            const float EPSILON = 0.00001f;
                            const float searchX = hit->getX();
                            const float searchY = hit->getY();
                            const float searchZ = hit->getZ();
                            const auto searchHash = hit->getIdentifierHash();
                            bool found = false;

                            const auto* coordCandidates = hitIndex.getHitsByCoord(searchX, searchY, searchZ);
                            if (coordCandidates) {
                                for (const FPGATrackSimHit* candidateHit : *coordCandidates) {
                                    if (candidateHit->getIdentifierHash() != searchHash &&
                                        abs(candidateHit->getX() - searchX) < EPSILON &&
                                        abs(candidateHit->getY() - searchY) < EPSILON &&
                                        abs(candidateHit->getZ() - searchZ) < EPSILON) {
                                        theseHits.push_back(makeNonOwningHitPtr(candidateHit));
                                        found = true;
                                        break;
                                    }
                                }
                            }
                            if (!found) {
                                ATH_MSG_WARNING("For a SP in layer " << layer << " Couldn't find a matching strip SP");
                            }
                        }
                        else {
                            std::shared_ptr<FPGATrackSimHit> guessedSecondHitPtr = std::make_shared<FPGATrackSimHit>();
                            guessedSecondHitPtr->setX(0);
                            guessedSecondHitPtr->setY(0);
                            guessedSecondHitPtr->setZ(0);
                            guessedSecondHitPtr->setPhysLayer(lastHit->getPhysLayer(true)+1);
                            guessedSecondHitPtr->setHitType(HitType::undefined);
                            if(isFineIDInStrip(fineID))  guessedSecondHitPtr->setDetType(SiliconTech::strip);
                            else  guessedSecondHitPtr->setDetType(SiliconTech::pixel);

                            theseHits.push_back(guessedSecondHitPtr);
                        }
                    }
                    // Store the hits for now
                    listofHitsFound.push_back(std::move(theseHits));
                }
            }
            } // end HitSearchLoop timing

            {
            Athena::Chrono chronoSort("NNPathfinder:HitSorting", m_chronoSvc.get());
            
            // Use partial_sort instead of sort since we only need the top N hits
            const size_t nToSort = (m_maxBranches.value() >= 0) ? 
                std::min(static_cast<size_t>(m_maxBranches.value()), listofHitsFound.size()) : 
                listofHitsFound.size();
            
            // Sort the hit by the distance
            if (m_useCartesian) {
                auto comparatorFunc = [&](const auto& a, const auto& b) {
                    return cartesianComparator(a, b, predr, predz);
                };
                if (nToSort < listofHitsFound.size()) {
                    std::partial_sort(listofHitsFound.begin(),
                                      listofHitsFound.begin() + nToSort,
                                      listofHitsFound.end(),
                                      comparatorFunc);
                } else {
                    std::sort(listofHitsFound.begin(), listofHitsFound.end(), comparatorFunc);
                }
            }
            else {
                const double zScale = getZScale();
                const double phiScale = getPhiScale();
                const double rScale = getRScale();
                const double zScale2 = zScale * zScale;
                const double phiScale2 = phiScale * phiScale;
                const double rScale2 = rScale * rScale;

                auto comparatorFunc = [&](const auto& a, const auto& b) {
                    return polarComparator(a, b, predr, predphi, predz, zScale2, phiScale2, rScale2);
                };
                if (nToSort < listofHitsFound.size()) {
                    std::partial_sort(listofHitsFound.begin(),
                                      listofHitsFound.begin() + nToSort,
                                      listofHitsFound.end(),
                                      comparatorFunc);
                } else {
                    std::sort(listofHitsFound.begin(), listofHitsFound.end(), comparatorFunc);
                }
            }
            } // end HitSorting timing

            // Select the top N hits
            std::vector<std::vector<std::shared_ptr<const FPGATrackSimHit>>> cleanHitsToGrow;

            // If max branches are limited, pick only the top N from the list of hits found at each level
            if (m_maxBranches.value() >= 0) {
                int nHitsToChoose = std::min(int(m_maxBranches.value()), int(listofHitsFound.size()));
                cleanHitsToGrow.reserve(nHitsToChoose);
                std::copy(listofHitsFound.begin(), listofHitsFound.begin() + nHitsToChoose, std::back_inserter(cleanHitsToGrow));
            }
            else {
                cleanHitsToGrow = std::move(listofHitsFound);
            }

            {
            Athena::Chrono chronoRoadBuilding("NNPathfinder:RoadBuilding", m_chronoSvc.get());
            for (auto& hitsFound: cleanHitsToGrow) {

                // We got a hit, lets make a road
                miniRoad newRoad;
                if(!addHitToRoad(newRoad, currentRoad, std::move(hitsFound))) {
                    ATH_MSG_WARNING("Failed to make a new road");
                    continue;
                }
                roadsToExtrapolate.push_back(newRoad);
                foundhitForRoad = true;
                if(m_debugEvent) {
                    ATH_MSG_DEBUG("------ road grown with hit from layer "<<layer<<" to");
                    printRoad(newRoad);
                }
            }
            // If the hit wasn't found, push a fake hit
            if (!foundhitForRoad) {
                // did not find a hit to extrapolate to, check if we need to delete this road. if not, add a guessed hit if still useful
                if (currentRoad.getNWCLayers() >= m_maxMiss) {
                    // we don't want this road, so we continue
                    continue;
                }
                else {
                    std::vector<std::shared_ptr<const FPGATrackSimHit>> theseHits;
                    // first make the fake hit that we will add
                    if (!getFakeHit(currentRoad, predhit, fineID, theseHits)) {
                        ATH_MSG_WARNING("Failed adding a guessed hit in extrapolation");
                        continue;
                    }

                    if((isFineIDInPixel(fineID) && theseHits.size() != 1) || (isFineIDInStrip(fineID) && theseHits.size() != 2)) {
                        continue;
                    }
                    // add the hit to the road
                    miniRoad newroad;
                    if (!addHitToRoad(newroad, currentRoad, std::move(theseHits))) {
                        ATH_MSG_WARNING("Failed making a new road with fake hit");
                        continue;
                    }
                    roadsToExtrapolate.push_back(std::move(newroad));
                }
            }
            } // end RoadBuilding timing
        }
        // This track has been extrapolated, copy the completed tracks to the full list with full road objects
        {
        Athena::Chrono chronoRoadConversion("NNPathfinder:RoadConversion", m_chronoSvc.get());
        for (const auto &miniroad : completedRoads) {
            FPGATrackSimRoad road;
            road.setWCLayers(miniroad.getWCLayers());
            road.setHitLayers(miniroad.getHitLayers());
            road.setRoadID(m_roads.size() - 1);
            // Set the "Hough x" and "Hough y" using the track parameters.
            road.setX(track->getPhi());
            road.setY(track->getQOverPt());
            road.setXBin(track->getHoughXBin());
            road.setYBin(track->getHoughYBin());
            road.setSubRegion(track->getSubRegion());

            // just force the right number of layers now, in case we find fewer than expected (needed downstream)
            std::vector<std::vector<std::shared_ptr<const FPGATrackSimHit>>> roadhits = miniroad.getVecHits();
            unsigned nexpected = m_nLayers_1stStage+m_nLayers_2ndStage;
            if (roadhits.size() > nexpected) { // cut off the last ones
                roadhits.resize(nexpected);
            }
            else if (roadhits.size() < nexpected) { // fill with missing hits
                for (unsigned layer = roadhits.size(); layer < nexpected; layer++) {
                    std::shared_ptr<FPGATrackSimHit> emptyHitPtr = std::make_shared<FPGATrackSimHit>();
                    emptyHitPtr->setX(0);
                    emptyHitPtr->setY(0);
                    emptyHitPtr->setZ(0);
                    emptyHitPtr->setLayer(layer);
                    emptyHitPtr->setHitType(HitType::wildcard);
                   
                    roadhits.emplace_back(1,emptyHitPtr);
                    layer_bitmask_t wclayers = road.getWCLayers();
                    wclayers |= (1 << layer);
                    road.setWCLayers(wclayers);
                }
            }
            road.setHits(std::move(roadhits));

            m_roads.push_back(std::move(road));
        }
        } // end RoadConversion timing
    }
    // Copy the roads we found into the output argument and return success.
    roads.reserve(m_roads.size());
    for (FPGATrackSimRoad & r : m_roads)
    {
        if (r.getNWCLayers() >= m_maxMiss) continue; // extra check on this
        roads.emplace_back(r);
    }
    ATH_MSG_DEBUG("Found " << roads.size() << " new roads in second stage.");

    return StatusCode::SUCCESS;
}

StatusCode FPGATrackSimNNPathfinderExtensionTool::fillInputTensorForNN(miniRoad& thisroad, std::vector<float>& inputTensorValues)
{
    std::vector<std::shared_ptr<const FPGATrackSimHit>> hitsR;

    std::vector<std::shared_ptr<const FPGATrackSimHit>> hits = thisroad.getHits();


    for (unsigned ihit = 0; ihit < hits.size(); ihit++) {
      if (ihit < m_nLayers_1stStage && !(hits[ihit]->isReal())) continue; // skip guessed hits for the 1st stage
       hitsR.push_back(hits[ihit]);
    }
    // Sort in increasing 3D distance.
    std::sort(hitsR.begin(), hitsR.end(), [](auto& a, auto& b){
        double dist_a = std::hypot(a->getX(), a->getY(), a->getZ());
        double dist_b = std::hypot(b->getX(), b->getY(), b->getZ());
        return dist_a < dist_b;
    });

    if(m_debugEvent) ATH_MSG_DEBUG("hitsR");
    for (const auto& thit : hitsR)
    {
        if (m_debugEvent) ATH_MSG_DEBUG(thit->getX() << " " << thit->getY() << " " << thit->getZ() << " and phi = " << thit->getGPhi());
    }

    // Remove all the duplicate space points
    std::vector<std::shared_ptr<const FPGATrackSimHit>> cleanHits;
    bool skipHit = false;
    for (auto thit : hitsR)
    {
        if(skipHit)
        {
            skipHit = false;
            continue;
        }
        if (thit->isPixel())
        {
            cleanHits.push_back(std::move(thit));
        }
        else if (thit->isStrip() && (thit->getHitType() == HitType::spacepoint))
        {
            // This is a proper strips SP, push the first hit back and skip the next one since its a duplicate
            cleanHits.push_back(std::move(thit));
            skipHit = true;
        }
        else if (thit->isStrip() && (thit->getHitType() == HitType::guessed))
        {
            // this is a guessed strip SP, push the first hit back and skip the next one since its a duplicate
            cleanHits.push_back(std::move(thit));
            skipHit = true;
        }
        else if (thit->isStrip() && (thit->getHitType() == HitType::undefined))
        {
            // this is a fake hit, inserted for a unpaired SP, continue
            continue;
        }
        else if (thit->isStrip() && thit->isReal())
        {
            // What is left here is a unpaired hit, push it back
            cleanHits.push_back(std::move(thit));
        }
        else
        {
            ATH_MSG_WARNING("No clue how to deal with this hit in the NN predicition ");
            continue;
        }
    }

    // Reverse the hits as we changed the ordering before
    if(!m_doOutsideIn)
    {
        std::reverse(cleanHits.begin(), cleanHits.end());
    }

    // Select the top N hits
    std::vector<std::shared_ptr<const FPGATrackSimHit>> hitsToEncode;
    std::copy(cleanHits.begin(), cleanHits.begin() + m_predictionWindowLength, std::back_inserter(hitsToEncode));

    if(m_debugEvent) ATH_MSG_DEBUG("Clean hits");
    for (const auto& thit : cleanHits)
    {
        if (m_debugEvent) ATH_MSG_DEBUG(thit->getX() << " " << thit->getY() << " " << thit->getZ() << " " << thit->isStrip() << " and gphi = " << thit->getGPhi());
    }

    // Reverse this vector so we can encode it the format as expected from the NN
    std::reverse(hitsToEncode.begin(), hitsToEncode.end());

    if (m_debugEvent) ATH_MSG_DEBUG("Input for NN prediction");
    for (const auto& thit : hitsToEncode)
    {
        if (m_useCartesian) {
            inputTensorValues.push_back(thit->getX() / getXScale());
            inputTensorValues.push_back(thit->getY() / getYScale());
            inputTensorValues.push_back(thit->getZ() / getZScale());
        }
        else {
            inputTensorValues.push_back(thit->getR() / getRScale());
            inputTensorValues.push_back(thit->getGPhi() / getPhiScale());
            inputTensorValues.push_back(thit->getZ() / getZScale());
        }

        if (m_debugEvent) ATH_MSG_DEBUG(thit->getX() << " " << thit->getY() << " " << thit->getZ() << " and gphi = " << thit->getGPhi());
    }

    return StatusCode::SUCCESS;

}


StatusCode FPGATrackSimNNPathfinderExtensionTool::getPredictedHit(std::vector<float>& inputTensorValues, std::vector<float>& outputTensorValues, long& fineID)
{
    std::vector<float> NNVoloutput = m_extensionVolNN.runONNXInference(inputTensorValues);
    fineID = std::distance(NNVoloutput.begin(),std::max_element(NNVoloutput.begin(), NNVoloutput.end()));
    // Insert the output for the second stage NN
    inputTensorValues.insert(inputTensorValues.end(), NNVoloutput.begin(), NNVoloutput.end()); //now we append the first NN to the list of coordinates

    // use the above to predict the next hit position
    outputTensorValues = m_extensionHitNN.runONNXInference(inputTensorValues);

    // now scale back
    if (m_useCartesian) {
      outputTensorValues[0] *= getXScale();
      outputTensorValues[1] *= getYScale();
      outputTensorValues[2] *= getZScale();
    }
    else {
      outputTensorValues[0] *= getRScale();
      outputTensorValues[1] *= getPhiScale();
      outputTensorValues[2] *= getZScale();
    }

    return StatusCode::SUCCESS;
}

StatusCode FPGATrackSimNNPathfinderExtensionTool::addHitToRoad(miniRoad& newroad, miniRoad& currentRoad, const std::vector<std::shared_ptr<const FPGATrackSimHit>>& hits)
{
  newroad.setHits(currentRoad.getHits());
  newroad.addHits(hits);
  return StatusCode::SUCCESS;
}

StatusCode FPGATrackSimNNPathfinderExtensionTool::getFakeHit(miniRoad& currentRoad, std::vector<float>& predhit, const long& fineID, std::vector<std::shared_ptr<const FPGATrackSimHit>>& hits) {

  unsigned guessedLayer(0);

    std::shared_ptr<FPGATrackSimHit> guessedHitPtr = std::make_shared<FPGATrackSimHit>();
    if (m_useCartesian) {
      guessedHitPtr->setX(predhit[0]);
      guessedHitPtr->setY(predhit[1]);
      guessedHitPtr->setZ(predhit[2]);
    }
    else {
      double r = predhit[0];
      double phi = predhit[1];
      guessedHitPtr->setX(r*cos(phi));
      guessedHitPtr->setY(r*sin(phi));
      guessedHitPtr->setZ(predhit[2]);
    }
     

    if (m_doOutsideIn) { // outside in
      // TODO
    }
    else { // nope, inside out
      guessedLayer = currentRoad.getNHits();
    }

    guessedHitPtr->setLayer(guessedLayer);
    guessedHitPtr->setHitType(HitType::guessed);

    if(isFineIDInStrip(fineID))  {
      guessedHitPtr->setDetType(SiliconTech::strip);
      guessedHitPtr->setPhysLayer(0); // it is just to set the side, it's not actually 0
    }
    else  guessedHitPtr->setDetType(SiliconTech::pixel);


    // Make sure that the hit is inside the boundaries of the layers
    if(guessedLayer < (m_nLayers_1stStage + m_nLayers_2ndStage ))
    {
        hits.push_back(guessedHitPtr);
    }
    // if in strips, add the second hit into the list
    if(isFineIDInStrip(fineID))
    {
        std::shared_ptr<FPGATrackSimHit> guessedSecondHitPtr = std::make_shared<FPGATrackSimHit>();
        guessedSecondHitPtr->setX(0);
        guessedSecondHitPtr->setY(0);
        guessedSecondHitPtr->setZ(0);
        guessedSecondHitPtr->setLayer( guessedLayer + 1 );
        guessedSecondHitPtr->setHitType(HitType::guessed);

        guessedSecondHitPtr->setDetType(SiliconTech::strip);
	guessedHitPtr->setPhysLayer(1); // it is just to set the side, it's not actually 1
        // Make sure that the hit is inside the boundaries of the layers
        if((guessedLayer + 1) < (m_nLayers_1stStage + m_nLayers_2ndStage ))
        {
            hits.push_back(guessedSecondHitPtr);
        }
    }



    return StatusCode::SUCCESS;

}

void FPGATrackSimNNPathfinderExtensionTool::printRoad(miniRoad& currentRoad)
{
    if (!m_debugEvent) return;

    // print this road
    if (m_debugEvent)
    {
        std::vector<std::shared_ptr<const FPGATrackSimHit>> hitsR;
        for (const auto& hit : currentRoad.getHits()) {
            hitsR.push_back(hit);
        }

        // If outside in, sort in increasing R, otherwise, decreasing R
        if (m_doOutsideIn)
        {
            std::sort(hitsR.begin(), hitsR.end(), [](const auto& a, const auto& b) {
                if (a->getR() == b->getR()) return a->getLayer() < b->getLayer();
                return a->getR() < b->getR();
                });
        }
        else
        {
            std::sort(hitsR.begin(), hitsR.end(), [](const auto& a, const auto& b) {
                return a->getR() > b->getR();
                });
        }

        for (unsigned long i = 0; i < hitsR.size(); i++)
        {
            ATH_MSG_DEBUG("Hit i " << i << " X: " << hitsR[i]->getX() << " Y: " << hitsR[i]->getY() << " Z: " << hitsR[i]->getZ() << " R: " << hitsR[i]->getR() << " hitType: " << hitsR[i]->getHitType() << " getDetType: " << hitsR[i]->getDetType() << "phi = " << hitsR[i]->getGPhi());
        }
    }

}

StatusCode FPGATrackSimNNPathfinderExtensionTool::getLastLayer(miniRoad& currentRoad, unsigned& lastHitLayer, std::shared_ptr<const FPGATrackSimHit>& lastHit)
{
    lastHitLayer = currentRoad.getNHits()-1;
    lastHit = currentRoad.getHit(lastHitLayer);
    return StatusCode::SUCCESS;

}
