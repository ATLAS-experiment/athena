/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file OfflineJetDecoratorAlg.cxx
 * @author Marco Aparo <marco.aparo@cern.ch>
 **/

/// Local includes
#include "OfflineJetDecoratorAlg.h"
#include "TrackParametersHelper.h"


///----------------------------------------
///------- Parametrized constructor -------
///----------------------------------------
IDTPM::OfflineJetDecoratorAlg::OfflineJetDecoratorAlg(
    const std::string& name,
    ISvcLocator* pSvcLocator ) :
  AthReentrantAlgorithm( name, pSvcLocator ) { }


///--------------------------
///------- initialize -------
///--------------------------
StatusCode IDTPM::OfflineJetDecoratorAlg::initialize() {

  ATH_CHECK( m_offlineTrkParticlesName.initialize(
                not m_offlineTrkParticlesName.key().empty() ) );

  ATH_CHECK( m_jetsName.initialize( not m_jetsName.key().empty() ) );

  /// Create decorations for ID tracks
  IDTPM::createDecoratorKeysAndAccessor( 
      *this, m_offlineTrkParticlesName,
      m_prefix.value(), m_decor_jet_names, m_decor_jet );

  if( m_decor_jet.size() != NDecorations ) {
    ATH_MSG_ERROR( "Incorrect booking of jet decorations" );
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}


///-----------------------
///------- execute -------
///-----------------------
StatusCode IDTPM::OfflineJetDecoratorAlg::execute( const EventContext& ctx ) const {

  /// retrieve offline track particle container
  SG::ReadHandle< xAOD::TrackParticleContainer > ptracks( m_offlineTrkParticlesName, ctx );
  if( not ptracks.isValid() ) {
    ATH_MSG_ERROR( "Failed to retrieve track particles container" );
    return StatusCode::FAILURE;
  }

  /// retrieve jet container
  SG::ReadHandle< xAOD::JetContainer > pjets( m_jetsName, ctx );
  if( not pjets.isValid() ) {
    ATH_MSG_ERROR( "Failed to retrieve jets container" );
    return StatusCode::FAILURE;
  }

  /// check if ALL required decorations exist already. If so return SUCCESS
  if( IDTPM::decorationsAllExist( *ptracks, m_decor_jet ) ) {
    ATH_MSG_DEBUG( "All decorations already exist. Exiting gracefully" );
    return StatusCode::SUCCESS;
  }

  /// Creating decorators (for non-yet-existing decorations)
  std::vector< IDTPM::OptionalDecoration<xAOD::TrackParticleContainer, ElementJetLink_t> >
      jet_decor( IDTPM::createDecoratorsIfNeeded( *ptracks, m_decor_jet, ctx ) );

  if( jet_decor.empty() ) {
    ATH_MSG_ERROR( "Failed to book jet decorations" );
    return StatusCode::FAILURE;
  }

  for( const xAOD::TrackParticle* track : *ptracks ) {
    /// decorate current track with jet ElementLink(s)
    ATH_CHECK( decorateJetTrack( *track, jet_decor, *pjets.ptr() ) );
  }

  return StatusCode::SUCCESS;
}


///-------------------------------
///---- decorateJetTrack ----
///-------------------------------
StatusCode IDTPM::OfflineJetDecoratorAlg::decorateJetTrack(
                const xAOD::TrackParticle& track,
                std::vector< IDTPM::OptionalDecoration< xAOD::TrackParticleContainer,
                                                        ElementJetLink_t > >& jet_decor,
                const xAOD::JetContainer& jets ) const
{
  /// Define accessors
  //static const SG::ConstAccessor<
  //    std::vector< ElementLink< xAOD::IParticleContainer > > > ghostTruth( "GhostTruth" );
  static const SG::ConstAccessor< int > truthJetTagLabel( "HadronConeExclTruthLabelID" );

  /// loop jet container to look for jet that includes this track
  for( const xAOD::Jet* jet : jets ) {
    /// pass jet cuts
    if( not passJetCuts( *jet ) ) continue;

    /// check if track is within max DeltaR from the jet core
    if( deltaR( *jet, track ) > m_maxTrkJetDR.value() ) continue;

    /// check if this is a truth c/b-jet
    bool isTruthCjet = false;
    bool isTruthBjet = false;
    if( not truthJetTagLabel.isAvailable( *jet ) ) {
      ATH_MSG_WARNING( "Failed to extract b-tag truth label from jet" );
    } else {
      isTruthCjet = ( truthJetTagLabel( *jet ) == 4 );
      isTruthBjet = ( truthJetTagLabel( *jet ) == 5 );
    }

    /// Create jet element link
    ElementJetLink_t jetLink;
    jetLink.toContainedElement( jets, jet );

    ATH_MSG_DEBUG( "Found matching jet (en=" << jet->e() << "). Decorating track." );

    /// Decoration for DR-matched jet tracks
    IDTPM::decorateOrRejectQuietly( track, jet_decor[DRtruthJet], jetLink );

    if( isTruthBjet ) {
      /// Decoration for DR-matched truth b-jet tracks
      IDTPM::decorateOrRejectQuietly( track, jet_decor[DRtruthBjet], jetLink );
      /// Decoration for DR-matched truth heavy-flavor jet tracks
      IDTPM::decorateOrRejectQuietly( track, jet_decor[DRtruthHeavyJet], jetLink );
    } else if( isTruthCjet ) {
      /// Decoration for DR-matched truth c-jet tracks
      IDTPM::decorateOrRejectQuietly( track, jet_decor[DRtruthCjet], jetLink );
      /// Decoration for DR-matched truth heavy-flavor jet tracks
      IDTPM::decorateOrRejectQuietly( track, jet_decor[DRtruthHeavyJet], jetLink );
    } else {
      /// Decoration for DR-matched truth light-flavour jet tracks
      IDTPM::decorateOrRejectQuietly( track, jet_decor[DRtruthLightJet], jetLink );
    }

    /// check if track is linked to a ghost truth particle of this jet
    /// FIXME here...

  } // close jets loop

  return StatusCode::SUCCESS;
}

bool IDTPM::OfflineJetDecoratorAlg::passJetCuts( const xAOD::Jet& jet ) const
{
  const float jetPt = jet.pt();
  const float jetEta = std::abs( jet.eta() );
  
  if( jetEta < m_jetAbsEtaMin ) return false;
  if( jetEta > m_jetAbsEtaMax ) return false;
  if( jetPt < m_jetPtMin )      return false;
  if( jetPt > m_jetPtMax )      return false;
  return true;
}
