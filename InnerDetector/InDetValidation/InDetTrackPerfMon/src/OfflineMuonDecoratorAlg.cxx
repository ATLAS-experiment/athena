/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file OfflineMuonDecoratorAlg.cxx
 * @author Marco Aparo <marco.aparo@cern.ch>
 **/

/// Local includes
#include "OfflineMuonDecoratorAlg.h"


///----------------------------------------
///------- Parametrized constructor -------
///----------------------------------------
IDTPM::OfflineMuonDecoratorAlg::OfflineMuonDecoratorAlg(
    const std::string& name,
    ISvcLocator* pSvcLocator ) :
  AthReentrantAlgorithm( name, pSvcLocator ) { }


///----------------------------------
///------- General initialize -------
///----------------------------------
StatusCode IDTPM::OfflineMuonDecoratorAlg::initialize() {

  ATH_CHECK( m_offlineTrkParticlesName.initialize(
                not m_offlineTrkParticlesName.key().empty() ) );

  /// Muon collection, if required
  ATH_CHECK( m_muonsName.initialize( not m_muonsName.key().empty() ) );

  /// Create decorations for original ID tracks
  IDTPM::createDecoratorKeysAndAccessor( 
      *this, m_offlineTrkParticlesName,
      m_prefix.value(), m_decor_mu_names, m_decor_mu );

  if( m_decor_mu.size() != NDecorations ) {
    ATH_MSG_ERROR( "Incorrect booking of muon decorations" );
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}


///-----------------------
///------- execute -------
///-----------------------
StatusCode IDTPM::OfflineMuonDecoratorAlg::execute( const EventContext& ctx ) const {

  /// retrieve offline track particle container
  SG::ReadHandle< xAOD::TrackParticleContainer > ptracks( m_offlineTrkParticlesName, ctx );
  if( not ptracks.isValid() ) {
    ATH_MSG_ERROR( "Failed to retrieve track particles container" );
    return StatusCode::FAILURE;
  }

  /// retrieve muon container
  SG::ReadHandle< xAOD::MuonContainer > pmuons( m_muonsName, ctx );
  if( not pmuons.isValid() ) {
    ATH_MSG_ERROR( "Failed to retrieve muons container" );
    return StatusCode::FAILURE;
  }

  /// check if ALL required decorations exist already. If so return SUCCESS
  if( IDTPM::decorationsAllExist( *ptracks, m_decor_mu ) ) {
    ATH_MSG_INFO( "All decorations already exist. Exiting gracefully" );
    return StatusCode::SUCCESS;
  }

  /// Creating decorators (for non-yet-existing decorations)
  std::vector< IDTPM::OptionalDecoration<xAOD::TrackParticleContainer, ElementMuonLink_t> >
      mu_decor( IDTPM::createDecoratorsIfNeeded( *ptracks, m_decor_mu, ctx ) );

  if( mu_decor.empty() ) {
    ATH_MSG_ERROR( "Failed to book muon decorations" );
    return StatusCode::FAILURE;
  }

  for( const xAOD::TrackParticle* track : *ptracks ) {
    /// decorate current track with muon ElementLink(s)
    ATH_CHECK( decorateMuonTrack( *track, mu_decor, *pmuons.ptr() ) );
  }

  return StatusCode::SUCCESS;
}


///---------------------------
///---- decorateMuonTrack ----
///---------------------------
StatusCode IDTPM::OfflineMuonDecoratorAlg::decorateMuonTrack(
                const xAOD::TrackParticle& track,
                std::vector< IDTPM::OptionalDecoration< xAOD::TrackParticleContainer,
                                                        ElementMuonLink_t > >& mu_decor,
                const xAOD::MuonContainer& muons ) const {

  /// loop muon container to look for muon reconstructed with current ID track
  for( const xAOD::Muon* muon : muons ) {

    /// Exclude non-Combined muons
    if( muon->muonType() != xAOD::Muon::Combined ) continue;

    /// retrieve ID TrackParticle from muon
    const xAOD::TrackParticle* muTrack = m_useCombinedMuonTracks.value() ?
                          *( muon->combinedTrackParticleLink() ) : // Combined track
                          *( muon->inDetTrackParticleLink() );  // ID track

    if( not muTrack ) {
      ATH_MSG_ERROR( "Corrupted matched muon ID track" );
      return StatusCode::FAILURE;
    }

    if( muTrack == &track ) {
      /// Create muon element link
      ElementMuonLink_t muLink;
      muLink.toContainedElement( muons, muon );

      ATH_MSG_DEBUG( "Found matched muon (pt=" << muon->pt() << "). Decorating track." );

      /// Decoration for All muon (no quality selection)
      IDTPM::decorateOrRejectQuietly( track, mu_decor[All], muLink );

      /// Decoration for Tight muon
      if( muon->quality() <= xAOD::Muon::Tight ) {
        IDTPM::decorateOrRejectQuietly( track, mu_decor[Tight], muLink );
      }

      /// Decoration for Medium muon
      if( muon->quality() <= xAOD::Muon::Medium ) {
        IDTPM::decorateOrRejectQuietly( track, mu_decor[Medium], muLink );
      }

      /// Decoration for Loose muon
      if( muon->quality() <= xAOD::Muon::Loose ) {
        IDTPM::decorateOrRejectQuietly( track, mu_decor[Loose], muLink );
      }

      /// Decoration for VeryLoose muon
      if( muon->quality() <= xAOD::Muon::VeryLoose ) {
        IDTPM::decorateOrRejectQuietly( track, mu_decor[VeryLoose], muLink );
      }

      return StatusCode::SUCCESS;
    } // if( muTrack == &track )

  } // close muon loop

  return StatusCode::SUCCESS;
}
