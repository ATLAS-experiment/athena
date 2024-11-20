/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file TruthDecoratorAlg.cxx
 * @author Marco Aparo <marco.aparo@cern.ch>
 **/

/// Local includes
#include "TruthDecoratorAlg.h"


///----------------------------------------
///------- Parametrized constructor -------
///----------------------------------------
IDTPM::TruthDecoratorAlg::TruthDecoratorAlg(
    const std::string& name,
    ISvcLocator* pSvcLocator ) :
  AthReentrantAlgorithm( name, pSvcLocator ) { }


///----------------------------------
///------- General initialize -------
///----------------------------------
StatusCode IDTPM::TruthDecoratorAlg::initialize() {

  ATH_CHECK( m_truthParticlesName.initialize(
                not m_truthParticlesName.key().empty() ) );

  /// Create decorations for original ID tracks
  IDTPM::createDecoratorKeysAndAccessor( 
      *this, m_truthParticlesName,
      m_prefix.value(), m_decor_truth_names, m_decor_truth );

  return StatusCode::SUCCESS;
}


///-----------------------
///------- execute -------
///-----------------------
StatusCode IDTPM::TruthDecoratorAlg::execute( const EventContext& ctx ) const {

  /// retrieve truth particle container
  SG::ReadHandle< xAOD::TruthParticleContainer > ptruths( m_truthParticlesName, ctx );
  if( not ptruths.isValid() ) {
    ATH_MSG_ERROR( "Failed to retrieve truth particles container" );
    return StatusCode::FAILURE;
  }
  std::vector< IDTPM::OptionalDecoration<xAOD::TruthParticleContainer, int> >
      truth_decor( IDTPM::createDecoratorsIfNeeded( *ptruths, m_decor_truth, ctx ) );

  if( truth_decor.empty() ) {
    ATH_MSG_ERROR( "Failed to book muon decorations" );
    return StatusCode::FAILURE;
  }

  for( const xAOD::TruthParticle* truth : *ptruths ) {
    /// decorate current track with muon ElementLink(s)
    ATH_CHECK( decorateTruthParticle( *truth, truth_decor ) );
  }

  return StatusCode::SUCCESS;
}


///---------------------------
///---- decorateTruthParticle ----
///---------------------------
StatusCode IDTPM::TruthDecoratorAlg::decorateTruthParticle(
                const xAOD::TruthParticle& truth,
                std::vector< IDTPM::OptionalDecoration< xAOD::TruthParticleContainer,
                                                        int>>& truth_decor) const {

  /// Decoration for all truth particles

  int type = -9999;
  int origin = -9999;
  if (not m_truthClassifier.empty()) {
    auto truthClass = m_truthClassifier->particleTruthClassifier(&truth);
    type = static_cast<int>(truthClass.first);
    origin = static_cast<int>(truthClass.second);
  }

  IDTPM::decorateOrRejectQuietly( truth, truth_decor[truthType], type );
  IDTPM::decorateOrRejectQuietly( truth, truth_decor[truthOrigin], origin );

  return StatusCode::SUCCESS;
}
