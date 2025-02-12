// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration


/**
 * @file FPGATrackSimNNPathfinderExtensionTool.cxx
 * @author Ben Rosser - brosser@uchicago.edu
 * @date 2024/10/08
 * @brief Default track extension algorithm to produce "second stage" roads.
 * Much of this code originally written by Alec, ported/adapted to FPGATrackSim.
 */

#include "FPGATrackSimObjects/FPGATrackSimTypes.h"
#include "FPGATrackSimObjects/FPGATrackSimHit.h"
#include "FPGATrackSimBanks/FPGATrackSimSectorBank.h"

#include "FPGATrackSimAlgorithms/FPGATrackSimNNPathfinderExtensionTool.h"
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
    // Probably need to do something here.
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


    // Now, loop over the tracks.
    for (std::shared_ptr<const FPGATrackSimTrack> track : tracks)
    {
        if (track->passedOR() == 0) {
            continue;
        }

        const std::vector<FPGATrackSimHit> hitsOnTrack = track->getFPGATrackSimHits();

        FPGATrackSimRoad road;
        road.setNLayers(m_nLayers_2ndStage);


        size_t slice = track->getSubRegion();

        layer_bitmask_t hitLayers = 0;
        layer_bitmask_t wcLayers = 0;
        // Loop over all hits

        std::vector<std::vector<std::shared_ptr<const FPGATrackSimHit>>> road_hits;
        road_hits.resize(m_nLayers_1stStage + m_nLayers_2ndStage);
        for (auto &thit : hitsOnTrack) {
            // add this hit to the road, start with all of them
            road_hits[thit.getLayer()].push_back(std::make_shared<const FPGATrackSimHit>(thit));
            if (thit.isReal()) hitLayers |= 1 << thit.getLayer();
            else wcLayers |= 1 << thit.getLayer();
        }
        road.setHits(std::move(road_hits));
        road.setHitLayers(hitLayers);
        road.setWCLayers(wcLayers);	
        m_roads.push_back(road);


        std::vector<FPGATrackSimRoad> roadsToExtrapolate;
        roadsToExtrapolate.push_back(road);
        
        std::vector<FPGATrackSimRoad> completedRoads;

        while(roadsToExtrapolate.size() > 0)
        {
  	    if (m_maxBranches.value() >= 0 && (int)(roadsToExtrapolate.size()) >= m_maxBranches.value()) {
	         ATH_MSG_WARNING("We are at " << roadsToExtrapolate.size() << " roads to extroplate, bigger than maxbranches = " << m_maxBranches.value() << " so stopping");
      	        break;
    	    }
	      
	      
            FPGATrackSimRoad currentRoad = *roadsToExtrapolate.begin();
            // Erase this road from the vector
            roadsToExtrapolate.erase(roadsToExtrapolate.begin());

            // Check exit condition
            if (currentRoad.getHits_flat().size() == (m_nLayers_1stStage+m_nLayers_2ndStage))
            {
                completedRoads.push_back(currentRoad); 
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
                    continue; /// no NN prediction
                }
            }
            else
            {
                // Make sure we are not prediciting outside the outer most layer
                // if we are, road is done
                if (abs(predhit[0]) > 1000 || abs(predhit[1]) > 1000 || abs(predhit[2]) > 3000)
                { 
                    completedRoads.push_back(currentRoad); 
                    continue; /// no NN prediction	    
                }
            }


            // Now search for the hits
            bool foundhitForRoad = false;
            for (unsigned layer = m_nLayers_1stStage; layer < m_nLayers_2ndStage + m_nLayers_1stStage; layer++) 
            { // loop over layers, probably only one will be found
                for (const std::shared_ptr<const FPGATrackSimHit>& hit: m_phits_atLayer[slice][layer]) 
                { // loop over hits in that layer
                    if (getFineID(*hit) == fineID && hit->isReal()) 
                    { // a hit is in the right fine ID == layer
                        double hitz = hit->getZ();
                        double hitr = hit->getR();
                        double predr = sqrt(predhit[0]*predhit[0] + predhit[1] * predhit[1]);
                        double predz = predhit[2];
                        if (abs(hitr - predr) < m_windowR.value() && abs(hitz - predz) < m_windowZ.value()) 
                        {
                            // We got a hit, lets make a road
                            FPGATrackSimRoad newroad;
                            if(!addHitToRoad(newroad, currentRoad, hit))
                            {
                                ATH_MSG_WARNING("Failed to make a new road");
                                continue;
                            }

                            roadsToExtrapolate.push_back(newroad);
                            foundhitForRoad = true;

                            
                        }
                    }
                }
            }

            // If the hit wasn't found, push back the current prediction as a hit
            if (!foundhitForRoad) 
            {  // did not find a hit to extrapolate to, check if we need to delete this road. if not, add a guessed hit if still useful
                if (currentRoad.getNWCLayers() == m_maxMiss) { 
                    // we don't want this road, so we continue
                    continue;
                }
                else 
                {
                    std::shared_ptr<FPGATrackSimHit> guessedHitPtr = std::make_shared<FPGATrackSimHit>();
                    // first make the fake hit that we will add
                    if (!getFakeHit(currentRoad, slice, predhit, guessedHitPtr)) {
                        ATH_MSG_WARNING("Failed make a guessed hit");
                        continue;
                    }

                    // add the hit to the road
                    FPGATrackSimRoad newroad;
                    if (!addHitToRoad(newroad, currentRoad, guessedHitPtr)) {
                        ATH_MSG_WARNING("Failed make a new road with fake hit");
                        continue;
                    }

                    roadsToExtrapolate.push_back(newroad);
                }
            }
        }


        // This track has been extrapolated, copy the completed tracks to the full list
        for (auto road : completedRoads) {
            road.setRoadID(roads.size() - 1);	
            // Set the "Hough x" and "Hough y" using the track parameters.
            road.setX(track->getPhi());
            road.setY(track->getQOverPt());
            road.setXBin(track->getHoughXBin());
            road.setYBin(track->getHoughYBin());
            road.setSubRegion(track->getSubRegion());	  
            m_roads.push_back(road);
        }
    }

    // Copy the roads we found into the output argument and return success.
    roads.reserve(m_roads.size());
    for (FPGATrackSimRoad & r : m_roads) {
        if (r.getNWCLayers() >= m_maxMiss) continue; // extra check on this
        roads.emplace_back(std::make_shared<const FPGATrackSimRoad>(r));
    }
    ATH_MSG_DEBUG("Found " << m_roads.size() << " new roads in second stage.");

    return StatusCode::SUCCESS;
}

StatusCode FPGATrackSimNNPathfinderExtensionTool::fillInputTensorForNN(FPGATrackSimRoad& thisroad, std::vector<float>& inputTensorValues)
{
    std::vector<std::shared_ptr<const FPGATrackSimHit>> hitsR;

    for (auto &hit : thisroad.getHits_flat()) {
        hitsR.push_back(hit);
    }


    // If outside in, sort in increasing R, otherwise, decreasing R
    if(m_doOutsideIn)
    {
        std::sort(hitsR.begin(), hitsR.end(), [](auto& a, auto& b){
            return a->getR() < b->getR();
        });
    }
    else
    {
        std::sort(hitsR.begin(), hitsR.end(), [](auto& a, auto& b){
            return a->getR() > b->getR();
        });  
    }

    // Remove all the duplicate space points
    std::vector<std::shared_ptr<const FPGATrackSimHit>> cleanHits;
    bool skipHit = false;
    for (auto thit : hitsR) 
    {
        if(!thit->isReal() && thit->getLayer() < m_nLayers_1stStage) // don't look at missing hits from 1st stage
        {
	  continue;
        }
    
        if(skipHit)
        {
            skipHit = false;
            continue;
        }
        if (!thit->isStrip()) 
        {
            cleanHits.push_back(thit);
        }
        else
        {
            // if its strips, push the first hit back and skip the next one since its a duplicate
            cleanHits.push_back(thit);
            skipHit = true;
        }

    }
    // Select the top N hits
    std::vector<std::shared_ptr<const FPGATrackSimHit>> hitsToEncode;
    std::copy(cleanHits.begin(), cleanHits.begin() + m_predictionWindowLength, std::back_inserter(hitsToEncode));

    // Reverse this vector so we can encode it the format as expected from the NN
    std::reverse(hitsToEncode.begin(), hitsToEncode.end());

    for (auto thit : hitsToEncode) 
    {
        inputTensorValues.push_back(thit->getX()/ getXScale());
        inputTensorValues.push_back(thit->getY()/ getYScale());
        inputTensorValues.push_back(thit->getZ()/ getZScale());

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

StatusCode FPGATrackSimNNPathfinderExtensionTool::addHitToRoad(FPGATrackSimRoad& newroad, FPGATrackSimRoad& currentRoad, const std::shared_ptr<const FPGATrackSimHit>&hit)
{
    newroad.setNLayers(m_nLayers_1stStage + m_nLayers_2ndStage);
    std::vector<std::vector<std::shared_ptr<const FPGATrackSimHit>>> these_hits;
    these_hits.resize(m_nLayers_1stStage + m_nLayers_2ndStage);
    for (auto &thishit : currentRoad.getHits_flat()) {
        these_hits[thishit->getLayer()].push_back(thishit);
    }

    // add this hit to the road, real or not
    these_hits[hit->getLayer()].push_back(hit);
    newroad.setHits(std::move(these_hits));

    // Update the bitmasks depending on whether this was real or not
    unsigned wcLayers = currentRoad.getWCLayers();
    unsigned hitLayers = currentRoad.getHitLayers();

    if (!hit->isReal()) { // add a WC hit
      wcLayers |= (0x1 << hit->getLayer());
    }
    else { // add a real hit
      hitLayers |= 1 << hit->getLayer();
    }
    
    newroad.setHitLayers(hitLayers);
    newroad.setWCLayers(wcLayers);

    return StatusCode::SUCCESS;
}

StatusCode FPGATrackSimNNPathfinderExtensionTool::getFakeHit(FPGATrackSimRoad& currentRoad, size_t slice, std::vector<float>& predhit, std::shared_ptr<FPGATrackSimHit> &guessedHitPtr) {

    const FPGATrackSimRegionMap* rmap_2nd = m_FPGATrackSimMapping->SubRegionMap_2nd();

    int guessedLayer(0);

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
	  if (r < 5) {             // happens for guessed hits from pattern reco
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
      unsigned lastHitLayer(9999);
      double lastHitR(0); 
      
      for (auto &hit : currentRoad.getHits_flat()) 
	{
	  int layer = hit->getLayer();
	  double r = hit->getR();
	  if (r < 5) {             // happens for guessed hits from pattern reco
	    r = rmap_2nd->getAvgRadius(slice,layer); // use avg hit radius instead just to find the ordering of hits
	  }
	  if (r > lastHitR) { 
	    lastHitLayer = layer;
	    lastHitR = r;
	  }
	}
      guessedLayer = lastHitLayer+1;
    }   
    
    guessedHitPtr->setLayer(guessedLayer); 
    guessedHitPtr->setHitType(HitType::guessed);

    return StatusCode::SUCCESS;

}
