/**
 ** @file    TrackParameters.cxx
 ** @author  sutt@cern.ch
 **
 **  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 **
 **/


/// local include(s)
#include "TrackParameters.h"


IDTPM2::TrackParameters::TrackParameters( const std::string& name, ToolHandle<GenericMonitoringTool>* m ) 
  : IPlotBase( name, m )
{ }


void IDTPM2::TrackParameters::initialize() {
  MY_MSG_DEBUG <<  "Booking track parameters plots in " << name()  << endmsg; 
  m_reference.initialize( this, "reference" );
  m_test.initialize(      this, "test" );
  
  m_invmass = TIDA::Histogram<float>( monTool(),  "invmass" );
}


void IDTPM2::TrackParameters::ParHistograms::initialize( TrackParameters* p, const std::string& s )
{
  m_pt	 = TIDA::Histogram<float>( p->monTool(),  "track_pt",  s );
  m_eta	 = TIDA::Histogram<float>( p->monTool(),  "track_eta", s );
  m_phi	 = TIDA::Histogram<float>( p->monTool(),  "track_phi", s );
  m_d0	 = TIDA::Histogram<float>( p->monTool(),  "track_d0",  s );
  m_z0	 = TIDA::Histogram<float>( p->monTool(),  "track_z0",  s );
  m_z0sin	 = TIDA::Histogram<float>( p->monTool(),  "track_z0sin",  s );
  m_theta	 = TIDA::Histogram<float>( p->monTool(),  "track_theta",  s );
  m_qoverp	 = TIDA::Histogram<float>( p->monTool(),  "track_qoverp", s );
  //  if( m_trackType == "truth" ) {
  //  m_prodR	 = TIDA::Histogram<float>( p->monTool(), "track_prodR",   s );
  //  m_prodZ	 = TIDA::Histogram<float>( p->monTool(), "track_prodZ",   s );
  m_nSiHits	   = TIDA::Histogram<float>(   p->monTool(), "track_nSiHits", s );
  m_nSiHits_vs_eta = TIDA::Histogram2D<float>( p->monTool(), "track_nSiHits_vs_eta", s );
  // } else {
 
  m_Et	   = TIDA::Histogram<float>( p->monTool(),  "track_Et",     s );
  m_EtovPt = TIDA::Histogram<float>( p->monTool(),  "track_EtovPt", s );


  m_chi2	 = TIDA::Histogram<float>( p->monTool(), "track_chi2",   s );
  m_ndof	 = TIDA::Histogram<float>( p->monTool(), "track_ndof",   s );
  m_chi2OverNdof = TIDA::Histogram<float>( p->monTool(), "track_chi2OverNdof", s );


  m_author	 = TIDA::Histogram<float>( p->monTool(), "track_author", s );

  // m_time	 = TIDA::Histogram<float>( p->monTool(), "track_time",   s );
  // m_hasValidTime_eff_vs_eta	 = TIDA::Histogram<float>( p->monTool(), "track_hasValidTime_eff_vs_eta", s );

  /// what are all these plots for ?? why do we want z0 vs d0 ???
  /// surely we want everything vs pt, eta, d0 and phi
  //  m_eta_vs_pt	 = TIDA::Histogram2D<float>( p->monTool(), "track_eta_vs_pt",   s );
  //  m_eta_vs_phi	 = TIDA::Histogram2D<float>( p->monTool(), "track_eta_vs_phi",  s );
  //  m_z0_vs_d0	 = TIDA::Histogram2D<float>( p->monTool(), "track_z0_vs_d0",    s );
  //  m_z0sin_vs_d0	 = TIDA::Histogram2D<float>( p->monTool(), "track_z0sin_vs_d0", s );
  
  /// sigma plots ...
  m_sigma_pt	 = TIDA::Histogram<float>( p->monTool(), "sigma_pt",    s );
  m_sigma_eta	 = TIDA::Histogram<float>( p->monTool(), "sigma_eta",   s );
  m_sigma_phi	 = TIDA::Histogram<float>( p->monTool(), "sigma_phi",   s );
  m_sigma_d0	 = TIDA::Histogram<float>( p->monTool(), "sigma_d0",    s );
  m_sigma_z0	 = TIDA::Histogram<float>( p->monTool(), "sigma_z0",    s );
  m_sigma_z0sin	 = TIDA::Histogram<float>( p->monTool(), "sigma_z0sin", s );
  m_sigma_theta	 = TIDA::Histogram<float>( p->monTool(), "sigma_theta", s );
  
  /// significance plots ...
  /// why do we need significances for eta, phi etc ???
  // m_significance_pt	 = TIDA::Histogram<float>( p->monTool(), "significance_pt",  s );
  // m_significance_eta	 = TIDA::Histogram<float>( p->monTool(), "significance_eta", s );
  // m_significance_phi	 = TIDA::Histogram<float>( p->monTool(), "significance_phi", s );
  m_significance_d0	 = TIDA::Histogram<float>( p->monTool(), "significance_d0",  s );
  // m_significance_z0	 = TIDA::Histogram<float>( p->monTool(), "significance_z0",  s );
  // m_significance_z0sin	 = TIDA::Histogram<float>( p->monTool(), "significance_z0sin", s );
  // m_significance_theta	 = TIDA::Histogram<float>( p->monTool(), "significance_theta", s );

}




void IDTPM2::TrackParameters::execute( const TrackAnalysisStore& collections ) const {
  
  const std::vector<const TrackView*>& ref  = collections.reference_tracks();
  const std::vector<const TrackView*>& test = collections.test_tracks();

  /// do we want the matched tracks as well ?? it would be trivial to add them,
  /// or add them instead of the full set of test tracks, ie only the test tracks
  /// with a refernce match - that would be only about four extra lines.
  
  for ( size_t i=0 ; i<ref.size()  ; i++ ) m_reference.fill(  ref[i] );
  for ( size_t i=0 ; i<test.size() ; i++ )      m_test.fill( test[i] );

  /// could put this in the loop over the reference objects, but then it
  /// would call hasTagNProbe() and if (tnp) for every reference track, 
  /// and even for non tag-n-probe analyses - the beuaty of using execute()
  /// methods rather than fill methods for individual tracks all called
  /// from a single restrictive loop over the tracks
  
  if ( collections.hasTagNProbe() ) {  
    for ( size_t i=0 ; i<ref.size()  ; i++ ) {
      std::vector<double> masses = collections.tagNProbe()->masses( ref[i] );
      for ( size_t j=masses.size() ; j-- ; ) m_invmass.Fill( masses[i] );
    }
  }
  
}



void IDTPM2::TrackParameters::ParHistograms::fill( const TrackView* p ) const {

    /// Compute track parameters
    float ppt    = p->pt() / Gaudi::Units::GeV;
    float peta   = p->eta();

    float ptheta = p->theta();

    float pphi   = p->phi();
    float pd0    = p->d0();

    /// if( m_recomputeIP ) pd0 = getD0TrackInJet( particle ); 
    float pz0     = p->z0();
    float pz0sin  = p->z0()*std::sin(ptheta);
    float pqoverp = p->qOverP()*1000; // in GeV^-1
    // float pprodR  = prodR( particle );
    // float pprodZ  = prodZ( particle );
    // float pnSCTHits = p->nSCT();
    // float pnPixels  = p->nPixels();
    float pnSiHits = p->nSi();
    float pchi2    = p->chi2();
    float pndof    = p->ndof();
    /// what is this ???
    /// float pchi2OverNdof = ( pndof > 0 ) ? pchi2 / pndof : 0.;
    /// ????
    /// std::vector< unsigned int > pauthor = author( particle );
    
    /// Fill the histograms
    m_pt.Fill(     ppt    );
    m_eta.Fill(    peta   );
    m_phi.Fill(    pphi   );
    m_d0.Fill(     pd0    );
    m_z0.Fill(     pz0    );
    m_z0sin.Fill(  pz0sin  );
    m_theta.Fill(  ptheta  );
    m_qoverp.Fill( pqoverp );
    //    if( m_trackType == "truth" ) {
    //   m_prodR.fill( pprodR);
    //   m_prodZ.fill( pprodZ);
    m_nSiHits.Fill( pnSiHits);
    m_nSiHits_vs_eta.Fill( peta, pnSiHits );

    float ET = p->ET()*0.001;
    
    if ( p->hasParent() ) ET = p->parent()->ET()*0.001;

    /// useful debug - leave here for the time being ...
    //    if ( p->hasParent() ) std::cout << "\tPARENT: " << p << "\t" << p->parent() << "\t:: " << ET << "\t:: " << (ET/ppt) << std::endl;

    m_Et.Fill( ET );
    m_EtovPt.Fill( std::fabs(ET/ppt) );
    
    //  } else {
    m_chi2.Fill( pchi2  );
    m_ndof.Fill( pndof  );
    if ( pndof ) m_chi2OverNdof.Fill( pchi2/pndof ); /// ndof should never be 0 for a real particle
    //  for( unsigned int i=0; i<pauthor.size(); i++ ) {
    //	  m_author.Fill( pauthor[i]);
    //  }
    //  static const SG::Accessor< uint8_t > accValidTime("hasValidTime");
    //  static const SG::Accessor< float > accTime("time");
    //  if( accValidTime.isAvailable( particle ) ) {
    //	  bool phasValidTime = hasValidTime( particle );
    //	  m_hasValidTime_eff_vs_eta.Fill( peta, phasValidTime);
    //	  if( accTime.isAvailable( particle ) and phasValidTime ) {
    //	    m_time.fill( time( particle ));
    //    }
    //  }
    //  m_eta_vs_pt.Fill(  ppt,  peta ); /// eta vs pt ???? Why ???
    //  m_eta_vs_phi.Fill( pphi, peta );
    //  m_z0_vs_d0.Fill(   pd0,  pz0  );
    //  m_z0sin_vs_d0.Fill( pz0*std::sin(ptheta), pd0 );
    
    //    if( m_plotErrors ) {
    float ppt_err  = p->dpt() / Gaudi::Units::GeV;
    float peta_err = p->deta(); 
    float pphi_err = p->dphi();
    float pd0_err  = p->d0();
    float pz0_err  = p->dz0();
    //    float pz0sin_err = z0SinThetaError( particle );
    //    float ptheta_err = error( particle, Trk::theta );
    float ptheta_err = p->dtheta();
    
    m_sigma_pt.Fill(    ppt_err   );
    m_sigma_eta.Fill(   peta_err  );
    m_sigma_phi.Fill(   pphi_err  );
    m_sigma_d0.Fill(    pd0_err   );
    m_sigma_z0.Fill(    pz0_err   );
    //   m_sigma_z0sin.Fill( pz0sin_err );
    m_sigma_theta.Fill( ptheta_err );
    

    //   if ( ppt_err    ) m_significance_pt.fill(     ppt/ppt_err    );
    //   if ( peta_err   ) m_significance_eta.fill(    peta/peta_err  );
    /// phi significance ?? but phi is cyclic !
    //   if ( pphi_err   ) m_significance_phi.fill(    pphi/pphi_err  );
    if ( pd0_err ) m_significance_d0.Fill( pd0/pd0_err );
    //   if ( pz0_err ccc) m_significance_z0.fill(     pz0/pz0_err    );
    //   if ( pz0sin_err ) m_significance_z0sin.fill(  pz0sin/pz0sin_err );
    //   if ( ptheta_err ) m_significance_theta.fill(  ptheta/ptheta_err );

}



/// -------------------------
/// ----- finalizePlots -----
/// -------------------------
void IDTPM2::TrackParameters::finalize() { MY_MSG_DEBUG <<  "Finalising track parameters plots"  << endmsg; }
