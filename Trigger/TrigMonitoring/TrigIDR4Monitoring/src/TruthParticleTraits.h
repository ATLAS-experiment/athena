/* emacs: this is -*- c++ -*- */
/**
 **   @file    TruthParticleTraits.h        
 **                   
 **   @author  sutt
 **   @date    Wed 11 Mar 2026 19:20:53 GMT
 **
 **   $Id: TruthParticleTraits.h, v0.0   Wed 11 Mar 2026 19:20:53 GMT sutt $
 **
 **   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **
 **/


#ifndef  TRUTHPARTICLETRAITS_H
#define  TRUTHPARTICLETRAITS_H

#include "xAODTruth/TruthParticle.h"

#include "TrackTraits.h"

#include <cmath>

/// specialise for the TruthParticle - only override the functions
/// that are different

template<>
struct TrackTraits<xAOD::TruthParticle> : public TrackTraitsBase<xAOD::TruthParticle> {

  /// are these TrackParticle specific variables really needed ?? 
  /// Surely if we need them then we should only be calling them 
  /// in some special object that explicitly wants TrackParticles 

  using Accessor = SG::ConstAccessor<float>;
    
  /// what are we going to do with photons ? For a track based analysis the charge should never by 0
  /// but if we are using signed pt / p etc this will likely cause issues down the line
  /// especially if we start to throw exceptions for divide by 0 errors
  static float theta(const xAOD::TruthParticle& t) { return 2*std::atan(std::exp(-t.eta())); }
  static float     E(const xAOD::TruthParticle& t) { return t.e(); }
  static float    ET(const xAOD::TruthParticle& t) { return t.e()*std::sin(theta(t)); }
  static float     p(const xAOD::TruthParticle& t) { return t.charge()*std::sqrt( t.px()*t.px() + t.py()*t.py() + t.pz()*t.pz() ); }

  static float     m(const xAOD::TruthParticle& t) { return t.m(); }

  /// these need to be explicitly instantiated, or else we want to
  /// raise an exception, so no point checking isAvailable()
  
  static float z0(const xAOD::TruthParticle& t) {
    static const Accessor acc("z0");
    return acc(t);
  }
  
  static float d0(const xAOD::TruthParticle& t) {
    static const Accessor acc("d0");
    return acc(t);
  }

  static float phi(const xAOD::TruthParticle& t) {
    static const Accessor acc("phi");
    return acc(t);
  }

  static float  qOverP(const xAOD::TruthParticle& t) {
    float p_ = p(t);
    if ( p_==0 ) throw std::runtime_error( "divide by 0 - track p() is 0" );
    return 1/p_;
  }
  
};


#endif  // TRUTHPARTICLETRAITS_H 










