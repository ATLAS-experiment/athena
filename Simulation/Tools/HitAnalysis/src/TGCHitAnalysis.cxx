/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TGCHitAnalysis.h"

// Section of includes for TGC of the Muon Spectrometer tests
#include "GeoAdaptors/GeoMuonHits.h"

#include "MuonSimEvent/TGCSimHit.h"
#include "CLHEP/Vector/LorentzVector.h"

#include "TH1.h"
#include "TTree.h"
#include "TString.h"

#include <algorithm>
#include <math.h>
#include <functional>
#include <iostream>
#include <stdio.h>

StatusCode TGCHitAnalysis::initialize() {
  ATH_MSG_DEBUG( "Initializing TGCHitAnalysis" );

  // Grab the Ntuple and histogramming service for the tree
  ATH_CHECK(m_readKey.initialize());
  
  /** Histograms**/
  m_h_hits_x = new TH1D("h_hits_tgc_x","hits_x", 100,-5000, 5000);
  m_h_hits_x->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_hits_x->GetName(), m_h_hits_x));

  m_h_hits_y = new TH1D("h_hits_tgc_y", "hits_y", 100,-5000,5000);
  m_h_hits_y->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_hits_y->GetName(), m_h_hits_y));

  m_h_hits_z = new TH1D("h_hits_tgc_z", "hits_z", 100,-12000,12000);
  m_h_hits_z->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_hits_z->GetName(), m_h_hits_z));

  m_h_hits_r = new TH1D("h_hits_tgc_r", "hits_r", 100,2000,10000);
  m_h_hits_r->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_hits_r->GetName(), m_h_hits_r));

  m_h_xy = new TH2D("h_tgc_xy", "xy", 100,-5000.,5000.,100, -5000., 5000.);
  m_h_xy->StatOverflows();
  ATH_CHECK(histSvc()->regHist( m_path+m_h_xy->GetName(), m_h_xy));

  m_h_rz = new TH2D("h_tgc_rz", "rz", 100,2000.,10000.,100, -12000., 12000.);
  m_h_rz->StatOverflows();
  ATH_CHECK(histSvc()->regHist( m_path+m_h_rz->GetName(), m_h_rz));

  m_h_hits_eta = new TH1D("h_hits_tgc_eta", "hits_eta", 100,-10.0,10.0);
  m_h_hits_eta->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_hits_eta->GetName(), m_h_hits_eta));

  m_h_hits_phi = new TH1D("h_hits_tgc_phi", "hits_phi", 100,-3.2,3.2);
  m_h_hits_phi->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_hits_phi->GetName(), m_h_hits_phi));

  m_h_hits_lx = new TH1D("h_hits_tgc_lx","hits_lx", 100,-800, 800);
  m_h_hits_lx->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_hits_lx->GetName(), m_h_hits_lx));

  m_h_hits_ly = new TH1D("h_hits_tgc_ly", "hits_ly", 100,-800,800);
  m_h_hits_ly->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_hits_ly->GetName(), m_h_hits_ly));

  m_h_hits_lz = new TH1D("h_hits_tgc_lz", "hits_lz", 100,-800,800);
  m_h_hits_lz->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_hits_lz->GetName(), m_h_hits_lz));

  m_h_hits_dcx = new TH1D("h_hits_tgc_dcx","hits_dcx", 100,-1, 1);
  m_h_hits_dcx->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_hits_dcx->GetName(), m_h_hits_dcx));

  m_h_hits_dcy = new TH1D("h_hits_tgc_dcy", "hits_dcy", 100,-1,1);
  m_h_hits_dcy->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_hits_dcy->GetName(), m_h_hits_dcy));

  m_h_hits_dcz = new TH1D("h_hits_tgc_dcz", "hits_dcz", 100,-1,1);
  m_h_hits_dcz->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_hits_dcz->GetName(), m_h_hits_dcz));

  m_h_hits_time = new TH1D("h_hits_tgc_time","hits_time", 100,0, 250);
  m_h_hits_time->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_hits_time->GetName(), m_h_hits_time));

  m_h_hits_edep = new TH1D("h_hits_tgc_edep", "hits_edep", 100,0,0.5);
  m_h_hits_edep->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_hits_edep->GetName(), m_h_hits_edep));

  m_h_hits_kine = new TH1D("h_hits_tgc_kine", "hits_kine", 100,0,1000);
  m_h_hits_kine->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_hits_kine->GetName(), m_h_hits_kine));

  m_h_hits_step = new TH1D("h_hits_tgc_step", "hits_step", 100,0,50);
  m_h_hits_step->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_hits_step->GetName(), m_h_hits_step));

  
  return StatusCode::SUCCESS;
}		 


StatusCode TGCHitAnalysis::execute() {
  ATH_MSG_DEBUG( "In TGCHitAnalysis::execute()" );

  const EventContext& ctx{Gaudi::Hive::currentContext()};
  const TGCSimHitCollection* tgc_container{nullptr};
  ATH_CHECK(SG::get(tgc_container, m_readKey, ctx));
  for (TGCSimHitCollection::const_iterator i_hit = tgc_container->begin(); i_hit != tgc_container->end(); ++i_hit) {
      //TGCSimHitCollection::const_iterator i_hit;
      //for(auto i_hit : *tgc_container){
      GeoTGCHit ghit(*i_hit);
      if (!ghit) continue;

      Amg::Vector3D p = ghit.getGlobalPosition();
      m_h_hits_x->Fill(p.x());
      m_h_hits_y->Fill(p.y());
      m_h_hits_z->Fill(p.z());
      m_h_hits_r->Fill(p.perp());
      m_h_xy->Fill(p.x(), p.y());
      m_h_rz->Fill(p.perp(), p.z());
      m_h_hits_eta->Fill(p.eta());
      m_h_hits_phi->Fill(p.phi());
      m_h_hits_lx->Fill((*i_hit).localPosition().x());
      m_h_hits_ly->Fill((*i_hit).localPosition().y());
      m_h_hits_lz->Fill((*i_hit).localPosition().z());
      m_h_hits_dcx->Fill((*i_hit).localDireCos().x());
      m_h_hits_dcy->Fill((*i_hit).localDireCos().y());
      m_h_hits_dcz->Fill((*i_hit).localDireCos().z());
      m_h_hits_edep->Fill((*i_hit).energyDeposit());
      m_h_hits_time->Fill((*i_hit).globalTime());
      m_h_hits_step->Fill((*i_hit).stepLength());
      m_h_hits_kine->Fill((*i_hit).kineticEnergy());
    
    }
 
  return StatusCode::SUCCESS; 
}
