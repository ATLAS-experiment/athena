/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file    ResolutionPlots.cxx
 * @author  Thomas Strebler <thomas.strebler@cern.ch>
 **/

/// local include(s)
#include "InDetTrackPerfMon/ResolutionPlots.h"
#include "InDetTrackPerfMon/TrackParametersHelper.h"

/// To be ultimately migrated to IDTPM, no need to duplicate in the meantime
#include "InDetPhysValMonitoring/ResolutionHelper.h"


/// -----------------------
/// ----- Constructor -----
/// -----------------------
IDTPM::ResolutionPlots::ResolutionPlots(
    PlotMgr* pParent,
    const std::string& dirName,
    const std::string& anaTag,
    const std::string& testType,
    const std::string& refType,
    unsigned int method ) :
        PlotMgr( dirName, anaTag, pParent ), 
        m_testType( testType ), m_refType( refType ), m_method( method ) { }


/// ---------------------------
/// --- Book the histograms ---
/// ---------------------------
void IDTPM::ResolutionPlots::initializePlots()
{
  StatusCode sc = bookPlots();
  if( sc.isFailure() ) {
    ATH_MSG_ERROR( "Failed to book resolution plots" );
  }
}


StatusCode IDTPM::ResolutionPlots::bookPlots()
{
  ATH_MSG_DEBUG( "Booking resolution plots in " << getDirectory() );

  for( unsigned int i=1; i<NPARAMS; i++ ) {

    ATH_CHECK( retrieveAndBook( m_pull[i], "pull_"+m_paramName[i] ) );
    ATH_CHECK( retrieveAndBook( m_res[i], "res_"+m_paramName[i] ) );
    ATH_CHECK( retrieveAndBook( m_sigma[i], "sigma_"+m_testType+"_"+m_paramName[i] ) );
    ATH_CHECK( retrieveAndBook( m_significance[i], "significance_"+m_testType+"_"+m_paramName[i] ) );
    ATH_CHECK( retrieveAndBook( m_corr[i], "corr_"+m_testType+"_vs_"+m_refType+"_"+m_paramName[i] ) );

    /// loop all vs variables
    for( unsigned int j=0; j<NPARAMSOUT; j++ ) {

      /// resolutions
      ATH_CHECK( retrieveAndBook( m_resHelper[i][j],
          "resHelper_"+m_paramName[i]+"_vs_"+m_refType+"_"+m_paramName[j] ) );
      ATH_CHECK( retrieveAndBook( m_reswidth[i][j],
          "resolution_"+m_paramName[i]+"_vs_"+m_refType+"_"+m_paramName[j] ) );
      ATH_CHECK( retrieveAndBook( m_resmean[i][j],
          "resmean_"+m_paramName[i]+"_vs_"+m_refType+"_"+m_paramName[j] ) );

      /// pulls
      ATH_CHECK( retrieveAndBook( m_pullHelper[i][j],
          "pullHelper_"+m_paramName[i]+"_vs_"+m_refType+"_"+m_paramName[j] ) );
      ATH_CHECK( retrieveAndBook( m_pullwidth[i][j],
          "pullwidth_"+m_paramName[i]+"_vs_"+m_refType+"_"+m_paramName[j] ) );
      ATH_CHECK( retrieveAndBook( m_pullmean[i][j],
          "pullmean_"+m_paramName[i]+"_vs_"+m_refType+"_"+m_paramName[j] ) );

    } // close NPARAMSOUT loop
  } // close NPARAMS loop

  return StatusCode::SUCCESS;
}


/// -----------------------------
/// --- Dedicated fill method ---
/// -----------------------------
template< typename TEST, typename REF >
StatusCode IDTPM::ResolutionPlots::fillPlots(
	  const TEST& ptest, const REF& pref, float weight )
{
  float refP[ NPARAMS ];
  float refErrorP[ NPARAMS ];
  getTrackParameters( pref, refP, refErrorP );

  float testP[ NPARAMS ];
  float testErrorP[ NPARAMS ];
  getTrackParameters( ptest, testP, testErrorP );

  for( unsigned int i=1; i<NPARAMS; i++ ) {

    float residual = testP[i] - refP[i];
    if( i==PHI ) residual = deltaPhi( ptest, pref ); // angular difference
    float pull = testErrorP[i] > 0. ? residual / testErrorP[i] : -9999.;
    float sigma_test = testErrorP[i];
    float significance_test = testErrorP[i] > 0. ? testP[i] / testErrorP[i] : -9999.;

    if( i==QOVERPT ) {
      residual = testP[i] / refP[i] - 1.; // Relative q/pt resolution
      sigma_test *= 1. / std::abs( testP[i] ); // relative q/pt error
    }

    ATH_CHECK( fill( m_pull[i], pull, weight ) );
    ATH_CHECK( fill( m_res[i], residual, weight ) );
    ATH_CHECK( fill( m_sigma[i], sigma_test, weight ) );
    ATH_CHECK( fill( m_significance[i], significance_test, weight ) );
    ATH_CHECK( fill( m_corr[i], refP[i], testP[i], weight ) );

    for( unsigned int j=0; j<NPARAMSOUT; j++ ) {
      ATH_CHECK( fill( m_resHelper[i][j],  refP[j], residual, weight ) );
      ATH_CHECK( fill( m_pullHelper[i][j], refP[j], pull,     weight ) );
    }

  }
  
  return StatusCode::SUCCESS;
}

template StatusCode IDTPM::ResolutionPlots::fillPlots< xAOD::TrackParticle, xAOD::TrackParticle >(
    const xAOD::TrackParticle&, const xAOD::TrackParticle&, float weight );

template StatusCode IDTPM::ResolutionPlots::fillPlots< xAOD::TruthParticle, xAOD::TrackParticle >(
    const xAOD::TruthParticle&, const xAOD::TrackParticle&, float weight );

template StatusCode IDTPM::ResolutionPlots::fillPlots< xAOD::TrackParticle, xAOD::TruthParticle >(
    const xAOD::TrackParticle&, const xAOD::TruthParticle&, float weight );

/// N.B.: not a use-case. Just to avoid compilation errors
template StatusCode IDTPM::ResolutionPlots::fillPlots< xAOD::TruthParticle, xAOD::TruthParticle >(
    const xAOD::TruthParticle&, const xAOD::TruthParticle&, float weight );


/// -------------------------
/// ----- finalizePlots -----
/// -------------------------
void IDTPM::ResolutionPlots::finalizePlots()
{
  ATH_MSG_DEBUG( "Finalising resolution plots" );

  IDPVM::ResolutionHelper resolutionHelper;
  IDPVM::ResolutionHelper::methods thisMethod = IDPVM::ResolutionHelper::methods( m_method );

  for( unsigned int i=1; i<NPARAMS; i++ ) {
    for( unsigned int j=0; j<NPARAMSOUT; j++ ) {
      resolutionHelper.makeResolutions( m_resHelper[i][j], m_reswidth[i][j], m_resmean[i][j], thisMethod );
      resolutionHelper.makeResolutions( m_pullHelper[i][j], m_pullwidth[i][j], m_pullmean[i][j], thisMethod );
    }
  }

}


/// --------------------------
/// --- getTrackParameters ---
/// --------------------------
template< typename PARTICLE >
void IDTPM::ResolutionPlots::getTrackParameters(
	  const PARTICLE& p, float* params, float* errors )
{
  /// parameters
  params[ D0 ] = d0( p );
  params[ Z0 ] = z0( p );
  params[ QOVERP ] = qOverP( p ) * Gaudi::Units::GeV;
  params[ QOVERPT ] = qOverPT( p ) * Gaudi::Units::GeV;
  params[ THETA ] = theta( p );
  params[ PHI ] = phi( p );
  params[ PT ] = pT( p ) / Gaudi::Units::GeV;
  params[ Z0SIN ] = z0SinTheta( p );
  params[ ETA ] = eta( p );
  params[ INCLUSIVE ] = 1;

  /// errors
  errors[ D0 ] = error( p, Trk::d0 );
  errors[ Z0 ] = error( p, Trk::z0 );
  errors[ QOVERP ] = error( p, Trk::qOverP ) * Gaudi::Units::GeV;
  errors[ QOVERPT ] = qOverPTError( p ) * Gaudi::Units::GeV;
  errors[ THETA ] = error( p, Trk::theta );
  errors[ PHI ] = error( p, Trk::phi );
  errors[ PT ] = pTError( p ) / Gaudi::Units::GeV;
  errors[ Z0SIN ] = z0SinThetaError( p );
  errors[ ETA ] = etaError( p );
  errors[ INCLUSIVE ] = 0;
}

template void IDTPM::ResolutionPlots::getTrackParameters< xAOD::TrackParticle >(
    const xAOD::TrackParticle&, float*, float* );

template void IDTPM::ResolutionPlots::getTrackParameters< xAOD::TruthParticle >(
    const xAOD::TruthParticle&, float*, float* );
