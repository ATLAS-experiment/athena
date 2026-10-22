/**
 **   @file  TrackSelector.cxx
 **
 **   @date    Sun 26 Jul 2026 10:22:51 BST
 **
 **   @class TrackSelector
 **   @brief Uses the GenericSelectorTool and the OfflineSelectionTool
 **          and working points defined there for track selection (how 
 **          working points if we have to specify all the parameters?) 
 **
 **  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **/

#include "xAODTracking/TrackParticleContainer.h"
#include "xAODEgamma/ElectronContainer.h"

#include "TrackSelector.h"

/// the functions in find_bin are not nice
/// and need to be tidied up
#include "find_bin.h"

IDTPM2::TrackSelector::TrackSelector( const std::string& name )
  : GenericTrackSelector( name ) { }



StatusCode IDTPM2::TrackSelector::initialize() {
  ATH_CHECK( GenericTrackSelector::initialize() );  
  if ( m_useOfflineTool ) ATH_CHECK( m_offlineTool.retrieve() );
  return StatusCode::SUCCESS;
}


/// do we really need these ?? I mean, like really ?
/// all they do is return a bool if it is less than some value,
/// these make like more complicated that just putting in the
/// lines directly
/// you could get round these with just a single function,
/// just by passing in the value to be tested as well

template<typename T, typename U=T> 
bool select_min( const T& x, const std::vector<T>& xbins,
		 const U& y, const std::vector<U>& ycuts ) { return y>=ycuts.at( IDTPM2::bins::find_bin( xbins, x ) ); }

template<typename T, typename U=T> 
bool select_max( const T& x, const std::vector<T>& xbins,
		 const U& y, const std::vector<U>& ycuts ) { return y<=ycuts.at( IDTPM2::bins::find_bin( xbins, x ) ); }


  
bool IDTPM2::TrackSelector::selectTrack( const TrackView& t ) const {

  if ( m_useOfflineTool ) {
    /// is it a TrackParticle ...
    const xAOD::TrackParticle* track = t.as<xAOD::TrackParticle>();
    
    if ( !track ) {
      /// or an Electron ...
      const xAOD::Electron* electron = t.as<xAOD::Electron>();
      if ( electron ) track = electron->trackParticle();
      else return false;
    }
    
    /// Baseline selection, via InDetTrackSelectionTool
    if ( ! m_offlineTool->accept( track ) ) return false; // TODO vertex needs to be provided here
  }
  
  /// this was all changed before the TrackView, but can leave this as it for the
  /// time being ...
  /// Customised selections
  if ( !GenericTrackSelector::selectTrack(t) ) return false;

  /// this is *really* nasty code, has this "etaBins" which are just a vector,
  /// with loads of accessors and lambdas. Why not make it a proper class ?? 
  if ( !m_etaBins.empty() ) {
    /// WTF ???
    // if (!m_minHitsVec.empty() && !nHitsSelVec(*track, m_minHitsVec, m_etaBins))  return false;
    // if (!m_minPtVec.empty() && !minPtSelVec(*track, m_minPtVec, m_etaBins))  return false;
    // if (!m_maxD0Vec.empty() && !maxD0SelVec(*track, m_maxD0Vec, m_etaBins))  return false;
    // if (!m_maxZ0Vec.empty() && !maxZ0SelVec(*track, m_maxZ0Vec, m_etaBins))  return false;

    //  if (!m_minHitsVec.empty() && !vector_select( track, m_minHitsVec, m_etaBins))  return false;
    //  if (!m_minPtVec.empty() && !minPtSelVec(*track, m_minPtVec, m_etaBins))  return false;
    //  if (!m_maxD0Vec.empty() && !maxD0SelVec(*track, m_maxD0Vec, m_etaBins))  return false;
    //  if (!m_maxZ0Vec.empty() && !maxZ0SelVec(*track, m_maxZ0Vec, m_etaBins))  return false;

    /// signed PT ??
    /// all this stuff should go into a class, then we can
    /// define operators for >, >=, <, <= etc
    /// then we can just ask if ( t->pt < binclass(t->eta()) ) { ...
    /// such that given a value, it returns the corresponding cut
    /// for that value, then life would be, so, so much easier
    /// that with this stuff  
    if ( !m_minHitsVec.empty() && !select_min<float,unsigned>( t.eta(), m_etaBins, t.nSi(), m_minHitsVec.value() ))  return false;
    if ( !m_minPtVec.empty()   && !select_min<float>( t.eta(), m_etaBins, t.pt(),  m_minPtVec.value() ))    return false;
    if ( !m_maxZ0Vec.empty()   && !select_max<float>( t.eta(), m_etaBins, std::fabs(t.z0()), m_maxZ0Vec.value() ))  return false;
    if ( !m_maxD0Vec.empty()   && !select_max<float>( t.eta(), m_etaBins, std::fabs(t.d0()), m_maxD0Vec.value() ))  return false;

  }
  
  return true;
}

