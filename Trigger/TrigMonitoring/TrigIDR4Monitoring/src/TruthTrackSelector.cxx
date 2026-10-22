/**
 **  @file  TruthTrackSelector.cxx
 **
 **  @brief a selector of truth particles, including the flags predicate 
 **         ancestor navigation creation
 **
 **   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **
 **/


#include "TruthParticleTraits.h"

#include "TruthTrackSelector.h"
#include "TruthUtils/HepMCHelpers.h"

#include "AncestorFinder.h"
#include "MsgMacros.h"


IDTPM2::TruthTrackSelector::TruthTrackSelector( const std::string& name ) : GenericTrackSelector( name ) { }


template<typename T> 
bool isHeavyFlavour( const T* t ) {
  return t->isBottomHadron() || t->isCharmHadron();
}


StatusCode IDTPM2::TruthTrackSelector::initialize() {

  ATH_CHECK( GenericTrackSelector::initialize() );

  if ( !m_truthTool.empty() ) ATH_CHECK( m_truthTool.retrieve() );
  //  ATH_CHECK( m_trackTruthOriginTool.retrieve() );

  /// we can only have one of these, need the counter logic to make sure 
  /// the we aren't requesting too many ...

  int findercount = 0;

  using HadronFinder    = HadronAncestorFinder<xAOD::TruthParticle>;
  using PdgFinder       = PdgAncestorFinder<xAOD::TruthParticle>;
  using PredicateFinder = PredicateAncestorFinder<xAOD::TruthParticle>;

  if ( m_fromHadronPdgId==MC::BQUARK ) { m_ancestorFinder = std::make_unique<HadronFinder>( &xAOD::TruthParticle::isBottomHadron ); findercount++; } 
  if ( m_fromHadronPdgId==MC::CQUARK ) { m_ancestorFinder = std::make_unique<HadronFinder>( &xAOD::TruthParticle::isCharmHadron );  findercount++; } 
  if ( isset(m_fromPdgId) )            { m_ancestorFinder = std::make_unique<PdgFinder>( m_fromPdgId.value() ); findercount++; }
  if ( m_isFromHeavyFlavour || m_isFromLightFlavour ) { m_ancestorFinder = std::make_unique<PredicateFinder>( isHeavyFlavour<xAOD::TruthParticle> ); findercount++; }

  if ( findercount>1 ) {
    MY_MSG_ERROR << "too many Truth selections requested" << endmsg;
  }

  return StatusCode::SUCCESS;
}



bool IDTPM2::TruthTrackSelector::selectTrack( const TrackView& t ) const {

  const xAOD::TruthParticle* truth = t.as<xAOD::TruthParticle>();

  if ( !truth ) return false;

  /// Baseline selection, via AthTruthSelectionTool
  if ( m_truthTool && !m_truthTool->accept( truth ) )  return false;

  if ( !GenericTrackSelector::selectTrack(t) ) return false;
  
  /// in fact all these opnly make sense on a Truth Particle, so
  /// could just use the truth->... directly for these

  if (  std::abs(truth->pdgId())!=m_pdgId ) return false;

  if (  m_isHadron && !truth->isHadron() )  return false;

  if ( m_isPion ) {
    int pdgid = std::abs(truth->pdgId());
    if ( pdgid!=111 && pdgid!=211 ) return false;
  }
  
  /// NB: doesn't do the pt selection in a helper, but we just 
  ///     define another helper for that, but here just add the two if statements,
  /// - is still only  six lines.
  if ( m_ancestorFinder ) {
    const xAOD::TruthParticle* parent = m_ancestorFinder->findAncestor( truth );
    if ( m_isFromLightFlavour && parent ) return false;
    if ( !parent ) return false;
    if (  parent->pt()<m_minParentPt )  return false;
    if (  parent->pt()>m_maxParentPt )  return false;
  }
  
  return true;
}



