/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ForwardTrack_H
#define ForwardTrack_H

#include "Beamline.h"
#include "ConfigData.h"
#include "Particle.h"
#include "Point.h"

class ForwardTrack {

 public:
  
  ForwardTrack();
  
  void initialize(const ForwardTracker::ConfigData&);
  
  bool TrackParticle(ForwardTracker::Particle);

  inline const ForwardTracker::Point&    fPos() { return m_fPosition; }
  inline const ForwardTracker::Point&    fMom() { return m_fMomentum; }
  inline const ForwardTracker::Particle& fPar() { return m_fParticle; }

 private:

  ForwardTracker::Beamline m_fBeamline1;
  ForwardTracker::Beamline m_fBeamline2;
  ForwardTracker::Point    m_fPosition;
  ForwardTracker::Point    m_fMomentum;
  ForwardTracker::Particle m_fParticle;
};

#endif
