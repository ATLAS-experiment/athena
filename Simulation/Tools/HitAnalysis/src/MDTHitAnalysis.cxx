/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MDTHitAnalysis.h"

// Section of includes for MDT of the Muon Spectrometer tests
#include "GeoAdaptors/GeoMuonHits.h"

#include "MuonSimEvent/MDTSimHitCollection.h"
#include "MuonSimEvent/MDTSimHit.h"
#include "CLHEP/Vector/LorentzVector.h"

#include "TH1.h"

#include <algorithm>
#include <math.h>
#include <functional>
#include <iostream>
#include <stdio.h>


StatusCode MDTHitAnalysis::initialize() {
  ATH_MSG_DEBUG( "Initializing MDTHitAnalysis" );
  ATH_CHECK(m_readKey.initialize());

   
  /** Histograms */
  m_h_hits_x = new TH1D("h_hits_mdt_x","hits_x", 100,-25000, 25000);
  m_h_hits_x->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_hits_x->GetName(), m_h_hits_x));

  m_h_hits_y = new TH1D("h_hits_mdt_y", "hits_y", 100,-25000,25000);
  m_h_hits_y->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_hits_y->GetName(), m_h_hits_y));

  m_h_hits_z = new TH1D("h_hits_mdt_z", "hits_z", 100,-45000,45000);
  m_h_hits_z->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_hits_z->GetName(), m_h_hits_z));

  m_h_hits_r = new TH1D("h_hits_mdt_r", "hits_r", 100,4000,26000);
  m_h_hits_r->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_hits_r->GetName(), m_h_hits_r));

  m_h_xy = new TH2D("h_mdt_xy", "xy", 100,-25000.,25000.,100, -25000., 25000.);
  m_h_xy->StatOverflows();
  ATH_CHECK(histSvc()->regHist( m_path+m_h_xy->GetName(), m_h_xy));

  m_h_zr = new TH2D("h_mdt_zr", "zr", 100,-45000.,45000.,100, 4000., 26000.);
  m_h_zr->StatOverflows();
  ATH_CHECK(histSvc()->regHist( m_path+m_h_zr->GetName(), m_h_zr));

  m_h_hits_eta = new TH1D("h_hits_mdt_eta", "hits_eta", 100,-3.0,3.0);
  m_h_hits_eta->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_hits_eta->GetName(), m_h_hits_eta));

  m_h_hits_phi = new TH1D("h_hits_mdt_phi", "hits_phi", 100,-3.2,3.2);
  m_h_hits_phi->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_hits_phi->GetName(), m_h_hits_phi));

  m_h_hits_lx = new TH1D("h_hits_mdt_lx","hits_lx", 100,-20, 20);
  m_h_hits_lx->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_hits_lx->GetName(), m_h_hits_lx));

  m_h_hits_ly = new TH1D("h_hits_mdt_ly", "hits_ly", 100,-20,20);
  m_h_hits_ly->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_hits_ly->GetName(), m_h_hits_ly));

  m_h_hits_lz = new TH1D("h_hits_mdt_lz", "hits_lz", 100,-2000,2000);
  m_h_hits_lz->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_hits_lz->GetName(), m_h_hits_lz));

  m_h_hits_driftR = new TH1D("h_hits_mdt_driftR", "hits_driftR", 100,0,15);
  m_h_hits_driftR->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_hits_driftR->GetName(), m_h_hits_driftR));

  m_h_hits_time = new TH1D("h_hits_mdt_time","hits_time", 100,0, 150);
  m_h_hits_time->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_hits_time->GetName(), m_h_hits_time));

  m_h_hits_edep = new TH1D("h_hits_mdt_edep", "hits_edep", 100,0,0.2);
  m_h_hits_edep->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_hits_edep->GetName(), m_h_hits_edep));

  m_h_hits_kine = new TH1D("h_hits_mdt_kine", "hits_kine", 100,0,20000);
  m_h_hits_kine->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_hits_kine->GetName(), m_h_hits_kine));

  m_h_hits_step = new TH1D("h_hits_mdt_step", "hits_step", 100,0,100);
  m_h_hits_step->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_hits_step->GetName(), m_h_hits_step));

  
  return StatusCode::SUCCESS;
}


StatusCode MDTHitAnalysis::execute() {
  ATH_MSG_DEBUG( "In MDTHitAnalysis::execute()" );

  const EventContext& ctx{Gaudi::Hive::currentContext()};
  const MDTSimHitCollection* mdt_container{nullptr};
  ATH_CHECK(SG::get(mdt_container, m_readKey, ctx));
  for (MDTSimHitCollection::const_iterator i_hit = mdt_container->begin(); i_hit != mdt_container->end(); ++i_hit) {
      //MDTSimHitCollection::const_iterator i_hit;
      //for(auto i_hit : *mdt_container){
      GeoMDTHit ghit(*i_hit);
      if (!ghit) continue;

      Amg::Vector3D p = ghit.getGlobalPosition();
      m_h_hits_x->Fill(p.x());
      m_h_hits_y->Fill(p.y());
      m_h_hits_z->Fill(p.z());
      m_h_hits_r->Fill(p.perp());
      m_h_xy->Fill(p.x(), p.y());
      m_h_zr->Fill(p.z(),p.perp());
      m_h_hits_eta->Fill(p.eta());
      m_h_hits_phi->Fill(p.phi());
      m_h_hits_driftR->Fill((*i_hit).driftRadius());
      m_h_hits_lx->Fill( (*i_hit).localPosition().x());
      m_h_hits_ly->Fill( (*i_hit).localPosition().y());
      m_h_hits_lz->Fill( (*i_hit).localPosition().z());
      m_h_hits_edep->Fill((*i_hit).energyDeposit());
      m_h_hits_time->Fill((*i_hit).globalTime());
      m_h_hits_step->Fill((*i_hit).stepLength());
      m_h_hits_kine->Fill((*i_hit).kineticEnergy());
      
  }
  
  return StatusCode::SUCCESS;
}
