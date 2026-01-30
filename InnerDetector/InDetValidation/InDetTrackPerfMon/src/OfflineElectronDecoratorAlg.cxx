/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file OfflineElectronDecoratorAlg.cxx
 * @author Marco Aparo <marco.aparo@cern.ch>
 **/

/// Local includes
#include "InDetTrackPerfMon/OfflineElectronDecoratorAlg.h"

/// xAOD includes
#include "xAODEgamma/ElectronxAODHelpers.h" // xAOD::EgammaHelpers::getOriginalTrackParticle


///----------------------------------------
///------- Parametrized constructor -------
///----------------------------------------
IDTPM::OfflineElectronDecoratorAlg::OfflineElectronDecoratorAlg(
    const std::string& name,
    ISvcLocator* pSvcLocator ) :
  AthReentrantAlgorithm( name, pSvcLocator ) { }


///--------------------------
///------- initialize -------
///--------------------------
StatusCode IDTPM::OfflineElectronDecoratorAlg::initialize() {

  ATH_CHECK( m_offlineTrkParticlesName.initialize(
                ! m_offlineTrkParticlesName.key().empty() ) );

  ATH_CHECK( m_electronsName.initialize( ! m_electronsName.key().empty() ) );

  /// Create decorations for original (non GSF) ID tracks
  IDTPM::createDecoratorKeysAndAccessor( 
      *this, m_offlineTrkParticlesName,
      m_prefix.value(), m_decor_ele_names, m_decor_ele );

  if( m_decor_ele.size() != NDecorations ) {
    ATH_MSG_ERROR( "Incorrect booking of electron decorations" );
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}


///-----------------------
///------- execute -------
///-----------------------
StatusCode IDTPM::OfflineElectronDecoratorAlg::execute( const EventContext& ctx ) const {

  /// retrieve offline track particle container
  SG::ReadHandle< xAOD::TrackParticleContainer > ptracks( m_offlineTrkParticlesName, ctx );
  if( ! ptracks.isValid() ) {
    ATH_MSG_ERROR( "Failed to retrieve track particles container" );
    return StatusCode::FAILURE;
  }

  /// retrieve electron container
  SG::ReadHandle< xAOD::ElectronContainer > pelectrons( m_electronsName, ctx );
  if( ! pelectrons.isValid() ) {
    ATH_MSG_ERROR( "Failed to retrieve electrons container" );
    return StatusCode::FAILURE;
  }

  /// check if ALL required decorations exist already. If so return SUCCESS
  if( IDTPM::decorationsAllExist( *ptracks, m_decor_ele ) ) {
    ATH_MSG_DEBUG( "All decorations already exist. Exiting gracefully" );
    return StatusCode::SUCCESS;
  }

  /// Creating decorators (for non-yet-existing decorations)
  std::vector< IDTPM::OptionalDecoration<xAOD::TrackParticleContainer, ElementElectronLink_t> >
      ele_decor( IDTPM::createDecoratorsIfNeeded( *ptracks, m_decor_ele, ctx ) );

  if( ele_decor.empty() ) {
    ATH_MSG_ERROR( "Failed to book electron decorations" );
    return StatusCode::FAILURE;
  }

  for( const xAOD::TrackParticle* track : *ptracks ) {
    /// decorate current track with electron ElementLink(s)
    ATH_CHECK( decorateElectronTrack( *track, ele_decor, *pelectrons.ptr() ) );
  }

  return StatusCode::SUCCESS;
}


///-------------------------------
///---- decorateElectronTrack ----
///-------------------------------
StatusCode IDTPM::OfflineElectronDecoratorAlg::decorateElectronTrack(
                const xAOD::TrackParticle& track,
                std::vector< IDTPM::OptionalDecoration< xAOD::TrackParticleContainer,
                                                        ElementElectronLink_t > >& ele_decor,
                const xAOD::ElectronContainer& electrons ) const {

  /// loop electron container to look for electron reconstructed with track
  for( const xAOD::Electron* electron : electrons ) {

    /// retrieve TrackParticle from electron
    const xAOD::TrackParticle* eleTrack = m_useGSF.value() ?
                        electron->trackParticle() : // with GSF
                        xAOD::EgammaHelpers::getOriginalTrackParticle( electron ); // no GSF

    if( ! eleTrack ) {
      ATH_MSG_ERROR( "Corrupted matching electron ID track" );
      return StatusCode::FAILURE;
    }

    if( eleTrack == &track ) {
      /// Create electron element link
      ElementElectronLink_t eleLink;
      eleLink.toContainedElement( electrons, electron );

      ATH_MSG_DEBUG( "Found matching electron (e=" << electron->e() << "). Decorating track." );

      /// Decoration for All electron (no quality selection)
      IDTPM::decorateOrRejectQuietly( track, ele_decor[All], eleLink );

      /// Decoration for Tight electron
      if( electron->passSelection("Tight") ) {
        IDTPM::decorateOrRejectQuietly( track, ele_decor[Tight], eleLink );
      }

      /// Decoration for Medium electron
      if( electron->passSelection("Medium") ) {
        IDTPM::decorateOrRejectQuietly( track, ele_decor[Medium], eleLink );
      }

      /// Decoration for Loose electron
      if( electron->passSelection("Loose") ) {
        IDTPM::decorateOrRejectQuietly( track, ele_decor[Loose], eleLink );
      }

      /// Decoration for LHTight electron
      if( electron->passSelection("LHTight") ) {
        IDTPM::decorateOrRejectQuietly( track, ele_decor[LHTight], eleLink );
      }

      /// Decoration for LHMedium electron
      if( electron->passSelection("LHMedium") ) {
        IDTPM::decorateOrRejectQuietly( track, ele_decor[LHMedium], eleLink );
      }

      /// Decoration for LHLoose electron
      if( electron->passSelection("LHLoose") ) {
        IDTPM::decorateOrRejectQuietly( track, ele_decor[LHLoose], eleLink );
      }

      return StatusCode::SUCCESS;
    } // if( eleTrack == &track ) 

  } // close electron loop

  return StatusCode::SUCCESS;
}
