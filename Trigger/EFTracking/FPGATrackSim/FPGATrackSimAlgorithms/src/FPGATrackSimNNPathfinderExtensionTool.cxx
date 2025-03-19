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

#include <cmath>
#include <algorithm>

FPGATrackSimNNPathfinderExtensionTool::FPGATrackSimNNPathfinderExtensionTool(const std::string& algname, const std::string &name, const IInterface *ifc) :
    base_class(algname, name, ifc) {
        declareInterface<IFPGATrackSimTrackExtensionTool>(this);
    }


StatusCode FPGATrackSimNNPathfinderExtensionTool::initialize() {

    // Retrieve the mapping service.
    ATH_CHECK(m_FPGATrackSimMapping.retrieve());

    m_nLayers_1stStage = m_FPGATrackSimMapping->PlaneMap_1st(0)->getNLogiLayers();
    m_nLayers_2ndStage = m_FPGATrackSimMapping->PlaneMap_2nd(0)->getNLogiLayers() - m_nLayers_1stStage;

    if (m_windowR.size() != 1 && m_windowR.size() != m_nLayers_2ndStage) {
      ATH_MSG_ERROR("Window r size = " << m_windowR << " is not equal to 1 (for all layers) and not equal to " << m_nLayers_2ndStage);
      return StatusCode::FAILURE;
    }

    if (m_windowZ.size() != 1 && m_windowZ.size() != m_nLayers_2ndStage) {
      ATH_MSG_ERROR("Window z size = " << m_windowZ << " is not equal to 1 (for all layers) and not equal to " << m_nLayers_2ndStage);
      return StatusCode::FAILURE;
    }

    
    m_maxMiss = (m_nLayers_1stStage + m_nLayers_2ndStage) - m_threshold;

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

    // This now needs to be done once for each slice.
    for (size_t j=0; j<m_FPGATrackSimMapping->GetPlaneMap_2ndSliceSize(); j++){
        ATH_MSG_INFO("Processing second stage slice " << j);
        m_phits_atLayer[j] = std::map<unsigned, std::vector<std::shared_ptr<const FPGATrackSimHit>>>();
        for (unsigned i = m_nLayers_1stStage; i < m_nLayers_2ndStage +m_nLayers_1stStage ; i++) {
            ATH_MSG_INFO("Processing layer " << i);
            m_phits_atLayer[j][i] = std::vector<std::shared_ptr<const FPGATrackSimHit>>();
        }
    }
    ATH_CHECK(m_tHistSvc.retrieve());
    ATH_CHECK(bookTree());

    return StatusCode::SUCCESS;
}

StatusCode FPGATrackSimNNPathfinderExtensionTool::bookTree()
{
    m_tree = new TTree("NNPathFinderMonitoring","NNPathFinderMonitoring");
    m_tree->Branch("NcompletedRoads", &m_NcompletedRoads);
    m_tree->Branch("predictedHitsFineID", &m_predictedHitsFineID);
    m_tree->Branch("foundHitITkLayer", &m_foundHitITkLayer);
    m_tree->Branch("missingHitsOnRoad", &m_missingHitsOnRoad);
    m_tree->Branch("nHitsInSearchWindow", &m_nHitsInSearchWindow);
    m_tree->Branch("distanceOfPredictedHitToFoundHit", &m_distanceOfPredictedHitToFoundHit);
    m_tree->Branch("foundHitIsSP", &m_foundHitIsSP);

    ATH_CHECK(m_tHistSvc->regTree(Form("/FPGATRACKSIMOUTPUTNNPATHFINDER/%s", m_tree->GetName()), m_tree));

    return StatusCode::SUCCESS;
}


StatusCode FPGATrackSimNNPathfinderExtensionTool::extendTracks(const std::vector<std::shared_ptr<const FPGATrackSimHit>> & hits,
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
    // For now allow mapping into all subregions, may not be necessary in the future
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

    if(m_debugEvent) ATH_MSG_DEBUG("Got: "<<tracks.size()<<" tracks to extrapolate");

    // Now, loop over the tracks.
    for (std::shared_ptr<const FPGATrackSimTrack> track : tracks)
    {

        if(m_debugEvent) ATH_MSG_DEBUG("\033[1;31m-------------------------- extraploating Track ------------------ \033[0m");

        if (track->passedOR() == 0) {
            continue;
        }

        const std::vector<FPGATrackSimHit> hitsOnTrack = track->getFPGATrackSimHits();

        FPGATrackSimRoad road;
        road.setNLayers(m_nLayers_2ndStage);


        size_t slice = track->getSubRegion();
	float pt = track->getPt();
        layer_bitmask_t hitLayers = 0;
        layer_bitmask_t wcLayers = 0;
        // Loop over all hits

        std::vector<std::vector<std::shared_ptr<const FPGATrackSimHit>>> road_hits;
        road_hits.resize(m_nLayers_1stStage + m_nLayers_2ndStage);
        for (auto &thit : hitsOnTrack)
        {
            // Don't care about wildcard hits in extrapolation
            if(thit.getHitType() == HitType::wildcard) continue;

            // add this hit to the road, start with all of them
            road_hits[thit.getLayer()].push_back(std::make_shared<const FPGATrackSimHit>(thit));
            if (thit.isReal()) hitLayers |= 1 << thit.getLayer();
            else wcLayers |= 1 << thit.getLayer();
        }

        road.setHits(std::move(road_hits));
        road.setHitLayers(hitLayers);
        road.setWCLayers(wcLayers);


        if(m_debugEvent)
        {
            ATH_MSG_DEBUG("-----------------Hits in event");
            for (unsigned layer = m_nLayers_1stStage; layer < m_nLayers_2ndStage + m_nLayers_1stStage; layer++)
            {
                for (const std::shared_ptr<const FPGATrackSimHit>& hit: m_phits_atLayer[slice][layer])
                {
                    ATH_MSG_DEBUG("Hit "<<" X: "<<hit->getX()<<" Y: "<<hit->getY()<<" Z: "<<hit->getZ()<<" R: "<<hit->getR()<<" layer: "<<hit->getLayer()<<" hitType: "<<hit->getHitType()<<" getDetType: "<<hit->getDetType());
                }
            }
        }

        std::vector<FPGATrackSimRoad> roadsToExtrapolate;
        roadsToExtrapolate.push_back(road);

        std::vector<FPGATrackSimRoad> completedRoads;

        std::vector<unsigned long> currentRoadHitFineIDs;
        std::vector<std::vector<unsigned long>> tmp_predictedHitsFineID;

        std::vector<unsigned int> currentRoadHitITkLayer;
        std::vector<std::vector<unsigned int>> tmp_foundHitITkLayer;

        std::vector<float> currentRoadHitDistancePredFound;
        std::vector<std::vector<float>> tmp_foundHitDistancePredFound;


        for (unsigned int i = 0; i < roadsToExtrapolate.size(); i++){
            tmp_predictedHitsFineID.push_back(currentRoadHitFineIDs);
            tmp_foundHitITkLayer.push_back(currentRoadHitITkLayer);
            tmp_foundHitDistancePredFound.push_back(currentRoadHitDistancePredFound);
        }


        int count = 0;

        while(roadsToExtrapolate.size() > 0)
        {
            FPGATrackSimRoad currentRoad = *roadsToExtrapolate.begin();
            std::vector<unsigned long> tmp_currentRoadHitFineIDs = *tmp_predictedHitsFineID.begin();
            std::vector<unsigned int> tmp_currentRoadHitITkLayer = *tmp_foundHitITkLayer.begin();
            std::vector<float> tmp_currentRoadHitDistancePredFound = *tmp_foundHitDistancePredFound.begin();
            // Erase this road from the vector
            roadsToExtrapolate.erase(roadsToExtrapolate.begin());
            tmp_predictedHitsFineID.erase(tmp_predictedHitsFineID.begin());
            tmp_foundHitITkLayer.erase(tmp_foundHitITkLayer.begin());
            tmp_foundHitDistancePredFound.erase(tmp_foundHitDistancePredFound.begin());

            count ++;

            if(m_debugEvent) ATH_MSG_DEBUG("\033[1;31m-------------------------- extraploating road "<< count << "------------------ \033[0m");
            printRoad(currentRoad);



            // Check exit condition
            if (currentRoad.getHits_flat().size() >= (m_nLayers_1stStage+m_nLayers_2ndStage))
            {
                completedRoads.push_back(currentRoad);
                m_predictedHitsFineID.push_back(tmp_currentRoadHitFineIDs);
                m_foundHitITkLayer.push_back(tmp_currentRoadHitITkLayer);
                m_distanceOfPredictedHitToFoundHit.push_back(tmp_currentRoadHitDistancePredFound);
                continue; // this one is done
            }

            // Other try to find the next hit in this road
            std::vector<float> inputTensorValues;
            if(!fillInputTensorForNN(currentRoad, inputTensorValues))
            {
                ATH_MSG_WARNING("Failed to create input tensor for this road");
                continue;
            }


            std::vector<float> predhit;
            long fineID;
            if(!getPredictedHit(inputTensorValues, predhit, fineID))
            {
                ATH_MSG_WARNING("Failed to predict hit for this road");
                continue;
            }

            // Check if exist conditions are there
            if(m_doOutsideIn)
            {
                // Make sure we are not predicting inside the inner most layer (x and y < 25)
                // If we are, road is done
                if (abs(predhit[0]) < 25 && abs(predhit[1]) < 25)
                {
                    completedRoads.push_back(currentRoad);
                    m_predictedHitsFineID.push_back(tmp_currentRoadHitFineIDs);
                    m_foundHitITkLayer.push_back(tmp_currentRoadHitITkLayer);
                    m_distanceOfPredictedHitToFoundHit.push_back(tmp_currentRoadHitDistancePredFound);
                    continue;
                }
            }
            else
            {
                // Make sure we are not predicting outside the outer most layer
                // if we are, road is done
                double rad = std::sqrt(std::pow(predhit[0], 2) + std::pow(predhit[1], 2));
                if (abs(predhit[0]) > 1024 || abs(predhit[1]) > 1024 || rad > 1024 || abs(predhit[2]) > 3000)
                {
                    completedRoads.push_back(currentRoad);
                    m_predictedHitsFineID.push_back(tmp_currentRoadHitFineIDs);
                    m_foundHitITkLayer.push_back(tmp_currentRoadHitITkLayer);
                    m_distanceOfPredictedHitToFoundHit.push_back(tmp_currentRoadHitDistancePredFound);
                    continue;
                }
            }

            if(m_debugEvent)
            {
                ATH_MSG_DEBUG("Predicted hit at: "<<predhit[0]<<" "<<predhit[1]<<" "<<predhit[2]);
            }

            // Now search for the hits
            bool foundhitForRoad = false;
            bool skipSPInNextLayer = false;

            if(fineID == 215){
                ATH_MSG_DEBUG("Stopping condition reached");
                completedRoads.push_back(currentRoad);
                break;
            }

            // Get the last layer in the road. And only check layers above that
            unsigned lastLayerInRoad = 0;
            std::shared_ptr<const FPGATrackSimHit> lastHit;
            if(!getLastLayer(currentRoad, lastLayerInRoad, lastHit) or !lastHit)
            {
                ATH_MSG_WARNING("Failed to find last layer this road");
                continue;
            }

	          bool lastHitWasReal = lastHit->isReal();
	    
            //  if the last layer is m_nLayers_1stStage + m_nLayers_2ndStage, then we have hit the end already
            if((lastLayerInRoad + 1) >= m_nLayers_1stStage + m_nLayers_2ndStage)
            {
                completedRoads.push_back(currentRoad);
                continue;
            }

            for (unsigned layer = m_nLayers_1stStage; layer < m_nLayers_2ndStage + m_nLayers_1stStage; layer++)
            {
                // Local var for the internal loop
                bool skipSP = false;
                // loop over layers, probably only one will be found
                if(skipSPInNextLayer)
                {
                    skipSP = true;
                    skipSPInNextLayer = false;
                }
                // Only check the layers above the last layer in the road
                if(layer <= lastLayerInRoad)
                {
                    continue;
                }
                unsigned int hitsInWindow = 0;

                // List of all the hits, with their distances to the predicted point
                std::vector<std::vector<std::shared_ptr<const FPGATrackSimHit>>> listofHitsFound;

                for (const std::shared_ptr<const FPGATrackSimHit>& hit: m_phits_atLayer[slice][layer])
                {
                    if(skipSP && hit->isStrip() && hit->getHitType() == HitType::spacepoint) continue;

                    // loop over hits in that layer
                    if (getFineID(*hit) == fineID && hit->isReal())
                    {
                        // a hit is in the right fine ID == layer
                        double hitz = hit->getZ();
                        double hitr = hit->getR();
                        double predr = sqrt(predhit[0]*predhit[0] + predhit[1] * predhit[1]);
                        double predz = predhit[2];
			double windowR = m_windowR[0]; // default for all layers
			double windowZ = m_windowZ[0]; // default for all layers

			// But if available pick up per-window values
			if (m_windowR.size() > 1) {
			  windowR = m_windowR[layer-m_nLayers_1stStage]; // offset by n1st stage
			}
			if (m_windowZ.size() > 1) {
			  windowZ = m_windowZ[layer-m_nLayers_1stStage]; // offset by n1st stage
			}

			// If last hit was not real and we want to, scale the window
			if (m_missedHitRScaling > 0 && !lastHitWasReal) windowR *= m_missedHitRScaling;
			if (m_missedHitZScaling > 0 && !lastHitWasReal) windowZ *= m_missedHitZScaling;			
			
			// now scale windows for low pt, if desired
			if (pt < m_lowPtValueForWindowRScaling.value()) windowR *= m_lowPtWindowRScaling.value();
			if (pt < m_lowPtValueForWindowZScaling.value()) windowZ *= m_lowPtWindowZScaling.value();			
			
                        if (abs(hitr - predr) < windowR && abs(hitz - predz) < windowZ)
                        {
                            std::vector<std::shared_ptr<const FPGATrackSimHit>> theseHits {hit};
                            hitsInWindow = hitsInWindow + 1;

                            // If the hit is a space point, skip the next layer, as it will be duplicated space point and we have already taken care of that in the adding of the hits
                            if(hit->isStrip())
                            {
                                // If we are in odd layer, the corresponding surface is +1
                                // if we are in an even layer, the corresponsding surface is at -1
                                int layerToCheck = layer + 1;
                                if((layer % 2) == 0) layerToCheck = layer - 1;

                                if (hit->getHitType() == HitType::spacepoint) {
                                    skipSPInNextLayer = true;
                                    // find the hit in the next layer
                                    if(!findHitinNextStripLayer(hit,  m_phits_atLayer[slice][layerToCheck], theseHits))
                                    {
                                        ATH_MSG_WARNING("For a SP in layer "<<layer<<" Couldn't find a matching strip SP in layer "<<layerToCheck);
                                    }
                                }
                                else {
                                    std::shared_ptr<FPGATrackSimHit> guessedSecondHitPtr = std::make_shared<FPGATrackSimHit>();
                                    guessedSecondHitPtr->setX(0);
                                    guessedSecondHitPtr->setY(0);
                                    guessedSecondHitPtr->setZ(0);
                                    guessedSecondHitPtr->setLayer(layerToCheck);
                                    guessedSecondHitPtr->setHitType(HitType::undefined);
                                    if(isFineIDInStrip(fineID))  guessedSecondHitPtr->setDetType(SiliconTech::strip);
                                    else  guessedSecondHitPtr->setDetType(SiliconTech::pixel);

                                    theseHits.push_back(guessedSecondHitPtr);
                                }
                            }
                            // Store the hits for now
                            listofHitsFound.push_back(theseHits);
                        }
                    }
                }

                // Sort the hit by the distance
                std::sort(listofHitsFound.begin(), listofHitsFound.end(), [&predhit](auto& a, auto& b){
                    double predr = sqrt(predhit[0]*predhit[0] + predhit[1] * predhit[1]);
                    double predz = predhit[2];

                    // HitA
                    double hitz = a[0]->getZ();
                    double hitr = a[0]->getR();
                    float distance_a= sqrt((hitr - predr)*(hitr - predr) + (hitz - predz)*(hitz - predz));

                    // HitB
                    hitz = b[0]->getZ();
                    hitr = b[0]->getR();
                    float distance_b= sqrt((hitr - predr)*(hitr - predr) + (hitz - predz)*(hitz - predz));

                    return distance_a < distance_b;
                });

                // Select the top N hits
                std::vector<std::vector<std::shared_ptr<const FPGATrackSimHit>>> cleanHitsToGrow;

                // If max branches are limited, pick only the top N from the list of hits found at each level 
                if (m_maxBranches.value() >= 0) 
                {
                    int nHitsToChoose = std::min(int(m_maxBranches.value()), int(listofHitsFound.size()));
                    cleanHitsToGrow.reserve(nHitsToChoose);
                    std::copy(listofHitsFound.begin(), listofHitsFound.begin() + nHitsToChoose, std::back_inserter(cleanHitsToGrow));
                }
                else
                {
                    cleanHitsToGrow = std::move(listofHitsFound);
                }


                for (auto& hitsFound: cleanHitsToGrow)
                {
                    // get the first hit
                    auto hit = hitsFound[0];

                    // a hit is in the right fine ID == layer
                    double hitz = hit->getZ();
                    double hitr = hit->getR();
                    double predr = sqrt(predhit[0]*predhit[0] + predhit[1] * predhit[1]);
                    double predz = predhit[2];
                    float distancePredFound = sqrt((hitr - predr)*(hitr - predr) + (hitz - predz)*(hitz - predz));

                    // We got a hit, lets make a road
                    FPGATrackSimRoad newroad;
                    if(!addHitToRoad(newroad, currentRoad, std::move(hitsFound)))
                    {
                        ATH_MSG_WARNING("Failed to make a new road");
                        continue;
                    }

                    roadsToExtrapolate.push_back(newroad);
                    foundhitForRoad = true;
                    tmp_currentRoadHitFineIDs.push_back(fineID);
                    tmp_predictedHitsFineID.push_back(tmp_currentRoadHitFineIDs);
                    tmp_currentRoadHitITkLayer.push_back(hit->getLayerDisk());
                    tmp_foundHitITkLayer.push_back(tmp_currentRoadHitITkLayer);
                    tmp_currentRoadHitDistancePredFound.push_back(distancePredFound);
                    tmp_foundHitDistancePredFound.push_back(tmp_currentRoadHitDistancePredFound);

                    if(m_debugEvent)
                    {
                        ATH_MSG_DEBUG("------ road grown with hit from layer "<<layer<<" to");
                        printRoad(newroad);
                    }
                }
                
                if (hitsInWindow != 0) m_nHitsInSearchWindow.push_back(hitsInWindow);
            }
            // If the hit wasn't found, push a fake hit
            if (!foundhitForRoad)
            {

                // did not find a hit to extrapolate to, check if we need to delete this road. if not, add a guessed hit if still useful
                m_nHitsInSearchWindow.push_back(0);
                if (currentRoad.getNWCLayers() >= m_maxMiss)
                {
                    // we don't want this road, so we continue
                    continue;
                }
                else
                {
                    std::vector<std::shared_ptr<const FPGATrackSimHit>> theseHits;
                    // first make the fake hit that we will add
                    if (!getFakeHit(currentRoad, slice, predhit, fineID, theseHits)) {
                        ATH_MSG_WARNING("Failed adding a guessed hit in extrapolation");
                        continue;
                    }

                    if((isFineIDInPixel(fineID) && theseHits.size() != 1) || (isFineIDInStrip(fineID) && theseHits.size() != 2))
                    {
                        continue;
                    }

                    // add the hit to the road
                    FPGATrackSimRoad newroad;

                    if (!addHitToRoad(newroad, currentRoad, std::move(theseHits))) {
                        ATH_MSG_WARNING("Failed making a new road with fake hit");
                        continue;
                    }
                    roadsToExtrapolate.push_back(newroad);
                    tmp_predictedHitsFineID.push_back(tmp_currentRoadHitFineIDs);
                    tmp_foundHitITkLayer.push_back(tmp_currentRoadHitITkLayer);
                    tmp_foundHitDistancePredFound.push_back(tmp_currentRoadHitDistancePredFound);
                }
            }
        }

        m_NcompletedRoads.push_back(completedRoads.size());
        // This track has been extrapolated, copy the completed tracks to the full list
        for (auto road : completedRoads) {

            m_missingHitsOnRoad.push_back(road.getNWCLayers());
            road.setRoadID(roads.size() - 1);
            // Set the "Hough x" and "Hough y" using the track parameters.
            road.setX(track->getPhi());
            road.setY(track->getQOverPt());
            road.setXBin(track->getHoughXBin());
            road.setYBin(track->getHoughYBin());
            road.setSubRegion(track->getSubRegion());
            m_roads.push_back(road);
        }
        currentRoadHitFineIDs.clear();
        tmp_predictedHitsFineID.clear();
        currentRoadHitITkLayer.clear();
        tmp_foundHitITkLayer.clear();
        currentRoadHitDistancePredFound.clear();
        tmp_foundHitDistancePredFound.clear();
    }

    // Copy the roads we found into the output argument and return success.
    roads.reserve(m_roads.size());
    for (FPGATrackSimRoad & r : m_roads)
    {

        if (r.getNWCLayers() >= m_maxMiss) continue; // extra check on this
        roads.emplace_back(std::make_shared<const FPGATrackSimRoad>(r));
    }
    ATH_MSG_DEBUG("Found " << roads.size() << " new roads in second stage.");

    m_tree->Fill();
    m_NcompletedRoads.clear();
    m_predictedHitsFineID.clear();
    m_missingHitsOnRoad.clear();
    m_nHitsInSearchWindow.clear();
    m_distanceOfPredictedHitToFoundHit.clear();
    m_foundHitITkLayer.clear();
    m_foundHitIsSP.clear();

    return StatusCode::SUCCESS;
}

StatusCode FPGATrackSimNNPathfinderExtensionTool::fillInputTensorForNN(FPGATrackSimRoad& thisroad, std::vector<float>& inputTensorValues)
{
    std::vector<std::shared_ptr<const FPGATrackSimHit>> hitsR;

    for (auto &hit : thisroad.getHits_flat()) {
        hitsR.push_back(hit);
    }

    // Sort in increasing R. We will reverise it for inside out after the cleanup
    std::sort(hitsR.begin(), hitsR.end(), [](auto& a, auto& b){
        if(a->getLayer() == b->getLayer()) return a->getR() < b->getR();
        return a->getLayer() < b->getLayer();
    });


    if(m_debugEvent) ATH_MSG_DEBUG("hitsR");
    for (auto thit : hitsR)
    {
        if(m_debugEvent) ATH_MSG_DEBUG(thit->getX()<<" "<<thit->getY()<<" "<<thit->getZ());
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
            cleanHits.push_back(thit);
        }
        else if (thit->isStrip() && (thit->getHitType() == HitType::spacepoint))
        {
            // This is a proper strips SP, push the first hit back and skip the next one since its a duplicate
            cleanHits.push_back(thit);
            skipHit = true;
        }
        else if (thit->isStrip() && (thit->getHitType() == HitType::guessed))
        {
            // this is a guessed strip SP, push the first hit back and skip the next one since its a duplicate
            cleanHits.push_back(thit);
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
            cleanHits.push_back(thit);
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
    for (auto thit : cleanHits)
    {
        if(m_debugEvent) ATH_MSG_DEBUG(thit->getX()<<" "<<thit->getY()<<" "<<thit->getZ()<<" "<<thit->isStrip());
    }

    // Reverse this vector so we can encode it the format as expected from the NN
    std::reverse(hitsToEncode.begin(), hitsToEncode.end());

    if(m_debugEvent) ATH_MSG_DEBUG("Input for NN prediction");
    for (auto thit : hitsToEncode)
    {
        inputTensorValues.push_back(thit->getX()/ getXScale());
        inputTensorValues.push_back(thit->getY()/ getYScale());
        inputTensorValues.push_back(thit->getZ()/ getZScale());

        if(m_debugEvent) ATH_MSG_DEBUG(thit->getX()<<" "<<thit->getY()<<" "<<thit->getZ());
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
    outputTensorValues[0] *= getXScale();
    outputTensorValues[1] *= getYScale();
    outputTensorValues[2] *= getZScale();

    return StatusCode::SUCCESS;
}

StatusCode FPGATrackSimNNPathfinderExtensionTool::addHitToRoad(FPGATrackSimRoad& newroad, FPGATrackSimRoad& currentRoad, const std::vector<std::shared_ptr<const FPGATrackSimHit>>& hits)
{
    newroad.setNLayers(m_nLayers_1stStage + m_nLayers_2ndStage);
    std::vector<std::vector<std::shared_ptr<const FPGATrackSimHit>>> these_hits;
    these_hits.resize(m_nLayers_1stStage + m_nLayers_2ndStage);
    for (auto &thishit : currentRoad.getHits_flat()) {
        these_hits[thishit->getLayer()].push_back(thishit);
    }

    // add this hit to the road, real or not
    for (auto& hit: hits)
    {
        if(hit->getLayer() >= these_hits.size())
        {
            ATH_MSG_WARNING("Adding a hit with layer: "<<hit->getLayer()<<" which is beyond the total size of the layers");
            continue;
        }
        these_hits[hit->getLayer()].push_back(hit);
    }
    const auto nTheseHits = these_hits.size();
    newroad.setHits(std::move(these_hits));

    // Update the bitmasks depending on whether this was real or not
    unsigned wcLayers = currentRoad.getWCLayers();
    unsigned hitLayers = currentRoad.getHitLayers();

    for (auto& hit: hits)
    {
        if(hit->getLayer() > nTheseHits)
        {
            ATH_MSG_WARNING("Adding a hit with layer: "<<hit->getLayer()<<" which is beyond the total size of the layers");
            continue;
        }

        if (!hit->isReal()) { // add a WC hit
            wcLayers |= (0x1 << hit->getLayer());
        }
        else { // add a real hit
            hitLayers |= 1 << hit->getLayer();
        }
    }

    newroad.setHitLayers(hitLayers);
    newroad.setWCLayers(wcLayers);

    return StatusCode::SUCCESS;
}

StatusCode FPGATrackSimNNPathfinderExtensionTool::getFakeHit(FPGATrackSimRoad& currentRoad, size_t slice, std::vector<float>& predhit, const long& fineID, std::vector<std::shared_ptr<const FPGATrackSimHit>>& hits) {

    const FPGATrackSimRegionMap* rmap_2nd = m_FPGATrackSimMapping->SubRegionMap_2nd();

    int guessedLayer(0);

    std::shared_ptr<FPGATrackSimHit> guessedHitPtr = std::make_shared<FPGATrackSimHit>();
    guessedHitPtr->setX(predhit[0]);
    guessedHitPtr->setY(predhit[1]);
    guessedHitPtr->setZ(predhit[2]);

    if (m_doOutsideIn) { // outside in
        unsigned lastHitLayer(9999);
        double lastHitR(9999);

        for (auto &hit : currentRoad.getHits_flat())
        {
            int layer = hit->getLayer();
            double r = hit->getR();
            if (r < 5) {// happens for guessed hits from pattern reco
                r = rmap_2nd->getAvgRadius(slice,layer); // use avg hit radius instead just to find the ordering of hits
            }
            if (r < lastHitR) {
                lastHitLayer = layer;
                lastHitR = r;
            }
        }

        if (lastHitLayer == 0) { // this is the inner most 1st stage, so guess the last layer in 2nd stage
            guessedLayer = m_nLayers_2ndStage-1;
        }
        else if (lastHitLayer < m_nLayers_1stStage) {
            guessedLayer = lastHitLayer-1; // still in 1st stage, go in by one
        }
        else {
            guessedLayer = lastHitLayer-1; // in 2nd stage, go in by one
        }
    }
    else { // nope, inside out
        unsigned lastHitLayer(0);

        for (auto &hit : currentRoad.getHits_flat())
        {
            unsigned layer = hit->getLayer();
            if (layer > lastHitLayer)
            {
                lastHitLayer = layer;
            }
        }
        guessedLayer = lastHitLayer+1;
    }

    guessedHitPtr->setLayer(guessedLayer);
    guessedHitPtr->setHitType(HitType::guessed);

    if(isFineIDInStrip(fineID))  guessedHitPtr->setDetType(SiliconTech::strip);
    else  guessedHitPtr->setDetType(SiliconTech::pixel);


    // Make sure that the hit is inside the boundaries of the layers
    if(guessedHitPtr->getLayer() < m_nLayers_1stStage + m_nLayers_2ndStage )
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
        guessedSecondHitPtr->setLayer( guessedHitPtr->getLayer() + 1 );
        guessedSecondHitPtr->setHitType(HitType::guessed);

        guessedSecondHitPtr->setDetType(SiliconTech::strip);

        // Make sure that the hit is inside the boundaries of the layers
        if(guessedSecondHitPtr->getLayer() < m_nLayers_1stStage + m_nLayers_2ndStage )
        {
            hits.push_back(guessedSecondHitPtr);
        }
    }



    return StatusCode::SUCCESS;

}

StatusCode FPGATrackSimNNPathfinderExtensionTool::findHitinNextStripLayer(std::shared_ptr<const FPGATrackSimHit> hitToSearch, std::vector<std::shared_ptr<const FPGATrackSimHit>>& hitList, std::vector<std::shared_ptr<const FPGATrackSimHit>>& hits)
{
    float EPSILON = 0.00001;
    for (const std::shared_ptr<const FPGATrackSimHit>& hit: hitList)
    {
        if (abs(hit->getX() - hitToSearch->getX()) < EPSILON && abs(hit->getY() - hitToSearch->getY()) < EPSILON && abs(hit->getZ() - hitToSearch->getZ()) < EPSILON)
        {
            hits.push_back(hit);
            return StatusCode::SUCCESS;
        }

    }

    ATH_MSG_WARNING("Didn't find a matching space point");

    return StatusCode::FAILURE;
}

void FPGATrackSimNNPathfinderExtensionTool::printRoad(FPGATrackSimRoad& currentRoad)
{
    if(!m_debugEvent) return;

    // print this road
    if(m_debugEvent)
    {
        std::vector<std::shared_ptr<const FPGATrackSimHit>> hitsR;
        for (auto &hit : currentRoad.getHits_flat()) {
            hitsR.push_back(hit);
        }

        // If outside in, sort in increasing R, otherwise, decreasing R
        if(m_doOutsideIn)
        {
            std::sort(hitsR.begin(), hitsR.end(), [](auto& a, auto& b){
                if(a->getR() == b->getR()) return a->getLayer() < b->getLayer();
                return a->getR() < b->getR();
            });
        }
        else
        {
            std::sort(hitsR.begin(), hitsR.end(), [](auto& a, auto& b){
                if(a->getLayer() == b->getLayer()) return a->getR() > b->getR();
                return a->getLayer() > b->getLayer();
            });
        }

        for (unsigned long i = 0; i < hitsR.size(); i++)
        {
            ATH_MSG_DEBUG("Hit i "<<i<<" X: "<<hitsR[i]->getX()<<" Y: "<<hitsR[i]->getY()<<" Z: "<<hitsR[i]->getZ()<<" R: "<<hitsR[i]->getR()<<" layer: "<<hitsR[i]->getLayer()<<" hitType: "<<hitsR[i]->getHitType()<<" getDetType: "<<hitsR[i]->getDetType());
        }
    }

}


StatusCode FPGATrackSimNNPathfinderExtensionTool::getLastLayer(FPGATrackSimRoad& currentRoad, unsigned& lastHitLayer, std::shared_ptr<const FPGATrackSimHit>& lastHit)
{
    // Reset to inner most
    lastHitLayer = 0;

    for (auto &hit : currentRoad.getHits_flat())
    {
        unsigned layer = hit->getLayer();
        if (layer > lastHitLayer)
        {
            lastHitLayer = layer;
            lastHit = hit;
        }
    }

    return StatusCode::SUCCESS;

}
