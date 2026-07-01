/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/*
 * eflowCaloObjectMaker.cxx
 *
 *  Created on: Apr 16, 2015
 *      Author: zhangrui
 */

#include "eflowCaloObjectMaker.h"

#include "eflowRecCluster.h"
#include "eflowRecTrack.h"
#include "eflowTrackClusterLink.h"
#include "eflowCaloObject.h"

#include <vector>


unsigned int eflowCaloObjectMaker::makeTrkCluCaloObjects(eflowRecTrackContainer* eflowTrackContainer, eflowRecClusterContainer* eflowClusterContainer, eflowCaloObjectContainer* caloObjectContainer) {
  std::vector<eflowRecTrack*> tracksToRecover;
  std::vector<eflowRecCluster*> clustersToConsider;
  for (unsigned int iTrack=0; iTrack<eflowTrackContainer->size(); ++iTrack) {
    eflowRecTrack *thisEflowRecTrack = static_cast<eflowRecTrack*>(eflowTrackContainer->at(iTrack));
    tracksToRecover.push_back(thisEflowRecTrack);
  }
  for (unsigned int iCluster = 0; iCluster < eflowClusterContainer->size(); ++iCluster) {
    eflowRecCluster* thisEFRecCluster = eflowClusterContainer->at(iCluster);
    clustersToConsider.push_back(thisEFRecCluster);
  }

  return makeTrkCluCaloObjects(tracksToRecover, clustersToConsider, caloObjectContainer);

}

unsigned int eflowCaloObjectMaker::makeTrkCluCaloObjects(std::vector<eflowRecTrack*>& tracksToRecover, std::vector<eflowRecCluster*>& clustersToConsider, eflowCaloObjectContainer* caloObjectContainer) {
  unsigned int result(0);

  /* Create all eflowCaloObjects that contain only eflowRecTracks and cache eflowRecTracks matched with cluster */
  std::vector<eflowRecTrack*> tracksToConsider;
  unsigned int nTrack = tracksToRecover.size();
  for (unsigned int iTrack=0; iTrack<nTrack; ++iTrack) {
    eflowRecTrack *thisEflowRecTrack = static_cast<eflowRecTrack*>(tracksToRecover.at(iTrack));
    if(thisEflowRecTrack->getClusterMatches().empty()) {
      std::unique_ptr<eflowCaloObject> calob = std::make_unique<eflowCaloObject>();
      calob->addTrack(thisEflowRecTrack);
      caloObjectContainer->push_back(std::move(calob));
      result++;
    } else {
      tracksToConsider.push_back(thisEflowRecTrack);
    }
  }

  /* Create all eflowCaloObjects that contain only eflowRecCluster*/
  unsigned int nClusters = clustersToConsider.size();
  for (unsigned int iCluster = 0; iCluster < nClusters; ++iCluster) {
    eflowRecCluster* thisEFRecCluster = clustersToConsider.at(iCluster);
    if(thisEFRecCluster->getTrackMatches().empty()) {
      std::unique_ptr<eflowCaloObject> thisEflowCaloObject = std::make_unique<eflowCaloObject>();
      thisEflowCaloObject->addCluster(thisEFRecCluster);
      caloObjectContainer->push_back(std::move(thisEflowCaloObject));
      result++;
    }
  }

  // It is possible an event has no tracks to consider 
  if (!tracksToConsider.empty()){

    /* one loop creates one CaloObject */
    do {
      /* temporary vector for next loop */
      std::vector<eflowRecTrack*> trackForNextLoop;
      std::vector<eflowRecCluster*> clusterForNextLoop;
      
      /* track/cluster vector for CaloObject */
      std::vector<eflowRecTrack*> trackList;
      std::vector<eflowRecCluster*> clusterList;
      
      /* initiallize trackForNextLoop by the first track in tracksToConsider */
      trackForNextLoop.push_back(tracksToConsider.at(0));
      trackList.insert(trackList.end(), trackForNextLoop.begin(), trackForNextLoop.end());

      unsigned int time(0);
      /* iteratively searching tracks and clusters those belong to one CaloObject */
      do {
	clusterForNextLoop = uniqCluster(trackForNextLoop, clusterList);
	if(clusterForNextLoop.empty()) break;
	clusterList.insert(clusterList.end(), clusterForNextLoop.begin(), clusterForNextLoop.end());
	trackForNextLoop = uniqTrack(clusterForNextLoop, trackList);
	if (trackForNextLoop.empty()) break;
	trackList.insert(trackList.end(), trackForNextLoop.begin(), trackForNextLoop.end());
      } while (++time);
      
      /* store this eflowCaloObject */
      std::unique_ptr<eflowCaloObject> thisEflowCaloObject = std::make_unique<eflowCaloObject>();
      thisEflowCaloObject->addTracks(trackList);
      
      thisEflowCaloObject->addClusters(clusterList);
      for(auto *itr_cluster : clusterList) {
	thisEflowCaloObject->addTrackClusterLinks(itr_cluster->getTrackMatches());
      }
      caloObjectContainer->push_back(std::move(thisEflowCaloObject));
      
      /* update tracksToConsider */
      updateTracksToConsider(tracksToConsider, trackList);
      if(tracksToConsider.empty()) {
	++result;
	break;
      }

    } while (++result);
  }//If tracksToCosider is not empty


  return result;
}

std::vector<eflowRecCluster*> eflowCaloObjectMaker::uniqCluster(const std::vector<eflowRecTrack*>& trackForNextLoop, const std::vector<eflowRecCluster*>& clusterList) {
  std::vector<eflowRecCluster*> result;
  std::vector<int> allClusterId;
  for(auto *itr_track : trackForNextLoop) {
    const std::vector<eflowTrackClusterLink*>& allLinks = itr_track->getClusterMatches();
    for(auto *allLink : allLinks) {
      int id = allLink->getCluster()->getClusterId();
      bool notInList(true);
      for (auto *itr_cluster : clusterList) {
        if (id == itr_cluster->getClusterId()) {
          notInList = false;
          break;
        }
      }
      if ((find(allClusterId.begin(), allClusterId.end(), id) == allClusterId.end()) && notInList) {
        allClusterId.push_back(id);
        result.push_back(allLink->getCluster());
      } else {
        continue;
      }
    }
  }
  return result;
}

std::vector<eflowRecTrack*> eflowCaloObjectMaker::uniqTrack(const std::vector<eflowRecCluster*>& clusterForNextLoop, const std::vector<eflowRecTrack*>& trackList) {
  std::vector<eflowRecTrack*> result;
  std::vector<int> allTrackId;
  for(auto *itr_cluster : clusterForNextLoop) {
    std::vector<eflowTrackClusterLink*> allLinks = itr_cluster->getTrackMatches();
    for(auto *allLink : allLinks) {
      int id = allLink->getTrack()->getTrackId();
      bool notInList(true);
      for (auto *itr_track : trackList) {
        if (id == itr_track->getTrackId()) {
          notInList = false;
          break;
        }
      }
      if ((find(allTrackId.begin(), allTrackId.end(), id) == allTrackId.end()) && notInList) {
        allTrackId.push_back(id);
        result.push_back(allLink->getTrack());
      } else {
        continue;
      }
    }
  }
  return result;
}

void eflowCaloObjectMaker::updateTracksToConsider(std::vector<eflowRecTrack*>& total, const std::vector<eflowRecTrack*>& part) {
  unsigned int nPart = part.size();
  for(unsigned int i=0; i<nPart; ++i) {
    std::vector<eflowRecTrack*>::iterator itr = total.begin();
    while (itr != total.end()) {
      if ((*itr)->getTrackId() == part.at(i)->getTrackId()) { total.erase(itr); break; }
      ++itr;
    }
  }
}
