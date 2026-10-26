/**
 ** @file    VertexPlots.cxx
 ** @author  sutt@cern.ch
 **
 **          vertex analysis plots class implementation
 **          need to add the efficiencies as well as the
 **          differential resolutions, I mean, no efficiency
 **          vs number of tracks ? vs z ?? versus mu ??? 
 **
 ** Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **/


#include "VertexPlots.h"


/// -----------------------
/// ----- Constructor -----
/// -----------------------
IDTPM2::VertexPlots::VertexPlots( const std::string& name, ToolHandle<GenericMonitoringTool>* m )
  : IPlotBase( name, m ) { 
}




/// ---------------------------
/// --- Book the histograms ---
/// ---------------------------

void IDTPM2::VertexPlots::initialize() {

  /// parameter histograms ...
  m_reference.initialize( this, "reference");
  m_test.initialize(      this, "test");

  /// initialise resolutions and efficiencies here ... 

  m_vtx_ntest_vs_nref = TIDA::Histogram2D<float>( monTool(), "vtx_ntest_vs_nref" );
  
  /// residuals ... 
  m_vtx_res_x = TIDA::Histogram<float>( monTool(), "vtx_res_x" );     
  m_vtx_res_y = TIDA::Histogram<float>( monTool(), "vtx_res_y" );   
  m_vtx_res_z = TIDA::Histogram<float>( monTool(), "vtx_res_z" );
  
  /// diferential residuals ...
  m_vtx_res_z_vs_z       = TIDA::Histogram2D<float>   ( monTool(), "2D", "vtx_res_z_vs_z" );
  m_vtx_res_z_vs_ntracks = TIDA::Histogram2D<float>   ( monTool(), "2D", "vtx_res_z_vs_ntracks" );
  m_vtx_res_z_vs_nvtx    = TIDA::Histogram2D<float>   ( monTool(), "2D", "vtx_res_z_vs_nvtx" );
  m_vtx_res_z_vs_mu      = TIDA::Histogram2D<float>   ( monTool(), "2D", "vtx_res_z_vs_mu" );
  
  /// efficiencies ...
  m_vtx_eff_vs_z       = TIDA::Histogram2D<float>   ( monTool(), "vtx_eff_vs_z"       );
  m_vtx_eff_vs_mu      = TIDA::Histogram2D<float>   ( monTool(), "vtx_eff_vs_mu"      );
  m_vtx_eff_vs_nvtx    = TIDA::Histogram2D<float>   ( monTool(), "vtx_eff_vs_nvtx"    );
  m_vtx_eff_vs_ntracks = TIDA::Histogram2D<float>   ( monTool(), "vtx_eff_vs_ntracks" );
  
}


void IDTPM2::VertexPlots::ParHistograms::initialize( VertexPlots* p, const std::string& s ) {
  
  //  MY_MSG_DEBUG <<  "Booking vertex parameters plots in "  << endmsg; 
  
  m_nVtx         	 = TIDA::Histogram<float>(   p->monTool(),  "nVtx",             s );  
  m_nVtx_vs_actualMu	 = TIDA::Histogram2D<float>( p->monTool(),  "nVtx_vs_actualMu", s );

  /// vertex parameters plots
  m_vtx_x	 = TIDA::Histogram<float>( p->monTool(),          "vtx_x", s );
  m_vtx_y	 = TIDA::Histogram<float>( p->monTool(),          "vtx_y", s );
  m_vtx_z	 = TIDA::Histogram<float>( p->monTool(),          "vtx_z", s );
  m_vtx_time	 = TIDA::Histogram<float>( p->monTool(),       "vtx_time", s );
  //  if( m_vertexType != "truth" ) {
  /// only for reco vertices
  m_vtx_x_err	 = TIDA::Histogram<float>( p->monTool(),      "vtx_x_err", s );
  m_vtx_y_err	 = TIDA::Histogram<float>( p->monTool(),      "vtx_y_err", s );
  m_vtx_z_err	 = TIDA::Histogram<float>( p->monTool(),      "vtx_z_err", s );
  m_vtx_time_err	 = TIDA::Histogram<float>( p->monTool(),   "vtx_time_err", s );
  m_vtx_chi2OverNdof	 = TIDA::Histogram<float>( p->monTool(),  "vtx_chi2OverNdof", s );
  m_vtx_type	 = TIDA::Histogram<float>( p->monTool(),          "vtx_type", s );
  //  }
  
  //if( m_doTrackPlots ) {
  /// vertex-associated tracks plots
  m_vtx_nTracks	         = TIDA::Histogram<float>( p->monTool(),        "vtx_nTracks",s );
  m_vtx_track_weight	 = TIDA::Histogram<float>( p->monTool(),   "vtx_track_weight", s );
  m_vtx_track_pt	 = TIDA::Histogram<float>( p->monTool(),       "vtx_track_pt", s );
  m_vtx_track_eta	 = TIDA::Histogram<float>( p->monTool(),      "vtx_track_eta", s );
  m_vtx_track_nSiHits	 = TIDA::Histogram<float>( p->monTool(),  "vtx_track_nSiHits", s );
  m_vtx_track_nSiHoles	 = TIDA::Histogram<float>( p->monTool(), "vtx_track_nSiHoles", s );
  m_vtx_track_d0	 = TIDA::Histogram<float>( p->monTool(),       "vtx_track_d0", s );
  m_vtx_track_z0	 = TIDA::Histogram<float>( p->monTool(),       "vtx_track_z0", s );
  m_vtx_track_d0_err	 = TIDA::Histogram<float>( p->monTool(),   "vtx_track_d0_err", s );
  m_vtx_track_z0_err	 = TIDA::Histogram<float>( p->monTool(),   "vtx_track_z0_err", s );

  //  }
  //  } else {
  /// vertex multiplicity vs pileup plots
  //  m_nVtx_vs_actualMu_2D	 = TIDA::Histogram<float>( monTool(),    "nVtx_vs_actualMu_2D" );
  // if( m_doTruthMuPlots ) {
  // m_nVtx_vs_truthMu_2D	 = TIDA::Histogram<float>( monTool(),    "nVtx_vs_truthMu_2D" );
  // m_nVtx_vs_truthMu	 = TIDA::Histogram<float>( monTool(),       "nVtx_vs_truthMu" );
  // }

}


void IDTPM2::VertexPlots::ParHistograms::fill( const VertexView* v ) const {

    /// vertex parameters plots
    float vx    = v->x();
    float vy    = v->y();
    float vz    = v->z();

    m_vtx_x.Fill( vx );
    m_vtx_y.Fill( vy );
    m_vtx_z.Fill( vz );
    
    /// only for reco vertices
    float vxErr  = v->dx(); 
    float vyErr  = v->dy(); 
    float vzErr  = v->dz();
    
    m_vtx_x_err.Fill( vxErr ); 
    m_vtx_y_err.Fill( vyErr ); 
    m_vtx_z_err.Fill( vzErr );
    
    //  float vtimeErr  = timeErr( vertex );
    
    float vchi2   = v->chiSquared();
    float vndof   = v->numberDoF();
    
    if ( vndof!=0 ) m_vtx_chi2OverNdof.Fill( vchi2/vndof );

    float vtype  = v->vertexType();

    /// why do we even want this time if it is not
    /// even available ???
    //    float vtime = v->time();
    
    //    bool vhasValidTime = bool( hasValidTime( vertex ) );

    /// but what if it doesn't have a valid time ?
    /// why are we still filling everything else
    /// in that case ? 
    // if( vhasValidTime ) {
    //   m_vtx_time.Fill( vtime );
    //   m_vtx_time_err.fill(  vtimeErr, weight );
    // }
    
    m_vtx_type.Fill( vtype );

    
    /// vertex multiplicity vs pileup plots
    
    size_t nvTracks = v->nTrackParticles();
    
    m_vtx_nTracks.Fill( nvTracks );

#if 0
    /// need to work out how we actually want get the track-to-vertex
    /// associated traxks - probably add them to the VertexView
    
    for( size_t it=0 ; it<nvTracks ; it++ ) {
      m_vtx_track_weight.Fill(  associatedTrackWeights[it], weight );
      
      float tpt       = pT( *associatedTracks[it] ) / Gaudi::Units::GeV;
      float teta      = eta( *associatedTracks[it] );
      float tnSiHits  = nSiHits( *associatedTracks[it] );
      float tnSiHoles = nSiHoles( *associatedTracks[it] );
      float td0       = d0( *associatedTracks[it] );
      float tz0       = z0( *associatedTracks[it] ) - vz;
      float td0Err    = error( *associatedTracks[it], Trk::d0 );
      // TODO: should this also account for the vertex z error?
      float tz0Err    = error( *associatedTracks[it], Trk::z0 );

      m_vtx_track_pt.Fill(        tpt       ); //,  weight );
      m_vtx_track_eta.Fill(       teta      ); //,  weight );
      m_vtx_track_nSiHits.Fill(   tnSiHits  ); //,  weight );
      m_vtx_track_nSiHoles.Fill(  tnSiHoles ); //,  weight );
      m_vtx_track_d0.Fill(        td0       ); // ,    weight );
      m_vtx_track_z0.Fill(        tz0       ); // ,     weight );
      m_vtx_track_d0_err.Fill(    td0Err    ); // ,     weight );
      m_vtx_track_z0_err.Fill(    tz0Err    ); // ,     weight );
    } // close loop over tracks
#endif
    
}


void IDTPM2::VertexPlots::execute( const TrackAnalysisStore& collections ) const {

  //  long    lumiblock = 0;
  double  mu        = 0;

  const xAOD::EventInfo* eventinfo = collections.eventInfo();

  if (eventinfo) {
    /// don't need run and event number ??
    //    runnumber = collections.eventInfo()->runNumber(); 
    //    eventid   = collections.eventInfo()->eventNumber(); 
    //    lumiblock = collections.eventInfo()->lumiBlock(); 
    mu  = collections.eventInfo()->averageInteractionsPerCrossing();
  }

  const std::vector<const VertexView*>& ref  = collections.refvertices();
  const std::vector<const VertexView*>& test = collections.testvertices();

  /// multiplicity histograms ...
  
  m_vtx_ntest_vs_nref.Fill( ref.size(), test.size() );
  
  m_reference.m_nVtx.Fill( ref.size() );
  m_reference.m_nVtx_vs_actualMu.Fill( mu, ref.size() );
  
  m_test.m_nVtx.Fill( test.size() );
  m_test.m_nVtx_vs_actualMu.Fill( mu, test.size() );

  /// fill parameter histograms ...

  for ( size_t i=0 ;  i<ref.size() ; i++ ) {  m_reference.fill(  ref[i] ); }
  for ( size_t i=0 ; i<test.size() ; i++ ) {       m_test.fill( test[i] ); }

  // std::cout << "ref: " << ref.size() << "\ttest: " << test.size() << std::endl; 

  // now residuals andc efficiencies ...

  const TIDA::Associator<const VertexView>* matcher = collections.vertex_matcher();

  for ( size_t i=0 ; i<ref.size() ; i++ ) {  

    const VertexView* vr = ref[i];

    const VertexView* vt = matcher->matched( ref[i] );

    bool matched = false;
    
    if ( vt ) matched = true;

    size_t nvTracks = vr->nTrackParticles();

    m_vtx_eff_vs_z.Fill(        vr->z(), matched );
    m_vtx_eff_vs_mu.Fill(            mu, matched );
    m_vtx_eff_vs_nvtx.Fill(  ref.size(), matched );
    m_vtx_eff_vs_ntracks.Fill( nvTracks, matched );
    
    /// could loop over these independently, but we have the matcheing
    /// information, so might as well do it here ...

    if ( matched ) {

      float dx = vt->x() - vr->x();
      float dy = vt->y() - vr->y();
      float dz = vt->z() - vr->z();
      
      m_vtx_res_x.Fill( dx );
      m_vtx_res_y.Fill( dy );
      m_vtx_res_z.Fill( dz );

      m_vtx_res_z_vs_z.Fill(       vr->z(), dz );
      m_vtx_res_z_vs_ntracks.Fill( nvTracks, dz );
      m_vtx_res_z_vs_nvtx.Fill(    ref.size(), dz );
      m_vtx_res_z_vs_mu.Fill(      mu, dz );

    }

  }

  /// in the original IDTPM code information on the associated
  /// tracks and weights was passed in separately. Really, if need
  /// them, they should be added to the vertex itself - so they would be
  /// added as methods on the VertexView. Will come back to this ...
  
  // if( m_doTrackPlots ) {
  //    size_t nvTracks = associatedTracks.size();
  //    if( nvTracks != associatedTrackWeights.size() ) {
  //      MY_MSG_ERROR <<  "Vertex-associated track number mismatch"  << endmsg;
  //      return StatusCode::FAILURE;
  //    }
  // 
  
  // if( m_doTruthMuPlots ) {
  //    m_nVtx_vs_truthMu_2D.fill(  truthMu,    nVertices,  weight );
  //    m_nVtx_vs_truthMu.fill(     truthMu,    nVertices,  weight );
  // }

}


/// -------------------------
/// ----- finalizePlots -----
/// -------------------------
void IDTPM2::VertexPlots::finalize() {
  
  MY_MSG_DEBUG <<  "Finalising vertex parameters plots"  << endmsg;

  std::string realpath = realPath( "/Vertex/" ); // basestr+stub;

  MY_MSG_DEBUG  << "realpath: " << realpath << endmsg;
   
  processResolution( realpath+"vtx_res_z_vs_z/" );
  processResolution( realpath+"vtx_res_z_vs_ntracks/" );
  processResolution( realpath+"vtx_res_z_vs_nvtx/" );
  processResolution( realpath+"vtx_res_z_vs_mu/" );

  //  std::cout << "resplot: " << name() << "\t" << m_vtx_res_z_vs_z.name()       << std::endl;
  //  std::cout << "resplot: " << name() << "\t" << m_vtx_res_z_vs_ntracks.name() << std::endl;
  //  std::cout << "resplot: " << name() << "\t" << m_vtx_res_z_vs_nvtx.name()    << std::endl;
  //  std::cout << "resplot: " << name() << "\t" << m_vtx_res_z_vs_mu.name()      << std::endl;

}
