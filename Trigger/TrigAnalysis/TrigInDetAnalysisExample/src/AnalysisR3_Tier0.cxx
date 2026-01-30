/**
 **     @file    AnalysisR3_Tier0.cxx
 **
 **     @author  mark sutton
 **     @date    $Id: AnalysisR3_Tier0.cxx   Thu 18 May 2017 15:35:34 CEST 
 **
 **     Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **/


#include <cmath>

#include "TrigInDetAnalysis/Track.h"
#include "TrigInDetAnalysisExample/AnalysisR3_Tier0.h"
#include "TrigInDetAnalysisExample/VtxR3Analysis.h"

#include "TrigInDetAnalysisExample/ChainString.h"

AnalysisR3_Tier0::AnalysisR3_Tier0(const std::string& name,
                               double /*pTCut*/,
                               double /*etaCut*/,
                               double /*d0Cut*/,
                               double /*z0Cut*/)
  : TrackAnalysis(name),
    // m_pTCut(pTCut), /// left in commented for development purposes
    // m_etaCut(etaCut),
    // m_d0Cut(d0Cut),
    // m_z0Cut(z0Cut),
    m_nVtx(0),
    m_debug(false),
    m_eventid(0),
    m_vtxanal(0),
    m_monTool(0)
{}

void AnalysisR3_Tier0::initialise() {
  if ( m_monTool ) initialise_R3();
}

void AnalysisR3_Tier0::initialise_R2() { }

void AnalysisR3_Tier0::initialise_R3() {

  ChainString cname = name();

  //  std::cout << "AnalysisR3_Tier0::initialise() name " << name() << std::endl; 

#if 0
  /// here for development purposes ...
  std::cout << "\nAnalysisR3_Tier0:: chain specification: " << cname << "\t" << cname.raw() << std::endl;
  std::cout << "\tchain: " << cname.head()    << std::endl;
  std::cout << "\tkey:   " << cname.tail()    << std::endl;
  std::cout << "\troi:   " << cname.roi()     << std::endl;
  std::cout << "\tvtx:   " << cname.vtx()     << std::endl;
  std::cout << "\tte:    " << cname.element() << std::endl;
#endif

  m_debug = false;

  m_hchain = TIDA::Histogram<float>( monTool(),  "Chain" );

  m_hroieta = TIDA::Histogram<float>( monTool(),  "roi_eta" );

  /// Limit the bins - to only the first 77 bins - so a range up to ~ 1000
  /// leave the previous selection commented for the time being

  m_hntrk = TIDA::Histogram<float>( monTool(),  "reftrk_N" );

  /// reference track distributions

  m_htrkpT  = TIDA::Histogram<float>( monTool(), "reftrk_pT" );
  m_htrkphi = TIDA::Histogram<float>( monTool(), "reftrk_phi" );
  m_htrketa = TIDA::Histogram<float>( monTool(), "reftrk_eta" );
  m_htrkd0  = TIDA::Histogram<float>( monTool(), "reftrk_d0" );
      
  m_htrkz0  = TIDA::Histogram<float>( monTool(), "reftrk_z0" );

  /// the error estimates are always positive ...
  m_htrkdd0  = TIDA::Histogram<float>( monTool(), "reftrk_dd0" );
  m_htrkdz0  = TIDA::Histogram<float>( monTool(), "reftrk_dz0" );

  m_htrkd0sig = TIDA::Histogram<float>( monTool(), "reftrk_d0sig" );


  /// test track distributions

  /// Limit the bins - to only the first 77 bins - so a range up to ~ 1000
  /// leave the previous selection commented for the time being
  //  m_hntrk_rec = TIDA::Histogram<float>( monTool(),  "testtrk_N", "Test tracks", 100, vnbins );
  m_hntrk_rec = TIDA::Histogram<float>( monTool(),  "testtrk_N" );


  //  m_htrkpT_rec  = TIDA::Histogram<float>( monTool(), "testtrk_pT" , "Test track pT",  25,    0.,   100.);
  m_htrkpT_rec  = TIDA::Histogram<float>( monTool(), "testtrk_pT" );
  m_htrkphi_rec = TIDA::Histogram<float>( monTool(), "testtrk_phi" );
  m_htrketa_rec = TIDA::Histogram<float>( monTool(), "testtrk_eta" );
  
  m_htrkd0_rec  = TIDA::Histogram<float>( monTool(), "testtrk_d0" );
 
  m_htrkz0_rec  = TIDA::Histogram<float>( monTool(), "testtrk_z0" );

  m_htrkdd0_rec  = TIDA::Histogram<float>( monTool(), "testtrk_dd0" );
  m_htrkdz0_rec  = TIDA::Histogram<float>( monTool(), "testtrk_dz0" );

  m_htrkd0sig_rec = TIDA::Histogram<float>( monTool(), "testtrk_d0sig" );



  /// trigger tracking efficiencies


  m_htotal_efficiency = TIDA::Histogram<float>( monTool(), "Eff_overall" );

  m_hpTeff    = TIDA::Histogram<float>( monTool(),  "Eff_pT" );
  m_hetaeff   = TIDA::Histogram<float>( monTool(),  "Eff_Eta" );
  m_hphieff   = TIDA::Histogram<float>( monTool(),  "Eff_Phi" );
  /// different binning for the histograms for the LRT instances now defined
  /// in the python file 
  m_hd0eff    = TIDA::Histogram<float>( monTool(),  "Eff_d0" );
  m_hz0eff    = TIDA::Histogram<float>( monTool(),  "Eff_z0" );
  m_hnVtxeff  = TIDA::Histogram<float>( monTool(),  "Eff_nVtx" );
  m_hntraxeff = TIDA::Histogram<float>( monTool(),  "Eff_ntrax" );

  
  m_hlbeff = TIDA::Histogram<float>( monTool(),  "Eff_lb" );


  m_htrkvtx_x_lb = TIDA::Histogram<float>( monTool(),  "trkvtx_x_vs_lb" );
  m_htrkvtx_y_lb = TIDA::Histogram<float>( monTool(),  "trkvtx_y_vs_lb" );
  m_htrkvtx_z_lb = TIDA::Histogram<float>( monTool(),  "trkvtx_z_vs_lb" );

  /// do we want to track the offline vertex ??? 
  /// leave this in in preparation ...
  //  m_hotrkvtx_x_lb = TIDA::Histogram<float>( monTool(),  "otrkvtx_x_vs_lb" );
  //  m_hotrkvtx_y_lb = TIDA::Histogram<float>( monTool(),  "otrkvtx_y_vs_lb" );
  //  m_hotrkvtx_z_lb = TIDA::Histogram<float>( monTool(),  "otrkvtx_z_vs_lb" );


  /// han config too stufid to deal with spaces in histogram names
  m_hnpixvseta     = TIDA::Histogram<float>( monTool(), "npix_vs_eta" );
  m_hnpixvseta_rec = TIDA::Histogram<float>( monTool(), "npix_vs_eta_rec" );

  m_hnsctvseta     = TIDA::Histogram<float>( monTool(), "nsct_vs_eta" );
  m_hnsctvseta_rec = TIDA::Histogram<float>( monTool(), "nsct_vs_eta_rec" );

  m_hntrtvseta     = TIDA::Histogram<float>( monTool(), "ntrt_vs_eta" );
  m_hntrtvseta_rec = TIDA::Histogram<float>( monTool(), "ntrt_vs_eta_rec" );

  m_hnpixvsphi     = TIDA::Histogram<float>( monTool(), "npix_vs_phi" );
  m_hnpixvsphi_rec = TIDA::Histogram<float>( monTool(), "npix_vs_phi_rec" );

  m_hnsctvsphi     = TIDA::Histogram<float>( monTool(), "nsct_vs_phi" );
  m_hnsctvsphi_rec = TIDA::Histogram<float>( monTool(), "nsct_vs_phi_rec" );

  m_hntrtvsphi     = TIDA::Histogram<float>( monTool(), "ntrt_vs_phi" );
  m_hntrtvsphi_rec = TIDA::Histogram<float>( monTool(), "ntrt_vs_phi_rec" );
  
  m_hnpixvsd0     = TIDA::Histogram<float>( monTool(), "npix_vs_d0" );
  m_hnpixvsd0_rec = TIDA::Histogram<float>( monTool(), "npix_vs_d0_rec" );
  
  m_hnsctvsd0     = TIDA::Histogram<float>( monTool(), "nsct_vs_d0" );
  m_hnsctvsd0_rec = TIDA::Histogram<float>( monTool(), "nsct_vs_d0_rec" );

  
  m_hnpixvspT     = TIDA::Histogram<float>( monTool(), "npix_vs_pT" );
  m_hnpixvspT_rec = TIDA::Histogram<float>( monTool(), "npix_vs_pT_rec" );

  m_hnsctvspT     = TIDA::Histogram<float>( monTool(), "nsct_vs_pT" );
  m_hnsctvspT_rec = TIDA::Histogram<float>( monTool(), "nsct_vs_pT_rec" );
  
  
  m_hnsihits_lb     = TIDA::Histogram<float>( monTool(),  "nsihits_lb" );
  m_hnsihits_lb_rec = TIDA::Histogram<float>( monTool(),  "nsihits_lb_rec" );
  
  
  m_hlayer_rec  = TIDA::Histogram<float>( monTool(), "layer_rec" );
  m_hlayer      = TIDA::Histogram<float>( monTool(), "layer" );

  /// trigger tracking differential resolutions


  m_hpTres  = TIDA::Histogram<float>( monTool(), "Res_pT" );
  m_hipTres = TIDA::Histogram<float>( monTool(), "Res_ipT" );
  m_hetares = TIDA::Histogram<float>( monTool(), "Res_eta" );
  m_hphires = TIDA::Histogram<float>( monTool(), "Res_phi" );
  m_hd0res  = TIDA::Histogram<float>( monTool(), "Res_d0" );
  m_hz0res  = TIDA::Histogram<float>( monTool(), "Res_z0" );


  /// residuals

  m_htrkpT_residual  = TIDA::Histogram<float>( monTool(), "residual_pT" );
  m_htrkipT_residual = TIDA::Histogram<float>( monTool(), "residual_ipT" );
  m_htrkphi_residual = TIDA::Histogram<float>( monTool(), "residual_phi" );
  m_htrketa_residual = TIDA::Histogram<float>( monTool(), "residual_eta" );
  m_htrkd0_residual  = TIDA::Histogram<float>( monTool(), "residual_d0" );
  m_htrkz0_residual  = TIDA::Histogram<float>( monTool(), "residual_z0" );

  m_htrkdd0_residual  = TIDA::Histogram<float>( monTool(), "residual_dd0" );
  m_htrkdz0_residual  = TIDA::Histogram<float>( monTool(), "residual_dz0" );



  m_hnpix     = TIDA::Histogram<float>( monTool(), "npix" );
  m_hnpix_rec = TIDA::Histogram<float>( monTool(), "npix_rec" );

  m_hnsct     = TIDA::Histogram<float>( monTool(), "nsct" );
  m_hnsct_rec = TIDA::Histogram<float>( monTool(), "nsct_rec" );

  m_hnsihits     = TIDA::Histogram<float>( monTool(), "nsiHits" );
  m_hnsihits_rec = TIDA::Histogram<float>( monTool(), "nsiHits_rec" );

  m_hntrt     = TIDA::Histogram<float>( monTool(), "ntrt" );
  m_hntrt_rec = TIDA::Histogram<float>( monTool(), "ntrt_rec" );

  m_chi2dof     = TIDA::Histogram<float>( monTool(), "chi2dof" );
  m_chi2dof_rec = TIDA::Histogram<float>( monTool(), "chi2dof_rec" );
  
  //  m_hmu = TIDA::Histogram<float>( monTool(),  "mu" );
  m_hmu = TIDA::Histogram<float>( monTool(),  "mu" );


  /// miscelaneous histograms

  m_hd0vsphi       = TIDA::Histogram<float>( monTool(),  "d0_vs_phi_prof" );
  m_hd0vsphi_rec   = TIDA::Histogram<float>( monTool(),  "d0_vs_phi_rec_prof" );

  /// should we protect this ? If initialise is called again do we really want 
  /// a new analysis ? Or should we just carry on with the ixisting on, so text 
  /// if m_vtxanal is non zero and skip it if so ? 

  if ( m_vtxanal ) delete m_vtxanal;
  m_vtxanal = 0;

  
#if 1

  /// vertex analyses if required ...
  //  if ( name().find("vtx")!=std::string::npos || name().find("Vtx")!=std::string::npos ||
  //       name().find("vx")!=std::string::npos  || name().find("Vx")!=std::string::npos ) { 

  if ( ChainString(name()).vtx()!="" ) { 

    m_vtxanal = new VtxR3Analysis("VTX");

    m_vtxanal->set_monTool( monTool() );

    /// is this needed ? 
    store().insert( m_vtxanal, "VTX" );

    /// initialise the vtx analysis
    m_vtxanal->initialise();

  }

#endif

}

extern TIDARoiDescriptor* agroi;


void AnalysisR3_Tier0::execute(const std::vector<TIDA::Track*>& referenceTracks,
			       const std::vector<TIDA::Track*>& testTracks,
			       TrackAssociator* associator, 
			       const TIDA::Event* tevt, 
			       double* beamline, 
			       TIDARoiDescriptor* roi) 
{ 
  
  /// Loop over reference tracks
  std::vector<TIDA::Track*>::const_iterator  reference    = referenceTracks.begin();
  std::vector<TIDA::Track*>::const_iterator  referenceEnd = referenceTracks.end();

  /// fill number of times this analysis was called - presumably 
  /// the number of passed RoIs for this chain 
  m_hchain->Fill( 0.5, 1 );

  
  if ( roi!=nullptr ) m_hroieta->Fill( roi->eta(), 1 );

  if ( tevt!=nullptr && m_eventid != tevt->event_number() ) {
    /// if the event number has changed, this is a new event
    /// so update the event counts
    //    m_eventid = event()->event_number(); 
    m_eventid = tevt->event_number(); 
    m_hchain->Fill( 1.5, 1 );

    m_hmu->Fill( tevt->mu() );

  }

  m_hntrk->Fill( referenceTracks.size() );
  m_hntrk_rec->Fill( testTracks.size() );

  /// fil the number of offline tracks
  m_hchain->Fill(4.5, testTracks.size() );

  for( ; reference!=referenceEnd ; ++reference ) {
    
    // Get reference parameters
    double referenceEta = (*reference)->eta();
    double referencePhi = phi((*reference)->phi());
    double referenceZ0  = (*reference)->z0();
    double referenceD0  = (*reference)->a0();
    double referencePT  = (*reference)->pT();

    double referenceDZ0  = (*reference)->dz0();
    double referenceDD0  = (*reference)->da0();
    
    // Find matched tracks
    const TIDA::Track* test = associator->matched(*reference);

    float     eff_weight = 0;
    if (test) eff_weight = 1;

    m_htotal_efficiency->Fill(0.5, eff_weight );

    m_hpTeff->Fill( std::fabs(referencePT)*0.001, eff_weight );
    m_hz0eff->Fill( referenceZ0, eff_weight );
    m_hd0eff->Fill( referenceD0, eff_weight );
    m_hetaeff->Fill( referenceEta, eff_weight );
    m_hphieff->Fill( referencePhi, eff_weight );
    /// m_hnVtxeff->Fill( m_nVtx, eff_weight ); /// don't use the class variable as this is not thread safe
    if (beamline) m_hnVtxeff->Fill( beamline[3], eff_weight ); /// this is a hack to make it thread safe

    if (tevt) m_hlbeff->Fill( tevt->lumi_block(), eff_weight );

    m_hntraxeff->Fill( referenceTracks.size(), eff_weight );
    
    m_htrkpT->Fill( std::fabs(referencePT)*0.001 );
    m_htrketa->Fill( referenceEta );
    m_htrkphi->Fill( referencePhi );
    m_htrkd0->Fill( referenceD0 );
    m_htrkz0->Fill( referenceZ0 );

    m_htrkdd0->Fill( referenceDD0 );
    m_htrkdz0->Fill( referenceDZ0 );

    if ( referenceDD0!=0 )  m_htrkd0sig->Fill( referenceD0/referenceDD0 );

    m_hnpixvseta->Fill( referenceEta,  int(((*reference)->pixelHits()+0.5)*0.5) ); 
    m_hnsctvseta->Fill( referenceEta,  (*reference)->sctHits() ); 
    m_hntrtvseta->Fill( referenceEta,  (*reference)->strawHits() ); 

    if ( (*reference)->dof()!=0 ) m_chi2dof->Fill( (*reference)->chi2()/(*reference)->dof() ); 

    m_hnpixvsphi->Fill( referencePhi,  int(((*reference)->pixelHits()+0.5)*0.5) ); 
    m_hnsctvsphi->Fill( referencePhi,  (*reference)->sctHits() ); 
    m_hntrtvsphi->Fill( referencePhi,  (*reference)->strawHits() ); 

    m_hnpixvsd0->Fill( referenceD0,  int(((*reference)->pixelHits()+0.5)*0.5) ); 
    m_hnsctvsd0->Fill( referenceD0,  (*reference)->sctHits() ); 

    m_hnpixvspT->Fill( std::fabs(referencePT)*0.001,  int(((*reference)->pixelHits()+0.5)*0.5) ); 
    m_hnsctvspT->Fill( std::fabs(referencePT)*0.001,  (*reference)->sctHits() ); 


    m_hnpix->Fill(  int(((*reference)->pixelHits()+0.5)*0.5) ); 
    m_hnsct->Fill(  (*reference)->sctHits() ); 
    m_hnsihits->Fill(  (*reference)->siHits() ); 
    m_hntrt->Fill(  (*reference)->strawHits() ); 
   

    //    m_hnsihits_lb->Fill( event()->lumi_block(), (*reference)->siHits() ); 
    if (tevt) m_hnsihits_lb->Fill( tevt->lumi_block(), (*reference)->siHits() );
 
    m_hd0vsphi->Fill(referencePhi, referenceD0 );
 
    /// fil the number of offline tracks
    m_hchain->Fill(2.5, 1);


    for ( size_t ilayer=0 ; ilayer<32 ; ilayer++ ) { 
      if ( (*reference)->hitPattern()&(1U<<ilayer) ) m_hlayer->Fill( ilayer );
    } 
    
    if (test) {

      m_hchain->Fill(3.5, 1);

      /// NB: do we want to fill the actual *trigger* quantities, or the 
      /// offline quantities for the *matched* tracks?

      /// residual profiles vs the reference variable      
      // m_hpTres->Fill( referencePT*0.001, (test->pT() - referencePT)*0.001 );
      // m_hipTres->Fill( 1000/referencePT, (1000/test->pT() - 1000/referencePT) );
      // m_hetares->Fill( referenceEta, test->eta() - referenceEta );
      // m_hphires->Fill( referencePhi, phi(test->phi() - referencePhi) );
      // m_hd0res->Fill( referenceD0, test->a0() - referenceD0 );
      // m_hz0res->Fill( referenceZ0, test->z0() - referenceZ0  );

      /// residual profiles vs eta - the more easy to understand
      m_hpTres->Fill( referenceEta, (test->pT() - referencePT)*0.001 );
      m_hipTres->Fill( referenceEta, (1000/test->pT() - 1000/referencePT) );
      m_hetares->Fill( referenceEta, test->eta() - referenceEta );
      m_hphires->Fill( referenceEta, phi(test->phi() - referencePhi) );
      m_hd0res->Fill( referenceEta, test->a0() - referenceD0 );
      m_hz0res->Fill( referenceEta, test->z0() - referenceZ0  );

      //    m_htrkvtx_x_lb->Fill( event()->lumi_block(), beamTestx() );
      //    m_htrkvtx_y_lb->Fill( event()->lumi_block(), beamTesty() );
      //    m_htrkvtx_z_lb->Fill( event()->lumi_block(), beamTestz() );

      if (tevt && beamline) {
        m_htrkvtx_x_lb->Fill( tevt->lumi_block(), beamline[0] );
        m_htrkvtx_y_lb->Fill( tevt->lumi_block(), beamline[1] );
        m_htrkvtx_z_lb->Fill( tevt->lumi_block(), beamline[2] );
      }

      for ( size_t ilayer=0 ; ilayer<32 ; ilayer++ ) { 
	if ( test->hitPattern()&(1U<<ilayer) ) m_hlayer_rec->Fill( ilayer );
      } 

      //      std::cout << "SUTT beam x " << beamTestx() << " " << "\tx " << beamTesty() << " " <<  "\ty " << beamTestz() << std::endl;

#if 0
      /// reference tracks values for tracks with a reference track match (not test track values) 
      m_htrkpT_rec->Fill( referencePT*0.001 );
      m_htrketa_rec->Fill( referenceEta );
      m_htrkphi_rec->Fill( referencePhi );
      m_htrkd0_rec->Fill( referenceD0 );
      m_htrkz0_rec->Fill( referenceZ0 );
     
#endif

      /// test track distributions for test tracks with a reference track match 
      m_htrkpT_rec->Fill( std::fabs(test->pT())*0.001 );
      m_htrketa_rec->Fill( test->eta() );
      m_htrkphi_rec->Fill( test->phi() );
      m_htrkd0_rec->Fill( test->a0() );
      m_htrkz0_rec->Fill( test->z0() );

      m_htrkdd0_rec->Fill( test->da0() );
      m_htrkdz0_rec->Fill( test->dz0() );

      if ( test->da0()!=0 )  m_htrkd0sig_rec->Fill( test->a0()/test->da0() );


      /// 1d residual distributions 
      m_htrkpT_residual->Fill( (test->pT() - referencePT)*0.001 );
      m_htrkipT_residual->Fill( (1000/test->pT() - 1000/referencePT) );
      m_htrketa_residual->Fill( test->eta() - referenceEta );
      m_htrkphi_residual->Fill( phi(test->phi() - referencePhi) );

      m_htrkd0_residual->Fill(  test->a0() - referenceD0 );
      m_htrkz0_residual->Fill( test->z0() - referenceZ0  );

      m_htrkdd0_residual->Fill( test->da0() - referenceDD0 );
      m_htrkdz0_residual->Fill( test->dz0() - referenceDZ0  );

      m_hnpixvseta_rec->Fill( referenceEta, int((test->pixelHits()+0.5)*0.5) ); 
      m_hnsctvseta_rec->Fill( referenceEta, test->sctHits() ); 

      m_hnpixvsphi_rec->Fill( referencePhi, int((test->pixelHits()+0.5)*0.5) ); 
      m_hnsctvsphi_rec->Fill( referencePhi, test->sctHits() ); 

      m_hnpixvsd0_rec->Fill( referenceD0, int((test->pixelHits()+0.5)*0.5) ); 
      m_hnsctvsd0_rec->Fill( referenceD0, test->sctHits() ); 

      m_hnpixvspT_rec->Fill( std::fabs(referencePT)*0.001,  int((test->pixelHits()+0.5)*0.5) ); 
      m_hnsctvspT_rec->Fill( std::fabs(referencePT)*0.001,  test->sctHits() ); 

      m_hnpix_rec->Fill(  int((test->pixelHits()+0.5)*0.5) ); 
      m_hnsct_rec->Fill(  test->sctHits() ); 
      m_hnsihits_rec->Fill(  test->siHits() ); 

      if (tevt) m_hnsihits_lb_rec->Fill( tevt->lumi_block(), test->siHits() );
    
 
      m_hntrt_rec->Fill(  test->strawHits() ); 

      m_hntrtvseta_rec->Fill( referenceEta, test->strawHits() ); 
      m_hntrtvsphi_rec->Fill( referencePhi, test->strawHits() ); 

      m_hd0vsphi_rec->Fill( test->phi(), test->a0() );

      if ( test->dof()!=0 ) m_chi2dof_rec->Fill( test->chi2()/test->dof() ); 

    }
    
  }
}




void AnalysisR3_Tier0::execute_vtx(const std::vector<TIDA::Vertex*>& vtx0, 
				   const std::vector<TIDA::Vertex*>& vtx1,
				   const TIDA::Event* tevt ) { 
  if ( m_vtxanal ) m_vtxanal->execute( vtx0, vtx1, tevt );
}


void AnalysisR3_Tier0::finalise() {
  if ( m_vtxanal ) { 
    m_vtxanal->finalise();
    delete m_vtxanal;
    m_vtxanal = 0;
  }
} 



double AnalysisR3_Tier0::phi(double p) {
  if(p < -M_PI) p += 2*M_PI;
  if(p >  M_PI) p -= 2*M_PI;
  return p;
}





