/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file TruthHitDecoratorAlg.cxx
 * @author shaun roe
 **/

/// Local includes
#include "TruthHitDecoratorAlg.h"

/// EDM includes
#include "xAODTruth/TruthVertex.h"
#include "TrkParameters/TrackParameters.h" // Contains typedef to Trk::CurvilinearParameters

/// ROOT includes
#include "TDatabasePDG.h"
#include "TParticlePDG.h"

/// STD includes
#include <cmath>
//#include <limits>


///---------------------------
///------- Constructor -------
///---------------------------
IDTPM::TruthHitDecoratorAlg::TruthHitDecoratorAlg(
    const std::string& name,
    ISvcLocator* pSvcLocator) :
  AthReentrantAlgorithm( name, pSvcLocator ) { }


///--------------------------
///------- initialize -------
///--------------------------
StatusCode IDTPM::TruthHitDecoratorAlg::initialize() {

  ATH_CHECK( m_extrapolator.retrieve() );
  ATH_CHECK( m_beamSpotDecoKey.initialize() );
  ATH_CHECK( m_truthPixelClusterName.initialize() );
  ATH_CHECK( m_truthSCTClusterName.initialize() );
  ATH_CHECK( m_truthParticleName.initialize() );

  /// Creating/booking decorations for truth particles
  IDTPM::createDecoratorKeysAndAccessor(
      *this, m_truthParticleName, 
      m_prefix.value(), m_decor_names, m_decor );

  if( m_decor.size() != NDecorations ) {
    ATH_MSG_ERROR( "Incorrect booking of truth hits decorations" );
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}


///-----------------------
///------- execute -------
///-----------------------
StatusCode IDTPM::TruthHitDecoratorAlg::execute(
    const EventContext& ctx ) const {

  /// retrieve truth particle container
  SG::ReadHandle< xAOD::TruthParticleContainer > ptruth( m_truthParticleName, ctx );
  if( not ptruth.isValid() ) {
    ATH_MSG_ERROR( "Failed to retrieve truth particle container" );
    return StatusCode::FAILURE;
  }

  std::vector< IDTPM::OptionalDecoration< xAOD::TruthParticleContainer, float > >
      float_decor( IDTPM::createDecoratorsIfNeeded(
          *ptruth, m_decor, ctx, msgLvl(MSG::DEBUG) ) );

  /// truthbarcode-cluster maps to be pre-stored at event level
  std::unordered_map< int, float > barcodeSCTclustercount;
  std::unordered_map< int, float > barcodePIXclustercount;
  
  /// Loop over the pixel and sct clusters
  /// to fill the truth barcode - cluster count maps
  SG::ReadHandle< xAOD::TrackMeasurementValidationContainer > sctClusters( m_truthSCTClusterName, ctx );
  SG::ReadHandle< xAOD::TrackMeasurementValidationContainer > pixelClusters( m_truthPixelClusterName, ctx );

  //only decorate the truth particles with truth silicon hits if both containers are available 
  if( sctClusters.isValid() && pixelClusters.isValid() ) {

    static const SG::AuxElement::ConstAccessor< std::vector<int> > barcodeAcc( "truth_barcode" );

    /// Loop over truth SCT clusters
    for( const xAOD::TrackMeasurementValidation* sctCluster : *sctClusters ) {
      std::vector<int> truth_barcode;
      if( barcodeAcc.isAvailable( *sctCluster ) ) {
        truth_barcode = barcodeAcc( *sctCluster );
        for( const int& barcode : truth_barcode ) {
          auto result = barcodeSCTclustercount.emplace( std::pair<int, float>(barcode, 0.0) ); 
          if( !result.second ) ++( result.first->second );
        }
      }
    } // close loop over truth SCT clusters

    /// Loop over truth Pixel clusters
    for( const xAOD::TrackMeasurementValidation* pixCluster : *pixelClusters ) {
      std::vector<int> truth_barcode;
      if( barcodeAcc.isAvailable( *pixCluster ) ) {
        truth_barcode = barcodeAcc( *pixCluster );
        for( const int& barcode : truth_barcode ) {
          auto result = barcodePIXclustercount.emplace( std::pair<int, float>(barcode, 0.0) ); 
          if( !result.second ) ++( result.first->second );
        }
      }
    } // close loop over truth Pixel clusters
  } // close if sctClusters and pixelClusters isValid

  if( float_decor.empty() ) {
    ATH_MSG_ERROR( "Failed to book Truth particles Hit decorations" );
    return StatusCode::FAILURE;
  }

  /// Retrieving BeamSpot info
  SG::ReadDecorHandle< xAOD::EventInfo, float > beamPosX( m_beamSpotDecoKey[0], ctx );
  SG::ReadDecorHandle< xAOD::EventInfo, float > beamPosY( m_beamSpotDecoKey[1], ctx );
  SG::ReadDecorHandle< xAOD::EventInfo, float > beamPosZ( m_beamSpotDecoKey[2], ctx );
  if( (not beamPosX.isValid()) or (not beamPosY.isValid()) or (not beamPosZ.isValid()) ) {
    ATH_MSG_WARNING( "Failed to retrieve beam position" );
    return StatusCode::RECOVERABLE;
  }
  Amg::Vector3D beamPos = Amg::Vector3D( beamPosX(0), beamPosY(0), beamPosZ(0) );

  for( const xAOD::TruthParticle* truth_particle : *ptruth ) {
    /// decorate current truth particle with hit info
    ATH_CHECK( decorateTruth( 
        *truth_particle, float_decor, beamPos,
        barcodePIXclustercount, barcodeSCTclustercount, ctx ) );
  }

  return StatusCode::SUCCESS;
}


///-----------------------
///---- decorateTruth ----
///-----------------------
StatusCode IDTPM::TruthHitDecoratorAlg::decorateTruth(
    const xAOD::TruthParticle& particle,
		std::vector< IDTPM::OptionalDecoration< xAOD::TruthParticleContainer, float > >& float_decor,
		const Amg::Vector3D& beamPos,
		std::unordered_map<int, float>& pixelMap,
		std::unordered_map<int, float>& sctMap,
    const EventContext& ctx ) const {

  /// Skip neutral truth particles
  if( particle.isNeutral() ) {
    return StatusCode::SUCCESS;
  }

  const Amg::Vector3D momentum( particle.px(), particle.py(), particle.pz() );
  const int pid( particle.pdgId() );
  double charge = particle.charge();

  if( std::isnan(charge) ) {
    ATH_MSG_DEBUG( "Charge not found on particle with pid " << pid );
    return StatusCode::SUCCESS;
  }
   
  /// Retrieve the cluster count from the pre-filled maps   
  std::unordered_map< int, float >::iterator it1, it2;
  it1 = pixelMap.find( particle.barcode() );
  it2 = sctMap.find( particle.barcode() );
  float nSiHits = 0;
  if( it1 !=pixelMap.end() )  nSiHits += (*it1).second; 
  if( it2 !=sctMap.end() )    nSiHits += (*it2).second; 
 
  /// Decoration for NSiHits
  IDTPM::decorateOrRejectQuietly( particle, float_decor[NSilHits], nSiHits );

  const xAOD::TruthVertex* ptruthVertex(nullptr);
  try {
    ptruthVertex = particle.prodVtx();
  } catch( const std::exception& e ) {
    if ( not m_errorEmitted ) {
      ATH_MSG_WARNING( "A non existent production vertex was requested in calculating the track parameters d0 etc" );
    }
    m_errorEmitted = true;
    return StatusCode::RECOVERABLE; //SUCCESS;
  }

  if( not ptruthVertex ) {
    ATH_MSG_DEBUG( "A production vertex pointer was retrieved, but it is NULL" );
    return StatusCode::SUCCESS;
  }

  const auto xPos = ptruthVertex->x();
  const auto yPos = ptruthVertex->y();
  const auto z_truth = ptruthVertex->z();
  const Amg::Vector3D position( xPos, yPos, z_truth );
  const float prodR_truth = std::sqrt( xPos * xPos + yPos * yPos );
  const Trk::CurvilinearParameters cParameters( position, momentum, charge );

  Trk::PerigeeSurface persf( beamPos );

  std::unique_ptr< const Trk::TrackParameters > tP(
      m_extrapolator->extrapolate( ctx, cParameters,
                                   persf, Trk::anyDirection, false ) );
  if( not tP ) {
    ATH_MSG_DEBUG( "The TrackParameters pointer for this TruthParticle is NULL" );
    return StatusCode::SUCCESS;
  }

  /// Other hit decorations
  float d0_truth     = tP->parameters()[ Trk::d0 ];
  float theta_truth  = tP->parameters()[ Trk::theta ];
  float z0_truth     = tP->parameters()[ Trk::z0 ];
  float phi_truth    = tP->parameters()[ Trk::phi ];
  float qOverP_truth = tP->parameters()[ Trk::qOverP ]; // P or Pt ??
  float z0st_truth   = z0_truth * std::sin( theta_truth );

  ATH_MSG_DEBUG( "Truth particle (pT = " << particle.pt() <<
                 " has impact parameter (d0, z0) = (" << d0_truth <<
                 " ," << z0_truth << ")" );

  IDTPM::decorateOrRejectQuietly( particle, float_decor[D0],     d0_truth );
  IDTPM::decorateOrRejectQuietly( particle, float_decor[Z0],     z0_truth );
  IDTPM::decorateOrRejectQuietly( particle, float_decor[Phi],    phi_truth );
  IDTPM::decorateOrRejectQuietly( particle, float_decor[Theta],  theta_truth );
  IDTPM::decorateOrRejectQuietly( particle, float_decor[Z0st],   z0st_truth );
  IDTPM::decorateOrRejectQuietly( particle, float_decor[QOverP], qOverP_truth );
  IDTPM::decorateOrRejectQuietly( particle, float_decor[ProdR],  prodR_truth );
  IDTPM::decorateOrRejectQuietly( particle, float_decor[ProdZ],  z_truth );

  return StatusCode::SUCCESS;
}
