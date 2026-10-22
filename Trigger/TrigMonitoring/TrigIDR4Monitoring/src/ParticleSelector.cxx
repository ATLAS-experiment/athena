/**
 **   @file  ParticleSelector.cxx
 **
 **   @date    Sun 16 Aug2026 21:53:51 BST
 **
 **   @class ParticleSelector
 **   @brief Uses the TrackSelector and TruthTrackSelector to create 
 **          a unified track selectfor
 **
 **  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **/


#include "ParticleSelector.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTruth/TruthParticle.h"



IDTPM2::ParticleSelector::ParticleSelector( const std::string& name )
  : GenericTrackSelector(name),
    TrackSelector(name),
    TruthTrackSelector(name) { }


StatusCode IDTPM2::ParticleSelector::initialize() {
  ATH_CHECK( GenericTrackSelector::initialize() );  
  ATH_CHECK( TrackSelector::initialize() );  
  ATH_CHECK( TruthTrackSelector::initialize() );  
  return StatusCode::SUCCESS;
}


bool IDTPM2::ParticleSelector::selectTrack( const TrackView& t ) const {
  if ( t.is<xAOD::TrackParticle>() ) return TrackSelector::selectTrack( t );
  if ( t.is<xAOD::TruthParticle>() ) return TruthTrackSelector::selectTrack( t );
  return GenericTrackSelector::selectTrack( t );
}

