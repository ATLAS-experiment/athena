/**
 **     @file    ConfVtxAnalysis.cxx
 **
 **     @author  mark sutton
 **     @date    Sun  9 Aug 2015 21:53:46 CEST 
 **
 **     Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 **/


#include "ConfVtxAnalysis.h"

#include "TrigInDetAnalysisUtils/VertexMatcher.h"
#include "TrigInDetAnalysisUtils/VertexNewMatcher.h"
#include "TrigInDetAnalysis/TIDAEvent.h"


ConfVtxAnalysis::ConfVtxAnalysis( const std::string& n, bool use_secVtx_limits ) : 
  VertexAnalysis( n ), m_initialised(false), m_finalised(false), m_use_secVtx_limits(use_secVtx_limits), m_dir(0) { }


extern TIDA::Event* gevent;


void ConfVtxAnalysis::initialise() { 
  
  //  std::cerr << "ConfVtxAnalysis::initialise() " << name() << std::endl;

  if ( m_initialised ) { 
    std::cerr << "ConfVtxAnalysis::initialise() " << name() << " already initialised" << std::endl;
    return;
  }

  m_initialised = true;
  m_finalised   = false;

  //  std::cout << "ConfVtxAnalysis::initialise() " << name() << std::endl;

  m_dir = new TIDDirectory(name());
  m_dir->push();

#if 0
  double vnbins[41] = {
    0.5,	1.5,	2.5,	3.5,	4.5,	5.5,	6.5,	7.5,	8.5,	9.5,
    10.5,	11.5,	12.5,	13.5,	14.5,	15.5,	16.5,	17.5,	18.5,	19.5,
    21.5,	22.5,	24.5,	25.5,	27.5,	29.5,
    31.5,	33.5,	35.5,	38.5,
    40.5,	43.5,	46.5,
    50.5,	53.5,	57.5,
    61.5,	65.5,	69.5,
    74.5,
    80.5 };


  double vnbins[81] = {
               -0.5,
		0.5,	1.5,	2.5,	3.5,	4.5,	5.5,	6.5,	7.5,	8.5,	9.5,
		10.5,	11.5,	12.5,	13.5,	14.5,	15.5,	16.5,	17.5,	18.5,	19.5,
		20.5,	21.5,	22.5,	23.5,	24.5,	25.5,	26.5,	27.5,	28.5,	29.5,
		31.5,	32.5,	33.5,	34.5,	36.5,	37.5,	39.5,
		40.5,	42.5,	43.5,	45.5,	47.5,	49.5,
		50.5,	52.5,	54.5,	57.5,	59.5,
		61.5,	63.5,	66.5,	69.5,
		71.5,	74.5,	77.5,
		80.5,	83.5,	86.5,
		90.5,	93.5,	97.5,
		100.5,	104.5,	108.5,
		113.5,	117.5,
		122.5,	126.5,
		131.5,	136.5,
		142.5,	147.5,
		153.5,	159.5,
		165.5,
		171.5,	178.5,
		185.5,
		192.5,
		200.5 };

#endif
  
  double vnbins[101] = { 
    -0.5,
     0.5,   1.5,   2.5,   3.5,   4.5,   5.5,   6.5,   7.5,   8.5,   9.5,   10.5,   11.5,   12.5,   13.5,   14.5,   15.5,   17.5,   18.5,   19.5,   21.5,
     23.5,   24.5,   26.5,   28.5,   30.5,   32.5,   35.5,   37.5,   40.5,   43.5,   46.5,   50.5,   53.5,   57.5,   61.5,   66.5,   71.5,   76.5,   81.5,   87.5,
     93.5,   100.5,   107.5,   114.5,   123.5,   131.5,   141.5,   150.5,   161.5,   172.5,   185.5,   198.5,   211.5,   226.5,   242.5,   259.5,   277.5,   297.5,   317.5,   340.5,
     363.5,   389.5,   416.5,   445.5,   476.5,   509.5,
     544.5,   582.5,   623.5,   666.5,   713.5,   762.5,   815.5,   872.5,   933.5,   998.5,   1067.5,
     1141.5,   1221.5,   1305.5,   1396.5,   1493.5,   1597.5,
     1708.5,   1827.5,   1953.5,   2089.5,
     2234.5,   2389.5,   2555.5,
     2733.5,   2923.5,   3125.5,
     3342.5,   3574.5,
     3823.5,   4088.5,
     4372.5,   4675.5,
     5000.5
  };
  
  double vrbins[41] = { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
                       10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
                       20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
                       30, 33, 36, 39, 42, 45,
                       50, 55, 60, 
                       70, 80 };

  m_hnvtx   = new TH1F( "nvtx", ";number of vertices",   101, -0.5,  100.5   );
  m_hzed    = new TH1F( "zed",   ";vtx z [mm]",          200, -300,   300   );
  m_hx      = new TH1F( "x",     ";vtx x [mm]",          800, -1,    1   );
  m_hy      = new TH1F( "y",     ";vtx y [mm]",          800, -1,    1   );
  m_hr      = new TH1F( "r",     ";vtx r [mm]",           40, vrbins );
  //  hntrax  = new TH1F( "ntrax", ";number of tracks", 201,   -0.5, 200.5 );
  m_hmu     = new TH1F( "mu",    ";<mu>",         81, -0.5, 80.5   );
  m_hlb     = new TH1F( "lb",    ";lumi block",  301, -0.5, 3009.5   );

  m_hnvtx_rec  = new TH1F( "nvtx_rec",  ";number of vertices",  101, -0.5, 100.5 );
  m_hzed_rec   = new TH1F( "zed_rec",   ";vtx z [mm]",          200, -300,   300 );
  m_hx_rec     = new TH1F( "x_rec",     ";vtx x [mm]",          800, -1,    1   );
  m_hy_rec     = new TH1F( "y_rec",     ";vtx y [mm]",          800, -1,    1   );
  m_hr_rec     = new TH1F( "r_rec",     ";vtx r [mm]",           40, vrbins );

  // different binning for primary and sec vtx analysis
  if (m_use_secVtx_limits) {
    m_hntrax      = new TH1F( "ntrax", ";number of tracks", 14, -0.5, 13.5 );
    m_hntrax_rec  = new TH1F( "ntrax_rec", ";number of tracks", 14, -0.5,  13.5 );
    m_h_dntrax    = new TH1F( "dntrax", ";trigger - offline tracks", 61, -30.5, 30.5 );
  }
  else {
    m_hntrax      = new TH1F( "ntrax", ";number of tracks", 80,  vnbins );
    m_hntrax_rec  = new TH1F( "ntrax_rec", ";number of tracks", 80, vnbins );
    m_h_dntrax    = new TH1F( "dntrax", ";trigger - offline tracks", 15, -7.5, 7.5 );
  }


  m_hzed_res = new TH1F( "zed_res", "Delta z [mm]", 400, -10, 10 );
  m_hx_res   = new TH1F( "x_res",   "Delta x [mm]", 200, -0.1, 0.1 );
  m_hy_res   = new TH1F( "y_res",   "Delta y [mm]", 200, -0.1, 0.1 );

  m_rdntrax_vs_zed   = new Resplot( "rdntrax_vs_zed",   100,  -300,  300,   61, -30.5, 30.5);
  m_rdntrax_vs_ntrax = new Resplot( "rdntrax_vs_ntrax", 14,   -0.5, 13.5,   15, -30.5, 30.5);
  m_rdntrax_vs_r     = new Resplot( "rdntrax_vs_r",     40, vrbins,   15, -7.5,  7.5 );

  m_rdz_vs_zed    = new Resplot( "rdz_vs_zed",   100, -300,    300,    1600, -20, 20 );
  m_rdz_vs_ntrax  = new Resplot( "rdz_vs_ntrax", 201,   -0.5,  200.5,  1600, -20, 20 );
  m_rdz_vs_nvtx   = new Resplot( "rdz_vs_nvtx",  81,    -0.5,   80.5,  1600, -20, 20 );
  m_rdz_vs_mu     = new Resplot( "rdz_vs_mu",    30,     0,     30,    1600, -20, 20 );
  m_rdz_vs_r      = new Resplot( "rdz_vs_r",     40,  vrbins,   1600,  -20,  20 );

  m_rdr_vs_zed    = new Resplot( "rdr_vs_zed",  100,   -300,  300, 800, -15, 15 );
  m_rdr_vs_r      = new Resplot( "rdr_vs_r",     40, vrbins,  800, -15,  15 );
  m_rdr_vs_ntrax  = new Resplot( "rdr_vs_ntrax", 14,   -0.5, 13.5, 800, -15, 15 );

  m_eff_zed   = new Efficiency1D( m_hzed,   "zed_eff" );
  m_eff_x     = new Efficiency1D( m_hx,     "x_eff" );
  m_eff_y     = new Efficiency1D( m_hy,     "y_eff" );
  m_eff_ntrax = new Efficiency1D( m_hntrax, "ntrax_eff" );
  m_eff_nvtx  = new Efficiency1D( m_hnvtx,  "nvtx_eff" );
  m_eff_mu    = new Efficiency1D( m_hmu, "mu_eff" );
  m_eff_lb    = new Efficiency1D( m_hlb, "lb_eff" );
  m_eff_r     = new Efficiency1D( m_hr, "r_eff" );

  m_rnvtxrec_nvtx = new Resplot( "rnvtxrec_vs_nvtx",   81,  -0.5,   80.5,  81,   -0.5,   80.5 );

  //  double ntrax[10] = { 0, 2, 5, 10, 15, 20, 30, 15, 100 }; 

  m_rdz_vs_lb    = new Resplot( "rdz_vs_lb",  301, -0.5, 3009.5,  800, -20, 20 );
  m_rdx_vs_lb    = new Resplot( "rdx_vs_lb",  301, -0.5, 3009.5,  300,  -3,  3 );
  m_rdy_vs_lb    = new Resplot( "rdy_vs_lb",  301, -0.5, 3009.5,  300,  -3,  3 );

  m_dir->pop();

}

template<typename T> 
std::ostream& operator<<( std::ostream& s, const std::vector<T*>& v) { 
  for ( size_t i=0 ; i<v.size() ; i++ ) { 
    if ( v[i] )  s << "\t" << *v[i] << "\n";
    else         s << "\t" << "----\n";
  }
  return s;
} 

void ConfVtxAnalysis::execute( const std::vector<TIDA::Vertex*>& vtx0,
			       const std::vector<TIDA::Vertex*>& vtx1,
			       const TIDA::Event* tevt ) { 


  if ( !m_initialised ) return;

  //  if ( vtx1.size()<2 ) return;

#if 0
    std::cout << "ConfVtxAnalysis::execute() " << name()
	      << "\tvtx0.size() " << vtx0.size()
	      << "\tvtx1.size() " << vtx1.size()
	      << std::endl;

    std::cout << "\tref:  " << vtx0 << std::endl;
    std::cout << "\ttest: " << vtx1 << std::endl;
#endif

  // use new matcher if tracks included
  if ( vtx0[0]->tracks().size() > 0 ) {
    VertexNewMatcher m( "vtx_matcher", 0.5 );
    execute_internal<VertexNewMatcher>(vtx0, vtx1, m, tevt);
  }
  else {
    VertexMatcher m( "vtx_matcher", 3 );
    execute_internal<VertexMatcher>(vtx0, vtx1, m, tevt);
  }
}


template<typename Matcher>
void ConfVtxAnalysis::execute_internal( const std::vector<TIDA::Vertex*>& vtx0,
			       const std::vector<TIDA::Vertex*>& vtx1,
             Matcher& m, 
			       const TIDA::Event* tevt ) {

  m.match( vtx0, vtx1 );

  m_hnvtx->Fill( vtx0.size() );
  m_hnvtx_rec->Fill( vtx1.size() );
  
  m_rnvtxrec_nvtx->Fill( vtx0.size(), vtx1.size() );

  // keep alternate functionality commented
  // std::cout << "gevent " << gevent << std::endl;

  /// pass in a parameter now, rather than using a global
  //    double mu = gevent->mu();
  //    double lb = gevent->lumi_block();
  double mu = tevt->mu();
  double lb = tevt->lumi_block();

  for ( unsigned i=0 ; i<vtx0.size() ; i++ ) { 

    /// reject vertices with no tracks in the Roi ...
    if ( vtx0[i]->Ntracks() == 0 ) continue;

    // offline vertex radial position
    double r = std::sqrt(vtx0[i]->x()*vtx0[i]->x() + vtx0[i]->y()*vtx0[i]->y()); 

    m_hx->Fill( vtx0[i]->x() );
    m_hy->Fill( vtx0[i]->y() );
    m_hzed->Fill( vtx0[i]->z() );
    m_hntrax->Fill( vtx0[i]->Ntracks() );
    m_hr->Fill( r );

    m_hlb->Fill( lb );
    m_hmu->Fill( mu );

    const TIDA::Vertex* mv = m.matched( vtx0[i] ); 

    //      std::cout << "\tvtx match: " << i << " " << mv << std::endl;
    if ( mv ) { 
      //	std::cout << "\ttest z " << mv->z() << "  : delta z " << (mv->z()-vtx0[i]->z()) << std::endl;
          
      /// ah ha ! can fill some silly old histograms here 
      /// ...

      // online vertex radial position
      double r_rec = std::sqrt(mv->x()*mv->x() + mv->y()*mv->y()); 
      
      int dntrax = mv->Ntracks() - vtx0[i]->Ntracks();

      m_hx_rec->Fill( mv->x() );
      m_hy_rec->Fill( mv->y() );
      m_hzed_rec->Fill( mv->z() );
      m_hntrax_rec->Fill( mv->Ntracks() );
      m_hr_rec->Fill( r_rec );

      m_hx_res->Fill( mv->x() - vtx0[i]->x() );
      m_hy_res->Fill( mv->y() - vtx0[i]->y() );
      m_hzed_res->Fill( mv->z() - vtx0[i]->z() );

      m_h_dntrax->Fill( dntrax );
          
      m_rdz_vs_zed->Fill(   vtx0[i]->z(),       mv->z() - vtx0[i]->z() );
      m_rdz_vs_ntrax->Fill( vtx0[i]->Ntracks(), mv->z() - vtx0[i]->z() );
      m_rdz_vs_nvtx->Fill( vtx0.size(),  mv->z() - vtx0[i]->z() ); /// this isn't really legitimate
      m_rdz_vs_mu->Fill( mu,  mv->z() - vtx0[i]->z() ); /// this isn't really legitimate
      m_rdz_vs_r->Fill( r, mv->z() - vtx0[i]->z() );

      m_rdntrax_vs_zed->Fill( vtx0[i]->z(), dntrax );
      m_rdntrax_vs_ntrax->Fill( vtx0[i]->Ntracks(), dntrax );
      m_rdntrax_vs_r->Fill( r, dntrax );
      
      m_rdr_vs_zed->Fill( vtx0[i]->z(), r_rec - r );
      m_rdr_vs_r->Fill( r, r_rec - r );
      m_rdr_vs_ntrax->Fill( vtx0[i]->Ntracks(), r_rec - r );

      m_eff_zed->Fill( vtx0[i]->z() );
      m_eff_x->Fill( vtx0[i]->x() );
      m_eff_y->Fill( vtx0[i]->y() );
      m_eff_r->Fill( r );

      m_eff_ntrax->Fill( vtx0[i]->Ntracks() );
      m_eff_nvtx->Fill( vtx0.size() );

      m_eff_mu->Fill( mu );
      m_eff_lb->Fill( lb );

      //	std::cout << "found vtx ref vertex size " << vtx0.size() << "\tonline " << vtx1.size() << std::endl;
      //	std::cout << "\tref:  " << *vtx0[i] << std::endl;
      //	for ( unsigned iv=0 ; iv<vtx1.size() ; iv++ ) if ( vtx1[iv] ) std::cout << "\t" << iv << " :  " << *vtx1[iv] << std::endl;
      
      /// what about beam tilts etc? where are these defined with respect to ?
      m_rdz_vs_lb->Fill( lb, mv->z() - vtx0[i]->z() );
      m_rdx_vs_lb->Fill( lb, mv->x() - vtx0[i]->x() );
      m_rdy_vs_lb->Fill( lb, mv->y() - vtx0[i]->y() );

    } else {
      //	std::cout << "\t" << "------" << std::endl;
    
      //	std::cout << "missing vtx ref vertex size " << vtx0.size() << "\tonline " << vtx1.size() << std::endl;
      //	std::cout << "\tref:  " << *vtx0[i] << std::endl;
      //	for ( unsigned iv=0 ; iv<vtx1.size() ; iv++ ) if ( vtx1[iv] ) std::cout << "\t" << iv << " :  " << *vtx1[iv] << std::endl;

#if 0
      static int vtxcount=0;

      if ( vtxcount<100 ) { 
        std::cout << "ConfVtxAnalysis::execute() " << name() << "\tnomatch\n"
            << "\tvtx0.size() " << vtx0.size()
            << "\tvtx1.size() " << vtx1.size()
            << std::endl;
        
        std::cout << "\tref:  " << vtx0 << std::endl;
        std::cout << "\ttest: " << vtx1 << std::endl;
        
        vtxcount++;

        //	  std::cout << *gevent << std::endl;
      }

      
#endif


      m_eff_x->FillDenom( vtx0[i]->x() );
      m_eff_y->FillDenom( vtx0[i]->y() );
      m_eff_zed->FillDenom( vtx0[i]->z() );
      m_eff_r->FillDenom( r );

      m_eff_ntrax->FillDenom( vtx0[i]->Ntracks() );
      m_eff_nvtx->FillDenom( vtx0.size() );

      m_eff_mu->FillDenom( mu );
      m_eff_lb->FillDenom( lb );
    }
  }
}

void ConfVtxAnalysis::finalise() { 

  if ( !m_initialised ) return;
  if (  m_finalised )   return;

  std::cout << "ConfVtxAnalysis::finalise() " << name() << std::endl;

  m_finalised = true;

  m_dir->push();

  m_hnvtx->Write();
  m_hzed->Write();
  m_hntrax->Write();
  m_hr->Write();

  m_hnvtx_rec->Write();
  m_hzed_rec->Write();
  m_hntrax_rec->Write();
  m_hr_rec->Write();

  m_hmu->Write();
  m_hlb->Write();
  m_h_dntrax->Write();
  m_hzed_res->Write();

  std::cout << "finalising resplots" << std::endl;

  m_rdz_vs_zed->Finalise( Resplot::FitNull95 );    m_rdz_vs_zed->Write();
  m_rdz_vs_ntrax->Finalise( Resplot::FitNull95 );  m_rdz_vs_ntrax->Write();
  m_rdz_vs_nvtx->Finalise( Resplot::FitNull95 );   m_rdz_vs_nvtx->Write();
  m_rdz_vs_mu->Finalise( Resplot::FitNull95 );   m_rdz_vs_mu->Write();
  m_rdz_vs_r->Finalise( Resplot::FitNull95 ); m_rdz_vs_r->Write();

  m_rdntrax_vs_zed->Finalise( Resplot::FitNull95 ); m_rdntrax_vs_zed->Write();
  m_rdntrax_vs_ntrax->Finalise( Resplot::FitNull95 ); m_rdntrax_vs_ntrax->Write();
  m_rdntrax_vs_r->Finalise( Resplot::FitNull95 ); m_rdntrax_vs_r->Write();

  m_rdr_vs_zed->Finalise( Resplot::FitNull95 ); m_rdr_vs_zed->Write();
  m_rdr_vs_r->Finalise( Resplot::FitNull95 ); m_rdr_vs_r->Write();
  m_rdr_vs_ntrax->Finalise( Resplot::FitNull95 ); m_rdr_vs_ntrax->Write();

  m_rnvtxrec_nvtx->Finalise( Resplot::FitNull95 );    m_rnvtxrec_nvtx->Write();

  m_eff_zed->finalise();   m_eff_zed->Bayes()->Write( (m_eff_zed->name()+"_tg").c_str() );
  m_eff_x->finalise();     m_eff_x->Bayes()->Write( (m_eff_x->name()+"_tg").c_str() );
  m_eff_y->finalise();     m_eff_y->Bayes()->Write( (m_eff_y->name()+"_tg").c_str() );
  m_eff_r->finalise(); m_eff_r->Bayes()->Write( (m_eff_r->name()+"_tg").c_str());

  m_eff_ntrax->finalise(); m_eff_ntrax->Bayes()->Write( (m_eff_ntrax->name()+"_tg").c_str() );
  m_eff_nvtx->finalise();  m_eff_nvtx->Bayes()->Write( (m_eff_nvtx->name()+"_tg").c_str() );

  m_eff_mu->finalise();  m_eff_mu->Bayes()->Write( (m_eff_mu->name()+"_tg").c_str() );
  m_eff_lb->finalise();  m_eff_lb->Bayes()->Write( (m_eff_lb->name()+"_tg").c_str() );


  m_rdx_vs_lb->Finalise( Resplot::FitNull95 );    m_rdx_vs_lb->Write();
  m_rdy_vs_lb->Finalise( Resplot::FitNull95 );    m_rdy_vs_lb->Write();
  m_rdz_vs_lb->Finalise( Resplot::FitNull95 );    m_rdz_vs_lb->Write();

  m_dir->pop();
}

