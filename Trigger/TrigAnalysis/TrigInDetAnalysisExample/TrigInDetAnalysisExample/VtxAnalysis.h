/* emacs: this is -*- c++ -*- */
/**
 **     @file    VtxAnalysis.h
 **
 **     @author  mark sutton
 **     @date    Sun  9 Aug 2015 00:02:23 CEST 
 **
 **     Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **/


#ifndef  VTXANALYSIS_H
#define  VTXANALYSIS_H

#include <iostream>
#include <memory>
#include <vector>

#include "TrigInDetAnalysis/VertexAnalysis.h"
#include "TrigInDetAnalysis/TIDDirectory.h"

#include "TH1F.h"
#include "TProfile.h"

class VtxAnalysis : public VertexAnalysis {

public:

  VtxAnalysis( const std::string& n );

  void initialise();

  void execute(const std::vector<TIDA::Vertex*>& vtx0,
	       const std::vector<TIDA::Vertex*>& vtx1, 
	       const TIDA::Event* te=0);
  
  void finalise();

private:

  bool m_initialised;

  std::unique_ptr<TIDDirectory> m_dir;

  TH1F*    m_hnvtx = nullptr;
  TH1F*    m_hzed = nullptr;
  TH1F*    m_hntrax = nullptr;

  TH1F*    m_hnvtx_rec = nullptr;
  TH1F*    m_hzed_rec = nullptr;
  TH1F*    m_hntrax_rec = nullptr;

  TH1F*    m_hzed_res = nullptr;

  TProfile* m_rdz_vs_zed = nullptr;
  TProfile* m_rdz_vs_ntrax = nullptr;
  TProfile* m_rdz_vs_nvtx = nullptr;
  //No currently used 
  //but retained in case
  //TProfile* m_rdz_vs_mu;

  TProfile* m_eff_zed = nullptr;
  TProfile* m_eff_ntrax = nullptr;
  TProfile* m_eff_nvtx = nullptr;
  TProfile* m_eff_mu = nullptr;
  TProfile* m_eff_lb = nullptr;
 
};


inline std::ostream& operator<<( std::ostream& s, const VtxAnalysis&  ) { 
  return s;
}


#endif  // VTXANALYSIS_H 










