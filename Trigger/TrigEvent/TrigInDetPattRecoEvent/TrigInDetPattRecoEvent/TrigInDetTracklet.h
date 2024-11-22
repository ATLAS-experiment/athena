/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGINDETPATTRECOEVENT_TRIGINDETTRACKLET_H
#define TRIGINDETPATTRECOEVENT_TRIGINDETTRACKLET_H

#include <vector>

#include "TrkSpacePoint/SpacePoint.h"

/** @class TrigInDetTracklet  
defines output data structure for TrigInDetTrackSeedingTool
*/

class TrigInDetTracklet {

 public:
  
 TrigInDetTracklet(float Q) : m_Q(Q) {};

  const std::vector<const Trk::SpacePoint*>& seed() const {
    return m_vSP;
  }

  float quality() const {
    return m_Q;
  }

  void addSpacePoint(const Trk::SpacePoint* pSP) {
    m_vSP.push_back(pSP);
  }

 protected:
  
  std::vector<const Trk::SpacePoint*> m_vSP;
  float m_Q;

};


#endif
