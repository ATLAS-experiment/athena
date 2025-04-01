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
    const FPGATrackSimRegionMap* rmap_2nd = m_FPGATrackSimMapping->SubRegionMap_2nd();

    // Create one "tower" per slice for this event.
    // Note that there now might be only one "slice", at least for the time being.
    if (m_slicedHitHeader) {
      for (int ireg = 0; ireg < rmap_2nd->getNRegions(); ireg++) {
	FPGATrackSimTowerInputHeader tower = FPGATrackSimTowerInputHeader(ireg);
	m_slicedHitHeader->addTower(tower);
      }
    }
    // JAA need to update this
    // for (const std::shared_ptr<const FPGATrackSimHit>& hit : hits) {
    //   // Also Store a copy of the hit object in the header class, for ROOT Output + TV creation.
    //   if (m_slicedHitHeader) m_slicedHitHeader->getTower(i)->addHit(*hit);
    // }
    if(m_debugEvent) ATH_MSG_DEBUG("Got: "<<tracks.size()<<" tracks to extrapolate");

    // Now, loop over the tracks.
    for (std::shared_ptr<const FPGATrackSimTrack> track : tracks)
    {
        if(m_debugEvent) ATH_MSG_DEBUG("\033[1;31m-------------------------- extraploating Track ------------------ \033[0m");

        if (track->passedOR() == 0) {
            continue;
        }

        const std::vector<FPGATrackSimHit> hitsOnTrack = track->getFPGATrackSimHits();
	miniRoad road;	
	float pt = track->getPt();
        for (const auto &thit : hitsOnTrack)
        {	  
	  road.addHit(std::make_shared<const FPGATrackSimHit>(thit)); // add all hits, we check if WC later
        }
        if(m_debugEvent)
        {
            ATH_MSG_DEBUG("-----------------Hits in event");
	    for (const std::shared_ptr<const FPGATrackSimHit>& hit: hits)
	      {
		ATH_MSG_DEBUG("Hit "<<" X: "<<hit->getX()<<" Y: "<<hit->getY()<<" Z: "<<hit->getZ()<<" R: "<<hit->getR()<<" hitType: "<<hit->getHitType()<<" getDetType: "<<hit->getDetType());
	      }
        }
        std::vector<miniRoad> roadsToExtrapolate;
        roadsToExtrapolate.push_back(road);

        std::vector<miniRoad> completedRoads;

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
	    miniRoad currentRoad = *roadsToExtrapolate.begin();
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
            if (currentRoad.getNHits() >= (m_nLayers_1stStage+m_nLayers_2ndStage))
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
            if(fineID == 215){
                ATH_MSG_DEBUG("Stopping condition reached");
                completedRoads.push_back(currentRoad);
                break;
            }

            // Get the last layer and hit in the road
            unsigned lastLayerInRoad = 0;
            std::shared_ptr<const FPGATrackSimHit> lastHit;
            if(!getLastLayer(currentRoad, lastLayerInRoad, lastHit) or !lastHit)
            {
                ATH_MSG_WARNING("Failed to find last layer this road");
                continue;
            }
	    unsigned layer = lastLayerInRoad+1; // the layer we're looking to find
	    bool lastHitWasReal = lastHit->isReal();
	    float lastHitR = lastHit->getR();
            if(layer >= (m_nLayers_1stStage + m_nLayers_2ndStage))
            {
                completedRoads.push_back(currentRoad);
                continue;
            }

	    unsigned int hitsInWindow = 0;
	    
	    // List of all the hits, with their distances to the predicted point
	    std::vector<std::vector<std::shared_ptr<const FPGATrackSimHit>>> listofHitsFound;

	    for (const std::shared_ptr<const FPGATrackSimHit>& hit: hits) 
	      {
		if (m_doOutsideIn && (hit->getR() > lastHitR)) continue;
		if (!m_doOutsideIn && (hit->getR() < lastHitR)) continue;
		if ((hit->getHitType() == HitType::spacepoint) && ((hit->getPhysLayer()) %2 == 1)) continue; // ignore outer parts of SP, they get added separately
		if(m_debugEvent)
		  {
		    ATH_MSG_DEBUG("In the hit loop hit at: "<<hit->getX() << " " << hit->getY() << " " << hit->getZ());
		  }
		
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
			    if (hit->getHitType() == HitType::spacepoint) {
			      // find the hit in the next layer
			      if(!findHitinNextStripLayer(hit,  hits, theseHits))
				{
				  ATH_MSG_WARNING("For a SP in layer "<<layer<<" Couldn't find a matching strip SP");
				}
			    }
			    else {
			      std::shared_ptr<FPGATrackSimHit> guessedSecondHitPtr = std::make_shared<FPGATrackSimHit>();
			      guessedSecondHitPtr->setX(0);
			      guessedSecondHitPtr->setY(0);
			      guessedSecondHitPtr->setZ(0);
			      guessedSecondHitPtr->setPhysLayer(lastHit->getPhysLayer()+1);
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
		miniRoad newRoad;
		if(!addHitToRoad(newRoad, currentRoad, std::move(hitsFound)))
		  {
		    ATH_MSG_WARNING("Failed to make a new road");
		    continue;
		  }
		roadsToExtrapolate.push_back(newRoad);
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
		    printRoad(newRoad);
		  }
	      }
	    if (hitsInWindow != 0) m_nHitsInSearchWindow.push_back(hitsInWindow);
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
                    if (!getFakeHit(currentRoad, predhit, fineID, theseHits)) {
                        ATH_MSG_WARNING("Failed adding a guessed hit in extrapolation");
                        continue;
                    }

                    if((isFineIDInPixel(fineID) && theseHits.size() != 1) || (isFineIDInStrip(fineID) && theseHits.size() != 2))
		      {
                        continue;
		      }
		    
                    // add the hit to the road
                    miniRoad newroad;
		    
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
        // This track has been extrapolated, copy the completed tracks to the full list with full road objects
        for (const auto &miniroad : completedRoads) {
	  FPGATrackSimRoad road;
	  road.setNLayers(miniroad.getNLayers());
	  road.setWCLayers(miniroad.getWCLayers());
	  road.setHitLayers(miniroad.getHitLayers());
	  m_missingHitsOnRoad.push_back(miniroad.getNWCLayers());
	  road.setRoadID(m_roads.size() - 1);
	  // Set the "Hough x" and "Hough y" using the track parameters.
	  road.setX(track->getPhi());
	  road.setY(track->getQOverPt());
	  road.setXBin(track->getHoughXBin());
	  road.setYBin(track->getHoughYBin());
	  road.setSubRegion(track->getSubRegion());
	  road.setHits(miniroad.getVecHits());
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

StatusCode FPGATrackSimNNPathfinderExtensionTool::fillInputTensorForNN(miniRoad& thisroad, std::vector<float>& inputTensorValues)
{
    std::vector<std::shared_ptr<const FPGATrackSimHit>> hitsR;

    std::vector<std::shared_ptr<const FPGATrackSimHit>> hits = thisroad.getHits();
    
      
    for (unsigned ihit = 0; ihit < hits.size(); ihit++) {
      if (ihit < m_nLayers_1stStage && !(hits[ihit]->isReal())) continue; // skip guessed hits for the 1st stage      
       hitsR.push_back(hits[ihit]);
    }
    // Sort in increasing R. We will reverise it for inside out after the cleanup
    std::sort(hitsR.begin(), hitsR.end(), [](auto& a, auto& b){
        return a->getR() < b->getR();
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

StatusCode FPGATrackSimNNPathfinderExtensionTool::addHitToRoad(miniRoad& newroad, miniRoad& currentRoad, const std::vector<std::shared_ptr<const FPGATrackSimHit>>& hits)
{
  newroad.setHits(currentRoad.getHits());
  newroad.addHits(hits);
  return StatusCode::SUCCESS;
}

StatusCode FPGATrackSimNNPathfinderExtensionTool::getFakeHit(miniRoad& currentRoad, std::vector<float>& predhit, const long& fineID, std::vector<std::shared_ptr<const FPGATrackSimHit>>& hits) {

  unsigned guessedLayer(0);

    std::shared_ptr<FPGATrackSimHit> guessedHitPtr = std::make_shared<FPGATrackSimHit>();
    guessedHitPtr->setX(predhit[0]);
    guessedHitPtr->setY(predhit[1]);
    guessedHitPtr->setZ(predhit[2]);

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

StatusCode FPGATrackSimNNPathfinderExtensionTool::findHitinNextStripLayer(std::shared_ptr<const FPGATrackSimHit> hitToSearch, const std::vector<std::shared_ptr<const FPGATrackSimHit>>& hitList, std::vector<std::shared_ptr<const FPGATrackSimHit>>& hits)
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

void FPGATrackSimNNPathfinderExtensionTool::printRoad(miniRoad& currentRoad)
{
    if(!m_debugEvent) return;

    // print this road
    if(m_debugEvent)
    {
        std::vector<std::shared_ptr<const FPGATrackSimHit>> hitsR;
        for (auto &hit : currentRoad.getHits()) {
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
                return a->getR() > b->getR();
            });
        }

        for (unsigned long i = 0; i < hitsR.size(); i++)
        {
            ATH_MSG_DEBUG("Hit i "<<i<<" X: "<<hitsR[i]->getX()<<" Y: "<<hitsR[i]->getY()<<" Z: "<<hitsR[i]->getZ()<<" R: "<<hitsR[i]->getR()<< " hitType: "<<hitsR[i]->getHitType()<<" getDetType: "<<hitsR[i]->getDetType());
        }
    }

}

StatusCode FPGATrackSimNNPathfinderExtensionTool::getLastLayer(miniRoad& currentRoad, unsigned& lastHitLayer, std::shared_ptr<const FPGATrackSimHit>& lastHit)
{
    lastHitLayer = currentRoad.getNHits()-1;
    lastHit = currentRoad.getHit(lastHitLayer);
    return StatusCode::SUCCESS;

}
