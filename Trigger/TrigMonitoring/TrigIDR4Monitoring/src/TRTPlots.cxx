/**
 **   @file    TRTPlots.cxx         
 **   
 **
 **   @author sutt
 **   @date   Wed 22 Jul 2026 16:16:38 BST
 **
 **   $Id: TRTPlots.cxx, v0.0   Wed 22 Jul 2026 16:16:38 BST sutt $
 **
 **   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **/

#include "TRTPlots.h"

IDTPM2::TRTPlots::TRTPlots( const std::string& name, ToolHandle<GenericMonitoringTool>* m )
  : IPlotBase( name, m ) { 
}


/// ---------------------------
/// --- Book the histograms ---
/// ---------------------------

void IDTPM2::TRTPlots::initialize() {
  /// parameter histograms ...
  m_reference.initialize( this, "reference");
  m_test.initialize(      this, "test");

}



void IDTPM2::TRTPlots::Histograms::initialize( TRTPlots* p, const std::string& s ) {

  static const std::array<bool,NTRTPars> proc = { true, false, false, true };
  
  for ( size_t i=0 ; i<nParNames() ; i++ ) {

    if ( !proc[i] ) continue;
    
    m_hits[i] = TIDA::Histogram<float>( p->monTool(), parName(i), s );
  
    /// should use Resplots for these really, as profiles are affected by outlyera...

    m_hits_vs_pt[i]  = TIDA::Histogram<float>( p->monTool(), parName(i)+"_vs_pt",  s );
    m_hits_vs_eta[i] = TIDA::Histogram<float>( p->monTool(), parName(i)+"_vs_eta", s );
    m_hits_vs_phi[i] = TIDA::Histogram<float>( p->monTool(), parName(i)+"_vs_phi", s );
    m_hits_vs_d0[i]  = TIDA::Histogram<float>( p->monTool(), parName(i)+"_vs_d0",  s );
 
  }
  
}


void  IDTPM2::TRTPlots::Histograms::fill( const TrackView* t ) const {
  

  /// Compute hit parameters
  std::array<float,NTRTPars> hitP = {};

  hitP[NTRTHits] = t->nTRT();
  //  hitP[NTRTHitsXe]   
  //  hitP[ NTRTHitsAr]  
  hitP[NTRTHighThresholdHits]  = t->nTRTHi(); 
  //  hitP[NTRTHighThresholdHitsXe] 
  //  hitP[NTRTHighThresholdHitsAr] 
  
  //  hitP[NTRTOutliers]
  //  hitP[NTRTHighThresholdOutliers] 
  
    
  float pt  = std::fabs(t->pt())*0.001;
  float eta = t->eta();
  float phi = t->phi();
  float d0  = t->d0();

  
  //  std::cout << "TRTs: track: " << *t << std::endl; 

  static const std::array<bool,NTRTPars> proc = { true, false, false, true };
  
  for ( int i=0 ; i<NTRTPars ; i++ ) {

    if ( !proc[i] ) continue;
    
    //    std::cout << "\tfill par; " << hitParName(i) << "\t: " << hitP[i] << std::endl;
    
    m_hits[i].Fill( hitP[i] );

    m_hits_vs_pt[i].Fill(  pt,  hitP[i] );
    m_hits_vs_eta[i].Fill( eta, hitP[i] );
    m_hits_vs_phi[i].Fill( phi, hitP[i] );
    m_hits_vs_d0[i].Fill(  d0,  hitP[i] );
  }
  
}



void IDTPM2::TRTPlots::execute( const TrackAnalysisStore& collections ) const {
  
#if 0
  /// if we need histograms vs mu ...
  
  double  mu = 0;
  
  const xAOD::EventInfo* eventinfo = collections.eventInfo();
  
  if (eventinfo) {
    /// don't need run and event number ??
    mu  = collections.eventInfo()->averageInteractionsPerCrossing();
  }
#endif
  
  const std::vector<const TrackView*>& ref  = collections.reference_tracks();
  const std::vector<const TrackView*>& test = collections.test_tracks();

  for ( size_t i=0 ;  i<ref.size() ; i++ ) {  m_reference.fill(  ref[i] ); }
  for ( size_t i=0 ; i<test.size() ; i++ ) {       m_test.fill( test[i] ); }
  
  /// do we want to do anything looping over the matched histograms ??

}
  
  
  /// Print out final stats on histograms
void IDTPM2::TRTPlots::finalize() { } 
  
