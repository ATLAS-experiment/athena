/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "RPCHitAnalysis.h"

// Section of includes for the RPC of the Muon Spectrometer tests
#include "GeoAdaptors/GeoMuonHits.h"

#include "MuonSimEvent/RPCSimHit.h"
#include "CLHEP/Vector/LorentzVector.h"




StatusCode RPCHitAnalysis::initialize() {
  ATH_MSG_DEBUG( "Initializing RPCHitAnalysis" );

  // Grab the Ntuple and histogramming service for the tree
  ATH_CHECK(m_readKey.initialize());
  /** Histograms**/
  m_h_hits_x = new TH1D("h_hits_rpc_x","hits_x", 100,-11000, 11000);
  m_h_hits_x->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_hits_x->GetName(), m_h_hits_x));

  m_h_hits_y = new TH1D("h_hits_rpc_y", "hits_y", 100,-11000,11000);
  m_h_hits_y->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_hits_y->GetName(), m_h_hits_y));

  m_h_hits_z = new TH1D("h_hits_rpc_z", "hits_z", 100,-12500, 12500);
  m_h_hits_z->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_hits_z->GetName(), m_h_hits_z));

  m_h_hits_r = new TH1D("h_hits_rpc_r", "hits_r", 100,6000,14000);
  m_h_hits_r->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_hits_r->GetName(), m_h_hits_r));

  m_h_xy = new TH2D("h_rpc_xy", "xy", 100,-11000.,11000.,100, -11000., 11000.);
  m_h_xy->StatOverflows();
  ATH_CHECK(histSvc()->regHist( m_path+m_h_xy->GetName(), m_h_xy));

  m_h_zr = new TH2D("m_rpc_zr", "zr", 100,-12500.,12500.,100, 6000., 14000.);
  m_h_zr->StatOverflows();
  ATH_CHECK(histSvc()->regHist( m_path+m_h_zr->GetName(), m_h_zr));

  m_h_hits_eta = new TH1D("h_hits_rpc_eta", "hits_eta", 100,-1.5,1.5);
  m_h_hits_eta->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_hits_eta->GetName(), m_h_hits_eta));

  m_h_hits_phi = new TH1D("h_hits_rpc_phi", "hits_phi", 100,-3.2,3.2);
  m_h_hits_phi->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_hits_phi->GetName(), m_h_hits_phi));

  m_h_hits_lx = new TH1D("h_hits_rpc_lx","hits_lx", 100,-10, 10);
  m_h_hits_lx->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_hits_lx->GetName(), m_h_hits_lx));

  m_h_hits_ly = new TH1D("h_hits_rpc_ly", "hits_ly", 100,-1500,1500);
  m_h_hits_ly->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_hits_ly->GetName(), m_h_hits_ly));

  m_h_hits_lz = new TH1D("h_hits_rpc_lz", "hits_lz", 100,-600,600);
  m_h_hits_lz->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_hits_lz->GetName(), m_h_hits_lz));

  m_h_hits_time = new TH1D("h_hits_rpc_time","hits_time", 100,0, 120);
  m_h_hits_time->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_hits_time->GetName(), m_h_hits_time));

  m_h_hits_edep = new TH1D("h_hits_rpc_edep", "hits_edep", 100,0,0.15);
  m_h_hits_edep->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_hits_edep->GetName(), m_h_hits_edep));

  m_h_hits_kine = new TH1D("h_hits_rpc_kine", "hits_kine", 100,0,500);
  m_h_hits_kine->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_hits_kine->GetName(), m_h_hits_kine));

  m_h_hits_step = new TH1D("h_hits_rpc_step", "hits_step", 100,0,25);
  m_h_hits_step->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_hits_step->GetName(), m_h_hits_step));
  
  return StatusCode::SUCCESS;
}


StatusCode RPCHitAnalysis::execute() {
  ATH_MSG_DEBUG( "In RPCHitAnalysis::execute()" );

  const EventContext& ctx{Gaudi::Hive::currentContext()};
  const RPCSimHitCollection* rpc_container{nullptr};
  ATH_CHECK(SG::get(rpc_container, m_readKey, ctx));
  for (RPCSimHitCollection::const_iterator i_hit = rpc_container->begin(); i_hit != rpc_container->end(); ++i_hit) {
      //RPCSimHitCollection::const_iterator i_hit;
      //for(auto i_hit : *rpc_container){
      GeoRPCHit ghit(*i_hit);
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
      m_h_hits_lx->Fill((*i_hit).localPosition().x());
      m_h_hits_ly->Fill((*i_hit).localPosition().y());
      m_h_hits_lz->Fill((*i_hit).localPosition().z());
      m_h_hits_edep->Fill((*i_hit).energyDeposit());
      m_h_hits_time->Fill((*i_hit).globalTime());
      m_h_hits_step->Fill((*i_hit).stepLength());
      m_h_hits_kine->Fill((*i_hit).kineticEnergy());
    }
  
  return StatusCode::SUCCESS;
}
