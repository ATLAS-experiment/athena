/**
 ** @file    Efficiencies.cxx
 ** @author  sutt@cern.ch 
 **
 **         based on a class by Marco Aparo <marco.aparo@cern.ch>
 **
 **  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **/ 

#include "Efficiencies.h"


/// -----------------------
/// ----- Constructor -----
/// -----------------------
IDTPM2::Efficiencies::Efficiencies( const std::string& name, ToolHandle<GenericMonitoringTool>* m ) 
  : IPlotBase( name, m )
{ }

/// ---------------------------
/// --- Book the histograms ---
/// ---------------------------
void IDTPM2::Efficiencies::initialize() {
  
  //   MY_MSG_DEBUG <<  "Booking efficiency plots in " << getDirectory()  << endmsg; 

  m_eff_vs_inclusive	 = TIDA::Histogram2D<float,bool>( monTool(), "eff_vs_inclusive" );
  m_eff_vs_ntracks	 = TIDA::Histogram2D<float,bool>( monTool(), "eff_vs_ntracks" );
  m_eff_vs_pt	         = TIDA::Histogram2D<float,bool>( monTool(), "eff_vs_pt" );
  // may still want these in the future ...
  //  m_eff_vs_logPt	 = TIDA::Histogram2D<float,bool>( monTool(), "eff_vs_logPt" );
  //  m_eff_vs_lowPt	 = TIDA::Histogram2D<float,bool>( monTool(), "eff_vs_lowPt" );
  m_eff_vs_eta	         = TIDA::Histogram2D<float,bool>( monTool(), "eff_vs_eta" );
  m_eff_vs_phi	         = TIDA::Histogram2D<float,bool>( monTool(), "eff_vs_phi" );
  m_eff_vs_d0	         = TIDA::Histogram2D<float,bool>( monTool(), "eff_vs_d0" );
  m_eff_vs_z0	         = TIDA::Histogram2D<float,bool>( monTool(), "eff_vs_z0" );

  m_eff_vs_Et	         = TIDA::Histogram2D<float,bool>( monTool(), "eff_vs_Et" );
  m_eff_vs_EtovPt	 = TIDA::Histogram2D<float,bool>( monTool(), "eff_vs_EtovPt" );

  
  /// why would we ever not want the efficiency vs mu ? 
  m_eff_vs_actualMu      = TIDA::Histogram2D<float,bool>( monTool(), "eff_vs_actualMu" );
 
  /// for later ...
  // if( m_doTruthMuPlots ) m_eff_vs_truthMu = TIDA::Histogram<float,bool>( monTool(), "eff_vs_truthMu" );
  // }

  ///  m_eff_vs_eta_vs_pt	 = TIDA::Histogram<float,bool>( monTool(), "eff_vs_eta_vs_pt" );
  ///  m_eff_vs_eta_vs_phi	 = TIDA::Histogram<float,bool>( monTool(), "eff_vs_eta_vs_phi" );
  ///  m_eff_vs_z0_vs_d0	 = TIDA::Histogram<float,bool>( monTool(), "eff_vs_z0_vs_d0" );
  ///  m_eff_vs_z0sin_vs_d0	 = TIDA::Histogram<float,bool>( monTool(), "eff_vs_z0sin_vs_d0" );


}


/// -----------------------------
/// --- Dedicated fill method ---
/// -----------------------------
void IDTPM2::Efficiencies::execute( const TrackAnalysisStore& collections ) const {  

  const std::vector<const TrackView*>& ref  = collections.reference_tracks();

  /// don't need the actual test tracks
  //  const std::vector<const TrackView*>& test = collections.test_tracks();

  double mu = collections.eventInfo()->actualInteractionsPerCrossing();
    
  const TIDA::Associator<const TrackView>* matcher = collections.matcher();

  for ( size_t i=0 ; i<ref.size(); i++ ) {

    const TrackView& p = *ref[i];

    bool matched = false;

    /// is there a matched track ??
    if ( matcher->matched(ref[i]) ) matched = true;
  
    /// Compute track parameters
    float ppt    = p.pt() / Gaudi::Units::GeV;
    float peta   = p.eta();
    float pphi   = p.phi();
    float pd0    = p.d0();
    float pz0    = p.z0();

    float Et     = p.ET() / Gaudi::Units::GeV;

    if ( p.hasParent() ) Et = p.parent()->ET()*0.001;
    
    float EtovPt = Et/ppt; // pt should never be 0 so don't bother to check that
    
    /// Fill the histograms
    /// do we really need the weighted histograms ???
    m_eff_vs_inclusive.Fill( 1,    matched );

    m_eff_vs_ntracks.Fill( ref.size(),  matched );

    m_eff_vs_pt.Fill(        ppt,  matched );
    //    m_eff_vs_logPt.Fill(     ppt,  matched );
    //    m_eff_vs_lowPt.Fill(     ppt,  matched );
    m_eff_vs_eta.Fill(       peta, matched );
    m_eff_vs_phi.Fill(       pphi, matched );
    m_eff_vs_d0.Fill(        pd0,  matched );
    m_eff_vs_z0.Fill(        pz0,  matched );

    m_eff_vs_Et.Fill(         Et,  matched );
    m_eff_vs_EtovPt.Fill( EtovPt,  matched );

    m_eff_vs_actualMu.Fill(   mu,  matched );

    /// do we really even want these ???
    //  m_eff_vs_eta_vs_pt.Fill(    ppt, matched, ppt );
    //  m_eff_vs_eta_vs_phi.Fill(   pphi,  pet, matched );
    //  m_eff_vs_z0_vs_d0.Fill(     pd0,   pz0, matched );
    //  float ptheta = p.theta(); /// no point setting it unless it is needed
    //  m_eff_vs_z0sin_vs_d0.Fill(  pz0*std::sin(ptheta), pd0, matched );
    
    /// truth mu ??
    //    if( m_doTruthMuPlots ) m_eff_vs_truthMu.Fill(	matched, matched );
  }
}


									  
/// -------------------------
/// ----- finalizePlots -----
/// -------------------------
void IDTPM2::Efficiencies::finalize() {  MY_MSG_DEBUG <<  "Finalising efficiency plots"  << endmsg; }


