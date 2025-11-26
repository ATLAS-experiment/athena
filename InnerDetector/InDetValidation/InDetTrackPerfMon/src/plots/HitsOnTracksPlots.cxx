/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file    HitsOnTracksPlots.cxx
 * @author  Marco Aparo <marco.aparo@cern.ch> 
 **/

/// local include(s)
#include "HitsOnTracksPlots.h"
#include "../TrackParametersHelper.h"


/// -------------------------
/// ----- Constructor A -----
/// -------------------------
IDTPM::HitsOnTracksPlots::HitsOnTracksPlots(
    PlotMgr* pParent,
    const std::string& dirName,
    const std::string& anaTag,
    const std::string& testType,
    const std::string& refType,
    bool isITk, bool doGlobalPlots, bool doTruthMuPlots, bool do1D, bool doExpert ) :
        PlotMgr( dirName, anaTag, pParent ),
        m_testType( testType ), m_refType( refType ),
        m_isITk( isITk ), m_doGlobalPlots( doGlobalPlots ),
        m_doTruthMuPlots( doTruthMuPlots ), m_do1D( do1D ), m_doExpert( doExpert )
{
  /// TODO - add expert plots also for Run3 
  m_NHITPARAMS = m_doExpert ? NHITPARAMSTOT : NHITPARAMSBASE;
  m_NRUN3HITPARAMS = NRUN3HITPARAMSBASE;
}


/// -------------------------
/// ----- Constructor B -----
/// -------------------------
IDTPM::HitsOnTracksPlots::HitsOnTracksPlots(
    PlotMgr* pParent,
    const std::string& dirName,
    const std::string& anaTag,
    const std::string& trackType,
    bool isITk, bool doGlobalPlots, bool doTruthMuPlots, bool doExpert ) :
        PlotMgr( dirName, anaTag, pParent ),
        m_testType( trackType ), m_refType( trackType ),
        m_isITk( isITk ), m_doGlobalPlots( doGlobalPlots ),
        m_doTruthMuPlots( doTruthMuPlots ), m_do1D( true ), m_doExpert( doExpert )
{
  /// TODO - add expert plots also for Run3
  m_NHITPARAMS = m_doExpert ? NHITPARAMSTOT : NHITPARAMSBASE;
  m_NRUN3HITPARAMS =  NRUN3HITPARAMSBASE;
}


/// ---------------------------
/// --- Book the histograms ---
/// ---------------------------
void IDTPM::HitsOnTracksPlots::initializePlots()
{
  StatusCode sc = bookPlots();
  if( sc.isFailure() ) {
    ATH_MSG_ERROR( "Failed to book hits on tracks plots" );
  }
}


StatusCode IDTPM::HitsOnTracksPlots::bookPlots()
{
  ATH_MSG_DEBUG( "Booking hits on tracks plots in " << getDirectory() ); 

  /// 1D plots
  /// e.g. "offl_nPixelHits"
  if( m_do1D ) {
    for( unsigned int ih=0; ih<m_NHITPARAMS; ih++ ) {
      ATH_CHECK( retrieveAndBook( m_hits[ih], m_testType+"_"+m_hitParamName[ih] ) );
    }

    if( not m_isITk ) {
      for( unsigned int ih=0; ih<m_NRUN3HITPARAMS; ih++ ) {
        ATH_CHECK( retrieveAndBook( m_hitsRun3[ih], m_testType+"_"+m_run3HitParamName[ih] ) );
      }
    }
  }

  for( unsigned int i=0; i<NPARAMS; i++ ) {
    /// TProfile plots vs NPARAMS
    /// e.g. "offl_nPixelHits_vs_offl_eta" or "offl_nPixelHits_vs_truth_eta"
    for( unsigned int ih=0; ih<m_NHITPARAMS; ih++ ) {
      ATH_CHECK( retrieveAndBook( m_hits_vs[ih][i],
          m_testType+"_"+m_hitParamName[ih]+"_vs_"+m_refType+"_"+m_paramName[i] ) );
    }

    if( not m_isITk ) {
      for( unsigned int ih=0; ih<m_NRUN3HITPARAMS; ih++ ) {
        ATH_CHECK( retrieveAndBook( m_hitsRun3_vs[ih][i],
            m_testType+"_"+m_run3HitParamName[ih]+"_vs_"+m_refType+"_"+m_paramName[i] ) );
      }
    }

    /// TProfile2D plots vs NPARAMS vs NPARAMS
    /// e.g. "offl_nPixelHits_vs_offl_eta_vs_phi" or "offl_nPixelHits_vs_truth_eta_vs_phi"
    for( unsigned int j=i+1; j<NPARAMS; j++ ) {
      for( unsigned int ih=0; ih<m_NHITPARAMS; ih++ ) {
        ATH_CHECK( retrieveAndBook( m_hits_vs2D[ih][i][j],
            m_testType+"_"+m_hitParamName[ih]+"_vs_"+m_refType+"_"+m_paramName[i]+"_vs_"+m_paramName[j] ) );
      }

      if( not m_isITk ) {
        for( unsigned int ih=0; ih<m_NRUN3HITPARAMS; ih++ ) {
          ATH_CHECK( retrieveAndBook( m_hitsRun3_vs2D[ih][i][j],
              m_testType+"_"+m_run3HitParamName[ih]+"_vs_"+m_refType+"_"+m_paramName[i]+"_vs_"+m_paramName[j] ) );
        }
      }
    } // close j loop over NPARAMS
  } // close i loop over NPARAMS

  if( m_doGlobalPlots ) {
    for( unsigned int i=0; i<NPARAMSMU; i++ ) {
      if( not m_doTruthMuPlots and i==TRUTHMU ) continue;
      if( m_do1D ) {
        /// TProfile plots vs mu (truth and actual)
        /// e.g. "offl_nPixelHits_vs_actualMu"
        for( unsigned int ih=0; ih<m_NHITPARAMS; ih++ ) {
          ATH_CHECK( retrieveAndBook( m_hits_vsMu[ih][i],
              m_testType+"_"+m_hitParamName[ih]+"_vs_"+m_paramMuName[i] ) );
        }

        if( not m_isITk ) {
          for( unsigned int ih=0; ih<m_NRUN3HITPARAMS; ih++ ) {
            ATH_CHECK( retrieveAndBook( m_hitsRun3_vsMu[ih][i],
                m_testType+"_"+m_run3HitParamName[ih]+"_vs_"+m_paramMuName[i] ) );
          }
        }
      } // close if m_do1D

      /// TProfile2D plots vs mu (truth and actual) vs NPARAMS
      /// e.g. "offl_nPixelHits_vs_actualMu_vs_offl_eta" or "offl_nPixelHits_vs_actualMu_vs_truth_eta"
      for( unsigned int j=0; j<NPARAMS; j++ ) {
        for( unsigned int ih=0; ih<m_NHITPARAMS; ih++ ) {
          ATH_CHECK( retrieveAndBook( m_hits_vsMu_vs[ih][i][j],
              m_testType+"_"+m_hitParamName[ih]+"_vs_"+m_paramMuName[i]+"_vs_"+m_refType+"_"+m_paramName[j] ) );
        }

        if( not m_isITk ) {
          for( unsigned int ih=0; ih<m_NRUN3HITPARAMS; ih++ ) {
            ATH_CHECK( retrieveAndBook( m_hitsRun3_vsMu_vs[ih][i][j],
                m_testType+"_"+m_run3HitParamName[ih]+"_vs_"+m_paramMuName[i]+"_vs_"+m_refType+"_"+m_paramName[j] ) );
          }
        }
      } // close j loop over NPARAMS
    } // close i loop over NPARAMSMU
  } // end if m_doGlobalPlots

  return StatusCode::SUCCESS;
}


/// -------------------------------
/// --- Dedicated fill method A --- 
/// -------------------------------
/// for tracks and/or truth particles
/// for test_Hits-vs-reference plots
template< typename PTEST, typename PREF >
StatusCode IDTPM::HitsOnTracksPlots::fillPlots(
    const PTEST& ptest, const PREF& pref, float truthMu, float actualMu, float weight )
{
  /// Compute hits parameters
  float hitP[ NHITPARAMSTOT ];
  hitP[ NInnerMostPixelHits ]        = nInnerMostPixelHits( ptest );
  hitP[ NInnerMostPixelEndcapHits ]  = nInnerMostPixelEndcapHits( ptest );
  hitP[ NNextToInnerMostPixelHits ]  = nNextToInnerMostPixelHits( ptest );
  hitP[ NNextToInnerMostPixelEndcapHits ]  = nNextToInnerMostPixelEndcapHits( ptest );
  hitP[ NInnerMostPixelSharedHits ]        = nInnerMostPixelSharedHits( ptest );
  hitP[ NInnerMostPixelSharedEndcapHits ]  = nInnerMostPixelSharedEndcapHits( ptest );
  hitP[ NPixelHits ]          = nPixelHits( ptest );
  hitP[ NPixelHoles ]         = nPixelHoles( ptest );
  hitP[ NPixelSharedHits ]    = nPixelSharedHits( ptest );
  hitP[ PixeldEdx ]           = pixeldEdx( ptest );
  hitP[ NSCTHits ]            = nSCTHits( ptest );
  hitP[ NSCTHoles ]           = nSCTHoles( ptest ); 
  hitP[ NSCTSharedHits ]      = nSCTSharedHits( ptest );
  hitP[ NInnerMostPixelOutliers ]           = nInnerMostPixelOutliers( ptest );
  hitP[ NInnerMostPixelEndcapOutliers ]     = nInnerMostPixelEndcapOutliers( ptest );
  hitP[ NInnerMostPixelSplitHits ]          = nInnerMostPixelSplitHits( ptest );
  hitP[ NInnerMostPixelSplitEndcapHits ]    = nInnerMostPixelSplitEndcapHits( ptest );
  hitP[ NExpectedInnerMostPixelHits ]       = nExpectedInnerMostPixelHits( ptest );
  hitP[ NExpectedNextToInnerMostPixelHits ] = nExpectedNextToInnerMostPixelHits( ptest );
  hitP[ NPixelOutliers ]      = nPixelOutliers( ptest );
  hitP[ NPixelContribLayers ] = nPixelContribLayers( ptest );
  hitP[ NPixelSplitHits ]     = nPixelSplitHits( ptest );
  hitP[ NPixelGangedHits ]    = nPixelGangedHits( ptest );
  hitP[ NPixelGangedHitsFlaggedFakes ]      =  nPixelGangedHitsFlaggedFakes( ptest );
  hitP[ NPixelDeadSensors ]   = nPixelDeadSensors( ptest );
  hitP[ NSCTOutliers ]        = nSCTOutliers( ptest );
  hitP[ NSCTDoubleHoles ]     = nSCTDoubleHoles( ptest );
  hitP[ NSCTDeadSensors ]     = nSCTDeadSensors( ptest );

  float hitRun3P[ NRUN3HITPARAMSTOT ];
  hitRun3P[ NTRTHits ]                = ( not m_isITk ) ? nTRTHits( ptest ) : -9999.;
  hitRun3P[ NTRTHitsXe ]              = ( not m_isITk ) ? nTRTHitsXe( ptest ) : -9999.;
  hitRun3P[ NTRTHitsAr ]              = ( not m_isITk ) ? nTRTHitsAr( ptest ) : -9999.;
  hitRun3P[ NTRTHighThresholdHits ]   = ( not m_isITk ) ? nTRTHighThresholdHits( ptest ) : -9999.;
  hitRun3P[ NTRTHighThresholdHitsXe ] = ( not m_isITk ) ? nTRTHighThresholdHitsXe( ptest ) : -9999.;
  hitRun3P[ NTRTHighThresholdHitsAr ] = ( not m_isITk ) ? nTRTHighThresholdHitsAr( ptest ) : -9999.;

  /// Compute track parameters
  float refP[ NPARAMS ];
  refP[ PT ]  = pT( pref ) / Gaudi::Units::GeV;
  refP[ ETA ] = eta( pref );
  refP[ PHI ] = phi( pref );

  /// mu parameters
  float muP[ NPARAMSMU ];
  muP[ TRUTHMU ]  = truthMu;
  muP[ ACTUALMU ] = actualMu;

  /// Fill the histograms

  /// 1D plots
  if( m_do1D ) {
    for( unsigned int ih=0; ih<m_NHITPARAMS; ih++ ) {
      ATH_CHECK( fill( m_hits[ih], hitP[ih], weight ) );
    }

    if( not m_isITk ) {
      for( unsigned int ih=0; ih<m_NRUN3HITPARAMS; ih++ ) {
        ATH_CHECK( fill( m_hitsRun3[ih], hitRun3P[ih], weight ) );
      }
    }
  }

  for( unsigned int i=0; i<NPARAMS; i++ ) {
    /// TProfile plots vs NPARAMS
    for( unsigned int ih=0; ih<m_NHITPARAMS; ih++ ) {
      ATH_CHECK( fill( m_hits_vs[ih][i], refP[i], hitP[ih], weight ) );
    }

    if( not m_isITk ) {
      for( unsigned int ih=0; ih<m_NRUN3HITPARAMS; ih++ ) {
        ATH_CHECK( fill( m_hitsRun3_vs[ih][i], refP[i], hitRun3P[ih], weight ) );
      }
    }

    /// TProfile2D plots vs NPARAMS vs NPARAMS
    for( unsigned int j=i+1; j<NPARAMS; j++ ) {
      for( unsigned int ih=0; ih<m_NHITPARAMS; ih++ ) {
        ATH_CHECK( fill( m_hits_vs2D[ih][i][j], refP[i], refP[j], hitP[ih], weight ) );
      }

      if( not m_isITk ) {
        for( unsigned int ih=0; ih<m_NRUN3HITPARAMS; ih++ ) {
          ATH_CHECK( fill( m_hitsRun3_vs2D[ih][i][j], refP[i], refP[j], hitRun3P[ih], weight ) );
        }
      }
    } // close j loop over NPARAMS
  } // close i loop over NPARAMS

  if( m_doGlobalPlots ) {
    for( unsigned int i=0; i<NPARAMSMU; i++ ) {
      if( not m_doTruthMuPlots and i==TRUTHMU ) continue;
      if( m_do1D ) {
        /// TProfile plots vs mu (truth and actual)
        for( unsigned int ih=0; ih<m_NHITPARAMS; ih++ ) {
          ATH_CHECK( fill( m_hits_vsMu[ih][i], muP[i], hitP[ih], weight ) );
        }

        if( not m_isITk ) {
          for( unsigned int ih=0; ih<m_NRUN3HITPARAMS; ih++ ) {
            ATH_CHECK( fill( m_hitsRun3_vsMu[ih][i], muP[i], hitRun3P[ih], weight ) );
          }
        }
      } // close if m_do1D

      /// TProfile2D plots vs mu (truth and actual) vs NPARAMS
      for( unsigned int j=0; j<NPARAMS; j++ ) {
        for( unsigned int ih=0; ih<m_NHITPARAMS; ih++ ) {
          ATH_CHECK( fill( m_hits_vsMu_vs[ih][i][j], muP[i], refP[j], hitP[ih], weight ) );
        }

        if( not m_isITk ) {
          for( unsigned int ih=0; ih<m_NRUN3HITPARAMS; ih++ ) {
            ATH_CHECK( fill( m_hitsRun3_vsMu_vs[ih][i][j], muP[i], refP[j], hitRun3P[ih], weight ) );
          }
        }
      } // close j loop over NPARAMS
    } // close i loop over NPARAMSMU
  } // end if m_doGlobalPlots

  return StatusCode::SUCCESS;
}

template StatusCode IDTPM::HitsOnTracksPlots::fillPlots< xAOD::TrackParticle >(
    const xAOD::TrackParticle&, const xAOD::TrackParticle&, float, float, float );

template StatusCode IDTPM::HitsOnTracksPlots::fillPlots< xAOD::TruthParticle >(
    const xAOD::TruthParticle&, const xAOD::TruthParticle&, float, float, float );

template StatusCode IDTPM::HitsOnTracksPlots::fillPlots< xAOD::TrackParticle, xAOD::TruthParticle >(
    const xAOD::TrackParticle&, const xAOD::TruthParticle&, float, float, float );

template StatusCode IDTPM::HitsOnTracksPlots::fillPlots< xAOD::TruthParticle, xAOD::TrackParticle >(
    const xAOD::TruthParticle&, const xAOD::TrackParticle&, float, float, float );


/// -------------------------------
/// --- Dedicated fill method B --- 
/// -------------------------------
/// for tracks and/or truth particles
/// for for only one track type
template< typename PARTICLE >
StatusCode IDTPM::HitsOnTracksPlots::fillPlots(
    const PARTICLE& particle, float truthMu, float actualMu, float weight )
{
  ATH_CHECK( fillPlots< PARTICLE >( particle, particle, truthMu, actualMu, weight ) );
  return StatusCode::SUCCESS;
}

template StatusCode IDTPM::HitsOnTracksPlots::fillPlots< xAOD::TrackParticle >(
    const xAOD::TrackParticle&, float, float, float );

template StatusCode IDTPM::HitsOnTracksPlots::fillPlots< xAOD::TruthParticle >(
    const xAOD::TruthParticle&, float, float, float );


/// -------------------------
/// ----- finalizePlots -----
/// -------------------------
void IDTPM::HitsOnTracksPlots::finalizePlots()
{
  ATH_MSG_DEBUG( "Finalising hits on tracks plots" );
  /// print stat here if needed
}
