/**
 **     @file    ConfAnalysis.cxx
 **
 **     @author  mark sutton
 **     @date    $Id: ConfAnalysis.cxx 800361 2017-03-12 14:33:19Z 
 **
 **     Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 **/


#include "ConfAnalysis.h"


#include "ConfVtxAnalysis.h"

#include "TrigInDetAnalysis/TrackFilter.h"
#include "TrigInDetAnalysis/TIDAEvent.h"
#include "TrigInDetAnalysis/TIDARoiDescriptor.h"

#include <fstream>

#include "BinConfig.h"

#include "TF1.h"
#include "TMath.h"

#include "globals.h"

bool PRINT_BRESIDUALS = false;

std::ofstream dumpfile("dumpfile.log");

void Normalise(TH1* h) { 

  for ( int i=1 ; i<=h->GetNbinsX() ; i++ ) {

    double del = h->GetBinLowEdge(i+1)-h->GetBinLowEdge(i);

    h->SetBinContent( i, h->GetBinContent(i)/del );
    h->SetBinError( i, h->GetBinError(i)/del );
  }

}




BinConfig g_binConfig("standard");

BinConfig electronBinConfig("electron");
BinConfig muonBinConfig("muon");
BinConfig tauBinConfig("tau");
BinConfig bjetBinConfig("bjet");
BinConfig cosmicBinConfig("cosmic");


/// book all the histograms

void ConfAnalysis::initialise() { 
  if ( !m_initialiseFirstEvent ) { 
    std::cout << "ConfAnalysis::initialise() " << std::endl;
    initialiseInternal();
  }
}

void ConfAnalysis::initialiseInternal() { 

  if ( m_initialised ) return;
    
  m_initialised = true;

  // std::cout << "ConfAnalysis::initialiseInternal() " << name() << std::endl;

  BinConfig& binConfig = g_binConfig;
 
  if       ( name().find("_e")!=std::string::npos   )   binConfig = electronBinConfig;
  else if  ( name().find("_mu")!=std::string::npos  )   binConfig =     muonBinConfig;
  else if  ( name().find("_tau")!=std::string::npos )   binConfig =      tauBinConfig;
  else if  ( name().find("_b")!=std::string::npos   )   binConfig =     bjetBinConfig;
  else if  ( name().find("cosmic")!=std::string::npos ) binConfig =   cosmicBinConfig;

  
  //+++ pT ranges
  //  double tmp_maxPt    = 50000.;
  //  double tmp_absResPt = 0.0005;
  //double tmp_maxPt    = 50.;
  double tmp_absResPt = 0.5;

  const int pTResBins = int(100*binConfig.ptres_NScale);

  //+++ Eta ranges
  double tmp_maxEta    = 3.;
  double tmp_absResEta = 0.04; // 0.0005;

  //+++ Phi ranges
  double tmp_maxPhi    = 3.142;
  double tmp_absResPhi = 0.02; // 0.0001;


  //  std::cout << "ConfAnalysis::initialise() " << name() << " config: " << binConfig << std::endl;


  int etaBins          = int(30*binConfig.eta_NScale);
  const int etaResBins = int(600*binConfig.eta_NScale);

  const int phiBins    = int(30*binConfig.phi_NScale);
  const int phiResBins = int(100*binConfig.phires_NScale);

  const int    zBins = int(150*binConfig.z0_NScale);
  const double zMax  = binConfig.z0Max;

  const int    zresBins = 100;      
  const double zresMax  = 10;

  const int    d0Bins = int(100*binConfig.d0_NScale);
  const double d0Max  = binConfig.d0Max;

  const int    d0resBins = 100;      
  const double d0resMax  = 5;

  // beamspot corrected position
  const int    a0Bins = int(300*binConfig.a0_NScale);
  const double a0Max  = binConfig.a0Max;

  const int    a0resBins = 100;      
  const double a0resMax  = 5;

  //+++ Book histograms

  // calculate a logarithmic binning in pt

  int     Npt  = 0;
  double  pt_a = 1;
  double  pt_b = 1;
  
  //  Npt = int(40*binConfig.pt_NScale);
  //  pt_a = 3.5;
  Npt = int(45*binConfig.pt_NScale);
  pt_a = 4;
  pt_b = 2;
  // etaBins = 12;
  //  }
  // else { 
  //  Npt = 40;
  //  pt_a = 4;
  // pt_b = 1;
  //  }


  
  const int  ptnbins = Npt;  
  std::vector<double> ptbinlimsv(ptnbins+1);
  double*   ptbinlims = &ptbinlimsv[0];
  //  for ( int i=0 ; i<=ptnbins ; i++ ) {     ptbinlims[i] = std::pow(10, 2.0*i/ptnbins+2)/1000; }
  //  for ( int i=0 ; i<=ptnbins ; i++ ) {     ptbinlims[i] = std::pow(10, 2.3*i/ptnbins+2); }
  for ( int i=0 ; i<=ptnbins ; i++ ) ptbinlims[i] = std::pow(10, pt_a*i/ptnbins+pt_b)/1000;  
 

  // ADDED BY JK - FOR SIGNED PT PLOTS
  //-----
  const int ptnbins2 = (2*ptnbins);
  // std::cout << "ptnbins2 = " << ptnbins2 << std::endl;
  std::vector<double> ptbinlims2v(ptnbins2 + 1);
  double* ptbinlims2 = &ptbinlims2v[0];
  //  std::cout << "ptbinlims2v.size() = " << ptbinlims2v.size() << std::endl;
  int ptnbin_counter = 0;
  for ( int i=ptnbins; i>0 ; i-- ) {
    ptbinlims2[ptnbin_counter] = std::pow(10, pt_a*i/ptnbins+pt_b)/(-2000);
    // std::cout << "ptbinlims[" << i << "] = " << ptbinlims[i] << "  ,  so ptbinlims2[" << ptnbin_counter << "] = " << ptbinlims2[ptnbin_counter] << std::endl;
    ptnbin_counter++;
  }

  for ( int i=0 ; i<ptnbins+1 ; i++ ) {
    ptbinlims2[ptnbin_counter] = std::pow(10, pt_a*i/ptnbins+pt_b)/2000;
    // std::cout << "ptbinlims[" << i << "] = " << ptbinlims[i] << "  ,  so ptbinlims2[" << ptnbin_counter << "] = " << ptbinlims2[ptnbin_counter] << std::endl;
    ptnbin_counter++;
  }
  //-----

  const int  iptnbins = 20;  
  const double minmaxipt=0.5;
  std::vector<double> iptbinlimsv(iptnbins+1);
  double*   iptbinlims = &iptbinlimsv[0];
  for ( int i=0 ; i<=iptnbins ; i++ ) {
    iptbinlims[i] = -minmaxipt+i*minmaxipt*2./iptnbins;  
  }



  TDirectory* dir = gDirectory;

  //  std::cout << "ConfAnalysis::initialize() Directory " << gDirectory->GetName() << " " << name() << std::endl;

  m_dir = new TIDDirectory(name());
  m_dir->push();

  //  std::cout << "ConfAnalysis::initialize() Directory " << gDirectory->GetName() << " " << name() << std::endl;

  if ( !m_initialiseFirstEvent ) std::cout << "ConfAnalysis::initialize() Directory " << name() << std::endl;
  
  if ( name() != gDirectory->GetName() )  std::cerr << "ConfAnalysis::initialize() Directory: problem with directory " << gDirectory->GetName() << " " << name() << std::endl;
  
  //  TIDDirectory d("histos");
  //  d.push();

  //  std::cout << "ConfAnalysis::initialize() Directory " << gDirectory->GetName() << " package directory, " << name() << std::endl;


  m_res.push_back( m_rnpix_eta  = new Resplot( "npix_eta",  2*etaBins, -tmp_maxEta, tmp_maxEta,  22, -0.5, 21.5 ) );
  m_res.push_back( m_rnsct_eta  = new Resplot( "nsct_eta",  2*etaBins, -tmp_maxEta, tmp_maxEta,  22, -0.5, 21.5 ) );
  m_res.push_back( m_rntrt_eta  = new Resplot( "ntrt_eta",  2*etaBins, -tmp_maxEta, tmp_maxEta, 100, -0.5, 99.5 ) );
  m_res.push_back( m_rnsihit_eta= new Resplot( "nsihit_eta",etaBins, -tmp_maxEta, tmp_maxEta,  22, -0.5, 21.5 ) );

  m_res.push_back( m_rnpix_lb  = new Resplot( "npix_lb", 250, 0, 2500,  22, -0.5, 21.5 ) );

  m_res.push_back(  m_rnpix_phi  = new Resplot( "npix_phi",  etaBins, -M_PI, M_PI,  22, -0.5, 21.5 ) );
  m_res.push_back(  m_rnsct_phi  = new Resplot( "nsct_phi",  etaBins, -M_PI, M_PI,  22, -0.5, 21.5 ) );
  m_res.push_back(  m_rntrt_phi  = new Resplot( "ntrt_phi",  etaBins, -M_PI, M_PI, 100, -0.5, 99.5 ) );

  m_res.push_back(  m_rnpix_pt = new Resplot( "npix_pt", ptnbins, ptbinlims,  22, -0.5, 21.5 ) );
  m_res.push_back(  m_rnsct_pt = new Resplot( "nsct_pt", ptnbins, ptbinlims,  22, -0.5, 21.5 ) );
  m_res.push_back(  m_rntrt_pt = new Resplot( "ntrt_pt", ptnbins, ptbinlims, 100, -0.5, 99.5 ) );

  m_res.push_back(  m_rnpixh_pt = new Resplot( "npixh_pt", ptnbins, ptbinlims,  22, -0.5, 21.5 ) );
  m_res.push_back(  m_rnscth_pt = new Resplot( "nscth_pt", ptnbins, ptbinlims,  22, -0.5, 21.5 ) );


  m_res.push_back(  m_rnpix_pt_bad = new Resplot( "npix_pt_bad", ptnbins, ptbinlims,  22, -0.5, 21.5 ) );
  m_res.push_back(  m_rnsct_pt_bad = new Resplot( "nsct_pt_bad", ptnbins, ptbinlims,  22, -0.5, 21.5 ) );
  m_res.push_back(  m_rntrt_pt_bad = new Resplot( "ntrt_pt_bad", ptnbins, ptbinlims, 100, -0.5, 99.5 ) );
  
  
  m_res.push_back(  m_rnpix_eta_rec  = new Resplot( "npix_eta_rec",  2*etaBins, -tmp_maxEta, tmp_maxEta,  22, -0.5, 21.5 ) );
  m_res.push_back(  m_rnsct_eta_rec  = new Resplot( "nsct_eta_rec",   2*etaBins, -tmp_maxEta, tmp_maxEta,  22, -0.5, 21.5 ) );
  m_res.push_back(  m_rntrt_eta_rec  = new Resplot( "ntrt_eta_rec",   2*etaBins, -tmp_maxEta, tmp_maxEta, 100, -0.5, 99.5 ) );
  m_res.push_back(  m_rnsihit_eta_rec= new Resplot( "nsihit_eta_rec", etaBins, -tmp_maxEta, tmp_maxEta,  22, -0.5, 21.5 ) );

  m_res.push_back(  m_rnpix_phi_rec  = new Resplot( "npix_phi_rec",  etaBins, -M_PI, M_PI,  22, -0.5, 21.5 ) );
  m_res.push_back(  m_rnsct_phi_rec  = new Resplot( "nsct_phi_rec",  etaBins, -M_PI, M_PI,  22, -0.5, 21.5 ) );
  m_res.push_back(  m_rntrt_phi_rec  = new Resplot( "ntrt_phi_rec",  etaBins, -M_PI, M_PI, 100, -0.5, 99.5 ) );

  m_res.push_back(  m_rnpix_pt_rec = new Resplot( "npix_pt_rec", ptnbins, ptbinlims,  22, -0.5, 21.5 ) );
  m_res.push_back(  m_rnsct_pt_rec = new Resplot( "nsct_pt_rec", ptnbins, ptbinlims,  22, -0.5, 21.5 ) );
  m_res.push_back(  m_rntrt_pt_rec = new Resplot( "ntrt_pt_rec", ptnbins, ptbinlims, 100, -0.5, 99.5 ) );

  m_res.push_back(  m_rnpixh_pt_rec = new Resplot( "npixh_pt_rec", ptnbins, ptbinlims,  22, -0.5, 21.5 ) );
  m_res.push_back(  m_rnscth_pt_rec = new Resplot( "nscth_pt_rec", ptnbins, ptbinlims,  22, -0.5, 21.5 ) );


  m_res.push_back(  m_rnsct_vs_npix     = new Resplot( "nsct_vs_npix",     12, -0.5,  11.5,   22, -0.5, 21.5 ) );
  m_res.push_back(  m_rnsct_vs_npix_rec = new Resplot( "nsct_vs_npix_rec", 12, -0.5,  11.5,   22, -0.5, 21.5 ) );

  m_res.push_back(  m_rChi2prob = new Resplot( "Chi2prob", ptnbins, ptbinlims,  20, 0,   1 ) );
  m_res.push_back(  m_rChi2     = new Resplot( "Chi2",     ptnbins, ptbinlims, 200, 0, 100 ) );
  m_res.push_back(  m_rChi2dof  = new Resplot( "Chi2dof",  ptnbins, ptbinlims, 100, 0,  10 ) );

  m_res.push_back(  m_rChi2prob_bad = new Resplot( "Chi2prob_bad", ptnbins, ptbinlims,  20, 0,   1 ) );
  m_res.push_back(  m_rChi2_bad     = new Resplot( "Chi2_bad",     ptnbins, ptbinlims, 200, 0, 100 ) );
  m_res.push_back(  m_rChi2dof_bad  = new Resplot( "Chi2dof_bad",  ptnbins, ptbinlims, 100, 0,  10 ) );

  m_res.push_back(  m_rChi2prob_rec = new Resplot( "Chi2prob_rec", ptnbins, ptbinlims,  20, 0,   1 ) );
  m_res.push_back(  m_rChi2_rec     = new Resplot( "Chi2_rec",     ptnbins, ptbinlims, 200, 0, 100 ) );
  m_res.push_back(  m_rChi2dof_rec  = new Resplot( "Chi2dof_rec",  ptnbins, ptbinlims, 100, 0,  10 ) );

  m_res.push_back( m_rChi2d_vs_Chi2d = new Resplot( "Chi2d_vs_Chi2d", 200, 0,   100, 200, 0, 100 ) );

  m_res.push_back( m_rDChi2dof = new Resplot( "DChi2dof", 200, 0,   100, 200, -100, 100 ) );

  /// additional resplots for additional si hit and hold monitoring

  double d0bins[40] = { -5.0,  -4.0,  -3.0,  -2.5,
                        -2.0,  -1.8,  -1.6,  -1.4,  -1.2,
                        -1.05, -0.95, -0.85, -0.75, -0.65, -0.55, -0.45, -0.35, -0.25, -0.15, -0.05,
                         0.05,  0.15,  0.25,  0.35,  0.45,  0.55,  0.65,  0.75,  0.85,  0.95,  1.05,
                         1.2,   1.4,   1.6,   1.8,   2.0,
                         2.5,   3.0,   4.0,   5.0 };



  m_res.push_back(  m_rnpix_d0 = new Resplot( "npix_d0", 39, d0bins,  22, -0.5, 21.5 ) );
  m_res.push_back(  m_rnsct_d0 = new Resplot( "nsct_d0", 39, d0bins,  22, -0.5, 21.5 ) );
  m_res.push_back(  m_rntrt_d0 = new Resplot( "ntrt_d0", 39, d0bins,  22, -0.5, 21.5 ) );

  m_res.push_back(  m_rnpixh_d0 = new Resplot( "npixh_d0", 39, d0bins,  22, -0.5, 21.5 ) );
  m_res.push_back(  m_rnscth_d0 = new Resplot( "nscth_d0", 39, d0bins,  22, -0.5, 21.5 ) );

  m_res.push_back(   m_rnsi_pt = new Resplot(  "nsi_pt", ptnbins, ptbinlims,  22, -0.5, 21.5 ) );
  m_res.push_back(  m_rnsih_pt = new Resplot( "nsih_pt", ptnbins, ptbinlims,  22, -0.5, 21.5 ) );

  m_res.push_back(   m_rnsi_d0 = new Resplot(  "nsi_d0", 39, d0bins,  22, -0.5, 21.5 ) );
  m_res.push_back(  m_rnsih_d0 = new Resplot( "nsih_d0", 39, d0bins,  22, -0.5, 21.5 ) );

  m_res.push_back(   m_rnsi_eta = new Resplot(  "nsi_eta", etaBins, -tmp_maxEta, tmp_maxEta,  22, -0.5, 21.5 ) );
  m_res.push_back(  m_rnsih_eta = new Resplot( "nsih_eta", etaBins, -tmp_maxEta, tmp_maxEta,  22, -0.5, 21.5 ) );

  m_res.push_back(   m_rnbl_d0 = new Resplot(  "nbl_d0", 39, d0bins,  5, -0.5, 4.5 ) );
  m_res.push_back(  m_rnblh_d0 = new Resplot( "nblh_d0", 39, d0bins,  5, -0.5, 4.5 ) );


  m_res.push_back( m_rnsct_lb  = new Resplot( "nsct_lb", 250, 0, 2500,  22, -0.5, 21.5 ) );

  m_res.push_back( m_rnpix_lb_rec  = new Resplot( "npix_lb_rec", 250, 0, 2500,  22, -0.5, 21.5 ) );
  m_res.push_back( m_rnsct_lb_rec  = new Resplot( "nsct_lb_rec", 250, 0, 2500,  22, -0.5, 21.5 ) );

  m_res.push_back(  m_rnpix_d0_rec = new Resplot( "npix_d0_rec", 39, d0bins,  22, -0.5, 21.5 ) );
  m_res.push_back(  m_rnsct_d0_rec = new Resplot( "nsct_d0_rec", 39, d0bins,  22, -0.5, 21.5 ) );
  m_res.push_back(  m_rntrt_d0_rec = new Resplot( "ntrt_d0_rec", 39, d0bins,  22, -0.5, 21.5 ) );

  //  int Nptbins = 7;
  //  double _ptlims[8] = { 0, 500, 1000, 1500, 2000, 5000, 8000, 12000 };


  //  addHistogram( hchi2=new TH1F("chi2", "chi2", 100, 0, 20) );

  // "reference" quantities
  addHistogram(  new TH1F(  "pT",   "pT",     ptnbins,   ptbinlims ) );
  addHistogram(  new TH1F( "eta",  "eta",     etaBins,  -tmp_maxEta, tmp_maxEta ) );
  addHistogram(  new TH1F( "phi",  "phi",     phiBins,  -tmp_maxPhi, tmp_maxPhi ) );
  addHistogram(  new TH1F(  "z0",   "z0",       zBins,        -zMax,       zMax ) );
  addHistogram(  new TH1F(  "d0",   "d0",      d0Bins,       -d0Max,      d0Max ) );
  addHistogram(  new TH1F(  "a0",   "a0",      a0Bins,       -a0Max,      a0Max ) );

  // error study histograms (reference)                                                                                                                           
  addHistogram(  new TH1F( "dpT",  "dpT",  80, 0, 20 ) );
  addHistogram(  new TH1F( "deta", "deta", 50, 0, 1  ) );
  addHistogram(  new TH1F( "dphi", "dphi", 50, 0, 1  ) );
  addHistogram(  new TH1F( "dz0",  "dz0", 100, 0, 2  ) );
  addHistogram(  new TH1F( "dd0",  "dd0",  50, 0, 0.5  ) );
  addHistogram(  new TH1F( "da0",  "da0",  50, 0, 0.5  ) );

  addHistogram(  new TH1F( "roi_deta", "roi_deta",  50, -1, 1  ) );
  addHistogram(  new TH1F( "roi_dphi", "roi_dphi",  50, -1, 1  ) );
  addHistogram(  new TH1F( "roi_dR",   "roi_dR",    50,  0, 1  ) );

  // tag and probe invariant mass histograms
  if ( m_TnP_tool ) {
    m_invmass = new TH1F( "invmass", "invariant mass;mass [GeV]", 320, 0, 200 );
    m_invmassObj = new TH1F( "invmassObj", "invariant mass;mass [GeV]", 320, 0, 200 );
    addHistogram( m_invmass );
    addHistogram( m_invmassObj );
  }

  /// 2D histograms for the 2D efficiencies
  TH2F* heta_vs_pt = new TH2F( "eta_vs_pt", "p_{T} [GeV],#eta",       ptnbins, ptbinlims, 10, -3, 3 );
  TH2F* hd0_vs_pt  = new TH2F( "d0_vs_pt",  "p_{T} [GeV],d_{0} [mm]", ptnbins, ptbinlims, 30, -300, 300 );
  

  m_eff_eta_vs_pt = new Efficiency2D( heta_vs_pt, "eff_eta_vs_pt" ); 
  m_eff_d0_vs_pt  = new Efficiency2D( hd0_vs_pt,  "eff_d0_vs_pt" ); 

  delete  heta_vs_pt;
  delete  hd0_vs_pt;


  // efficiencies and purities
  m_eff_pt  = new Efficiency1D( find("pT"),  "pT_eff"  );
  m_eff_pt->Hist()->GetXaxis()->SetTitle("P_{T} [GeV]");
  m_eff_pt->Hist()->GetYaxis()->SetTitle("Efficiency [%]");

  m_eff_eta = new Efficiency1D( find("eta"), "eta_eff" );
  m_eff_eta->Hist()->GetXaxis()->SetTitle("#eta");
  m_eff_eta->Hist()->GetYaxis()->SetTitle("Efficiency [%]");

  m_eff_phi = new Efficiency1D( find("phi"), "phi_eff" );
  m_eff_phi->Hist()->GetXaxis()->SetTitle("#phi");
  m_eff_phi->Hist()->GetYaxis()->SetTitle("Efficiency [%]");

  m_eff_z0  = new Efficiency1D( find("z0"),  "z0_eff"  );
  m_eff_z0->Hist()->GetXaxis()->SetTitle("z0");
  m_eff_z0->Hist()->GetYaxis()->SetTitle("Efficiency [%]");

  m_eff_d0  = new Efficiency1D( find("d0"),  "d0_eff"  );
  m_eff_d0->Hist()->GetXaxis()->SetTitle("d0");
  m_eff_d0->Hist()->GetYaxis()->SetTitle("Efficiency [%]");

  m_eff_a0  = new Efficiency1D( find("a0"),  "a0_eff"  );
  m_eff_a0->Hist()->GetXaxis()->SetTitle("a0");
  m_eff_a0->Hist()->GetYaxis()->SetTitle("Efficiency [%]");
          
  m_eff_ptm = new Efficiency1D( find("pT"), "pTm_eff" );
  m_eff_ptm->Hist()->GetXaxis()->SetTitle("Negative P_{T} [GeV]");
  m_eff_ptm->Hist()->GetYaxis()->SetTitle("Efficiency [%]");

  m_eff_ptp = new Efficiency1D( find("pT"), "pTp_eff" );
  m_eff_ptp->Hist()->GetXaxis()->SetTitle("Positive P_{T} [GeV]");
  m_eff_ptp->Hist()->GetYaxis()->SetTitle("Efficiency [%]");

  m_eff_roi_deta = new Efficiency1D( find("roi_deta"), "roi_deta_eff" );
  m_eff_roi_deta->Hist()->GetXaxis()->SetTitle("RoI #Delta#eta");
  m_eff_roi_deta->Hist()->GetYaxis()->SetTitle("Efficiency [%]");

  m_eff_roi_dphi = new Efficiency1D( find("roi_dphi"), "roi_dphi_eff" );
  m_eff_roi_dphi->Hist()->GetXaxis()->SetTitle("RoI #Delta#phi");
  m_eff_roi_dphi->Hist()->GetYaxis()->SetTitle("Efficiency [%]");

  m_eff_roi_dR = new Efficiency1D( find("roi_dR"), "roi_dR_eff" );
  m_eff_roi_dR->Hist()->GetXaxis()->SetTitle("RoI #Delta R");
  m_eff_roi_dR->Hist()->GetYaxis()->SetTitle("Efficiency [%]");

  // addHistogram ( m_hDeltaR = new TH1F("DeltaR", "DeltaR", 100, 0, 0.1 ) );
  addHistogram ( m_hDeltaR = new TH1F("DeltaR", "DeltaR", 100, 0, 0.2 ) );

  m_purity_pt  = new Efficiency1D( find("pT"),  "pT_pur"  );
  m_purity_eta = new Efficiency1D( find("eta"), "eta_pur" );
  m_purity_phi = new Efficiency1D( find("phi"), "phi_pur" );
  m_purity_z0  = new Efficiency1D( find("z0"),  "z0_pur"  );
  m_purity_d0  = new Efficiency1D( find("d0"),  "d0_pur"  );
  m_purity_a0  = new Efficiency1D( find("a0"),  "a0_pur"  );

  // "test" quantities
  addHistogram(    new TH1F(  "pT_rec",   "pT_rec",   ptnbins,    ptbinlims ) );
  addHistogram(    new TH1F( "eta_rec",  "eta_rec",   etaBins,  -tmp_maxEta, tmp_maxEta ) );
  addHistogram(    new TH1F( "phi_rec",  "phi_rec",   phiBins,  -tmp_maxPhi, tmp_maxPhi ) );
  addHistogram(    new TH1F(  "z0_rec",   "z0_rec",      zBins,      -zMax,       zMax ) );
  addHistogram(    new TH1F(  "d0_rec",   "d0_rec",     d0Bins,       -d0Max,      d0Max ) );
  addHistogram(    new TH1F(  "a0_rec",   "a0_rec",     a0Bins,       -a0Max,      a0Max ) );

  addHistogram2D(  new TH2F( "eta_phi_rec",  "eta_phi_rec", (tmp_maxEta+1)*30  ,  -tmp_maxEta-1, tmp_maxEta+1,   (tmp_maxPhi+1)*30,  -tmp_maxPhi-1, tmp_maxPhi+1 ) );
  addHistogram2D(  new TH2F( "phi_d0_rec",  "phi_d0_rec", (2*tmp_maxPhi+2)*15,  -tmp_maxPhi-1, tmp_maxPhi+1 ,d0Bins+20,       -d0Max+7,      d0Max-7 ));

  // error study histograms (test)                                                                                                                                                                  
  addHistogram(  new TH1F( "dpT_rec",  "dpT_rec",  80, 0, 20.00 ) );
  addHistogram(  new TH1F( "deta_rec", "deta_rec", 50, 0,  0.02 ) );
  addHistogram(  new TH1F( "dphi_rec", "dphi_rec", 50, 0,  0.02 ) );
  addHistogram(  new TH1F( "dz0_rec",  "dz0_rec", 100, 0,  1.5  ) );
  addHistogram(  new TH1F( "dd0_rec",  "dd0_rec",  50, 0,  0.5 ) );
  addHistogram(  new TH1F( "da0_rec",  "da0_rec",  50, 0,  0.5 ) );



  // error study histograms (pull test-ref)                                                                                                                       
  addHistogram( new TH1F("pT_pull",  "pT_pull",  100, -10, 10) );
  addHistogram( new TH1F("eta_pull", "eta_pull", 100, -10, 10) );
  addHistogram( new TH1F("phi_pull", "phi_pull", 100, -10, 10) );
  addHistogram( new TH1F("z0_pull",  "z0_pull",  100, -10, 10) );
  addHistogram( new TH1F("d0_pull",  "d0_pull",  100, -10, 10) );
  addHistogram( new TH1F("a0_pull",  "a0_pull",  100, -10, 10) );

  // error study histograms (pull test-ref) - SIMPLE VERSION                                                                                                      
  addHistogram( new TH1F("pT_pull_simple",  "pT_pull_simple",  100, -10, 10) );
  addHistogram( new TH1F("eta_pull_simple", "eta_pull_simple", 100, -10, 10) );
  addHistogram( new TH1F("phi_pull_simple", "phi_pull_simple", 100, -10, 10) );
  addHistogram( new TH1F("z0_pull_simple",  "z0_pull_simple",  100, -10, 10) );
  addHistogram( new TH1F("d0_pull_simple",  "d0_pull_simple",  100, -10, 10) );
  addHistogram( new TH1F("a0_pull_simple",  "a0_pull_simple",  100, -10, 10) );

  
  // resolutions
  TH1F* pT_res = new TH1F(  "pT_res",   "pT_res",  4*pTResBins,   -0.1,  0.1  ); 
  pT_res->GetXaxis()->SetTitle("#Delta P_{T} [GeV]");
  pT_res->GetYaxis()->SetTitle("Entries");
  addHistogram( pT_res );


  TH1F* spT_res = new TH1F( "spT_res",  "spT_res",  4*pTResBins,   -0.1,  0.1 );
  spT_res->GetXaxis()->SetTitle("#Delta sP_{T} [GeV]");
  spT_res->GetYaxis()->SetTitle("Entries");
  addHistogram( spT_res );


  TH1F* ipT_res = new TH1F( "ipT_res",  "ipT_res",  4*pTResBins,   -0.4,  0.4 );
  ipT_res->GetXaxis()->SetTitle("#Delta 1/P_{T} [GeV^{-1}]");
  ipT_res->GetYaxis()->SetTitle("Entries");
  addHistogram( ipT_res );

  TH1F* eta_res = new TH1F("eta_res",  "eta_res",   etaResBins,  -2*tmp_absResEta, 2*tmp_absResEta );
  eta_res->GetXaxis()->SetTitle("#Delta #eta");
  eta_res->GetYaxis()->SetTitle("Entries");
  addHistogram( eta_res );
  addHistogram( new TH1F("etai_res", "etai_res", 1000,  -0.04, 0.04 ) );
  
  //  TH1F* phi_res =  new TH1F( "phi_res",  "phi_res;#Delta #phi;Entries", 2*phiResBins,  -2*tmp_absResPhi, 2*tmp_absResPhi );
  //  phi_res->GetXaxis()->SetTitle("#Delta #phi");
  //  phi_res->GetYaxis()->SetTitle("Entries");
  //  addHistogram( phi_res );

  addHistogram(    new TH1F( "phi_res",   "phi_res;#Delta #phi;Entries", 2*phiResBins,  -2*tmp_absResPhi, 2*tmp_absResPhi ) );
  addHistogram(    new TH1F(  "z0_res",   "z0_res;#Deltaz_{0};Entries",  16*zresBins,        -8*zresMax,       8*zresMax ) );
  addHistogram(    new TH1F(  "d0_res",   "d0_res;#Deltad_{0};Entries",  4*d0resBins,     -0.2*d0resMax,    0.2*d0resMax ) );
  addHistogram(    new TH1F(  "a0_res",   "a0_res;#Deltaa_{0};Entries",  4*a0resBins,     -0.2*a0resMax,    0.2*a0resMax ) );


  addHistogram(    new TH1F( "dphi_res",  "dphi_res;#Delta #phi;Entries", 2*phiResBins,  -0.2*tmp_absResPhi, 0.2*tmp_absResPhi ) );
  addHistogram(    new TH1F(  "dz0_res",   "dz0_res;#Deltaz_{0};Entries",   8*zresBins,        -0.8*zresMax,       0.8*zresMax ) );
  addHistogram(    new TH1F(  "dd0_res",   "dd0_res;#Deltad_{0};Entries",  4*d0resBins,      -0.05*d0resMax,     0.05*d0resMax ) );
  addHistogram(    new TH1F(  "da0_res",   "da0_res;#Deltaa_{0};Entries",  4*a0resBins,      -0.05*a0resMax,     0.05*a0resMax ) );


  //  std::cout << "booking resplots" << std::endl;

  int factor = 10;

  int wfactor = 2;
  int zfactor = 16;


  m_rDd0res.push_back(  new Resplot("rDd0_vs_pt",  ptnbins, ptbinlims, 1200, -0.1, 0.1 ) );
  m_rDd0res.push_back(  new Resplot("rDd0_vs_eta",  5*etaBins, -tmp_maxEta, tmp_maxEta, 1200, -0.1, 0.1 ) );
  m_rDd0res.push_back(  new Resplot("rDd0_vs_zed",  zBins, -zMax, zMax, 1200, -0.1, 0.1 ) );
  m_rDd0res.push_back(  new Resplot("rDd0_vs_d0",   20, -1.5, 1.5, 1200, -0.1, 0.1 ) );
  //  m_rDd0res.push_back(  new Resplot("rDd0_vs_phi",  int(2*M_PI/0.1), -0.02*int(M_PI/0.02), 0.02*int(M_PI/0.02), 1200, -0.01, 0.05 ) );
  m_rDd0res.push_back(  new Resplot("rDd0_vs_phi",  128, -M_PI, M_PI, 1200, -0.1, 0.1 ) );


  m_rDa0res.push_back(  new Resplot("rDa0_vs_pt",  ptnbins, ptbinlims, 100, -0.1, 0.1 ) );
  m_rDa0res.push_back(  new Resplot("rDa0_vs_eta",  etaBins, -tmp_maxEta, tmp_maxEta, 100, -0.1, 0.1 ) );
  m_rDa0res.push_back(  new Resplot("rDa0_vs_zed",  0.2*zBins, -zMax, zMax, 100, -0.1, 0.1 ) );
  m_rDa0res.push_back(  new Resplot("rDa0_vs_da0", 20, -1.5, 1.5, 100, -0.1, 0.1 ) );
  m_rDa0res.push_back(  new Resplot("rDa0_vs_phi",     int(2*M_PI/0.02), -0.02*int(M_PI/0.02), 0.02*int(M_PI/0.02), 100, -0.1, 0.1 ) );
  m_rDa0res.push_back(  new Resplot("rDa0_rec_vs_phi", int(2*M_PI/0.02), -0.02*int(M_PI/0.02), 0.02*int(M_PI/0.02), 100, -0.1, 0.1 ) );

  m_rDa0res.push_back(  new Resplot("rda0_vs_phi",     int(2*M_PI/0.02), -0.02*int(M_PI/0.02), 0.02*int(M_PI/0.02), 100, -0.1, 0.1 ) );
  m_rDa0res.push_back(  new Resplot("rda0_rec_vs_phi", int(2*M_PI/0.02), -0.02*int(M_PI/0.02), 0.02*int(M_PI/0.02), 100, -0.1, 0.1 ) );

  //  m_rDd0res.push_back(  new Resplot("rDd0_vs_ipt",  iptnbins, iptbinlims, 100, -0.1, 0.1 ) );
  //  m_rDz0res.push_back(  new Resplot("rDz0_vs_ipt",  iptnbins, iptbinlims, 100, -1, 1 ) );


  m_rDz0res.push_back(  new Resplot("rDz0_vs_pt",   ptnbins, ptbinlims, 501, -1, 1 ) );
  m_rDz0res.push_back(  new Resplot("rDz0_vs_eta",  5*etaBins, -tmp_maxEta, tmp_maxEta, 500, -1, 1 ) );
  m_rDz0res.push_back(  new Resplot("rDz0_vs_zed",  zBins, -zMax, zMax, 500, -1, 1 ) );
  m_rDz0res.push_back(  new Resplot("rDz0_vs_phi",  128, -M_PI, M_PI, 500, -1, 1 ) );



  /// what is goping on here? the bins for the residuals should depend on the residual itself, not the x variable, 
  /// how come these are all different?  
  m_retares.push_back( new Resplot("reta_vs_ipt", iptnbins, iptbinlims, 2*etaResBins,  -wfactor*tmp_absResEta, wfactor*tmp_absResEta ) );
  m_rphires.push_back( new Resplot("rphi_vs_ipt", iptnbins, iptbinlims, 8*phiResBins,  -wfactor*tmp_absResPhi, wfactor*tmp_absResPhi ) );
  m_rzedres.push_back( new Resplot("rzed_vs_ipt", iptnbins, iptbinlims, 8*zfactor*zresBins,   -2*zfactor*zresMax,      2*zfactor*zresMax       ) );
  m_rzedthetares.push_back( new Resplot("rzedtheta_vs_ipt", iptnbins, iptbinlims, 24*zfactor*zresBins,   -2*zfactor*zresMax,      2*zfactor*zresMax       ) );
  m_riptres.push_back( new Resplot("ript_vs_ipt", iptnbins, iptbinlims, 16*pTResBins,  -wfactor*tmp_absResPt,  wfactor*tmp_absResPt  ) );
  m_rptres.push_back(  new Resplot("rpt_vs_ipt",  iptnbins, iptbinlims, 8*pTResBins,   -wfactor*tmp_absResPt,  wfactor*tmp_absResPt  ) );
  m_rd0res.push_back(  new Resplot("rd0_vs_ipt",  iptnbins, iptbinlims, factor*8*a0resBins,   -wfactor*a0resMax,  wfactor*a0resMax  ) );


  m_retares.push_back( new Resplot("reta_vs_pt", ptnbins, ptbinlims, 8*etaResBins,  -wfactor*tmp_absResEta, wfactor*tmp_absResEta ) );
  m_rphires.push_back( new Resplot("rphi_vs_pt", ptnbins, ptbinlims, 8*phiResBins,  -wfactor*tmp_absResPhi, wfactor*tmp_absResPhi ) );
  m_rzedres.push_back( new Resplot("rzed_vs_pt", ptnbins, ptbinlims, 24*zfactor*zresBins,   -2*zfactor*zresMax,      2*zfactor*zresMax       ) );
  m_rzedthetares.push_back( new Resplot("rzedtheta_vs_pt", ptnbins, ptbinlims, 24*zfactor*zresBins,   -2*zfactor*zresMax,      2*zfactor*zresMax       ) );
  m_riptres.push_back( new Resplot("ript_vs_pt", ptnbins, ptbinlims, 16*pTResBins,  -wfactor*tmp_absResPt,  wfactor*tmp_absResPt  ) );
  m_rptres.push_back(  new Resplot("rpt_vs_pt",  ptnbins, ptbinlims, 8*pTResBins,   -wfactor*tmp_absResPt,  wfactor*tmp_absResPt  ) );
  m_rd0res.push_back(  new Resplot("rd0_vs_pt",  ptnbins, ptbinlims, factor*24*a0resBins,   -wfactor*a0resMax,  wfactor*a0resMax  ) );

  m_retares.push_back( new Resplot("reta_vs_ET", ptnbins, ptbinlims, 8*etaResBins,  -wfactor*tmp_absResEta, wfactor*tmp_absResEta ) );
  m_rphires.push_back( new Resplot("rphi_vs_ET", ptnbins, ptbinlims, 8*phiResBins,  -wfactor*tmp_absResPhi, wfactor*tmp_absResPhi ) );
  m_rzedres.push_back( new Resplot("rzed_vs_ET", ptnbins, ptbinlims, 24*zfactor*zresBins,   -2*zfactor*zresMax,      2*zfactor*zresMax       ) );
  m_rzedthetares.push_back( new Resplot("rzedtheta_vs_ET", ptnbins, ptbinlims, 24*zfactor*zresBins,   -2*zfactor*zresMax,      2*zfactor*zresMax       ) );
  m_riptres.push_back( new Resplot("ript_vs_ET", ptnbins, ptbinlims, 16*pTResBins,  -wfactor*tmp_absResPt,  wfactor*tmp_absResPt  ) );
  m_rptres.push_back(  new Resplot("rpt_vs_ET",  ptnbins, ptbinlims, 8*pTResBins,   -wfactor*tmp_absResPt,  wfactor*tmp_absResPt  ) );
  m_rd0res.push_back(  new Resplot("rd0_vs_ET",  ptnbins, ptbinlims, factor*24*a0resBins,   -wfactor*a0resMax,  wfactor*a0resMax  ) );


  //  m_retares.push_back( new Resplot("reta_vs_eta", etaBins, -tmp_maxEta, tmp_maxEta,  4*etaResBins,  -tmp_absResEta, tmp_absResEta ) );
  m_retares.push_back( new Resplot("reta_vs_eta", etaBins, -tmp_maxEta, tmp_maxEta,  4*etaResBins,  -wfactor*tmp_absResEta, wfactor*tmp_absResEta ) );
  m_rphires.push_back( new Resplot("rphi_vs_eta", etaBins, -tmp_maxEta, tmp_maxEta,  8*phiResBins,  -wfactor*tmp_absResPhi, wfactor*tmp_absResPhi ) );
  m_rzedres.push_back( new Resplot("rzed_vs_eta", etaBins, -tmp_maxEta, tmp_maxEta,  12*zfactor*zresBins,   -2*zfactor*zresMax,  2*zfactor*zresMax       ) );

  m_rzedthetares.push_back( new Resplot("rzedtheta_vs_eta", etaBins, -tmp_maxEta, tmp_maxEta,  24*zfactor*zresBins,   -zfactor*zresMax,  zfactor*zresMax       ) );
  //m_rzedres.push_back( new Resplot("rzed_vs_eta", etaBins, -tmp_maxEta, tmp_maxEta,  4*zfactor*zresBins,   -2*zwidthfactor*zresMax,  2*zwidthfactor*zresMax       ) );
  m_riptres.push_back( new Resplot("ript_vs_eta", etaBins, -tmp_maxEta, tmp_maxEta,  16*pTResBins,   -tmp_absResPt,  tmp_absResPt  ) );
  m_rptres.push_back(  new Resplot("rpt_vs_eta",  etaBins, -tmp_maxEta, tmp_maxEta,  8*pTResBins,   -tmp_absResPt, tmp_absResPt  ) );
  m_rd0res.push_back(  new Resplot("rd0_vs_eta",  etaBins, -tmp_maxEta, tmp_maxEta,  factor*24*a0resBins,   -wfactor*a0resMax,  wfactor*a0resMax  ) );


  //  rphivsDd0res = new Resplot( "rphi_vs_Dd0", 10, 0, 0.1, int(2*M_PI/0.02), -0.2*int(M_PI/0.02), 0.2*int(M_PI/0.02) );
  m_hphivsDd0res[0] = new TH1F( "hphi_vs_Dd0_0", "phi for Dd0<0.1", int(2*M_PI/0.02), -0.02*int(M_PI/0.02), 0.02*int(M_PI/0.02) );
  m_hphivsDd0res[1] = new TH1F( "hphi_vs_Dd0_1", "phi for Dd0>0.1", int(2*M_PI/0.02), -0.02*int(M_PI/0.02), 0.02*int(M_PI/0.02) );
  m_hphivsDd0res[2] = new TH1F( "hphi_vs_Dd0_2", "all phi",         int(2*M_PI/0.02), -0.02*int(M_PI/0.02), 0.02*int(M_PI/0.02) );

  m_hphivsDa0res[0] = new TH1F( "hphi_vs_Da0_0", "phi for Da0<0.1", int(2*M_PI/0.02), -0.02*int(M_PI/0.02), 0.02*int(M_PI/0.02) );
  m_hphivsDa0res[1] = new TH1F( "hphi_vs_Da0_1", "phi for Da0>0.1", int(2*M_PI/0.02), -0.02*int(M_PI/0.02), 0.02*int(M_PI/0.02) );
  m_hphivsDa0res[2] = new TH1F( "hphi_vs_Da0_2", "all phi",         int(2*M_PI/0.02), -0.02*int(M_PI/0.02), 0.02*int(M_PI/0.02) );
  

  for ( unsigned ih=0 ; ih<3 ; ih++ ) { 
    m_hphivsDd0res[ih]->SetDirectory(0);
    m_hphivsDa0res[ih]->SetDirectory(0);
  }  

  m_retares.push_back( new Resplot("reta_vs_zed", 0.2*zBins, -zMax, zMax,  2*etaResBins,  -tmp_absResEta, tmp_absResEta ) );
  m_rphires.push_back( new Resplot("rphi_vs_zed", 0.2*zBins, -zMax, zMax,  8*phiResBins,  -wfactor*tmp_absResPhi, wfactor*tmp_absResPhi ) );
  m_rzedres.push_back( new Resplot("rzed_vs_zed", 0.2*zBins, -zMax, zMax,  8*zfactor*zresBins,   -2*zfactor*zresMax,   2*zfactor*zresMax       ) );
  m_rzedthetares.push_back( new Resplot("rzedtheta_vs_zed", 0.2*zBins, -zMax, zMax,  24*zfactor*zresBins,   -2*zfactor*zresMax,   2*zfactor*zresMax       ) );
  //m_rzedres.push_back( new Resplot("rzed_vs_zed", 0.2*zBins, -zMax, zMax,  4*zfactor*zresBins,   -2*zwidthfactor*zresMax,   2*zwidthfactor*zresMax       ) );
  //m_rzedres.push_back( new Resplot("rzed_vs_zed", zBins, -zMax, zMax,  4*zfactor*zresBins,   -2*zwidthfactor*zresMax,   2*zwidthfactor*zresMax       ) );
  m_riptres.push_back( new Resplot("ript_vs_zed", 0.2*zBins, -zMax, zMax,  2*pTResBins,     -2*tmp_absResPt,  2*tmp_absResPt  ) ); 
  m_rptres.push_back(  new Resplot("rpt_vs_zed",  0.2*zBins, -zMax, zMax,  8*pTResBins,   -tmp_absResPt,  tmp_absResPt  ) ); 
  m_rd0res.push_back(  new Resplot("rd0_vs_zed",  0.2*zBins, -zMax, zMax,  factor*8*a0resBins,  -wfactor*a0resMax,      wfactor*a0resMax  ) );


  m_retares.push_back( new Resplot("reta_vs_nvtx", 24, 0, 72,  4*etaResBins,  -tmp_absResEta, tmp_absResEta ) );
  m_rphires.push_back( new Resplot("rphi_vs_nvtx", 24, 0, 72,  8*phiResBins,  -wfactor*tmp_absResPhi, wfactor*tmp_absResPhi ) );
  m_rzedres.push_back( new Resplot("rzed_vs_nvtx", 24, 0, 72,  4*zfactor*zresBins,   -zfactor*zresMax,      zfactor*zresMax       ) );
  m_rzedthetares.push_back( new Resplot("rzedtheta_vs_nvtx", 24, 0, 72,  24*zfactor*zresBins,   -zfactor*zresMax,      zfactor*zresMax       ) );
  m_riptres.push_back( new Resplot("ript_vs_nvtx", 24, 0, 72,  4*pTResBins,   -tmp_absResPt,  tmp_absResPt  ) ); 
  m_rptres.push_back(  new Resplot("rpt_vs_nvtx",  24, 0, 72,  8*pTResBins,   -tmp_absResPt,  tmp_absResPt  ) ); 
  m_rd0res.push_back(  new Resplot("rd0_vs_nvtx",  24, 0, 72,  factor*8*a0resBins,   -wfactor*a0resMax,  wfactor*a0resMax  ) );


  m_retares.push_back( new Resplot("reta_vs_ntracks", 60, 0, 600,  4*etaResBins,  -tmp_absResEta, tmp_absResEta ) );
  m_rphires.push_back( new Resplot("rphi_vs_ntracks", 60, 0, 600,  8*phiResBins,  -wfactor*tmp_absResPhi, wfactor*tmp_absResPhi ) );
  m_rzedres.push_back( new Resplot("rzed_vs_ntracks", 60, 0, 600,  4*zfactor*zresBins,   -zfactor*zresMax,     zfactor*zresMax       ) );
  m_rzedthetares.push_back( new Resplot("rzedtheta_vs_ntracks", 60, 0, 600,  24*zfactor*zresBins,   -zfactor*zresMax,     zfactor*zresMax       ) );
  //m_rzedres.push_back( new Resplot("rzed_vs_ntracks", 60, 0, 600,  4*zfactor*zresBins,   -zfactor*0.5*zresMax,     zfactor*0.5*zresMax       ) );
  m_riptres.push_back( new Resplot("ript_vs_ntracks", 60, 0, 600,  4*pTResBins,   -tmp_absResPt,  tmp_absResPt  ) ); 
  m_rptres.push_back(  new Resplot("rpt_vs_ntracks",  60, 0, 600,  8*pTResBins,   -tmp_absResPt,  tmp_absResPt  ) ); 
  m_rd0res.push_back(  new Resplot("rd0_vs_ntracks",  60, 0, 600,  factor*8*a0resBins,   -wfactor*a0resMax,  wfactor*a0resMax  ) );


  m_retares.push_back( new Resplot("reta_vs_phi", 128, -M_PI, M_PI, 2*etaResBins,       -wfactor*tmp_absResEta, wfactor*tmp_absResEta ) );
  m_rphires.push_back( new Resplot("rphi_vs_phi", 128, -M_PI, M_PI, 8*phiResBins,       -wfactor*tmp_absResPhi, wfactor*tmp_absResPhi ) );
  m_rzedres.push_back( new Resplot("rzed_vs_phi", 128, -M_PI, M_PI, 8*zfactor*zresBins, -2*zfactor*zresMax,     2*zfactor*zresMax       ) );
  m_rzedthetares.push_back( new Resplot("rzedtheta_vs_phi", 128, -M_PI, M_PI, 24*zfactor*zresBins, -2*zfactor*zresMax,     2*zfactor*zresMax       ) );
  m_riptres.push_back( new Resplot("ript_vs_phi", 128, -M_PI, M_PI, 16*pTResBins,       -wfactor*tmp_absResPt,  wfactor*tmp_absResPt  ) );
  m_rptres.push_back(  new Resplot("rpt_vs_phi",  128, -M_PI, M_PI, 8*pTResBins,        -wfactor*tmp_absResPt,  wfactor*tmp_absResPt  ) );
  m_rd0res.push_back(  new Resplot("rd0_vs_phi",  128, -M_PI, M_PI, factor*8*a0resBins, -wfactor*a0resMax,      wfactor*a0resMax  ) );



  m_retares.push_back( new Resplot("reta_vs_mu", 24, 0, 72,  4*etaResBins,  -tmp_absResEta, tmp_absResEta ) );
  m_rphires.push_back( new Resplot("rphi_vs_mu", 24, 0, 72,  8*phiResBins,  -wfactor*tmp_absResPhi, wfactor*tmp_absResPhi ) );
  m_rzedres.push_back( new Resplot("rzed_vs_mu", 24, 0, 72,  4*zfactor*zresBins,   -zfactor*zresMax,      zfactor*zresMax       ) );
  m_rzedthetares.push_back( new Resplot("rzedtheta_vs_mu", 24, 0, 72,  24*zfactor*zresBins,   -zfactor*zresMax,      zfactor*zresMax       ) );
  m_riptres.push_back( new Resplot("ript_vs_mu", 24, 0, 72,  4*pTResBins,   -tmp_absResPt,  tmp_absResPt  ) );
  m_rptres.push_back(  new Resplot("rpt_vs_mu",  24, 0, 72,  8*pTResBins,   -tmp_absResPt,  tmp_absResPt  ) );
  m_rd0res.push_back(  new Resplot("rd0_vs_mu",  24, 0, 72,  factor*8*a0resBins,   -wfactor*a0resMax,  wfactor*a0resMax  ) );


  //ADDED BY JK
  //-----
  m_rzedres.push_back( new Resplot("rzed_vs_signed_pt", ptnbins2, ptbinlims2, 4*zfactor*zresBins,   -zfactor*zresMax,      zfactor*zresMax       ) );
  m_rzedres.push_back( new Resplot("rzed_vs_ABS_pt",    ptnbins,  ptbinlims,  4*zfactor*zresBins,   -2*zfactor*zresMax,  2*zfactor*zresMax       ) );
  m_rzedthetares.push_back( new Resplot("rzedtheta_vs_signed_pt", ptnbins2, ptbinlims2, 24*zfactor*zresBins,   -zfactor*zresMax,      zfactor*zresMax       ) );
  m_rzedthetares.push_back( new Resplot("rzedtheta_vs_ABS_pt",    ptnbins,  ptbinlims,  24*zfactor*zresBins,   -2*zfactor*zresMax,  2*zfactor*zresMax       ) );
  //m_rzedres.push_back( new Resplot("rzed_vs_signed_pt", ptnbins2, ptbinlims2, 4*zfactor*zresBins,   -2*zwidthfactor*zresMax,      2*zwidthfactor*zresMax       ) );
  //m_rzedres.push_back( new Resplot("rzed_vs_ABS_pt", ptnbins, ptbinlims, 4*zfactor*zresBins,   -2*zwidthfactor*zresMax,      2*zwidthfactor*zresMax       ) );
  m_rd0res.push_back( new Resplot("rd0_vs_signed_pt", ptnbins2, ptbinlims2, factor*8*a0resBins,   -wfactor*a0resMax,  wfactor*a0resMax  ) );
  m_rd0res.push_back( new Resplot("rd0_vs_ABS_pt",    ptnbins,  ptbinlims,  factor*8*a0resBins,   -wfactor*a0resMax,  wfactor*a0resMax  ) );
  //-----


  //  std::cout << "booked" << std::endl;

  m_retaresPull.push_back( new Resplot("retaPull_vs_ipt", iptnbins, iptbinlims, 2*etaResBins, -5,5));//-wfactor*tmp_absResEta, wfactor*tmp_absResEta ) );
  m_rphiresPull.push_back( new Resplot("rphiPull_vs_ipt", iptnbins, iptbinlims, 8*phiResBins, -5,5));//-wfactor*tmp_absResPhi, wfactor*tmp_absResPhi ) );
  m_rzedresPull.push_back( new Resplot("rzedPull_vs_ipt", iptnbins, iptbinlims, 4*zfactor*zresBins, -5,5));//-2*zfactor*zresMax, 2*zfactor*zresMax ) );
  //m_riptresPull.push_back( new Resplot("ript_vs_ipt", iptnbins, iptbinlims, 16*pTResBins, -wfactor*tmp_absResPt, wfactor*tmp_absResPt ) );
  m_rptresPull.push_back( new Resplot("rptPull_vs_ipt", iptnbins, iptbinlims, 8*pTResBins, -5,5));//-wfactor*tmp_absResPt, wfactor*tmp_absResPt ) );
  m_rd0resPull.push_back( new Resplot("rd0Pull_vs_ipt", iptnbins, iptbinlims, factor*8*a0resBins, -5,5));//-wfactor*a0resMax, wfactor*a0resMax ) ) ;

  m_retaresPull.push_back( new Resplot("retaPull_vs_pt", ptnbins, ptbinlims, 2*etaResBins,  -5,5));//-wfactor*tmp_absResEta, wfactor*tmp_absResEta ) );
  m_rphiresPull.push_back( new Resplot("rphiPull_vs_pt", ptnbins, ptbinlims, 8*phiResBins,  -5,5));//-wfactor*tmp_absResPhi, wfactor*tmp_absResPhi ) );
  m_rzedresPull.push_back( new Resplot("rzedPull_vs_pt", ptnbins, ptbinlims, 4*zfactor*zresBins,   -5,5));//-2*zfactor*zresMax,      2*zfactor*zresMax       ) );
  //rzedres.push_back( new Resplot("rzed_vs_pt", ptnbins, ptbinlims, 4*zfactor*zresBins,   -2*zwidthfactor*zresMax,      2*zwidthfactor*zresMax       ) );
  //m_riptresPull.push_back( new Resplot("ript_vs_pt", ptnbins, ptbinlims, 16*pTResBins,  -wfactor*tmp_absResPt,  wfactor*tmp_absResPt  ) );
  m_rptresPull.push_back(  new Resplot("rptPull_vs_pt",  ptnbins, ptbinlims, 8*pTResBins,   -5,5));//-wfactor*tmp_absResPt,  wfactor*tmp_absResPt  ) );
  m_rd0resPull.push_back(  new Resplot("rd0Pull_vs_pt",  ptnbins, ptbinlims, factor*8*a0resBins,   -5,5));//-wfactor*a0resMax,  wfactor*a0resMax  ) );

  m_retaresPull.push_back( new Resplot("retaPull_vs_eta", etaBins, -tmp_maxEta, tmp_maxEta,  4*etaResBins,  -5,5));//-tmp_absResEta, tmp_absResEta ) );
  m_rphiresPull.push_back( new Resplot("rphiPull_vs_eta", etaBins, -tmp_maxEta, tmp_maxEta,  8*phiResBins,  -5,5));//-wfactor*tmp_absResPhi, wfactor*tmp_absResPhi) );
  m_rzedresPull.push_back( new Resplot("rzedPull_vs_eta", etaBins, -tmp_maxEta, tmp_maxEta,  4*zfactor*zresBins,   -5,5));//-2*zfactor*zresMax,  2*zfactor*zresMax) );
  //rzedres.push_back( new Resplot("rzed_vs_eta", etaBins, -tmp_maxEta, tmp_maxEta,  4*zfactor*zresBins,   -2*zwidthfactor*zresMax,  2*zwidthfactor*zresMax) );
  //riptresPull.push_back( new Resplot("ript_vs_eta", etaBins, -tmp_maxEta, tmp_maxEta,  16*pTResBins,   -tmp_absResPt,  tmp_absResPt  ) );                       
  m_rptresPull.push_back(  new Resplot("rptPull_vs_eta",  etaBins, -tmp_maxEta, tmp_maxEta,  8*pTResBins,   -5,5));//-tmp_absResPt, tmp_absResPt  ) );
  m_rd0resPull.push_back(  new Resplot("rd0Pull_vs_eta",  etaBins, -tmp_maxEta, tmp_maxEta,  factor*8*a0resBins,   -5,5));//-wfactor*a0resMax,  wfactor*a0resMax  ));
  m_retaresPull.push_back( new Resplot("retaPull_vs_zed", 0.2*zBins, -zMax, zMax,  2*etaResBins,  -5,5));//-tmp_absResEta, tmp_absResEta ) );
  m_rphiresPull.push_back( new Resplot("rphiPull_vs_zed", 0.2*zBins, -zMax, zMax,  8*phiResBins,  -5,5));//-wfactor*tmp_absResPhi, wfactor*tmp_absResPhi ) );
  m_rzedresPull.push_back( new Resplot("rzedPull_vs_zed", 0.2*zBins, -zMax, zMax,  4*zfactor*zresBins,   -5,5));//-2*zfactor*zresMax,   2*zfactor*zresMax       ) ) ;
  //rzedres.push_back( new Resplot("rzed_vs_zed", 0.2*zBins, -zMax, zMax,  4*zfactor*zresBins,   -2*zwidthfactor*zresMax,   2*zwidthfactor*zresMax       ) );  
  //rzedres.push_back( new Resplot("rzed_vs_zed", zBins, -zMax, zMax,  4*zfactor*zresBins,   -2*zwidthfactor*zresMax,   2*zwidthfactor*zresMax       ) ); 
  //riptresPull.push_back( new Resplot("ript_vs_zed", 0.2*zBins, -zMax, zMax,  2*pTResBins,     -2*tmp_absResPt,  2*tmp_absResPt  ) );                            
  m_rptresPull.push_back(  new Resplot("rptPull_vs_zed",  0.2*zBins, -zMax, zMax,  8*pTResBins,   -5,5));//-tmp_absResPt,  tmp_absResPt  ) );
  m_rd0resPull.push_back(  new Resplot("rd0Pull_vs_zed",  0.2*zBins, -zMax, zMax,  factor*8*a0resBins,  -5,5));//-wfactor*a0resMax,      wfactor*a0resMax  ) );
  
  m_retaresPull.push_back( new Resplot("retaPull_vs_nvtx", 12, 0, 36,  4*etaResBins,  -5,5));//-tmp_absResEta, tmp_absResEta ) );
  m_rphiresPull.push_back( new Resplot("rphiPull_vs_nvtx", 12, 0, 36,  8*phiResBins,  -5,5));//-wfactor*tmp_absResPhi, wfactor*tmp_absResPhi ) );
  m_rzedresPull.push_back( new Resplot("rzedPull_vs_nvtx", 12, 0, 36,  4*zfactor*zresBins,   -5,5));//-zfactor*zresMax,      zfactor*zresMax       ) );
  //rzedres.push_back( new Resplot("rzed_vs_nvtx", 12, 0, 36,  4*zfactor*zresBins,   -zfactor*0.5*zresMax,      zfactor*0.5*zresMax       ) );         
  
  //riptresPull.push_back( new Resplot("ript_vs_nvtx", 12, 0, 36,  4*pTResBins,   -tmp_absResPt,  tmp_absResPt  ) );                                              
  m_rptresPull.push_back(  new Resplot("rptPull_vs_nvtx",  12, 0, 36,  8*pTResBins,   -5,5));//-tmp_absResPt,  tmp_absResPt  ) );
  m_rd0resPull.push_back(  new Resplot("rd0Pull_vs_nvtx",  12, 0, 36,  factor*8*a0resBins,   -5,5));//-wfactor*a0resMax,  wfactor*a0resMax  ) );

  m_retaresPull.push_back( new Resplot("retaPull_vs_ntracks", 60, 0, 600,  4*etaResBins,  -5,5));//-tmp_absResEta, tmp_absResEta ) );
  m_rphiresPull.push_back( new Resplot("rphiPull_vs_ntracks", 60, 0, 600,  8*phiResBins,  -5,5));//-wfactor*tmp_absResPhi, wfactor*tmp_absResPhi ) );
  m_rzedresPull.push_back( new Resplot("rzedPull_vs_ntracks", 60, 0, 600,  4*zfactor*zresBins,   -5,5));//-zfactor*zresMax,     zfactor*zresMax       ) );
  //rzedres.push_back( new Resplot("rzed_vs_ntracks", 60, 0, 600,  4*zfactor*zresBins,   -zfactor*0.5*zresMax,     zfactor*0.5*zresMax       ) );    
  //riptresPull.push_back( new Resplot("ript_vs_ntracks", 60, 0, 600,  4*pTResBins,   -tmp_absResPt,  tmp_absResPt  ) );                                          
  m_rptresPull.push_back(  new Resplot("rptPull_vs_ntracks",  60, 0, 600,  8*pTResBins,   -5,5));//-tmp_absResPt,  tmp_absResPt  ) );
  m_rd0resPull.push_back(  new Resplot("rd0Pull_vs_ntracks",  60, 0, 600,  factor*8*a0resBins,   -5,5));//-wfactor*a0resMax,  wfactor*a0resMax  ) );
  
  m_rzedresPull.push_back( new Resplot("rzedPull_vs_signed_pt", ptnbins2, ptbinlims2, 4*zfactor*zresBins,   -5,5));//-2*zfactor*zresMax,      2*zfactor*zresMax  ) );
  m_rzedresPull.push_back( new Resplot("rzedPull_vs_ABS_pt", ptnbins, ptbinlims, 4*zfactor*zresBins,   -5,5));//-2*zfactor*zresMax,      2*zfactor*zresMax       ));
  //rzedres.push_back( new Resplot("rzed_vs_signed_pt", ptnbins2, ptbinlims2, 4*zfactor*zresBins,   -2*zwidthfactor*zresMax,      2*zwidthfactor*zresMax       ));
  //rzedres.push_back( new Resplot("rzed_vs_ABS_pt", ptnbins, ptbinlims, 4*zfactor*zresBins,   -2*zwidthfactor*zresMax,      2*zwidthfactor*zresMax       ) );              
  m_rd0resPull.push_back( new Resplot("rd0Pull_vs_signed_pt", ptnbins2, ptbinlims2, factor*8*a0resBins,   -5,5));//-wfactor*a0resMax,  wfactor*a0resMax  ) );
  m_rd0resPull.push_back( new Resplot("rd0Pull_vs_ABS_pt", ptnbins, ptbinlims, factor*8*a0resBins,   -5,5));//-wfactor*a0resMax,  wfactor*a0resMax  ) );

  m_rzedreslb = new Resplot("rzed_vs_lb", 301, -0.5, 3009.5, 8*zfactor*zresBins, -2*zfactor*zresMax,     2*zfactor*zresMax  );

  m_rzedlb     = new Resplot("zed_vs_lb",     301, -0.5, 3009.5, 100, -300, 300 );
  m_rzedlb_rec = new Resplot("zed_vs_lb_rec", 301, -0.5, 3009.5, 100, -300, 300 );
  

  m_rd0_vs_phi     = new Resplot( "d0_vs_phi",     20, -M_PI, M_PI, 200, -10, 10 );
  m_rd0_vs_phi_rec = new Resplot( "d0_vs_phi_rec", 20, -M_PI, M_PI, 200, -10, 10 );


  /// Roi - track residuals

  m_rRoi_deta_vs_eta = new Resplot( "rRoi_deta_vs_eta", 20, -2.5, 2.5, 101,  -0.5,  0.5 );
  m_rRoi_dphi_vs_eta = new Resplot( "rRoi_dphi_vs_eta", 20, -2.5, 2.5, 101,  -0.5,  0.5 );
  m_rRoi_dzed_vs_eta = new Resplot( "rRoi_dzed_vs_eta", 20, -2.5, 2.5, 401, -30.0, 30.0 );

  //  std::cout << "ROI RESPLOTS " <<  m_rRoi_deta_vs_eta->Name() << " " << m_rRoi_dphi_vs_eta->Name() << std::endl;


  // hit occupancies

  int   NHits = 40;
  int Ntracks = 10000;

  addHistogram( new TH1F( "nsct",     "nsct",     NHits, -0.5, float(NHits-0.5) ) );
  addHistogram( new TH1F( "nsct_rec", "nsct_rec", NHits, -0.5, float(NHits-0.5) ) );

  addHistogram( new TH1F( "npix",     "npix",     NHits, -0.5, float(NHits-0.5) ) );
  addHistogram( new TH1F( "npix_rec", "npix_rec", NHits, -0.5, float(NHits-0.5) ) );

  addHistogram( new TH1F( "nsi",     "nsi",     NHits, -0.5, float(NHits-0.5) ) );
  addHistogram( new TH1F( "nsi_rec", "nsi_rec", NHits, -0.5, float(NHits-0.5) ) );
  addHistogram( new TH1F( "nsi_matched", "nsi_matched", NHits, -0.5, float(NHits-0.5) ) );


  addHistogram( new TH1F( "ntrt",     "ntrt",     NHits, -0.5, float(NHits-0.5) ) );
  addHistogram( new TH1F( "ntrt_rec", "ntrt_rec", NHits, -0.5, float(NHits-0.5) ) );

  addHistogram( new TH1F( "nstraw",     "nstraw",     NHits*4, -0.5, float(4*NHits-0.5) ) );
  addHistogram( new TH1F( "nstraw_rec", "nstraw_rec", NHits*4, -0.5, float(4*NHits-0.5) ) );

  addHistogram( new TH1F( "ntracks",     "ntracks",     Ntracks+1, -0.5, float(Ntracks+0.5) ) );
  addHistogram( new TH1F( "ntracks_rec", "ntracks_rec", Ntracks+1, -0.5, float(Ntracks+0.5) ) );


  // beam offset fitting histos
  m_h2  = new Resplot( "d0vphi",       phiBins, -3.142, 3.142, d0Bins, -d0Max, d0Max );
  m_h2r = new Resplot( "d0vphi_rec",   phiBins, -3.142, 3.142, d0Bins, -d0Max, d0Max );
  m_h2m = new Resplot( "d0vphi_match", phiBins, -3.142, 3.142, d0Bins, -d0Max, d0Max );

  m_h2a0  = new Resplot( "a0vphi",       phiBins, -3.142, 3.142, d0Bins, -d0Max, d0Max );
  m_h2a0r = new Resplot( "a0vphi_rec",   phiBins, -3.142, 3.142, d0Bins, -d0Max, d0Max );

  /// efficiency vs lumi block


  TH1F heffvlb("eff vs lb", "eff vs lb", 301, -0.5, 3009.5 );

    //	       100, 
    //	       1270515000, 1270560000 
    //	       1272040000, 1272200000
    //    );
    // 1270518944,
    // 1270558762 );
    
    //	       1260470000, 
    //	       1260680000 ); 
    //	       1260470000, 
    //	       1260690000 ); 
    // TH1F heffvlb("eff vs lb", "eff vs lb", 600, 0, 3000);
    
    //  TH1F heffvlb("eff vs lb", "eff vs lb", 600, 1260400000, 1260700000); 


  m_eff_vs_lb = new Efficiency1D( &heffvlb );

  //  m_z_vs_lb = new Resplot("z vs lb", 100, 1270515000,  1270560000, 100, -250, 250);
  m_z_vs_lb = new Resplot("z vs lb", 301, -0.5,  3009.5, 100, -250, 250);

  //m_rmap[142165] = 0;
  //m_rmap[142166] = 500;
  //m_rmap[142174] = 1000;
  //m_rmap[142191] = 1500;
  //m_rmap[142193] = 2000;
  //m_rmap[142195] = 2500;


  m_deltaR_v_eta = new Resplot("R v eta", 10, -2.5, 2.5,     100, 0, 0.1 );
  m_deltaR_v_pt  = new Resplot("R v pt", ptnbins, ptbinlims, 100, 0, 0.1 );


  TH1F* eff_vs_mult = new TH1F( "eff_vs_mult", "eff_vs_mult", 25, 0, 25 );

  m_eff_vs_mult = new Efficiency1D( eff_vs_mult, "eff_mult" );


  m_n_vtx_tracks   = new TH1F("nvtxtracks", "nvtxtracks", 150, 0, 600);
  m_eff_vs_ntracks = new Efficiency1D( m_n_vtx_tracks, "eff_vs_ntracks");



  //double oldnbins[21] = { 1,   5,   10,  15,  20, 
  //	25,  30,  35,  40,  45, 
  //	50,  60,  80,  100, 150, 
  //	200, 250, 300, 400, 500, 600 };


  //double _nbins[23] = {  0,         29.4,    39.9179, 48.4358, 56.1813,  62.9524,  70.1377, 76.8907,
  //	83.4667,   90.559,  97.4902, 105.737, 113.547,	121.281,  129.015, 139.824,
  //	150.589,   164.093, 179.096, 206.867, 400,      500,      600 };

  double nbins[23] = {  0,      29.5,  39.5,  48.5,  56.5,  63.5,  70.5,  77.5,
			83.5,   91.5,  97.5,  106.5, 114.5, 121.5, 129.5, 140.5,
			151.5,  164.5, 200.5, 250.5, 300.5, 400.5, 600 };

  TH1F* n_vtx_tracks2   = new TH1F("nvtxtracks2", "nvtxtracks2", 22, nbins);
  m_eff_vs_ntracks2 = new Efficiency1D( n_vtx_tracks2, "eff_vs_ntracks2");
  delete n_vtx_tracks2;

  m_n_vtx       = new TH1F("nvtx", "nvtx", 81, -0.5, 80.5);
  m_eff_vs_nvtx = new Efficiency1D( m_n_vtx, "eff_vs_nvtx");
  //m_mu          = new TH1F("mu", "mu", 3000, -0.5, 29.5);
  m_mu          = new TH1F("mu", "mu", 90, 0, 90);
  m_eff_vs_mu   = new Efficiency1D( m_mu, "eff_vs_mu");


  /// electron specific histograms

  double etovpt_bins[39] = {
    0,        0.1,      0.2,     0.3,     0.4,      0.5,       0.6,      0.7,      0.8,      0.9, 
    1,        1.08571,  1.17877, 1.2798,  1.3895,   1.50859,   1.63789,  1.77828,  1.9307,   2.09618, 
    2.27585,  2.47091,  2.6827,  2.91263, 3.16228,  3.43332,  3.72759,  4.04709,  4.39397,  4.77058,  
    5.17947,  5.62341,  6.1054,  6.6287,  7.19686,  7.81371,  8.48343,  9.21055,  10
  }; 

  m_etovpt_raw    = new TH1F("etovpt_raw", "ET / pT", 100, 0, 10 );

  m_etovpt        = new TH1F("etovpt", "ET / pT", 38, etovpt_bins );
  m_eff_vs_etovpt = new Efficiency1D( m_etovpt, "eff_vs_etovpt");


  m_et          = new TH1F("ET", "ET; E_{T} [GeV]", ptnbins, ptbinlims );
  m_eff_vs_et   = new Efficiency1D( m_et, "eff_vs_ET" );


  //  std::cout << "initialize() Directory " << gDirectory->GetName() << " on leaving" << std::endl;

  ConfVtxAnalysis* vtxanal = 0;
  store().find( vtxanal, "rvtx" );
  if ( vtxanal ) vtxanal->initialise();

  //  std::cout << "initialize() Directory " << gDirectory->GetName() << " on leaving" << std::endl;


  m_dir->pop();

  dir->cd();



}





// fit Gaussian to histogram

TF1* FitFWGaussian(TH1D* s, double a, double b) {

  //  std::cout << "FitFWGaussian " << s->GetName() << std::endl;

  //  TF1* f1 = new TF1("gausr", "gaus"); 
  TF1* f1 = new TF1("gausr", "[0]*exp(-(x-[1])*(x-[1])/([2]*[2]))"); 

  f1->SetNpx(5000);
  f1->SetLineWidth(1);

  f1->SetParameter(0,s->GetBinContent(s->GetMaximumBin()) );
  f1->SetParameter(1,s->GetBinCenter(s->GetMaximumBin()));
  //  f1->SetParameter(2,s->GetRMS());
  f1->SetParameter(2,1.5);
  f1->FixParameter(2,1.5);

  f1->SetParName(0, "Norm");
  f1->SetParName(1, "Mean");
  f1->SetParName(2, "Sigma");

  int nbins=0;
  for (int j=1 ; j<=s->GetNbinsX() ; j++) if (s->GetBinContent(j)) nbins++;

  if (nbins>2) {
    if ( a==-999 || b==-999 )  s->Fit(f1,"Q");
    else                       s->Fit(f1,"Q", "", a, b);
  }
  else for ( int j=0 ; j<3 ; j++ ) f1->SetParameter(j, 0);

  //  std::cout << "return" << std::endl;

  return f1;
}





void fitSin( TH1D* h, const std::string& parent="" ) { 

  TF1* fsin = new TF1( "sinp", "sqrt([0]*[0])*sin([1]-x)+[2]" ); // , -M_PI, M_PI );

  fsin->SetParameter(0,1);
  fsin->SetParameter(1,0);
  fsin->SetParameter(2,0);

  fsin->SetLineWidth(1);

  //  h->SetTitle(";track #phi;d_{0} [mm]");
 
  std::cout << __FUNCTION__ << " " << h->GetName() << std::endl; 
  h->Fit( fsin, "", "", -M_PI, M_PI );

  double radius = fsin->GetParameter(0);
  double phi    = fsin->GetParameter(1);
  
  double y = radius*sin(phi);
  double x = radius*cos(phi);
  
  std::cout << parent << "\t" << h->GetTitle() << "\tx = " << x << "\ty = " << y << std::endl;

  delete fsin;
}




/// calculate the efficiencies and write them out with all the histograms 

void ConfAnalysis::finalise() {

  //  gDirectory->pwd();

  if ( !m_initialised ) return;

  std::cout << "ConfAnalysis::finalise() " << name();

  if ( name().size()<19 ) std::cout << "\t";
  if ( name().size()<30 ) std::cout << "\t";
  if ( name().size()<41 ) std::cout << "\t";
  if ( name().size()<52 ) std::cout << "\t";

   
  std::cout << "\tNreco "    << m_Nreco
	    << "\tNref "     << m_Nref
	    << "\tNmatched " << m_Nmatched;

  if (m_Nref) {
    std::cout << " tracks approx " << (100*m_Nmatched)/m_Nref << "%" ;
  }
  std::cout  << std::endl;

  //  if ( m_Nreco==0 ) return;

  //  TIDDirectory d( name() );
  //  d.push();

  m_dir->push();

  std::map<std::string, TH1F*>::iterator hitr=m_histos.begin();
  std::map<std::string, TH1F*>::iterator hend=m_histos.end();
  for ( ; hitr!=hend ; ++hitr ) hitr->second->Write();     
  //  std::cout << "DBG >" << eff_pt->Hist()->GetName() << "< DBG" << std::endl;

  //  std::vector<Efficiency*> heff = { eff_pt,

  const unsigned Neff = 11;
  Efficiency1D*  heff[Neff] = { m_eff_pt,
			      m_eff_eta,
			      m_eff_phi,
			      m_eff_z0,
			      m_eff_d0,
			      m_eff_a0,
			      m_eff_ptm,
			      m_eff_ptp,
			      m_eff_roi_deta,
			      m_eff_roi_dphi,
			      m_eff_roi_dR };

  for ( unsigned i=0 ; i<Neff ; i++ ) {
    heff[i]->finalise();  
    heff[i]->Bayes()->Write( ( heff[i]->name()+"_tg" ).c_str() );
  } // heff[i]->Hist()->Write(); } 

  //  std::cout << "DBG >" << m_purity_pt->Hist()->GetName() << "< DBG" << std::endl;

  m_eff_vs_mult->finalise();

  //  Normalise(n_vtx_tracks);

  m_eff_vs_ntracks->finalise();
  m_eff_vs_ntracks2->finalise();

  m_eff_vs_nvtx->finalise();
  m_eff_vs_mu->finalise();

  m_eff_vs_etovpt->finalise();

  m_eff_vs_et->finalise();
  
  m_eff_eta_vs_pt->finalise();
  m_eff_d0_vs_pt->finalise();


  const unsigned Npurity = 6;
  Efficiency1D* hpurity[Npurity] = {
    m_purity_pt,
    m_purity_eta,
    m_purity_phi,
    m_purity_z0,
    m_purity_d0,
    m_purity_a0 };

  for ( unsigned i = 0 ; i<Npurity ; i++ ) hpurity[i]->finalise();

  m_rd0_vs_phi->Finalise(Resplot::FitNull95);
  fitSin( m_rd0_vs_phi->Mean(), name()+"/rd0_vs_phi" );
  m_rd0_vs_phi->Write();

  m_rd0_vs_phi_rec->Finalise(Resplot::FitNull95);
  fitSin( m_rd0_vs_phi_rec->Mean(), name()+"/rd0_vs_phi_rec" );
  m_rd0_vs_phi_rec->Write();
  

  std::string spstr[5] = { "npix", "nsct", "nsi", "ntrt", "nbl" };
  for ( int i=m_res.size() ; i-- ; ) {
    TF1* (*resfit)(TH1D* s, double a, double b) = Resplot::FitNull95;    
    for ( int ir=0 ; ir<5 ; ir++ ) if ( m_res[i]->Name().find(spstr[ir])!=std::string::npos ) { resfit = Resplot::FitNull; break; }
    m_res[i]->Finalise( resfit );
    m_res[i]->Write();
    delete m_res[i];
  }

  m_deltaR_v_eta->Finalise();   m_deltaR_v_eta->Write();
  m_deltaR_v_pt->Finalise();    m_deltaR_v_pt->Write();

  for ( unsigned i=m_rDd0res.size() ; i-- ; ) {
    m_rDd0res[i]->Finalise(Resplot::FitNull95);
    m_rDd0res[i]->Write();
    delete m_rDd0res[i];
  }

  for ( unsigned i=m_rDa0res.size() ; i-- ; ) {
    m_rDa0res[i]->Finalise(Resplot::FitNull95);
    m_rDa0res[i]->Write();
    delete m_rDa0res[i];
  }

  for ( unsigned i=m_rDz0res.size() ; i-- ; ) {
    m_rDz0res[i]->Finalise(Resplot::FitNull95);
    m_rDz0res[i]->Write();
    delete m_rDz0res[i];
  }


  for ( unsigned ih=0 ; ih<2 ; ih++ ) { 
    m_hphivsDd0res[ih]->Divide( m_hphivsDd0res[2] );
    m_hphivsDa0res[ih]->Divide( m_hphivsDa0res[2] );
  }  

  for ( unsigned ih=0 ; ih<3 ; ih++ ) { 
    m_hphivsDd0res[ih]->Write();
    m_hphivsDa0res[ih]->Write();

    delete m_hphivsDd0res[ih];
    delete m_hphivsDa0res[ih];
  }  

  /// roi residuals 

  m_rRoi_deta_vs_eta->Finalise(Resplot::FitNull95);
  m_rRoi_dphi_vs_eta->Finalise(Resplot::FitNull95);
  m_rRoi_dzed_vs_eta->Finalise(Resplot::FitNull95);
  
  m_rRoi_deta_vs_eta->Write();
  m_rRoi_dphi_vs_eta->Write();
  m_rRoi_dzed_vs_eta->Write();

  delete  m_rRoi_deta_vs_eta;
  delete  m_rRoi_dphi_vs_eta;
  delete  m_rRoi_dzed_vs_eta;

  /// standard residuals



  for ( unsigned i=m_retares.size() ; i-- ; ) {

#if 1

    m_retares[i]->Finalise(Resplot::FitNull95);
    m_rphires[i]->Finalise(Resplot::FitNull95);
    m_riptres[i]->Finalise(Resplot::FitNull95);
    m_rzedres[i]->Finalise(Resplot::FitNull95);
    m_rzedthetares[i]->Finalise(Resplot::FitNull95);
    //    m_rptres[i]->Finalise(Resplot::FitBreit);
    //m_rptres[i]->Finalise(Resplot::FitNull95);
    m_rd0res[i]->Finalise(Resplot::FitNull95);
    //  m_rd0res[i]->Finalise(Resplot::FitCentralGaussian);
    //  m_rd0res_rms[i]->Finalise(Resplot::FitNull);

#else

    m_retares[i]->Finalise();
    m_rphires[i]->Finalise();
    m_rzedres[i]->Finalise();
    m_riptres[i]->Finalise();
    m_rptres[i]->Finalise();
    m_rd0res[i]->Finalise();

#endif    

    m_retares[i]->Write();
    m_rphires[i]->Write();
    m_rzedres[i]->Write();
    m_rzedthetares[i]->Write();
    m_riptres[i]->Write();
    m_rptres[i]->Write();
    m_rd0res[i]->Write();

    delete m_retares[i];
    delete m_rphires[i];
    delete m_rzedres[i];
    delete m_rzedthetares[i];
    delete m_riptres[i];
    delete m_rptres[i];
    delete m_rd0res[i];


  }

  m_rzedreslb->Finalise(Resplot::FitNull95);

  m_rzedlb->Finalise(Resplot::FitNull95);
  m_rzedlb_rec->Finalise(Resplot::FitNull95);

  for ( unsigned i=m_retaresPull.size() ; i-- ; ) {

    m_retaresPull[i]->Finalise(Resplot::FitNull);
    m_rphiresPull[i]->Finalise(Resplot::FitNull);
    m_rptresPull[i]->Finalise(Resplot::FitNull);
    m_rzedresPull[i]->Finalise(Resplot::FitNull);
    m_rd0resPull[i]->Finalise(Resplot::FitNull);

    m_retaresPull[i]->Write();
    m_rphiresPull[i]->Write();
    m_rptresPull[i]->Write();
    m_rzedresPull[i]->Write();
    m_rd0resPull[i]->Write();

    delete m_retaresPull[i];
    delete m_rphiresPull[i];
    delete m_rptresPull[i];
    delete m_rzedresPull[i];
    delete m_rd0resPull[i];

  }


  m_rzedreslb->Write();
  delete m_rzedreslb;

  m_rzedlb->Write();
  delete m_rzedlb;

  m_rzedlb_rec->Write();
  delete m_rzedlb_rec;

  //  td->cd();

  //ADDED BY JK
  //-----Only one more element in d0 and z0 vectors than eta now
  m_rzedres[m_rzedres.size()-1]->Finalise(Resplot::FitNull95);
  m_rzedres[m_rzedres.size()-1]->Write();
  m_rzedthetares.back()->Finalise(Resplot::FitNull95);
  m_rzedthetares.back()->Write();
  m_rd0res.back()->Finalise(Resplot::FitNull95);
  m_rd0res.back()->Write();
  //-----

  m_eff_vs_lb->finalise();

  m_z_vs_lb->Finalise(Resplot::FitNull95); m_z_vs_lb->Write();
  delete m_z_vs_lb;

  //  TH1F* hefflb = eff_vs_lb->Hist();
  //  hefflb->Fit("pol0");

  //  ConfVtxAnalysis* vtxanal = 0;
  //  store().find( vtxanal, "rvtx" );
  //  if ( vtxanal ) vtxanal->finalise();

  //  d.pop();

  ConfVtxAnalysis* vtxanal = 0;
  store().find( vtxanal, "rvtx" );
  if ( vtxanal ) vtxanal->finalise();

  m_dir->pop();

}

extern int Nvtxtracks;
extern int NvtxCount;


/// fill all the histograms - matched histograms, efficiencies etc


double wrapphi( double phi ) { 
  if ( phi<-M_PI ) phi += 2*M_PI;
  if ( phi> M_PI ) phi -= 2*M_PI;
  return phi;
} 


void ConfAnalysis::execute( const std::vector<TIDA::Track*>& reftracks,
			    const std::vector<TIDA::Track*>& testtracks,
			    TrackAssociator* matcher, 
			    TrigObjectMatcher* objects ) { 

  //  leave this commented code in for debug purposes ...
  //  if ( objects ) std::cout << "TrigObjectMatcher: " << objects << std::endl;

  if ( !m_initialised ) initialiseInternal();
      
  if ( m_print ) { 
    std::cout << "ConfAnalysis::execute() \t " << name() 
	      << "\tref "  <<  reftracks.size() 
	      << "\ttest " << testtracks.size();
    if ( groi ) std::cout << "\tgroi "  << groi << " " << *groi;
    std::cout << std::endl;
  }    

  //  std::cout << "ConfAnalysis (resolutions really) filling " << std::endl;

  // should have these as a class variable   
  static std::string  varName[16] = { "pT",   "eta",  "phi",  "z0",    "d0",     "a0",  
				      "nsct", "npix", "nsi",  "ntrt",  "nstraw", 
				      "dd0",  "da0",  "dz0",  "deta",  "dphi" };  

  std::map<std::string, TH1F*>::iterator hmitr = m_histos.find("ntracks");
  if ( hmitr!=m_histos.end() )   hmitr->second->Fill( reftracks.size() );

  hmitr = m_histos.find("ntracks_rec");
  if ( hmitr!=m_histos.end() )   hmitr->second->Fill( testtracks.size() );

  bool dump = false;

  m_Nreco += testtracks.size();
  m_Nref  += reftracks.size();


  //  std::cout << "ConfAnalysis ref tracks " << std::endl; 

  // why don't we use this vertex position any more ???
  // m_Nref = 0;
  // for ( int i=reftracks.size() ; i-- ; ) {
  //  double phit = reftracks[i]->phi();  
  //  //  double a0t  =  reftracks[i]->a0() + sin(phit)*m_xBeamReference - cos(phit)*m_yBeamReference; 
  //  //   if ( std::fabs(a0t)<a0 ) m_Nref++; ???
  // }

  //  if ( testtracks.size() ) std::cout << "NTRACKS " << testtracks.size() << std::endl;

  for ( int i=reftracks.size() ; i-- ; ) { 

    /// fill roi residuals
    
    if ( groi!=0 ) { 
      //  std::cout << "ConfAnalysis::Fill() groi " << *groi << std::endl;

      double deta = reftracks[i]->eta() - groi->eta();
      double dphi = wrapphi( reftracks[i]->phi() - groi->phi() );
      double dzed = reftracks[i]->z0() - groi->zed();
      
      m_rRoi_deta_vs_eta->Fill( groi->eta(), deta );
      m_rRoi_dphi_vs_eta->Fill( groi->eta(), dphi );
      m_rRoi_dzed_vs_eta->Fill( groi->eta(), dzed );
    }

    /// kinematics
    double ipTt = 1./(reftracks[i]->pT()/1000.);     
    double pTt  = reftracks[i]->pT()/1000; 

    double etat = reftracks[i]->eta(); 
    double phit = reftracks[i]->phi(); 

    double thetat = 2*std::atan( exp( (-1)*etat ) );

    /// correct the tracks during creation rather than during the analysis
    ///    double z0t = reftracks[i]->z0()+((std::cos(phit)*m_xBeamReference + std::sin(phit)*m_yBeamReference)/std::tan(thetat));    
    ///    double d0t  = reftracks[i]->a0(); 
    ///    double a0t  = reftracks[i]->a0() + std::sin(phit)*m_xBeamReference - std::cos(phit)*m_yBeamReference; 

    /// BUT we should still allow them to be corrected, but afetr setting with a flag instead

    double z0t  = reftracks[i]->z0();
    double d0t  = reftracks[i]->a0() - std::sin(phit)*m_xBeamReference + std::cos(phit)*m_yBeamReference;  
    double a0t  = reftracks[i]->a0();

    if ( m_xBeamReference!=0 || m_yBeamReference!=0 ) { 
      /// this is such a mess - why, why, why, why, why oh why, can't we just have one convention 
      /// and stick to it.
      z0t = reftracks[i]->z0()+((std::cos(phit)*m_xBeamReference + std::sin(phit)*m_yBeamReference)/std::tan(thetat));    
      d0t  = reftracks[i]->a0(); 
      a0t  = reftracks[i]->a0() + std::sin(phit)*m_xBeamReference - std::cos(phit)*m_yBeamReference; 
    }

    //   if ( m_lfirst ) { 
    //       std::cout << "\na0t " << a0t << "(shifted " << d0t << ")" << std::endl;
    //       std::cout << "\nz0t " << z0t                              << std::endl;
    
    //       std::cout << "\txBeamReference " << m_xBeamReference << "\tyBeamReference " << m_yBeamReference << std::endl;  
    //   }
    


    //    std::cout << "a0t " << a0t << std::endl;

    m_rChi2prob->Fill( pTt, TMath::Prob(reftracks[i]->chi2(),reftracks[i]->dof()) );
    m_rChi2->Fill( pTt, reftracks[i]->chi2() );
    m_rChi2dof->Fill( pTt, reftracks[i]->chi2()/reftracks[i]->dof() );

    //    static double xbeamref = 0;
    //    if ( m_lfirst || m_xBeamReference!=xbeamref) { 
    //      std::cout << __FUNCTION__ << "\tbeamline " << m_xBeamReference << " " << m_yBeamReference << " (ref)" << std::endl;
    //    }
    //    xbeamref = m_xBeamReference;

 
    /// error estimates
    double dpTt  = reftracks[i]->dpT()/1000;
    double detat = reftracks[i]->deta();
    double dphit = reftracks[i]->dphi();

    //RoI variables
    float droi_detat = groi->eta() - reftracks[i]->eta();
    float droi_dphit = groi->phi() - reftracks[i]->phi();
    if ( droi_dphit<-M_PI ) droi_dphit +=2*M_PI;
    if ( droi_dphit>M_PI )  droi_dphit -=2*M_PI;
    float droi_dRt = std::sqrt(droi_dphit*droi_dphit + droi_detat*droi_detat);

    
    //    double dz0t = reftracks[i]->dz0()+((std::cos(phit)*m_xBeamReference + std::sin(phit)*m_yBeamReference)/std::tan(thetat));
    //    double dd0t = reftracks[i]->da0();
    double dz0t = reftracks[i]->dz0();
    double dd0t = reftracks[i]->da0() - std::sin(phit)*m_xBeamReference + std::cos(phit)*m_yBeamReference;
    
    // this will be changed when we know the beam spot position
    //   double a0t  =  reftracks[i]->a0() + sin(phit)*m_xBeam - cos(phit)*m_yBeam; 
    double da0t = reftracks[i]->da0();

#if 0
    std::cout << "etat = " << etat << " +/- " << detat << std::endl;
    std::cout << "phit = " << phit << " +/- " << dphit << std::endl;
    std::cout << "z0t = " << z0t << " +/- " << dz0t << std::endl;
    std::cout << "d0t = " << d0t << " +/- " << dd0t << std::endl;
    std::cout << "a0t = " << a0t << " +/- " << da0t << std::endl;
    std::cout << "pTt = " << pTt << " +/- " << dpTt << std::endl;
#endif

    if ( std::fabs(a0t)>a0 ) continue;

    //    double chi2t = reftracks[i]->chi2(); 
    //    hchi2->Fill( chi2t );

    double nsctt = reftracks[i]->sctHits(); 
    double npixt = reftracks[i]->pixelHits()*0.5; 
    double nsit  = reftracks[i]->pixelHits()*0.5 + reftracks[i]->sctHits(); 

    double nsctht = reftracks[i]->sctHoles(); 
    double npixht = reftracks[i]->pixelHoles(); 
    double nsiht  = reftracks[i]->pixelHoles() + reftracks[i]->sctHoles(); 

    double nbl    = reftracks[i]->bLayerHits();
    double nblh   = ( ( reftracks[i]->expectBL() && reftracks[i]->bLayerHits()<1 ) ? 1 : 0 ); 

    //    double ntrtt   = reftracks[i]->trHits(); 
    double nstrawt = reftracks[i]->strawHits(); 

    //    double ts_scale = (ts-1260400000)*3000.0/(1260700000-1260400000); 

    //    std::cout << "Fill h2 " << " " << m_h2m << " " << *reftracks[i] << std::endl;

    m_h2->Fill( phit, d0t );
    m_h2a0->Fill( phit, a0t );

    m_rd0_vs_phi->Fill( phit, a0t );

    double mu_val = gevent->mu();

    m_mu->Fill( mu_val );


    const TIDA::Track* matchedreco = matcher->matched(reftracks[i]); 

    //    std::cout << "\t\tConfAnalysis " << name() << "\t" << i << " " << *reftracks[i] << " -> ";        

    // raw reference track distributions 
    double vpart[16] = { std::fabs(pTt), etat, phit, z0t, d0t, a0t, nsctt, npixt, nsctt, nsit, nstrawt, dd0t, da0t, dz0t, detat, dphit  };  
  
    /// NB: the dd0, da0 etc plots will be filled only for ref tracks 
    ///     with *matched* test tracks 
    for ( int it=0 ; it<11 ; it++ ) { 
      // std::string hname = varName[it];
      // std::map<std::string, TH1F*>::iterator hmitr = m_histos.find(hname);
      //  if ( hmitr!=m_histos.end() )   hmitr->second->Fill( vpart[it] );

      if ( TH1F* hptr = find( varName[it] ) ) hptr->Fill( vpart[it] ); 
      else std::cerr << "hmmm histo " << varName[it] << " not found" << std::endl;
    }  


    m_rnpix_eta->Fill( etat, npixt*1.0 );
    m_rnsct_eta->Fill( etat, nsctt*1.0 );
    m_rntrt_eta->Fill( etat, nstrawt*1.0 );
    m_rnsihit_eta->Fill( etat, npixt + nsctt*1.);
    
    m_rnpix_phi->Fill(  phit, npixt*1.0 );
    m_rnsct_phi->Fill(  phit, nsctt*1.0 );
    m_rntrt_phi->Fill(  phit, nstrawt*1.0 );
    
    m_rnpix_pt->Fill( std::fabs(pTt), npixt*1.0 );
    m_rnsct_pt->Fill( std::fabs(pTt), nsctt*1.0 );
    m_rntrt_pt->Fill( std::fabs(pTt), nstrawt*1.0 );


    m_rnpix_d0->Fill( a0t, npixt*1.0 );
    m_rnsct_d0->Fill( a0t, nsctt*1.0 );
    m_rntrt_d0->Fill( a0t, nstrawt*1.0 );

    m_rnpixh_d0->Fill( a0t, npixht );
    m_rnscth_d0->Fill( a0t, nsctht );

    m_rnsi_pt->Fill( std::fabs(pTt), nsit );
    m_rnsih_pt->Fill( std::fabs(pTt), nsiht );

    m_rnsi_d0->Fill( a0t, nsit );
    m_rnsih_d0->Fill( a0t, nsiht );

    m_rnsi_eta->Fill( etat, nsit );
    m_rnsih_eta->Fill(etat, nsiht );

    m_rnbl_d0->Fill(  a0t, nbl  );
    m_rnblh_d0->Fill( a0t, nblh );

    
    m_rnpixh_pt->Fill( std::fabs(pTt), npixht );
    m_rnscth_pt->Fill( std::fabs(pTt), nsctht );

    m_rnpix_lb->Fill( gevent->lumi_block(), npixt*1.0 );
    m_rnsct_lb->Fill( gevent->lumi_block(), nsctt*1.0 );


    m_rnsct_vs_npix->Fill( npixt, nsctt );

    double                 etovpt_val = 0;
    const TrackTrigObject* tobj       = 0;
    
    double ET=0;

    if ( objects ) { 
      tobj = objects->object( reftracks[i]->id() );
      if ( tobj ) { 
	/// track pt is signed - whereas the cal based 
	/// object ET (massless pt really) is not 
	etovpt_val = std::fabs( tobj->pt()/reftracks[i]->pT() );
	m_etovpt->Fill( etovpt_val );
	m_etovpt_raw->Fill( etovpt_val );
	m_et->Fill( tobj->pt()*0.001 );
	ET = std::fabs(tobj->pt()*0.001);
      }
    }


    if ( matchedreco )  {

      m_Nmatched++;

      // efficiency histos
      m_eff_pt->Fill(std::fabs(pTt));
      m_eff_z0->Fill(z0t);
      m_eff_eta->Fill(etat);
      m_eff_phi->Fill(phit);
      m_eff_d0->Fill(d0t);
      m_eff_a0->Fill(a0t);
      m_eff_roi_deta->Fill(droi_detat);
      m_eff_roi_dphi->Fill(droi_dphit);
      m_eff_roi_dR->Fill(droi_dRt);

      // signed pT
      if ( pTt<0 ) m_eff_ptm->Fill(std::fabs(pTt));
      else         m_eff_ptp->Fill(std::fabs(pTt));

      m_eff_vs_mult->Fill( m_Nref );


      m_eff_eta_vs_pt->Fill( std::fabs(pTt), etat );    
      m_eff_d0_vs_pt->Fill( std::fabs(pTt), a0t );    



      //    m_eff_vs_lb->Fill( m_rmap[r]+lb );
      // m_eff_vs_lb->Fill( ts_scale );
      //   m_eff_vs_lb->Fill( ts );

      m_eff_vs_lb->Fill( gevent->lumi_block() );

      if ( tobj ) { 
	m_eff_vs_etovpt->Fill(etovpt_val);
	m_eff_vs_et->Fill( std::fabs(tobj->pt()*0.001) );    
      }


      /// fill residual histos

      /// kinematics
      double pTr  = matchedreco->pT()/1000;
      double etar = matchedreco->eta();
      double phir = matchedreco->phi();
      //double z0r  = matchedreco->z0() + std::cos(phir)*m_xBeamTest + std::sin(phir)*m_yBeamTest; ; 
      double thetar = 2*std::atan( exp( (-1)*etar) );

      //      double z0r    = matchedreco->z0()+((std::cos(phir)*m_xBeamTest + std::sin(phir)*m_yBeamTest)/std::tan(thetar));    
      //      double d0r  = matchedreco->a0(); 
      //      double a0r  = matchedreco->a0() + sin(phir)*m_xBeamTest - cos(phir)*m_yBeamTest; // this will be changed when we know the beam spot position

      
      


      //      static bool tfirst = true;
      //      static double xbeamtest = 0;
      //      if ( m_lfirst || xbeamtest!=m_xBeamTest) { 
      //	std::cout << __FUNCTION__ << "\tbeamline " << m_xBeamTest << " " << m_yBeamTest << " (test)" << std::endl;
      //      }
      //      xbeamtest = m_xBeamTest;

      double d0r  = 0;
      double a0r  = 0;
      double z0r  = 0;
      
      d0r  = matchedreco->a0()  - sin(phir)*m_xBeamTest + cos(phir)*m_yBeamTest; // this will be changed when we know the beam spot position 
      a0r  = matchedreco->a0();
      z0r  = matchedreco->z0();

      if ( m_xBeamTest!=0 || m_yBeamTest!=0 ) {
	d0r  = matchedreco->a0(); 
	a0r  = matchedreco->a0() + sin(phir)*m_xBeamTest - cos(phir)*m_yBeamTest; // this will be changed when we know the beam spot position
	z0r  = matchedreco->z0()+((std::cos(phir)*m_xBeamTest + std::sin(phir)*m_yBeamTest)/std::tan(thetar));    
      }


      //      if ( m_lfirst ) { 
      //	std::cout << "\na0r " << a0r << "(shifted " << d0r << ")" << std::endl;
      //	std::cout << "\nz0r " << z0r                              << std::endl;
      //	std::cout << "\txBeamReference " << m_xBeamReference << "\tyBeamReference " << m_yBeamReference << std::endl;  
      //      }


      //      static int it=0;

      //      std::cout << "it " << it++ << std::endl; 
      
      //      m_lfirst = false;

      double nsctr = matchedreco->sctHits(); 
      double npixr = matchedreco->pixelHits()*0.5; 
      double nsir  = matchedreco->pixelHits()*0.5 + matchedreco->sctHits(); 
      
      double nscthr = matchedreco->sctHoles(); 
      double npixhr = matchedreco->pixelHoles(); 


      m_rnpix_lb_rec->Fill( gevent->lumi_block(), npixr*1.0 );
      m_rnsct_lb_rec->Fill( gevent->lumi_block(), nsctr*1.0 );


      //double ntrtr   = matchedreco->trHits(); 
      double nstrawr = matchedreco->strawHits(); 

      /// kinematic error estimates

      double dpTr  = matchedreco->dpT()/1000;
      double detar = matchedreco->deta();
      double dphir = matchedreco->dphi();

      //      double dz0r  = matchedreco->dz0()+((std::cos(phir)*m_xBeamTest + std::sin(phir)*m_yBeamTest)/std::tan(thetar));
      //      double dd0r  = matchedreco->da0();
      //      double da0r  = matchedreco->da0() + sin(phir)*m_xBeamTest - cos(phir)*m_yBeamTest;

      double dz0r  = matchedreco->dz0();
      double dd0r  = matchedreco->da0() - sin(phir)*m_xBeamTest + cos(phir)*m_yBeamTest;
      double da0r  = matchedreco->da0();

#if 0
      std::cout << "etar = " << etar << " +/- " << detar << std::endl;
      std::cout << "phir = " << phir << " +/- " << dphir << std::endl;
      std::cout << "pTr = " << pTr << " +/- " << dpTr << std::endl;
      std::cout << "a0r = " << a0r << " +/- " << da0r << std::endl;
      std::cout << "d0r = " << d0r << " +/- " << dd0r << std::endl;
      std::cout << "z0r = " << z0r << " +/- " << dz0r << std::endl;
#endif

      if ( m_h2m ) m_h2m->Fill( phit, d0t );

      m_rd0_vs_phi_rec->Fill( phir, a0r );


      /// fill them all the resplots from a loop ...
      double resfiller[9] = { std::fabs(ipTt), std::fabs(pTt), ET, etat, z0t, double(NvtxCount), double(Nvtxtracks), phit, mu_val };
      
      double Delta_ipt =  1.0/pTr - 1.0/pTt;
      double Delta_pt  =  pTr - pTt;

      if ( pTt<0 ) { 
	Delta_ipt *= -1;
	Delta_pt  *= -1;
      }

      for ( int irfill=0 ; irfill<9 ; irfill++ ) { 
        m_retares[irfill]->Fill( resfiller[irfill],  etar-etat );
        m_rphires[irfill]->Fill( resfiller[irfill],  phir-phit );
        m_rzedres[irfill]->Fill( resfiller[irfill],  z0r-z0t );
        m_rzedthetares[irfill]->Fill( resfiller[irfill],  z0r*std::sin(thetar)-z0t*std::sin(thetat) );
        m_riptres[irfill]->Fill( resfiller[irfill],  Delta_ipt );
        m_rptres[irfill]->Fill(  resfiller[irfill], Delta_pt );
        m_rd0res[irfill]->Fill(  resfiller[irfill],  a0r-a0t );
      }
      
      double lb = gevent->lumi_block();

      m_rzedreslb->Fill( lb,  z0r-z0t );
      m_rzedlb->Fill( lb,  z0t );
      m_rzedlb_rec->Fill( lb,  z0r );

      for ( int irfill=0 ; irfill<6 ; irfill++ ) { 
        m_rphiresPull[irfill]->Fill( resfiller[irfill], (phir - phit) / sqrt( (dphit*dphit) + (dphir*dphir) ) );
        m_retaresPull[irfill]->Fill( resfiller[irfill], (etar - etat) / sqrt( (detat*detat) + (detar*detar) ) );
        m_rptresPull[irfill]->Fill(  resfiller[irfill], Delta_pt / sqrt( (dpTt*dpTt) + (dpTr*dpTr) ) );
        m_rzedresPull[irfill]->Fill( resfiller[irfill], (z0r - z0t) / sqrt( (dz0t*dz0t) + (dz0r*dz0r) ) );
        m_rd0resPull[irfill]->Fill(  resfiller[irfill], (a0r - a0t) / sqrt( (da0t*da0t) + (da0r*da0r) ) );
      }
      
      m_rDz0res[0]->Fill( std::fabs(pTt), dz0r-dz0t );
      m_rDz0res[1]->Fill( etat, dz0r-dz0t );
      m_rDz0res[2]->Fill( z0t, dz0r-dz0t );
      m_rDz0res[3]->Fill( phit, dz0r-dz0t );
      
      if ( dumpflag ) {
	std::ostream& dumpstream = dumpfile; 
        if ( dz0t>0 && std::fabs( dz0r-dz0t )>0.04 ) { 
	  dump = true;
	  dumpstream << "POOR sigma(z0) agreement \n\trefrack:  " << *reftracks[i] << "\n\ttestrack: " << *matchedreco << std::endl; 
	  //	    std::cout << "dz0r dz0t" << dz0r << "\t" << dz0t << std::endl;
	}
      }
      
      /// rDx0res[3] = { vs pt, vs eta, vs zed } 
      m_rDd0res[0]->Fill( std::fabs(pTt), dd0r-dd0t );
      m_rDd0res[1]->Fill( etat, dd0r-dd0t );
      m_rDd0res[2]->Fill( z0t, dd0r-dd0t );
      m_rDd0res[3]->Fill( d0t, dd0r-dd0t );
      m_rDd0res[4]->Fill( phit, dd0r-dd0t );
      
      m_rDa0res[0]->Fill( std::fabs(pTt), da0r-da0t );
      m_rDa0res[1]->Fill( etat, da0r-da0t );
      m_rDa0res[2]->Fill( z0t, da0r-da0t );
      m_rDa0res[3]->Fill( da0t, da0r-da0t );
      
      const double Deltaphi = 2*M_PI/NMod;
	
      double phistart = 11.0819;
      if ( NMod==22 ) phistart = 7.05803;

      double phit_wrap = phit - phistart*M_PI/180;
      
      if ( phit_wrap<-M_PI ) phit_wrap += 2*M_PI;
      
      double iphi = phit_wrap - (Deltaphi*int((phit_wrap+M_PI)/Deltaphi) - M_PI);
      
      //	double iphi = phit - M_PI*int(7*(phit+M_PI)/M_PI)/7 - 11.0819*M_PI/180 + M_PI;
      //	double iphi = phit - M_PI*int(7*phit/M_PI)/7.0 - 11.0819*M_PI/180;
      
      m_rDa0res[4]->Fill( phit, da0r-da0t );
      m_rDa0res[5]->Fill( iphi, da0r-da0t );
      
      m_rDa0res[6]->Fill( iphi, da0t );
      m_rDa0res[7]->Fill( iphi, da0r );

      //ADDED BY JK
      //-----
      /// FIXME: this stuff is all insane with all the indexing with respect to the 
      ///        size of other vectors, so this will need to be tidied up 
      m_rzedres[m_rphires.size()-1]->Fill( resfiller[1], z0r-z0t );
      m_rzedres[m_rphires.size()+1]->Fill( resfiller[1], z0r-z0t );

      m_rzedthetares[m_rphires.size()-1]->Fill( resfiller[1],   z0r*std::sin(thetar)-z0t*std::sin(thetat) );
      m_rzedthetares[m_rphires.size()]->Fill( resfiller[1], z0r*std::sin(thetar)-z0t*std::sin(thetat) );

      m_rd0res[m_rphires.size()]->Fill( resfiller[1], a0r-a0t );
      m_rd0res[m_rphires.size()+1]->Fill( fabs(resfiller[1]), a0r-a0t ); //

      m_rzedresPull[m_rphires.size()-3]->Fill(   resfiller[1],       (z0r - z0t) / std::sqrt( (dz0t*dz0t) + (dz0r*dz0r) ) );
      m_rzedresPull[m_rphires.size()-2]->Fill( fabs(resfiller[1]), (z0r - z0t) / std::sqrt( (dz0t*dz0t) + (dz0r*dz0r) ) );
      m_rd0resPull[m_rphires.size()-3]->Fill(    resfiller[1],       (a0r - a0t) / std::sqrt( (da0t*da0t) + (da0r*da0r) ) );
      m_rd0resPull[m_rphires.size()-2]->Fill(  fabs(resfiller[1]), (a0r - a0t) / std::sqrt( (da0t*da0t) + (da0r*da0r) ) );
      
      //-----
	
      m_rnpix_eta_rec->Fill(  etat, npixr*1.0 );
      m_rnsct_eta_rec->Fill(  etat, nsctr*1.0 );
      m_rntrt_eta_rec->Fill(  etat, nstrawr*1.0 );
      m_rnsihit_eta_rec->Fill(  etat, npixr*0.5 + nsctr*1.0);

      m_rnpix_phi_rec->Fill(  phit, npixr*1.0 );
      m_rnsct_phi_rec->Fill(  phit, nsctr*1.0 );
      m_rntrt_phi_rec->Fill(  phit, nstrawr*1.0 );

      m_rnpix_pt_rec->Fill( std::fabs(pTt), npixr*1.0 );
      m_rnsct_pt_rec->Fill( std::fabs(pTt), nsctr*1.0 );
      m_rntrt_pt_rec->Fill( std::fabs(pTt), nstrawr*1.0 );

      m_rnpixh_pt_rec->Fill( std::fabs(pTt), npixhr*0.5 );
      m_rnscth_pt_rec->Fill( std::fabs(pTt), nscthr*1.0 );

      m_rnpix_d0_rec->Fill( a0t, npixr*1.0 );
      m_rnsct_d0_rec->Fill( a0t, nsctr*1.0 );
      m_rntrt_d0_rec->Fill( a0t, nstrawr*1.0 );

      m_eff_vs_ntracks->Fill( Nvtxtracks );
      m_eff_vs_ntracks2->Fill( Nvtxtracks );
      m_n_vtx_tracks->Fill( Nvtxtracks );

      m_eff_vs_nvtx->Fill( NvtxCount );
      m_n_vtx->Fill( NvtxCount );

      m_eff_vs_mu->Fill( mu_val );

      double vres[6] = { Delta_ipt, etar-etat, phir-phit, z0r-z0t, d0r-d0t, a0r-a0t };
      for ( int it=0 ; it<6 ; it++ ) { 
	if ( it==0 ) { 
	  find("ipT_res")->Fill( vres[0] ); 
	  find("spT_res")->Fill( 1.0/pTr-1.0/pTt ); 
	}
	if ( TH1F* hptr = find(varName[it]+"_res") ) hptr->Fill( vres[it] ); 
	else std::cerr << "hmmm histo " << varName[it]+"_res" << " not found" << std::endl;
      }  
      //2D plot                                                                                                                                                   
      if ( TH2F* hptr = find2D("eta_phi_rec") ) {
        hptr->Fill( etar,phir );
        hptr->GetXaxis()->SetTitle("#eta");
        hptr->GetYaxis()->SetTitle("#phi");
        //hptr->SetFillStyle("COLZ");                                                                                                                             
      }
      if ( TH2F* hptr = find2D("phi_d0_Rec") ) {
        hptr->Fill( phir,d0r );
        hptr->GetXaxis()->SetTitle("#phi");
        hptr->GetYaxis()->SetTitle("d_{0} [mm]");
        //hptr->SetFillStyle("COLZ");                                                                                                                             
      }
    
      // raw matched test track errors                                                                                                                            
      if ( TH1F* hptr = find("dpT_rec") ) hptr->Fill(dpTr);
      if ( TH1F* hptr =find("deta_rec"))  hptr->Fill(detar);
      if ( TH1F* hptr =find("dphi_rec"))  hptr->Fill(dphir);
      if ( TH1F* hptr =find("dz0_rec"))   hptr->Fill(dz0r);
      if ( TH1F* hptr =find("dd0_rec"))   hptr->Fill(dd0r);
      if ( TH1F* hptr =find("da0_rec"))   hptr->Fill(da0r);

      // raw matched reference track errors                                                                                                                       
      if ( TH1F* hptr = find("dpT") ) hptr->Fill(dpTt);
      if ( TH1F* hptr = find("deta")) hptr->Fill(detat);
      if ( TH1F* hptr = find("dphi")) hptr->Fill(dphit);
      if ( TH1F* hptr = find("dz0"))  hptr->Fill(dz0t);
      if ( TH1F* hptr = find("dd0"))  hptr->Fill(dd0t);
      if ( TH1F* hptr = find("da0"))  hptr->Fill(da0t);


      if ( TH1F* hptr = find("dd0_res"))  hptr->Fill(dd0r-dd0t);
      if ( TH1F* hptr = find("da0_res"))  hptr->Fill(da0r-da0t);
      if ( TH1F* hptr = find("dz0_res"))  hptr->Fill(dz0r-dz0t);
      
      double Dd0 = dd0r-dd0t;
      double Da0 = da0r-da0t;
	
      double Ddphi = dphir - dphit;

      m_hphivsDd0res[2]->Fill( phit );

      //      if ( matchedreco->bLayerHits()<=3 ) std::cout << "\nov2\t" << Dd0 << " " << *reftracks[i] << std::endl;
      //      else                                std::cout << "\nov4\t" << Dd0 << " " << *reftracks[i] << std::endl;



      if (  PRINT_BRESIDUALS ) { 
	if ( matchedreco->bLayerHits()<=3 ) std::cout << "\nov2\t" << Dd0 << " " << *matchedreco << std::endl;
	else                                std::cout << "\nov4\t" << Dd0 << " " << *matchedreco << std::endl;
      }

      if ( std::fabs(Dd0)<0.01 ) { 
	m_hphivsDd0res[0]->Fill( phit );

	if ( PRINT_BRESIDUALS ) { 
	  std::cout << "close residual " << Dd0 << " " << Ddphi
		    << " "<< reftracks[i]->bLayerHits()-matchedreco->bLayerHits()
		    << " "<< reftracks[i]->pixelHits()-matchedreco->pixelHits();
	  std::cout << "\nccr\t" << Dd0 << " " << Ddphi << " " << *reftracks[i];
	  std::cout << "\ncct\t" << Dd0 << " " << Ddphi << " " << *matchedreco << std::endl;
	}
      } 
      else { 
	m_hphivsDd0res[1]->Fill( phit );
	if ( PRINT_BRESIDUALS ) { 
	  std::cout << "far   residual " << Dd0 << " " << Ddphi
		    << " "<< reftracks[i]->bLayerHits()-matchedreco->bLayerHits()
		    << " "<< reftracks[i]->pixelHits()-matchedreco->pixelHits();
	  std::cout << "\nffr\t" << Dd0 << " " << Ddphi << " " << *reftracks[i];
	  std::cout << "\nfft\t" << Dd0 << " " << Ddphi << " " << *matchedreco << std::endl;
	}
      }


      m_hphivsDa0res[2]->Fill( iphi );
      if ( std::fabs(Da0)<0.01 ) m_hphivsDa0res[0]->Fill( iphi );
      else                       m_hphivsDa0res[1]->Fill( iphi );
 

      // pull stats                                                                                                                                               
      double pull_pt  = Delta_pt / std::sqrt( (dpTt*dpTt) + (dpTr*dpTr) );
      double pull_eta = (etar - etat) / std::sqrt( (detat*detat) + (detar*detar) );
      double pull_phi = (phir - phit) / std::sqrt( (dphit*dphit) + (dphir*dphir) );
      double pull_z0  = (z0r - z0t) / std::sqrt( (dz0t*dz0t) + (dz0r*dz0r) );
      double pull_d0  = (d0r - d0t) / std::sqrt( (dd0t*dd0t) + (dd0r*dd0r) );
      double pull_a0  = (a0r - a0t) / std::sqrt( (da0t*da0t) + (da0r*da0r) );

      if ( TH1F* hptr = find("pT_pull") ) hptr->Fill(pull_pt);
      if ( TH1F* hptr = find("eta_pull")) hptr->Fill(pull_eta);
      if ( TH1F* hptr = find("phi_pull")) hptr->Fill(pull_phi);
      if ( TH1F* hptr = find("z0_pull")) hptr->Fill(pull_z0);
      if ( TH1F* hptr = find("d0_pull")) hptr->Fill(pull_d0);
      if ( TH1F* hptr = find("a0_pull")) hptr->Fill(pull_a0);

      // pull stats - SIMPLE VERSION                                                                                                                             
      double pull_pt_simp  = Delta_pt / sqrt( dpTr*dpTr );
      double pull_eta_simp = (etar - etat) / sqrt( detar*detar );
      double pull_phi_simp = (phir - phit) / sqrt( dphir*dphir );
      double pull_z0_simp  = (z0r - z0t) / sqrt( dz0r*dz0r );
      double pull_d0_simp  = (d0r - d0t) / sqrt( dd0r*dd0r );
      double pull_a0_simp  = (a0r - a0t) / sqrt( da0r*da0r );

      if ( TH1F* hptr = find("pT_pull_simple") ) hptr->Fill(pull_pt_simp);
      if ( TH1F* hptr = find("eta_pull_simple")) hptr->Fill(pull_eta_simp);
      if ( TH1F* hptr = find("phi_pull_simple")) hptr->Fill(pull_phi_simp);
      if ( TH1F* hptr = find("z0_pull_simple"))  hptr->Fill(pull_z0_simp);
      if ( TH1F* hptr = find("d0_pull_simple"))  hptr->Fill(pull_d0_simp);
      if ( TH1F* hptr = find("a0_pull_simple"))  hptr->Fill(pull_a0_simp);


      if ( TH1F* hptr = find("etai_res") ) hptr->Fill( etat-etar ); 


      double Delphi = phit-phir;
      double Deleta = etat-etar;

      if ( Delphi<-M_PI ) Delphi+=2*M_PI;
      if ( Delphi>M_PI ) Delphi -=2*M_PI;

      double DeltaR = std::sqrt(Delphi*Delphi+Deleta*Deleta);

      m_hDeltaR->Fill(DeltaR);

      m_deltaR_v_eta->Fill(etat, DeltaR);
      m_deltaR_v_pt->Fill(std::fabs(pTt), DeltaR);

      // in this loop over the reference tracks, could fill efficiency 
      // histograms

      //       m_eff_vs_lb->Fill( m_rmap[r]+lb );

      if ( TH1F* hptr = find("nsi_matched"))  hptr->Fill(nsir);

      /// matched track distributions


      m_rChi2prob_rec->Fill( std::fabs(pTr), TMath::Prob(matchedreco->chi2(),matchedreco->dof()) );
      m_rChi2_rec->Fill( std::fabs(pTr), matchedreco->chi2() );
      m_rChi2dof_rec->Fill( std::fabs(pTr), matchedreco->chi2()/matchedreco->dof() );

      m_rChi2d_vs_Chi2d->Fill( reftracks[i]->chi2()/reftracks[i]->dof(),
			     matchedreco->chi2()/matchedreco->dof() );

      m_rDChi2dof->Fill( reftracks[i]->chi2()/reftracks[i]->dof(),
		       (matchedreco->chi2()/matchedreco->dof())-(reftracks[i]->chi2()/reftracks[i]->dof()) );
            
    }
    else {
      
      /// fill the track occupancies etc for the missed tracks

      m_rnpix_pt_bad->Fill( std::fabs(pTt), npixt*0.5 );
      m_rnsct_pt_bad->Fill( std::fabs(pTt), nsctt*1.0 );
      m_rntrt_pt_bad->Fill( std::fabs(pTt), nstrawt*1.0 );
      
      m_rChi2prob_bad->Fill( std::fabs(pTt), TMath::Prob(reftracks[i]->chi2(),reftracks[i]->dof()) );
      m_rChi2_bad->Fill( std::fabs(pTt), reftracks[i]->chi2() );
      m_rChi2dof_bad->Fill( std::fabs(pTt), reftracks[i]->chi2()/reftracks[i]->dof() );
      

      // fill efficiencies with unmatched histos
      //       std::cout << "NULL" << std::endl;
      m_eff_pt->FillDenom(std::fabs(pTt));
      m_eff_z0->FillDenom(z0t);
      m_eff_eta->FillDenom(etat);
      m_eff_phi->FillDenom(phit);
      m_eff_d0->FillDenom(d0t);
      m_eff_a0->FillDenom(a0t);

      // signed pT
      if ( pTt<0 ) m_eff_ptm->FillDenom(std::fabs(pTt));
      else         m_eff_ptp->FillDenom(std::fabs(pTt));

      m_eff_roi_deta->FillDenom(droi_detat);
      m_eff_roi_dphi->FillDenom(droi_dphit);
      m_eff_roi_dR->FillDenom(droi_dRt);

      m_eff_vs_mult->FillDenom( m_Nref );

      m_eff_eta_vs_pt->FillDenom( std::fabs(pTt), etat );
      m_eff_d0_vs_pt->FillDenom( std::fabs(pTt), a0t );

      dump = false; 

      m_eff_vs_ntracks->FillDenom( Nvtxtracks );
      m_eff_vs_ntracks2->FillDenom( Nvtxtracks );
      m_n_vtx_tracks->Fill( Nvtxtracks );


      m_eff_vs_nvtx->FillDenom( NvtxCount );
      m_n_vtx->Fill( NvtxCount );

      double mu_val = gevent->mu();

      m_eff_vs_mu->FillDenom(mu_val);

      if ( tobj ) { 
	m_eff_vs_etovpt->FillDenom(etovpt_val);    
	m_eff_vs_et->FillDenom( std::fabs(tobj->pt()*0.001) );    
      }

      if ( dumpflag ) {
	std::ostream& dumpstream = dumpfile; 
  
	if ( std::fabs(pTt)>1 ) { 
	  dump = true; 

	  hipt = true;
	  dumpstream << m_name << "\tMISSING TRACK run " << r << "\tevent " << ev 
		    << "\tlb " << lb << "\tN vertices " << NvtxCount << std::endl;
	  dumpstream << m_name << "\tMISSING TRACK RoI   " << *groi << std::endl;
	  dumpstream << m_name << "\tMISSING TRACK Track " << *reftracks[i];
	  if ( std::fabs(pTt)>=30 ) dumpstream << "\tvery high pt";
	  if ( std::fabs(pTt)>4 &&
	       std::fabs(pTt)<30  ) dumpstream << "\t     high pt";
	  dumpstream << std::endl;

	  if ( std::fabs(pTt)>=20 ){
	    dumpstream << "Test tracks " << std::endl;
	    for (unsigned int ii=0; ii<testtracks.size(); ii++){
	      dumpstream << *testtracks[ii] << std::endl;
	    }
	  }
	}
      }


      //      m_eff_vs_lb->FillDenom( ts );
      m_eff_vs_lb->FillDenom( gevent->lumi_block() );
    }

  }

  //  return;


  // for fake/purity histograms, loop over the test tracks
  // and get the corresponding matched reference tracks from the 
  // reverse map in the TrackAssociator class  - revmatched() 

  static int icount = 0;

  //  if ( icount%1000 ) std::cout << "chain " << name() << "\t " << m_Nreco << " tracks" << std::endl;
  // if ( icount%1000 ) 
  if ( m_print ) std::cout << "ConfAnalysis::execute() \t " << name() << "\t " << icount << " events\t " << testtracks.size() << " tracks (" << m_Nreco << ")" << "\n---------------" << std::endl;

  icount++;

  for ( int i=testtracks.size() ; i-- ; ) { 

    //    std::cout << "\t\tConfAnalysis purity " << name() << "\t" << i << " " << *testtracks[i] << " -> ";

    //    double pTr  = std::fabs(testtracks[i]->pT()); 
    double pTr     = testtracks[i]->pT()/1000; 
    double etar    = testtracks[i]->eta(); 
    double phir    = testtracks[i]->phi(); 
    double thetar  = 2*std::atan( exp( (-1)*etar) );
    
    double z0r    = testtracks[i]->z0(); //  + ((std::cos(phir)*m_xBeamTest + std::sin(phir)*m_yBeamTest)/std::tan(thetar));
    double d0r    = testtracks[i]->a0() - sin(phir)*m_xBeamTest + cos(phir)*m_yBeamTest; // this will be changed when we know the beam spot position
    double a0r    = testtracks[i]->a0(); 
    //    double a0rp = testtracks[i]->a0() - sin(phir)*m_xBeam - cos(phir)*m_yBeam; // this will be changed when we know the beam spot position

    if ( m_xBeamTest!=0 && m_yBeamTest!=0 ) { 
      d0r  = testtracks[i]->a0(); 
      a0r  = testtracks[i]->a0() + sin(phir)*m_xBeamTest - cos(phir)*m_yBeamTest; // this will be changed when we know the beam spot position
      z0r  = testtracks[i]->z0()+((std::cos(phir)*m_xBeamTest + std::sin(phir)*m_yBeamTest)/std::tan(thetar));    
    }
    

    //    std::cout << "d0 " << d0r << "\tphi " << phir << "\tx " << m_xBeamTest << "\ty " << m_yBeamTest << std::endl;

    double nsctr = testtracks[i]->sctHits(); 
    double npixr = testtracks[i]->pixelHits()*0.5; 
    double nsir = testtracks[i]->pixelHits()*0.5 + testtracks[i]->sctHits(); 

    double ntrtr   = testtracks[i]->trHits(); 
    double nstrawr = testtracks[i]->strawHits(); 


    m_rnsct_vs_npix_rec->Fill( npixr, nsctr );


#if 0
    double dpTr_b  = testtracks[i]->dpT()/1000;
    double detar_b = testtracks[i]->deta();
    double dphir_b = testtracks[i]->dphi();
    double dz0r_b  = testtracks[i]->dz0(); // + ((std::cos(phir)*m_xBeamTest + std::sin(phir)*m_yBeamTest)/std::tan(thetar));
    double dd0r_b  = testtracks[i]->da0() - sin(phir)*m_xBeamTest + cos(phir)*m_yBeamTest;
    double da0r_b  = testtracks[i]->da0(); 


    std::cout << "pTr_b  = " << pTr  << " +/- " << dpTr_b  << std::endl;
    std::cout << "phir_b = " << phir << " +/- " << dphir_b << std::endl;
    std::cout << "z0r_b  = " << z0r  << " +/- " << dz0r_b  << std::endl;
    std::cout << "d0r_b  = " << d0r  << " +/- " << dd0r_b  << std::endl;
    std::cout << "a0r_b  = " << a0r  << " +/- " << da0r_b  << std::endl;
    std::cout << "etar_b = " << etar << " +/- " << detar_b << std::endl;
#endif

    //    double ts_scale = (ts-1260400000)*3000.0/(1260700000-1260400000); 

    //    m_z_vs_lb->Fill( m_rmap[r]+lb, z0r );
    //    m_z_vs_lb->Fill( ts, z0r );
    m_z_vs_lb->Fill( gevent->lumi_block(), z0r );

    //    hnpix_v_sct_rec->Fill( nsctr*0.5, npixr*1.0 );

    if ( m_h2r )   m_h2r->Fill( phir, d0r );
    if ( m_h2a0r ) m_h2a0r->Fill( phir, a0r );

    const TIDA::Track* matchedref = matcher->revmatched(testtracks[i]); 

    //    if ( matchedref )  std::cout << *matchedref << std::endl;
    //    else               std::cout << "NULL" << std::endl;     

#if 1       
    // raw test track distributions 
    double vpart[11] = { std::fabs(pTr), etar, phir, z0r, d0r, a0r, nsctr, npixr, nsir, ntrtr, nstrawr };
    for ( int it=0 ; it<11 ; it++ ) { 
      // std::string hname = name()+"_"+varName[it]+"_rec";
      //      std::string hname = varName[it]+"_rec";
      //      std::map<std::string, TH1F*>::iterator hmitr = m_histos.find(hname);
      //      if ( hmitr!=m_histos.end() )   hmitr->second->Fill( vpar[it] );
      //      else std::cerr << "hmmm histo " << hname << " not found" << std::endl;
      if ( TH1F* hptr = find(varName[it]+"_rec") ) hptr->Fill( vpart[it] ); 
      else std::cerr << "hmmm histo " << varName[it]+"_rec" << " not found" << std::endl;
    }  
    //2D plot
    if ( TH2F* hptr = find2D("eta_phi_rec") ) {
        hptr->Fill( etar,phir );
	hptr->GetXaxis()->SetTitle("#eta");
	hptr->GetYaxis()->SetTitle("#phi");
	//hptr->SetFillStyle("COLZ");
    }
    if ( TH2F* hptr = find2D("phi_d0_rec") ) {
        hptr->Fill( phir,d0r );
	hptr->GetXaxis()->SetTitle("#phi");
	hptr->GetYaxis()->SetTitle("d_{0} [mm]");
	//hptr->SetFillStyle("COLZ");
    }
#endif    


    // purities
    if ( matchedref )  {

      //       std::cout << *matchedref << std::endl;

      m_purity_pt->Fill(std::fabs(pTr));
      m_purity_z0->Fill(z0r);
      m_purity_eta->Fill(etar);
      m_purity_phi->Fill(phir);
      m_purity_d0->Fill(d0r);
      m_purity_a0->Fill(a0r);

      //  hnpix_v_sct_match->Fill( nsctr*0.5, npixr*0.5 );

    }
    else { 
      //       std::cout << "NULL" << std::endl;
      m_purity_pt->FillDenom(std::fabs(pTr));
      m_purity_z0->FillDenom(z0r);
      m_purity_eta->FillDenom(etar);
      m_purity_phi->FillDenom(phir);
      m_purity_d0->FillDenom(d0r);
      m_purity_a0->FillDenom(a0r);
    }

  }

  if ( dump && m_print ) { 

    std::cout << "ConfAnalysis::execute() missed a high pT track - dumping tracks" << std::endl;

    for ( int i=reftracks.size() ; i-- ; ) {

      if ( std::fabs( reftracks[i]->pT() ) > 1000 ) { 
	std::cout << "\t dump " << *reftracks[i];
	const TIDA::Track* matchedreco = matcher->matched(reftracks[i]); 
	if ( matchedreco ) std::cout << " <--> " << *matchedreco << std::endl;
	else               std::cout << std::endl;
      }

    }

    for ( int i=testtracks.size() ; i-- ; ) {     
      const TIDA::Track* matchedref = matcher->revmatched(testtracks[i]); 
      if ( matchedref==0 ) std::cout << "\t\t\t\t\t " << *testtracks[i] << std::endl;      
    }

  }

  if ( m_print ) std::cout << "ConfAnalysis::execute() exiting" << std::endl;

}





