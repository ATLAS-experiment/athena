/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file    ResolutionPlots.cxx
 * @author  Thomas Strebler <thomas.strebler@cern.ch>
 **/

/// local include(s)
#include "ResolutionPlots.h"
#include "../TrackParametersHelper.h"

/// To be ultimately migrated to IDTPM, no need to duplicate in the meantime
#include "InDetPhysValMonitoring/ResolutionHelper.h"

/// Delta phi
#include "FourMomUtils/xAODP4Helpers.h"

/// -----------------------
/// ----- Constructor -----
/// -----------------------
IDTPM::ResolutionPlots::ResolutionPlots(
    PlotMgr* pParent, const std::string& dirName, 
    const std::string& anaTag, const std::string& trackType ) :
        PlotMgr( dirName, anaTag, pParent ), 
        m_trackType( trackType ) { }


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

  for(unsigned int i=0; i<NPARAMS; i++){
    ATH_CHECK( retrieveAndBook( m_resHelperEta[i],
				"resHelper_"+m_trackType+"_eta_" + m_paramProp[i] ) );
    ATH_CHECK( retrieveAndBook( m_reswidth_vs_eta[i],
				"resolution_vs_"+m_trackType+"_eta_" + m_paramProp[i] ) );
    ATH_CHECK( retrieveAndBook( m_resmean_vs_eta[i],
				"resmean_vs_"+m_trackType+"_eta_" + m_paramProp[i] ) );
    ATH_CHECK( retrieveAndBook( m_resHelperPt[i],
				"resHelper_"+m_trackType+"_pt_" + m_paramProp[i] ) );
    ATH_CHECK( retrieveAndBook( m_reswidth_vs_pt[i],
				"resolution_vs_"+m_trackType+"_pt_" + m_paramProp[i] ) );
    ATH_CHECK( retrieveAndBook( m_resmean_vs_pt[i],
				"resmean_vs_"+m_trackType+"_pt_" + m_paramProp[i] ) );
  }

  return StatusCode::SUCCESS;
}


/// -----------------------------
/// --- Dedicated fill method ---
/// -----------------------------
template< typename REF, typename TEST >
StatusCode IDTPM::ResolutionPlots::fillPlots(
    const REF& particle_ref, const TEST& track_test, float weight )
{
  float refP[NPARAMS];
  float testP[NPARAMS];

  refP[D0] = d0(particle_ref);
  refP[Z0] = z0(particle_ref);
  refP[QOVERP] = qOverP(particle_ref) * Gaudi::Units::GeV;
  refP[QOVERPT] = qOverPT(particle_ref) * Gaudi::Units::GeV;
  refP[THETA] = theta(particle_ref);
  refP[PHI] = phi(particle_ref);
  refP[PT] = pT(particle_ref) / Gaudi::Units::GeV;
  refP[Z0SIN] = z0SinTheta(particle_ref);

  testP[D0] = d0(track_test);
  testP[Z0] = z0(track_test);
  testP[QOVERP] = qOverP(track_test) * Gaudi::Units::GeV;
  testP[QOVERPT] = qOverPT(track_test) * Gaudi::Units::GeV;
  testP[THETA] = theta(track_test);
  testP[PHI] = phi(track_test);
  testP[PT] = pT(track_test) / Gaudi::Units::GeV;
  testP[Z0SIN] = z0SinTheta(track_test);

  float eta_ref = eta(particle_ref);
  float pt_ref = pT(particle_ref) / Gaudi::Units::GeV;

  for(unsigned int i=0; i<NPARAMS; i++){
    float residual = testP[i]-refP[i];
    if(i==QOVERPT){
      // relative q/pt resolution
      residual = testP[i]/refP[i] - 1.;
    }
    else if(i==PHI){
      residual = xAOD::P4Helpers::deltaPhi(testP[i], refP[i]);
    }
    ATH_CHECK( fill( m_resHelperEta[i], eta_ref, residual, weight) );
    ATH_CHECK( fill( m_resHelperPt[i], pt_ref, residual, weight) );
  }
  
  return StatusCode::SUCCESS;
}


template StatusCode IDTPM::ResolutionPlots::fillPlots< xAOD::TrackParticle, xAOD::TrackParticle >(
    const xAOD::TrackParticle&, const xAOD::TrackParticle&, float weight );

template StatusCode IDTPM::ResolutionPlots::fillPlots< xAOD::TruthParticle, xAOD::TrackParticle >(
    const xAOD::TruthParticle&, const xAOD::TrackParticle&, float weight );

template StatusCode IDTPM::ResolutionPlots::fillPlots< xAOD::TrackParticle, xAOD::TruthParticle >(
    const xAOD::TrackParticle&, const xAOD::TruthParticle&, float weight );


/// -------------------------
/// ----- finalizePlots -----
/// -------------------------
void IDTPM::ResolutionPlots::finalizePlots()
{
  ATH_MSG_DEBUG( "Finalising resolution plots" );

  IDPVM::ResolutionHelper resolutionHelper;

  for (unsigned int i = 0; i < NPARAMS; i++) {
    resolutionHelper.makeResolutions(m_resHelperEta[i],
				     m_reswidth_vs_eta[i], m_resmean_vs_eta[i],
				     nullptr, false,
				     IDPVM::ResolutionHelper::iterRMS_convergence);
    resolutionHelper.makeResolutions(m_resHelperPt[i],
				     m_reswidth_vs_pt[i], m_resmean_vs_pt[i],
				     nullptr, false,
				     IDPVM::ResolutionHelper::iterRMS_convergence);
  }

}
