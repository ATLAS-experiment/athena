/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file TruthDecoratorAlg.cxx
 * @author Federica Piazza <federica.piazza@cern.ch>, Marco Aparo <marco.aparo@cern.ch>
 **/

/// Local includes
#include "InDetTrackPerfMon/TruthDecoratorAlg.h"


///----------------------------------------
///------- Parametrized constructor -------
///----------------------------------------
IDTPM::TruthDecoratorAlg::TruthDecoratorAlg(
    const std::string& name,
    ISvcLocator* pSvcLocator ) :
  AthReentrantAlgorithm( name, pSvcLocator ) { }


///--------------------------
///------- initialize -------
///--------------------------
StatusCode IDTPM::TruthDecoratorAlg::initialize() {

  ATH_CHECK( m_truthParticlesName.initialize(
                ! m_truthParticlesName.key().empty() ) );

  ATH_CHECK( m_truthClassifier.retrieve() );

  /// Create truth class decorations for all truth particles
  IDTPM::createDecoratorKeysAndAccessor(
      *this, m_truthParticlesName,
      m_prefix.value(), m_decor_truth_names, m_decor_truth );

  if( m_decor_truth.size() != NDecorations ) {
    ATH_MSG_ERROR( "Incorrect booking of truth class decorations" );
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}


///-----------------------
///------- execute -------
///-----------------------
StatusCode IDTPM::TruthDecoratorAlg::execute( const EventContext& ctx ) const {

  /// retrieve truth particle container
  SG::ReadHandle< xAOD::TruthParticleContainer > ptruths( m_truthParticlesName, ctx );
  if( ! ptruths.isValid() ) {
    ATH_MSG_ERROR( "Failed to retrieve truth particles container" );
    return StatusCode::FAILURE;
  }

  /// check if ALL required decorations exist already. If so return SUCCESS // FIXME This should be handled in the configuration!!
  if( IDTPM::decorationsAllExist( *ptruths, m_decor_truth ) ) {
    ATH_MSG_DEBUG( "All decorations already exist. Exiting gracefully" );
    return StatusCode::SUCCESS;
  }

  /// Creating decorators (for non-yet-existing decorations)
  std::vector< IDTPM::OptionalDecoration<xAOD::TruthParticleContainer, int> >
      truth_decor( IDTPM::createDecoratorsIfNeeded( *ptruths, m_decor_truth, ctx ) );

  if( truth_decor.empty() ) {
    ATH_MSG_ERROR( "Failed to book truth class decorations" );
    return StatusCode::FAILURE;
  }

  for( const xAOD::TruthParticle* truth : *ptruths ) {
    /// decorate current truth particle with its origin && type classes
    ATH_CHECK( decorateTruthParticle( *truth, truth_decor ) );
  }

  return StatusCode::SUCCESS;
}


///-------------------------------
///---- decorateTruthParticle ----
///-------------------------------
StatusCode IDTPM::TruthDecoratorAlg::decorateTruthParticle(
                const xAOD::TruthParticle& truth,
                std::vector< IDTPM::OptionalDecoration< xAOD::TruthParticleContainer,
                                                        int>>& truth_decor) const {

  /// Getting truth classes
  auto truthClass = m_truthClassifier->particleTruthClassifier( &truth );
  int type = static_cast<int>( truthClass.first );
  int origin = static_cast<int>( truthClass.second );

  /// Adding decorations
  IDTPM::decorateOrRejectQuietly( truth, truth_decor[Type],   type );
  IDTPM::decorateOrRejectQuietly( truth, truth_decor[Origin], origin );

  return StatusCode::SUCCESS;
}
