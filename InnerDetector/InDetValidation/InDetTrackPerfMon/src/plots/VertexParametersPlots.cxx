/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file    VertexParametersPlots.cxx
 * @author  Marco Aparo <marco.aparo@cern.ch> 
 **/

/// local include(s)
#include "VertexParametersPlots.h"
#include "../TrackParametersHelper.h" // also includes VertexParametersHelper.h


/// -----------------------
/// ----- Constructor -----
/// -----------------------
IDTPM::VertexParametersPlots::VertexParametersPlots(
    PlotMgr* pParent,
    const std::string& dirName,
    const std::string& anaTag,
    const std::string& vertexType,
    bool doTrackPlots,
    bool doGlobalPlots,
    bool doTruthMuPlots ) :
        PlotMgr( dirName, anaTag, pParent ), 
        m_vertexType( vertexType ),
        m_doTrackPlots( doTrackPlots ),
        m_doGlobalPlots( doGlobalPlots ),
        m_doTruthMuPlots( doTruthMuPlots ) { }


/// ---------------------------
/// --- Book the histograms ---
/// ---------------------------
void IDTPM::VertexParametersPlots::initializePlots()
{
  StatusCode sc = bookPlots();
  if( sc.isFailure() ) {
    ATH_MSG_ERROR( "Failed to book vertex parameters plots" );
  }
}


StatusCode IDTPM::VertexParametersPlots::bookPlots()
{
  ATH_MSG_DEBUG( "Booking vertex parameters plots in " << getDirectory() ); 

  if( not m_doGlobalPlots ) {
    /// vertex parameters plots
    ATH_CHECK( retrieveAndBook( m_vtx_x,          m_vertexType+"_vtx_x" ) );
    ATH_CHECK( retrieveAndBook( m_vtx_y,          m_vertexType+"_vtx_y" ) );
    ATH_CHECK( retrieveAndBook( m_vtx_z,          m_vertexType+"_vtx_z" ) );
    ATH_CHECK( retrieveAndBook( m_vtx_time,       m_vertexType+"_vtx_time" ) );
    if( m_vertexType != "truth" ) {
      /// only for reco vertices
      ATH_CHECK( retrieveAndBook( m_vtx_x_err,      m_vertexType+"_vtx_x_err" ) );
      ATH_CHECK( retrieveAndBook( m_vtx_y_err,      m_vertexType+"_vtx_y_err" ) );
      ATH_CHECK( retrieveAndBook( m_vtx_z_err,      m_vertexType+"_vtx_z_err" ) );
      ATH_CHECK( retrieveAndBook( m_vtx_time_err,   m_vertexType+"_vtx_time_err" ) );
      ATH_CHECK( retrieveAndBook( m_vtx_chi2OverNdof,  m_vertexType+"_vtx_chi2OverNdof" ) );
      ATH_CHECK( retrieveAndBook( m_vtx_type,          m_vertexType+"_vtx_type" ) );
    }
    if( m_doTrackPlots ) {
      /// vertex-associated tracks plots
      ATH_CHECK( retrieveAndBook( m_vtx_nTracks,        m_vertexType+"_vtx_nTracks" ) );
      ATH_CHECK( retrieveAndBook( m_vtx_track_weight,   m_vertexType+"_vtx_track_weight" ) );
      ATH_CHECK( retrieveAndBook( m_vtx_track_pt,       m_vertexType+"_vtx_track_pt" ) );
      ATH_CHECK( retrieveAndBook( m_vtx_track_eta,      m_vertexType+"_vtx_track_eta" ) );
      ATH_CHECK( retrieveAndBook( m_vtx_track_nSiHits,  m_vertexType+"_vtx_track_nSiHits" ) );
      ATH_CHECK( retrieveAndBook( m_vtx_track_nSiHoles, m_vertexType+"_vtx_track_nSiHoles" ) );
      ATH_CHECK( retrieveAndBook( m_vtx_track_d0,       m_vertexType+"_vtx_track_d0" ) );
      ATH_CHECK( retrieveAndBook( m_vtx_track_z0,       m_vertexType+"_vtx_track_z0" ) );
      ATH_CHECK( retrieveAndBook( m_vtx_track_d0_err,   m_vertexType+"_vtx_track_d0_err" ) );
      ATH_CHECK( retrieveAndBook( m_vtx_track_z0_err,   m_vertexType+"_vtx_track_z0_err" ) );
    }
  } else {
    /// vertex multiplicity vs pileup plots
    ATH_CHECK( retrieveAndBook( m_nVtx_vs_actualMu_2D,    m_vertexType+"_nVtx_vs_actualMu_2D" ) );
    ATH_CHECK( retrieveAndBook( m_nVtx_vs_actualMu,       m_vertexType+"_nVtx_vs_actualMu" ) );
    if( m_doTruthMuPlots ) {
      ATH_CHECK( retrieveAndBook( m_nVtx_vs_truthMu_2D,    m_vertexType+"_nVtx_vs_truthMu_2D" ) );
      ATH_CHECK( retrieveAndBook( m_nVtx_vs_truthMu,       m_vertexType+"_nVtx_vs_truthMu" ) );
    }
  }

  return StatusCode::SUCCESS;
}


/// ------------------------------
/// --- Dedicated fill methods ---
/// ------------------------------
/// for reco and/or truth vertices parmeters plots
template< typename VERTEX, typename PARTICLE >
StatusCode IDTPM::VertexParametersPlots::fillPlots(
    const VERTEX& vertex,
    const std::vector< const PARTICLE* >& associatedTracks,
    const std::vector< float >& associatedTrackWeights,
    float weight )
{
  if( m_doGlobalPlots ) return StatusCode::SUCCESS;

  /// vertex parameters plots
  float vx    = posX( vertex );
  float vy    = posY( vertex );
  float vz    = posZ( vertex );
  float vtime = time( vertex );
  bool vhasValidTime = bool( hasValidTime( vertex ) );

  ATH_CHECK( fill( m_vtx_x,   vx,   weight ) );
  ATH_CHECK( fill( m_vtx_y,   vy,   weight ) );
  ATH_CHECK( fill( m_vtx_z,   vz,   weight ) );
  if( vhasValidTime ) ATH_CHECK( fill( m_vtx_time,  vtime,  weight ) );

  if( m_vertexType != "truth" ) {
    /// only for reco vertices
    float vxErr     = error( vertex, VposX );
    float vyErr     = error( vertex, VposY );
    float vzErr     = error( vertex, VposZ );
    float vtimeErr  = timeErr( vertex );
    float vchi2         = chiSquared( vertex );
    float vndof         = ndof( vertex );
    float vchi2OverNdof = ( vndof > 0 ) ? vchi2 / vndof : -9999.;
    float vtype         = vertexType( vertex );

    ATH_CHECK( fill( m_vtx_x_err,   vxErr, weight ) );
    ATH_CHECK( fill( m_vtx_y_err,   vyErr, weight ) );
    ATH_CHECK( fill( m_vtx_z_err,   vzErr, weight ) );
    if( vhasValidTime ) ATH_CHECK( fill( m_vtx_time_err,  vtimeErr, weight ) );
    ATH_CHECK( fill( m_vtx_chi2OverNdof,  vchi2OverNdof,  weight ) );
    ATH_CHECK( fill( m_vtx_type,          vtype,          weight ) );
  } // close if m_vertexType != "truth"

  /// vertex-associated tracks plots
  if( m_doTrackPlots ) {
    size_t nvTracks = associatedTracks.size();
    if( nvTracks != associatedTrackWeights.size() ) {
      ATH_MSG_ERROR( "Vertex-associated track number mismatch" );
      return StatusCode::FAILURE;
    }

    ATH_CHECK( fill( m_vtx_nTracks, nvTracks, weight ) );

    for( size_t it=0 ; it<nvTracks ; it++ ) {
      ATH_CHECK( fill( m_vtx_track_weight,  associatedTrackWeights[it], weight ) );

      float tpt       = pT( *associatedTracks[it] ) / Gaudi::Units::GeV;
      float teta      = eta( *associatedTracks[it] );
      float tnSiHits  = nSiHits( *associatedTracks[it] );
      float tnSiHoles = nSiHoles( *associatedTracks[it] );
      float td0       = d0( *associatedTracks[it] );
      float tz0       = z0( *associatedTracks[it] ) - vz;
      float td0Err    = error( *associatedTracks[it], Trk::d0 );
      // TODO: should this also account for the vertex z error?
      float tz0Err    = error( *associatedTracks[it], Trk::z0 );

      ATH_CHECK( fill( m_vtx_track_pt,        tpt,        weight ) );
      ATH_CHECK( fill( m_vtx_track_eta,       teta,       weight ) );
      ATH_CHECK( fill( m_vtx_track_nSiHits,   tnSiHits,   weight ) );
      ATH_CHECK( fill( m_vtx_track_nSiHoles,  tnSiHoles,  weight ) );
      ATH_CHECK( fill( m_vtx_track_d0,        td0,        weight ) );
      ATH_CHECK( fill( m_vtx_track_z0,        tz0,        weight ) );
      ATH_CHECK( fill( m_vtx_track_d0_err,    td0Err,     weight ) );
      ATH_CHECK( fill( m_vtx_track_z0_err,    tz0Err,     weight ) );
    } // close loop over tracks
  } // close if m_doTrackPlots

  return StatusCode::SUCCESS;
}

template StatusCode IDTPM::VertexParametersPlots::fillPlots< xAOD::Vertex, xAOD::TrackParticle >(
    const xAOD::Vertex& vertex,
    const std::vector< const xAOD::TrackParticle* >& associatedTracks,
    const std::vector< float >& associatedTrackWeights,
    float weight );

template StatusCode IDTPM::VertexParametersPlots::fillPlots< xAOD::TruthVertex, xAOD::TruthParticle >(
    const xAOD::TruthVertex& vertex,
    const std::vector< const xAOD::TruthParticle* >& associatedTracks,
    const std::vector< float >& associatedTrackWeights,
    float weight );


/// for vertex multiplicity vs pile-up
StatusCode IDTPM::VertexParametersPlots::fillPlots(
    int nVertices,
    float truthMu,
    float actualMu,
    float weight )
{
  if( not m_doGlobalPlots ) return StatusCode::SUCCESS;

  /// vertex multiplicity vs pileup plots
  ATH_CHECK( fill( m_nVtx_vs_actualMu_2D,   actualMu,   nVertices,  weight ) );
  ATH_CHECK( fill( m_nVtx_vs_actualMu,      actualMu,   nVertices,  weight ) );
  if( m_doTruthMuPlots ) {
    ATH_CHECK( fill( m_nVtx_vs_truthMu_2D,  truthMu,    nVertices,  weight ) );
    ATH_CHECK( fill( m_nVtx_vs_truthMu,     truthMu,    nVertices,  weight ) );
  }

  return StatusCode::SUCCESS;
}


/// -------------------------
/// ----- finalizePlots -----
/// -------------------------
void IDTPM::VertexParametersPlots::finalizePlots()
{
  ATH_MSG_DEBUG( "Finalising vertex parameters plots" );
  /// print stat here if needed
}
