/* emacs: this is -*- c++ -*- */
/**
 **     @file    ConfAnalysis.h
 **
 **     @author  mark sutton
 **     @date    $Id: ConfAnalysis.h 800361 2017-03-12 14:33:19Z 
 **
 **     Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 **/


#ifndef ANALYSIS_CONFANALYSIS_H
#define ANALYSIS_CONFANALYSIS_H

#include <iostream>
#include <vector>
#include <map>

#include "TrigInDetAnalysis/TrackAnalysis.h"
#include "TrigInDetAnalysis/Track.h"
#include "TrigInDetAnalysis/TIDDirectory.h"
#include "TrigInDetAnalysis/Efficiency1D.h"
#include "TrigInDetAnalysis/Efficiency2D.h"
#include "TrigInDetAnalysis/TIDARoiDescriptor.h"
#include "TrigInDetAnalysis/TrigObjectMatcher.h"

#include "TrigInDetAnalysisExample/ChainString.h"
#include "TrigInDetAnalysisUtils/TagNProbe.h"


#include "Resplot.h"


#include "TH1F.h"
#include "TH2F.h"
// #include "Reflex/Reflex.h"

// using namespace ROOT::Reflex;


inline const std::string clean( std::string s) { 
  std::replace(s.begin(), s.end(), ':', '_');
  std::replace(s.begin(), s.end(), ';', '_');
  return s; /// return to keep the compiler quiet
}


class ConfAnalysis : public TrackAnalysis { 
  
  using TrackAnalysis::execute;

public:
  
  ConfAnalysis( const std::string& name, const ChainString& config, TagNProbe* TnP_tool=0 ) : 
    TrackAnalysis( clean(name) ), 
    m_config(config)
    {
    m_hphivsDd0res[0]=0;
    m_hphivsDd0res[1]=0;
    m_hphivsDd0res[2]=0;
    m_hphivsDa0res[0]=0;
    m_hphivsDa0res[1]=0;
    m_hphivsDa0res[2]=0;
    std::cout << "ConfAnalysis::ConfAnalysis() " << TrackAnalysis::name() << " ..." << std::endl;
    setTnPtool( TnP_tool );
  }
  
  ~ConfAnalysis() {
    // std::cout << "ConfAnalysis::~ConfAnalysis() " << name() << std::endl;
    std::map<std::string, TH1F*>::iterator hitr=m_histos.begin();
    std::map<std::string, TH1F*>::iterator hend=m_histos.end();
    for ( ; hitr!=hend ; ++hitr ) delete hitr->second;     
    //2D histograms
    std::map<std::string, TH2F*>::iterator hitr2D=m_histos2D.begin();                                                                                           
    std::map<std::string, TH2F*>::iterator hend2D=m_histos2D.end();
    for ( ; hitr2D!=hend2D ; ++hitr2D ) delete hitr2D->second;     
    // tag and probe object
    if ( m_TnP_tool ) delete m_TnP_tool;
  }  
  
  virtual void initialise();
  virtual void initialiseInternal();

  virtual void execute(const std::vector<TIDA::Track*>& reftracks,
                       const std::vector<TIDA::Track*>& testtracks,
                       TrackAssociator* matcher, 
		       TrigObjectMatcher* objects );

  virtual void execute(const std::vector<TIDA::Track*>& reftracks,
                       const std::vector<TIDA::Track*>& testtracks,
                       TrackAssociator* matcher ) { 
    execute( reftracks, testtracks, matcher, (TrigObjectMatcher*)0 );
  }

  virtual void execute(const std::vector<TIDA::Track*>& reftracks,
                       const std::vector<TIDA::Track*>& testtracks,
                       TrackAssociator* matcher,
		       const TIDA::Event* ) { 
    execute( reftracks, testtracks, matcher, (TrigObjectMatcher*)0 );
  }


#if 0
  virtual void execute(const std::vector<TIDA::Track*>& reftracks,
                       const std::vector<TIDA::Track*>& testtracks,
                       TrackAssociator* matcher, 
		       TIDARoiDescriptor* roi );
#endif

  virtual void finalise();
  
  void setprint(bool p) { m_print=p; }

  void setRoi(const TIDARoiDescriptor* roi) { m_roi = roi; }

  void initialiseFirstEvent(bool b=true) { m_initialiseFirstEvent=b; }

  const ChainString& config() const { return m_config; }

  // methods for tag and probe invariant mass plots
  
  void setTnPtool(TagNProbe* TnP_tool) { m_TnP_tool = TnP_tool; }
  
  virtual TagNProbe* getTnPtool() { return m_TnP_tool; }

  virtual TH1F* getHist_invmass() { return m_invmass; }

  virtual TH1F* getHist_invmassObj() { return m_invmassObj; }

private:
  
  void addHistogram( TH1F* h ) { 
      std::string name = h->GetName();
      m_histos.insert( std::map<std::string, TH1F*>::value_type( name, h) );
  }
  void addHistogram2D( TH2F* h ) {                                                                                                                              
      std::string name = h->GetName();                                                                                                                          
      m_histos2D.insert( std::map<std::string, TH2F*>::value_type( name, h) );                                                                                  
  }                              
  
  TH1F* find( const std::string& n ) { 
      std::map<std::string, TH1F*>::iterator hmitr = m_histos.find(n);
      if ( hmitr!=m_histos.end() ) return hmitr->second;
      else                         return 0; 
  }

  TH2F* find2D( const std::string& n ) {                                                                                                                        
    std::map<std::string, TH2F*>::iterator hmitr = m_histos2D.find(n);                                                                                        
    if ( hmitr!=m_histos2D.end() ) return hmitr->second;                                                                                                      
    else                         return 0;                                                                                                                    
  }              

private:

  ChainString  m_config;

  TIDDirectory* m_dir = 0;

  std::map<std::string, TH1F*> m_histos;
  std::map<std::string, TH2F*> m_histos2D;

  // tag and probe invariant mass histograms
  TH1F* m_invmass = 0;
  TH1F* m_invmassObj = 0;

  // tag and probe object
  TagNProbe* m_TnP_tool = 0;

  Efficiency2D* m_eff_eta_vs_pt = 0;
  Efficiency2D* m_eff_d0_vs_pt  = 0;


  Efficiency1D* m_eff_pt = 0;
  Efficiency1D* m_eff_ptp = 0;
  Efficiency1D* m_eff_ptm = 0;

  Efficiency1D* m_eff_eta = 0;
  Efficiency1D* m_eff_phi = 0;
  Efficiency1D* m_eff_z0 = 0;
  Efficiency1D* m_eff_d0 = 0;
  Efficiency1D* m_eff_a0 = 0;

  Efficiency1D* m_eff_roi_deta = 0;
  Efficiency1D* m_eff_roi_dphi = 0;
  Efficiency1D* m_eff_roi_dR = 0;

  Efficiency1D* m_purity_pt = 0;
  Efficiency1D* m_purity_eta = 0;
  Efficiency1D* m_purity_phi = 0;
  Efficiency1D* m_purity_z0 = 0;
  Efficiency1D* m_purity_d0 = 0;
  Efficiency1D* m_purity_a0 = 0;

#if 0
  TH2F* m_h2;
  TH2F* m_h2m;
  TH2F* m_h2r;
#endif

  Resplot* m_h2 = 0;
  Resplot* m_h2m = 0;
  Resplot* m_h2r = 0;

  Resplot* m_h2a0 = 0;
  Resplot* m_h2a0r = 0;

  Resplot* m_rChi2prob = 0;
  Resplot* m_rChi2 = 0;
  Resplot* m_rChi2dof = 0;

  Resplot* m_rChi2prob_bad = 0;
  Resplot* m_rChi2_bad = 0;
  Resplot* m_rChi2dof_bad = 0;

  Resplot* m_rChi2prob_rec = 0;
  Resplot* m_rChi2_rec = 0;
  Resplot* m_rChi2dof_rec = 0;

  Resplot* m_rChi2d_vs_Chi2d = 0;
  Resplot* m_rDChi2dof = 0;

  //  TH2F* hnpix_v_sct;
  //  TH2F* hnpix_v_sct_rec;
  //  TH2F* hnpix_v_sct_match;

  TH1F* m_hDeltaR = 0;

  /// number of reconstructed tracks 
  int m_Nreco = 0;
  int m_Nref = 0;
  int m_Nmatched = 0;

  Resplot* m_rnpix_eta = 0;
  Resplot* m_rnsct_eta = 0;
  Resplot* m_rntrt_eta = 0;
  Resplot* m_rnsihit_eta = 0;

  Resplot* m_rnpix_lb = 0;
  Resplot* m_rnsct_lb = 0;

  Resplot* m_rnpix_lb_rec = 0;
  Resplot* m_rnsct_lb_rec = 0;

  Resplot* m_rnpix_phi = 0;
  Resplot* m_rnsct_phi = 0;
  Resplot* m_rntrt_phi = 0;

  Resplot* m_rnpix_pt = 0;
  Resplot* m_rnsct_pt = 0;
  Resplot* m_rntrt_pt = 0;

  Resplot* m_rnpix_d0 = 0; // new
  Resplot* m_rnsct_d0 = 0; // new
  Resplot* m_rntrt_d0 = 0; // new

  Resplot* m_rnpix_d0_rec = 0; // new
  Resplot* m_rnsct_d0_rec = 0; // new
  Resplot* m_rntrt_d0_rec = 0; // new

  Resplot* m_rnpixh_pt = 0;
  Resplot* m_rnscth_pt = 0;

  Resplot* m_rnpixh_d0 = 0; // new
  Resplot* m_rnscth_d0 = 0; // new

  Resplot* m_rnsi_pt = 0;  // new
  Resplot* m_rnsih_pt = 0; // new

  Resplot* m_rnsi_eta = 0;  // new
  Resplot* m_rnsih_eta = 0; // new

  Resplot* m_rnsi_d0 = 0;  // new
  Resplot* m_rnsih_d0 = 0; // new

  Resplot* m_rnbl_d0 = 0;  // new
  Resplot* m_rnblh_d0 = 0; // new

  Resplot* m_rnpix_pt_bad = 0;
  Resplot* m_rnsct_pt_bad = 0;
  Resplot* m_rntrt_pt_bad = 0;

  Resplot* m_rnpix_eta_rec = 0;
  Resplot* m_rnsct_eta_rec = 0;
  Resplot* m_rntrt_eta_rec = 0;
  Resplot* m_rnsihit_eta_rec = 0;

  Resplot* m_rnpix_phi_rec = 0;
  Resplot* m_rnsct_phi_rec = 0;
  Resplot* m_rntrt_phi_rec = 0;

  Resplot* m_rnpix_pt_rec = 0;
  Resplot* m_rnsct_pt_rec = 0;
  Resplot* m_rntrt_pt_rec = 0;

  Resplot* m_rnpixh_pt_rec = 0;
  Resplot* m_rnscth_pt_rec = 0;

  std::vector<Resplot*> m_res;

  std::vector<Resplot*> m_retares;
  std::vector<Resplot*> m_rphires;
  std::vector<Resplot*> m_rzedres;
  std::vector<Resplot*> m_rzedthetares;
  std::vector<Resplot*> m_riptres;
  std::vector<Resplot*> m_rptres;
  std::vector<Resplot*> m_rd0res;
  std::vector<Resplot*> m_rDd0res;
  std::vector<Resplot*> m_rDa0res;
  std::vector<Resplot*> m_rDz0res;

  Resplot* m_rzedreslb = 0;

  Resplot* m_rzedlb = 0;
  Resplot* m_rzedlb_rec = 0;

  //  std::vector<Resplot*> rd0res_95;
  //  std::vector<Resplot*> rd0res_rms;

  std::vector<Resplot*> m_retaresPull;
  std::vector<Resplot*> m_rphiresPull;
  std::vector<Resplot*> m_rzedresPull;
  std::vector<Resplot*> m_riptresPull;
  std::vector<Resplot*> m_rptresPull;
  std::vector<Resplot*> m_rd0resPull;

  Resplot* m_deltaR_v_eta = 0;
  Resplot* m_deltaR_v_pt = 0;

  TH1F*  m_hphivsDd0res[3];
  TH1F*  m_hphivsDa0res[3];

  Efficiency1D* m_eff_vs_lb = 0;

  Resplot* m_z_vs_lb = 0;

  std::map<int, int> m_rmap;

  Efficiency1D* m_eff_vs_mult = 0;

  TH1F* m_n_vtx_tracks = 0;
  Efficiency1D* m_eff_vs_ntracks = 0;
  Efficiency1D* m_eff_vs_ntracks2 = 0;

  TH1F* m_n_vtx = 0;
  Efficiency1D* m_eff_vs_nvtx = 0;
  TH1F* m_mu = 0;
  Efficiency1D* m_eff_vs_mu = 0;

  /// beam spot dependent

  Resplot* m_rd0_vs_phi = 0;
  Resplot* m_rd0_vs_phi_rec = 0;

  /// Residuals

  Resplot* m_rRoi_deta_vs_eta = 0;
  Resplot* m_rRoi_dphi_vs_eta = 0;
  Resplot* m_rRoi_dzed_vs_eta = 0;

  /// electron specific ET/PT related stuff
  TH1F* m_etovpt_raw = 0;
  TH1F* m_etovpt = 0;
  Efficiency1D* m_eff_vs_etovpt = 0;

  TH1F* m_et = 0;
  Efficiency1D* m_eff_vs_et = 0;

  /// flag to print out the matched tracks etc
  bool m_print = false;

  const TIDARoiDescriptor* m_roi = 0;

  bool m_initialised = false;
  bool m_initialiseFirstEvent = false;

  Resplot* m_rnsct_vs_npix = 0;
  Resplot* m_rnsct_vs_npix_rec = 0;
};



inline std::ostream& operator<<( std::ostream& s, const ConfAnalysis& ca ) {
  return s << ca.name();
}





#endif  // ANALYSIS_CONFANALYSIS_H 










