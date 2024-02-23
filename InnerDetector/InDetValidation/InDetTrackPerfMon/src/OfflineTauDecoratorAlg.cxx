/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file OfflineTauDecoratorAlg.cxx
 * @author Marco Aparo <marco.aparo@cern.ch>
 **/

/// Local includes
#include "OfflineTauDecoratorAlg.h"


///----------------------------------------
///------- Parametrized constructor -------
///----------------------------------------
IDTPM::OfflineTauDecoratorAlg::OfflineTauDecoratorAlg(
    const std::string& name,
    ISvcLocator* pSvcLocator ) :
  AthReentrantAlgorithm( name, pSvcLocator ) { }


///--------------------------
///------- initialize -------
///--------------------------
StatusCode IDTPM::OfflineTauDecoratorAlg::initialize() {

  ATH_CHECK( m_offlineTrkParticlesName.initialize(
                not m_offlineTrkParticlesName.key().empty() ) );

  ATH_CHECK( m_tausName.initialize( not m_tausName.key().empty() ) );

  /// Create decorations for original ID tracks
  IDTPM::createDecoratorKeysAndAccessor( 
      *this, m_offlineTrkParticlesName,
      m_prefix.value(), m_decor_tau_names, m_decor_tau );

  if( m_decor_tau.size() != NDecorations ) {
    ATH_MSG_ERROR( "Incorrect booking of tau " <<
                   m_tauType.value() << " " <<
                   m_tauNprongs.value() << 
                   "prong decorations" );
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}


///-----------------------
///------- execute -------
///-----------------------
StatusCode IDTPM::OfflineTauDecoratorAlg::execute( const EventContext& ctx ) const {

  /// retrieve offline track particle container
  SG::ReadHandle< xAOD::TrackParticleContainer > ptracks( m_offlineTrkParticlesName, ctx );
  if( not ptracks.isValid() ) {
    ATH_MSG_ERROR( "Failed to retrieve track particles container" );
    return StatusCode::FAILURE;
  }

  /// retrieve tau container
  SG::ReadHandle< xAOD::TauJetContainer > ptaus( m_tausName, ctx );
  if( not ptaus.isValid() ) {
    ATH_MSG_ERROR( "Failed to retrieve taus container" );
    return StatusCode::FAILURE;
  }

  std::vector< IDTPM::OptionalDecoration<xAOD::TrackParticleContainer, ElementTauLink_t> >
      tau_decor( IDTPM::createDecoratorsIfNeeded( *ptracks, m_decor_tau, ctx ) );

  if( tau_decor.empty() ) {
    ATH_MSG_ERROR( "Failed to book tau " <<
                   m_tauType.value() << " " <<
                   m_tauNprongs.value() << 
                   "prong decorations" );
    return StatusCode::FAILURE;
  }

  for( const xAOD::TrackParticle* track : *ptracks ) {
    /// decorate current track with tau ElementLink(s)
    ATH_CHECK( decorateTauTrack( *track, tau_decor, *ptaus.ptr() ) );
  }

  return StatusCode::SUCCESS;
}


///--------------------------
///---- decorateTauTrack ----
///--------------------------
StatusCode IDTPM::OfflineTauDecoratorAlg::decorateTauTrack(
                const xAOD::TrackParticle& track,
                std::vector< IDTPM::OptionalDecoration< xAOD::TrackParticleContainer,
                                                        ElementTauLink_t > >& tau_decor,
                const xAOD::TauJetContainer& taus ) const { 

  /// loop muon container to look for muon reconstructed with current ID track
  for( const xAOD::TauJet* tau : taus ) {

    /// Select number of tau tracks (1prong or 3prong)
    int NTauTracks = tau->nTracks();
    if( m_tauNprongs.value() > 0 and NTauTracks != (int)m_tauNprongs.value() ) continue;

    /// retrieve ID TrackParticles from jet
    std::vector< const xAOD::TrackParticle* > tauTracks;
    for( size_t iprong=0; iprong<(size_t)NTauTracks; iprong++ ) {

      std::vector< ElementTrackLink_t > tracklink = tau->track( iprong )->trackLinks();
      ATH_MSG_DEBUG( "TauLinkVec size (" << m_tauNprongs.value() <<
                     "prong " << m_tauType.value() << "tau) = " << tracklink.size() );

      /// adding found tau tracks to tauTracks vector 
      for( size_t ilink=0; ilink<tracklink.size() ; ilink++ ) {
        if( tracklink.at(ilink).isValid() ) tauTracks.push_back( *(tracklink.at(ilink)) );
      }

    } // close NTauTracks loop

    ATH_MSG_DEBUG( "TauVec size (" << m_tauNprongs.value() << "prong " <<
                   m_tauType.value() << "tau) = " << tauTracks.size() );

    /// Loop over all the ID tracks of this hadronic tau
    for( const xAOD::TrackParticle* tauTrack : tauTracks ) {

      if( not tauTrack ) {
        ATH_MSG_ERROR( "Corrupted matching tau ID track" );
        continue;
      }

      if( tauTrack == &track ) {
        /// Createn tau element link
        ElementTauLink_t tauLink;
        tauLink.toContainedElement( taus, tau );

        bool isTight( false ), isMedium( false );
        bool isLoose( false ), isVeryLoose( false );

        if( m_tauType.value() == "BDT" ) {
          isTight     = tau->isTau( xAOD::TauJetParameters::JetBDTSigTight );
          isMedium    = tau->isTau( xAOD::TauJetParameters::JetBDTSigMedium );
          isLoose     = tau->isTau( xAOD::TauJetParameters::JetBDTSigLoose );
          isVeryLoose = tau->isTau( xAOD::TauJetParameters::JetBDTSigVeryLoose );

        } else if( m_tauType.value() == "RNN" ) {
          isTight     = tau->isTau( xAOD::TauJetParameters::JetRNNSigTight );
          isMedium    = tau->isTau( xAOD::TauJetParameters::JetRNNSigMedium );
          isLoose     = tau->isTau( xAOD::TauJetParameters::JetRNNSigLoose );
          isVeryLoose = tau->isTau( xAOD::TauJetParameters::JetRNNSigVeryLoose );

        } else {
          ATH_MSG_ERROR( "Unknown Tau type: " << m_tauType.value() );
          return StatusCode::FAILURE;
        }

        /// Decoration for All tau (no quality selection)
        if( isTight or isMedium or isLoose or isVeryLoose ) {
          ATH_MSG_DEBUG( "Found matching tau " <<
                         m_tauType.value() << " " <<
                         m_tauNprongs.value() << 
                         "prong (pt=" << tau->pt() <<
                         "). Decorating track." );
          IDTPM::decorateOrRejectQuietly( track, tau_decor[All], tauLink );
          return StatusCode::SUCCESS;
        }

        /// Decoration for Tight tau
        if( isTight ) {
          IDTPM::decorateOrRejectQuietly( track, tau_decor[Tight], tauLink );
        }

        /// Decoration for Medium tau
        if( isMedium ) {
          IDTPM::decorateOrRejectQuietly( track, tau_decor[Medium], tauLink );
        }

        /// Decoration for Loose tau
        if( isLoose ) {
          IDTPM::decorateOrRejectQuietly( track, tau_decor[Loose], tauLink );
        }

        /// Decoration for VeryLoose tau
        if( isVeryLoose ) {
          IDTPM::decorateOrRejectQuietly( track, tau_decor[VeryLoose], tauLink );
        }

      } // if( tauTrack == &track ) 

    } // close tauTracks loop

  } // close tau loop

  return StatusCode::SUCCESS;
}
