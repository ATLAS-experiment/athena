/* emacs: this is -*- c++ -*- */
/**
 **     @file    ConfVtxAnalysis.h
 **
 **     @author  mark sutton
 **     @date    Sun  9 Aug 2015 00:02:23 CEST 
 **
 **     Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **/


#ifndef  CONFVTXANALYSIS_H
#define  CONFVTXANALYSIS_H

#include <iostream>
#include <memory>

#include "TrigInDetAnalysis/VertexAnalysis.h"
#include "TrigInDetAnalysis/TIDAVertex.h"
#include "TrigInDetAnalysis/TIDDirectory.h"
#include "TrigInDetAnalysis/Efficiency1D.h"

#include "Resplot.h"

class ConfVtxAnalysis : public VertexAnalysis {

public:

  ConfVtxAnalysis( const std::string& n, bool use_secVtx_limits=false );

  void initialise();

  void execute(const std::vector<TIDA::Vertex*>& vtx0,
	      const std::vector<TIDA::Vertex*>& vtx1,
	      const TIDA::Event* tevt=0 );

  void finalise();

private:

  template<typename Matcher>
  void execute_internal(const std::vector<TIDA::Vertex*>& vtx0,
	      const std::vector<TIDA::Vertex*>& vtx1,
              Matcher& m, 
	      const TIDA::Event* tevt=0 );

  bool m_initialised;
  bool m_finalised;

  bool m_use_secVtx_limits;

  std::unique_ptr<TIDDirectory> m_dir;

  TH1F*    m_hnvtx = 0;
  TH1F*    m_hzed = 0;
  TH1F*    m_hx = 0;
  TH1F*    m_hy = 0;
  TH1F*    m_hntrax = 0;
  TH1F*    m_hmu = 0;
  TH1F*    m_hlb = 0;
  TH1F*    m_hr = 0;

  TH1F*    m_hnvtx_rec = 0;
  TH1F*    m_hzed_rec = 0;
  TH1F*    m_hx_rec   = 0;
  TH1F*    m_hy_rec   = 0;
  TH1F*    m_hntrax_rec = 0;
  TH1F*    m_hr_rec = 0;

  TH1F*    m_hzed_res = 0;
  TH1F*    m_hx_res = 0;
  TH1F*    m_hy_res = 0;

  TH1F*    m_h_dntrax = 0;

  Resplot* m_rdntrax_vs_zed = 0;
  Resplot* m_rdntrax_vs_r = 0;
  Resplot* m_rdntrax_vs_ntrax = 0;

  Resplot* m_rdz_vs_zed = 0;
  Resplot* m_rdz_vs_ntrax = 0;
  Resplot* m_rdz_vs_r = 0;
  Resplot* m_rdz_vs_nvtx = 0;
  Resplot* m_rdz_vs_mu = 0;

  Resplot* m_rdr_vs_zed = 0;
  Resplot* m_rdr_vs_ntrax = 0;
  Resplot* m_rdr_vs_r = 0;

  Resplot* m_rnvtxrec_nvtx = 0;

  Efficiency1D* m_eff_zed = 0;
  Efficiency1D* m_eff_x = 0;
  Efficiency1D* m_eff_y = 0;
  Efficiency1D* m_eff_ntrax = 0;
  Efficiency1D* m_eff_nvtx = 0;
  Efficiency1D* m_eff_mu = 0;
  Efficiency1D* m_eff_lb = 0;
  Efficiency1D* m_eff_r = 0;

  Resplot* m_rdx_vs_lb = 0;
  Resplot* m_rdy_vs_lb = 0;
  Resplot* m_rdz_vs_lb = 0;
 
  //  Contour<Efficiency1D>* eff_zed_vs_ntrax;

};


inline std::ostream& operator<<( std::ostream& s, const ConfVtxAnalysis&  ) { 
  return s;
}


#endif  // CONFVTXANALYSIS_H 










