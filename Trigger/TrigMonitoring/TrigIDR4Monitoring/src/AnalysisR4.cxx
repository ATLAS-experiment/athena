/**
 **     @file    AnalysisR4.cxx
 **
 **     @author  mark sutton
 **     @date    $Id: AnalysisR4.cxx   Thu 28 Sep 2025 15:35:34 CEST 
 **
 **     Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **/



#include "AnalysisR4.h"

#include "TrigInDetAnalysisExample/ChainString.h"
#include "InDetTrackPerfMon/TrackParametersHelper.h"

#include "InDetTrackPerfMon/TruthParticleTraits.h"
#include "InDetTrackPerfMon/TrackParticleTraits.h"
#include "InDetTrackPerfMon/TrackView.h"



#include <cmath>
#include <iostream>



AnalysisR4::AnalysisR4(const std::string& name,
                               double /*pTCut*/,
                               double /*etaCut*/,
                               double /*d0Cut*/,
                               double /*z0Cut*/)
  : m_name(name), m_monTool(0)
{}


AnalysisR4::AnalysisR4(const std::string& name)
  : m_name(name), m_monTool(0)
{}



void AnalysisR4::initialise() {

  if ( !monTool() ) return;
  
  ChainString cname = name();

  //  std::cout << "AnalysisR4::initialise() name " << name() << std::endl; 

#if 0
  /// here for development purposes ...
  std::cout << "\nAnalysisR4:: chain specification: " << cname << "\t" << cname.raw() << std::endl;
  std::cout << "\tchain: " << cname.head()    << std::endl;
  std::cout << "\tkey:   " << cname.tail()    << std::endl;
  std::cout << "\troi:   " << cname.roi()     << std::endl;
  std::cout << "\tvtx:   " << cname.vtx()     << std::endl;
  std::cout << "\tte:    " << cname.element() << std::endl;
#endif

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
  m_hd0eff    = TIDA::Histogram<float>( monTool(),  "Eff_d0" );
  
  m_hz0eff    = TIDA::Histogram<float>( monTool(),  "Eff_z0" );
  m_hnVtxeff  = TIDA::Histogram<float>( monTool(),  "Eff_nVtx" );

  m_hd0vsphi     = TIDA::Histogram<float>( monTool(), "d0_vs_phi_prof" );
  m_hd0vsphi_rec = TIDA::Histogram<float>( monTool(), "d0_vs_phi_rec_prof" );

  m_hlbeff = TIDA::Histogram<float>( monTool(),  "Eff_lb" );
  // m_hmueff = TIDA::Histogram<float>( monTool(),  "Eff_mu" );


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
  
  m_hmu = TIDA::Histogram<float>( monTool(),  "mu" );
  
}

void AnalysisR4::execute( IDTPM::TrackAnalysisCollections& collections ) { 

  /// if ( !m_initialised ) return;
  
  std::cout << "AnalysisR4::execute() trigTracks[FULL] = " << collections.trigTrackVec( IDTPM::TrackAnalysisCollections::FULL ).size() << std::endl;
  std::cout << "AnalysisR4::execute() offlTracks[FULL] = " << collections.offlTrackVec( IDTPM::TrackAnalysisCollections::FULL ).size() << std::endl;
  std::cout << "AnalysisR4::execute() truthParts[FULL] = " << collections.truthPartVec( IDTPM::TrackAnalysisCollections::FULL ).size() << std::endl;

  m_hchain->Fill( 0.5, 1 );
  m_hchain->Fill( 1.5, 1 ); /// this is not correct - we need to be able to ensure that this is only filled once per event

  // --- Fill reference track distributions ---                                                                         
  const auto& refTracks  = collections.offlTrackVec( IDTPM::TrackAnalysisCollections::FULL );
  const auto& testTracks = collections.trigTrackVec( IDTPM::TrackAnalysisCollections::FULL );  /// nope, need to sort out all the roi stuff

  const xAOD::EventInfo* eventinfo = collections.eventInfo();
  
  long    runnumber = 0;
  long    eventid   = 0;
  long    lumiblock = 0;
  double  mu        = 0;
  
  if (eventinfo) {  
    runnumber = collections.eventInfo()->runNumber(); 
    eventid   = collections.eventInfo()->eventNumber(); 
    lumiblock = collections.eventInfo()->lumiBlock(); 
    mu        = collections.eventInfo()->averageInteractionsPerCrossing();
  }

  //  if ( roi!=nullptr ) m_hroieta->Fill( roi->eta(), 1 );

  // if ( tevt!=nullptr && m_eventid != tevt->event_number() ) {
  /// if the event number has changed, this is a new event
  /// update the event counts
  //    m_eventid = event()->event_number(); 
  
  /// ONLY UPDATE IF WE CHANGE EVENT ID 
  //  m_hchain->Fill( 1.5, 1 );
  // }
  
  m_hmu->Fill( mu );
  
  m_hntrk->Fill( refTracks.size() );

  m_hntrk_rec->Fill( testTracks.size() );
  
  /// fil the number of offline tracks
  m_hchain->Fill( 4.5, testTracks.size() );
  
  int itrack = -1;
  
  for ( const xAOD::TrackParticle* reftrk : refTracks ) {

    //    std::cout << "\t" << itrack << " " << reftrk << std::endl;

    itrack++;
    
    if ( !reftrk ) continue;

    /// fil the number of offline tracks
    m_hchain->Fill(2.5, 1);
    
    /// only need a factory and unique pointers because we don't
    /// have a specific type - when we properly template everything,
    /// then we will have a proper allocated type and we can just
    /// create a TrackAdaptor<T> reference(trk); directly

    //    std::unique_ptr<ITrackAdaptor> reference = makeAdaptor(reftrk);

    /// make a temnporary pointer to avoid changing all the code
    TrackView   referencetmp(reftrk);
    TrackView*  reference = &referencetmp;

    //    std::cout << sizeof(TrackView) << std::endl;
    //    std::cout << sizeof(TrackAdaptor<xAOD::TrackParticle>) << std::endl;
    
    //    std::cout << "pt:  " << makeAdaptor(reftrk)->pt()  << " " << reference->pt() << std::endl;
    //    std::cout << "eta: " << reference->eta() << " " << tr.eta() << std::endl;
    
    //    m_htrkpT->Fill( IDTPM::pT(*trk)&0.001 );
    //    m_htrkpT->Fill( reference->pt()*0.001 );
    //    m_htrketa->Fill( reference->eta() );
    //    m_htrkphi->Fill( reference->phi() );
    //    m_htrkd0->Fill( reference->d0() );
    //    m_htrkz0->Fill( reference->z0() );
    
    
    // Get reference parameters
    double referencePT  = reference->pt()*0.001;
    double referenceEta = reference->eta();
    double referencePhi = reference->phi();
    double referenceZ0  = reference->z0();
    double referenceD0  = reference->d0();

    double referenceDZ0  = reference->dz0();
    double referenceDD0  = reference->dd0();

    //    std::cout << itrack << "\tpt: " << referencePT << "\t" << referenceEta << " " << referenceZ0 << std::endl;
    //    itrack++;

#if 0
    // Find matched tracks
    const TIDA::Track* test = associator->matched(*reference);
#endif

    
    //    std::unique_ptr<ITrackAdaptor> test = makeAdaptor(reftrk);

    /// make a temnporary pointer to avoid changing all the code
    TrackView   testtmp(reftrk);
    TrackView*  test = &testtmp;


    
    float     eff_weight = 0;
    if (test) eff_weight = 1;

    
    m_htotal_efficiency->Fill(0.5, eff_weight );

    m_hpTeff->Fill( std::fabs(referencePT), eff_weight );
    m_hz0eff->Fill( referenceZ0, eff_weight );
    m_hd0eff->Fill( referenceD0, eff_weight );
    m_hetaeff->Fill( referenceEta, eff_weight );
    m_hphieff->Fill( referencePhi, eff_weight );
    /// m_hnVtxeff->Fill( m_nVtx, eff_weight ); /// don't use the class variable as this is not thread safe
    //    if (beamline) m_hnVtxeff->Fill( beamline[3], eff_weight ); /// this is a hack to make it thread safe

    m_hlbeff->Fill( lumiblock, eff_weight );

    //    m_hmueff->Fill( mu, eff_weight );

    
    m_htrkpT->Fill( std::fabs(referencePT) );
    m_htrketa->Fill( referenceEta );
    m_htrkphi->Fill( referencePhi );
    m_htrkd0->Fill( referenceD0 );
    m_htrkz0->Fill( referenceZ0 );

    m_htrkdd0->Fill( referenceDD0 );
    m_htrkdz0->Fill( referenceDZ0 );

    // if ( referenceDD0!=0 )

    m_htrkd0sig->Fill( referenceD0/referenceDD0 );

    m_hnpixvseta->Fill( referenceEta,  reference->nPixels() ); 
    m_hnsctvseta->Fill( referenceEta,  reference->nSCT() ); 
    m_hntrtvseta->Fill( referenceEta,  reference->nTRT() ); 

    
    if ( reference->ndof()!=0 ) m_chi2dof->Fill( reference->chi2()/reference->ndof() ); 

        
    //    m_hnpixvsphi->Fill( referencePhi,  int(reference->nPixels()+0.5)*0.5) ); 
    //    m_hnpixvsphi->Fill( referencePhi,  reference->nPixels() ); 
    //    m_hnsctvsphi->Fill( referencePhi,  reference->nSCT() ); 
    //    m_hntrtvsphi->Fill( referencePhi,  reference->strawHits() );

    m_hnpix->Fill( reference->nPixels() ); 
    m_hnpixvsphi->Fill( referencePhi,  reference->nPixels() ); 

    m_hnsctvsphi->Fill( referencePhi,  reference->nSCT() ); 
    m_hntrtvsphi->Fill( referencePhi,  reference->nTRT() ); 

    m_hnpixvsd0->Fill( referenceD0,  reference->nPixels() );
    m_hnsctvsd0->Fill( referenceD0,  reference->nSCT() ); 

    //    m_hnpixvspT->Fill( std::fabs(referencePT),  int((reference->nPixels()+0.5)*0.5) ); 
    //    m_hnsctvspT->Fill( std::fabs(referencePT),  reference->nSCT() ); 

    m_hnpixvspT->Fill( std::fabs(referencePT),  reference->nPixels() ); 
    m_hnsctvspT->Fill( std::fabs(referencePT),  reference->nSCT() ); 


    m_hnsct->Fill(  reference->nSCT() ); 
    m_hnsihits->Fill(  reference->nSi() ); 
    m_hntrt->Fill(  reference->nTRT() ); 
   
    m_hnsihits_lb->Fill( lumiblock, reference->nSi() );

    //    for ( size_t ilayer=0 ; ilayer<32 ; ilayer++ ) { 
    //       if ( reference->hitPattern()&(1U<<ilayer) ) m_hlayer->Fill( ilayer );
    //    } 

    m_hd0vsphi->Fill(referencePhi, referenceD0 );

    if (test) { 
    
      m_hchain->Fill(3.5, 1);

      /// NB: do we want to fill the actual *trigger* quantities, or the 
      /// offline quantities for the *matched* tracks?

      /// residual profiles vs the reference variable      
      // m_hpTres->Fill( referencePT, (test->pT() - referencePT)*0.001;
      // m_hipTres->Fill( 1000/referencePT, (1000/test->pT() - 1000/referencePT) );
      // m_hetares->Fill( referenceEta, test->eta() - referenceEta );
      // m_hphires->Fill( referencePhi, phi(test->phi() - referencePhi) );
      // m_hd0res->Fill( referenceD0, test->d0() - referenceD0 );
      // m_hz0res->Fill( referenceZ0, test->z0() - referenceZ0  );

      /// residual profiles vs eta - the more easy to understand
      m_hpTres->Fill( referenceEta, (test->pt()*0.001 - referencePT) );
      m_hipTres->Fill( referenceEta, (1000/test->pt() - 1/referencePT) );
      m_hetares->Fill( referenceEta, test->eta() - referenceEta );
      //    m_hphires->Fill( referenceEta, phi(test->phi() - referencePhi) );
      m_hphires->Fill( referenceEta, test->phi() - referencePhi ); /// <<<<<<<<<<<<<< need proper delta phi
      m_hd0res->Fill( referenceEta, test->d0() - referenceD0 );
      m_hz0res->Fill( referenceEta, test->z0() - referenceZ0  );

      //      m_htrkvtx_x_lb->Fill( lumiblock(), beamTestx() );
      //      m_htrkvtx_y_lb->Fill( lumiblock(), beamTesty() );
      //      m_htrkvtx_z_lb->Fill( lumiblock(), beamTestz() );

#if 0
      if (tevt && beamline) {
        m_htrkvtx_x_lb->Fill( tevt->lumi_block(), beamline[0] );
        m_htrkvtx_y_lb->Fill( tevt->lumi_block(), beamline[1] );
        m_htrkvtx_z_lb->Fill( tevt->lumi_block(), beamline[2] );
      }
      
      for ( size_t ilayer=0 ; ilayer<32 ; ilayer++ ) { 
	if ( test->hitPattern()&(1U<<ilayer) ) m_hlayer_rec->Fill( ilayer );
      } 
#endif

      //      std::cout << "SUTT beam x " << beamTestx() << " " << "\tx " << beamTesty() << " " <<  "\ty " << beamTestz() << std::endl;

#if 0
      /// reference tracks values for tracks with a reference track match (not test track values) 
      m_htrkpT_rec->Fill( referencePT );
      m_htrketa_rec->Fill( referenceEta );
      m_htrkphi_rec->Fill( referencePhi );
      m_htrkd0_rec->Fill( referenceD0 );
      m_htrkz0_rec->Fill( referenceZ0 );
     
#endif

      /// test track distributions for test tracks with a reference track match 
      m_htrkpT_rec->Fill( std::fabs(test->pt())*0.001 );
      m_htrketa_rec->Fill( test->eta() );
      m_htrkphi_rec->Fill( test->phi() );
      m_htrkd0_rec->Fill( test->d0() );
      m_htrkz0_rec->Fill( test->z0() );

      m_htrkdd0_rec->Fill( test->dd0() );
      m_htrkdz0_rec->Fill( test->dz0() );

      //      if ( test->dd0()!=0 )  m_htrkd0sig_rec->Fill( test->d0()/test->dd0() );
      m_htrkd0sig_rec->Fill( test->d0()/test->dd0() );


      /// 1d residual distributions 
      m_htrkpT_residual->Fill( (test->pt()*0.001 - referencePT) );
      m_htrkipT_residual->Fill( (1000/test->pt() - 1/referencePT) );
      m_htrketa_residual->Fill( test->eta() - referenceEta );
      m_htrkphi_residual->Fill( test->phi() - referencePhi ); //// <<<<<<< need proper Delta phi

      m_htrkd0_residual->Fill( test->d0() - referenceD0 );
      m_htrkz0_residual->Fill( test->z0() - referenceZ0  );

      m_htrkdd0_residual->Fill( test->dd0() - referenceDD0 );
      m_htrkdz0_residual->Fill( test->dz0() - referenceDZ0  );

      m_hnpixvseta_rec->Fill( referenceEta, test->nPixels() ); 
      m_hnsctvseta_rec->Fill( referenceEta, test->nSCT() ); 

      m_hnpixvsphi_rec->Fill( referencePhi, test->nPixels() ); 
      m_hnsctvsphi_rec->Fill( referencePhi, test->nSCT() ); 

      m_hnpixvsd0_rec->Fill( referenceD0, test->nPixels() ); 
      m_hnsctvsd0_rec->Fill( referenceD0, test->nSCT() ); 

      m_hnpixvspT_rec->Fill( std::fabs(referencePT),  test->nPixels() ); 
      m_hnsctvspT_rec->Fill( std::fabs(referencePT),  test->nSCT() ); 

      m_hnpix_rec->Fill(  test->nPixels() ); 
      m_hnsct_rec->Fill(  test->nSCT() ); 
      m_hnsihits_rec->Fill(  test->nSi() ); 

#if 0
      if (tevt) m_hnsihits_lb_rec->Fill( tevt->lumi_block(), test->nSi() );
#endif
 
      m_hntrt_rec->Fill(  test->nTRT() ); 

      m_hntrtvseta_rec->Fill( referenceEta, test->nTRT() ); 
      m_hntrtvsphi_rec->Fill( referencePhi, test->nTRT() ); 

      m_hd0vsphi_rec->Fill( test->phi(), test->d0() );

      if ( test->ndof()!=0 ) m_chi2dof_rec->Fill( test->chi2()/test->ndof() ); 

    }

  }

}




void AnalysisR4::finalise() { } 







