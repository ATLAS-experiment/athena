/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TRTHitAnalysis.h"

// Section of includes for TRT tests
#include "GeoAdaptors/GeoTRTUncompressedHit.h"
#include "TRT_ReadoutGeometry/TRT_DetectorManager.h"
#include "StoreGate/ReadHandle.h"



StatusCode TRTHitAnalysis::initialize() {
  ATH_MSG_DEBUG( "Initializing TRTHitAnalysis" );

  // Grab the Ntuple and histogramming service for the tree
  ATH_CHECK(m_readKey.initialize());
  
  /** Histograms **/
  m_h_TRT_x = new TH1D("h_TRT_x","hits_x", 100,-1100, 1100);
  m_h_TRT_x->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_TRT_x->GetName(), m_h_TRT_x));

  m_h_TRT_y = new TH1D("h_TRT_y", "hits_y", 100,-1100,1100);
  m_h_TRT_y->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_TRT_y->GetName(), m_h_TRT_y));

  m_h_TRT_z = new TH1D("h_TRT_z", "hits_z", 100,-3000,3000);
  m_h_TRT_z->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_TRT_z->GetName(), m_h_TRT_z));

  m_h_TRT_r = new TH1D("h_TRT_r", "hits_r", 100,500,1100);
  m_h_TRT_r->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_TRT_r->GetName(), m_h_TRT_r));

  m_h_TRT_xy = new TH2D("h_TRT_xy", "xy", 100,-1100.,1100.,100, -1100., 1100.);
  m_h_TRT_xy->StatOverflows();
  ATH_CHECK(histSvc()->regHist( m_path+m_h_TRT_xy->GetName(), m_h_TRT_xy));

  m_h_TRT_zr = new TH2D("h_TRT_zr", "zr", 100,-3000,3000,100,500.,1100.);
  m_h_TRT_zr->StatOverflows();
  ATH_CHECK(histSvc()->regHist( m_path+m_h_TRT_zr->GetName(), m_h_TRT_zr));

  m_h_TRT_time_photons = new TH1D("h_TRT_time_photons", "hits_time_photons", 100,0,500);
  m_h_TRT_time_photons->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_TRT_time_photons->GetName(), m_h_TRT_time_photons));

  m_h_TRT_time_nonphotons = new TH1D("h_TRT_time_nonphotons", "hits_time_nonphotons", 100,0,500);
  m_h_TRT_time_nonphotons->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_TRT_time_nonphotons->GetName(), m_h_TRT_time_nonphotons));

  m_h_TRT_edep_photons = new TH1D("h_TRT_edep_photons", "hits_edep_photons", 100,0,500);
  m_h_TRT_edep_photons->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_TRT_edep_photons->GetName(), m_h_TRT_edep_photons));

  m_h_TRT_edep_nonphotons = new TH1D("TRT_edep_nonphotons", "hits_edep_nonphotons", 100,0,500);
  m_h_TRT_edep_nonphotons->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_TRT_edep_nonphotons->GetName(), m_h_TRT_edep_nonphotons));

  m_h_TRT_kine_photons = new TH1D("h_TRT_kine_photons", "hits_kine_photons", 100,0,2);
  m_h_TRT_kine_photons->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_TRT_kine_photons->GetName(), m_h_TRT_kine_photons));

  m_h_TRT_kine_nonphotons = new TH1D("h_TRT_kine_nonphotons", "hits_kine_nonphotons", 100,0,1000);
  m_h_TRT_kine_nonphotons->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_TRT_kine_nonphotons->GetName(), m_h_TRT_kine_nonphotons));

  m_h_TRT_barcode = new TH1D("h_TRT_barcode", "hits_barcode", 100,-500,300000);
  m_h_TRT_barcode->StatOverflows();
  ATH_CHECK(histSvc()->regHist(m_path + m_h_TRT_barcode->GetName(), m_h_TRT_barcode));

  /** now add branches and leaves to the tree */
  m_tree = new TTree("TRT","TRT");
  std::string fullNtupleName =  "/" + m_ntupleFileName + "/";
  ATH_CHECK(histSvc()->regTree(fullNtupleName,m_tree));

  m_tree->Branch("x", &m_TRT_x);
  m_tree->Branch("y", &m_TRT_y);
  m_tree->Branch("z", &m_TRT_z);
  m_tree->Branch("r", &m_TRT_r);
  m_tree->Branch("time_photons", &m_TRT_time_photons);
  m_tree->Branch("time_nonphotons", &m_TRT_time_nonphotons);
  m_tree->Branch("EnergyDeposit_photons", &m_TRT_edep_photons);
  m_tree->Branch("EnergyDeposit_nonphotons", &m_TRT_edep_nonphotons);
  m_tree->Branch("KineticEnergy_photons", &m_TRT_kine_photons);
  m_tree->Branch("KineticEnergy_nonphotons", &m_TRT_kine_nonphotons);
  m_tree->Branch("barcode", &m_TRT_barcode);
  
  ATH_CHECK(detStore()->retrieve(m_detMgr,"TRT"));
  
  return StatusCode::SUCCESS;
}		 


StatusCode TRTHitAnalysis::execute() {
  ATH_MSG_DEBUG( "In TRTHitAnalysis::execute()" );

  m_TRT_x->clear();
  m_TRT_y->clear();
  m_TRT_z->clear();
  m_TRT_r->clear();
  m_TRT_time_photons->clear();
  m_TRT_time_nonphotons->clear();
  m_TRT_edep_photons->clear();
  m_TRT_edep_nonphotons->clear();
  m_TRT_kine_photons->clear();
  m_TRT_kine_nonphotons->clear();
  m_TRT_barcode->clear();
  
  const EventContext& ctx{Gaudi::Hive::currentContext()};
  const TRTUncompressedHitCollection* p_collection{nullptr};
  ATH_CHECK(SG::get(p_collection, m_readKey, ctx));
  for (TRTUncompressedHitConstIter i_hit = p_collection->begin(); i_hit != p_collection->end(); ++i_hit) {
      GeoTRTUncompressedHit ghit(*i_hit);
      HepGeom::Point3D<double> p = ghit.getGlobalPosition(m_detMgr);

      m_h_TRT_x->Fill(p.x());
      m_h_TRT_y->Fill(p.y());
      m_h_TRT_z->Fill(p.z());
      m_h_TRT_r->Fill(p.perp());
      m_h_TRT_xy->Fill(p.x(), p.y());
      m_h_TRT_zr->Fill(p.z(),sqrt(pow(p.x(),2)+pow(p.y(),2)));
      m_h_TRT_barcode->Fill(HepMC::barcode(i_hit->particleLink()));

      m_TRT_x->push_back(p.x());
      m_TRT_y->push_back(p.y());
      m_TRT_z->push_back(p.z());
      m_TRT_r->push_back(p.perp());
      m_TRT_barcode->push_back(HepMC::barcode(i_hit->particleLink()));
      
      int particleId(i_hit->GetParticleEncoding());
      if(particleId == 22 || static_cast<int>(std::abs(particleId)/100000)==41 || static_cast<int>(std::abs(particleId)/10000000)==1) {
	m_h_TRT_time_photons->Fill(i_hit->GetGlobalTime());
	m_h_TRT_edep_photons->Fill(i_hit->GetEnergyDeposit());
	m_h_TRT_kine_photons->Fill(i_hit->GetKineticEnergy());
	m_TRT_time_photons->push_back(i_hit->GetGlobalTime());
	m_TRT_edep_photons->push_back(i_hit->GetEnergyDeposit());
	m_TRT_kine_photons->push_back(i_hit->GetKineticEnergy());
      }
      else {
	m_h_TRT_time_nonphotons->Fill(i_hit->GetGlobalTime());
	m_h_TRT_edep_nonphotons->Fill(i_hit->GetEnergyDeposit());
	m_h_TRT_kine_nonphotons->Fill(i_hit->GetKineticEnergy());	  
	m_TRT_time_nonphotons->push_back(i_hit->GetGlobalTime());
	m_TRT_edep_nonphotons->push_back(i_hit->GetEnergyDeposit());
	m_TRT_kine_nonphotons->push_back(i_hit->GetKineticEnergy());
      }
  } // End while hits
 
  m_tree->Fill();      

  return StatusCode::SUCCESS;
}
