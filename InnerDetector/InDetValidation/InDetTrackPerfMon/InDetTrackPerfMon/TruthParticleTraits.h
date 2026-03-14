/* emacs: this is -*- c++ -*- */
/**
 **   @file    TruthParticleTraits.h        
 **                   
 **   @author  sutt
 **   @date    Wed 11 Mar 2026 19:20:53 GMT
 **
 **   $Id: TruthParticleTraits.h, v0.0   Wed 11 Mar 2026 19:20:53 GMT sutt $
 **
 **   Copyright (C) 2026 sutt (sutt@cern.ch)    
 **
 **/


#ifndef  TRUTHPARTICLETRAITS_H
#define  TRUTHPARTICLETRAITS_H

#include "xAODTruth/TruthParticle.h"

#include "TrackTraits.h"

/// specialise for the TruthParticle - only override the functions
/// that are different

template<>
struct TrackTraits<xAOD::TruthParticle> : public TrackTraitsBase<xAOD::TruthParticle> {

  /// are these TrackParticle specific variables really needed ?? 
  /// Surely if we need them then we should only be calling them 
  /// in some special object that explicitly wants TrackParticles 
    
};



#endif  // TRUTHPARTICLETRAITS_H 










