/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/*
 * FPGATrackSimRoad_test.cxx: Unit tests for FPGATrackSimRoad and FPGATrackSimHit
 */

#include <cstdio>
#include <iostream>
#include <cassert>
#include <memory>
#include "FPGATrackSimObjects/FPGATrackSimRoad.h"
//coverity[UNCAUGHT_EXCEPT]
int main(int, char**)
{

  const int roadid(8675309);

  std::vector<std::vector<std::shared_ptr<const FPGATrackSimHit>>> hitVec;
  FPGATrackSimHit hits[8];
  for (unsigned i = 0; i < 8; i++) {
    hits[i].setPhiIndex(i*i);
    hits[i].setEtaIndex(i+1);
    hits[i].setLayer(i);
    hits[i].setHitType(HitType::mapped);
    std::shared_ptr<const FPGATrackSimHit> hitP = std::make_shared<const FPGATrackSimHit>(hits[i]);
    std::vector<std::shared_ptr<const FPGATrackSimHit>> hitPVec;
    hitPVec.emplace_back(std::move(hitP));
    hitVec.emplace_back(std::move(hitPVec));
  }
  
  FPGATrackSimRoad road;
  road.setHits(std::move(hitVec));
  road.setRoadID(roadid);
  std::cout << "road id = " << road.getRoadID() << std::endl;
  for (unsigned i = 0; i < 8; i++) {
    const std::vector<std::shared_ptr<const FPGATrackSimHit>> returned_hits = road.getHits(i);
    std::cout << "layer = " << returned_hits[0]->getLayer() << " and phi = " << returned_hits[0]->getPhiIndex() << 
      " and eta = " << returned_hits[0]->getEtaIndex() << std::endl;
  }

  return 0;

}

