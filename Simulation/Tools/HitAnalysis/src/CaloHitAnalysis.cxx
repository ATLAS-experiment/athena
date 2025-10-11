/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Base class
#include "CaloHitAnalysis.h"

// Section of includes for LAr calo tests
#include "CaloDetDescr/CaloDetDescrElement.h"
#include "CaloDetDescr/CaloDetDescrManager.h"

// Section of includes for tile calo tests
#include "TileDetDescr/TileDetDescrManager.h"
#include "CaloIdentifier/TileID.h"
#include "TileSimEvent/TileHit.h"

//Section of includes for Calibrated Calo hits
#include "GeoAdaptors/GeoCaloCalibHit.h"

#include "TString.h"
#include  "TH1.h"
#include  "TH2.h"
#include  "TTree.h"

#include <algorithm>
#include <math.h>
#include <functional>
#include <iostream>



StatusCode CaloHitAnalysis::initialize() {
  ATH_MSG_DEBUG( "Initializing CaloHitAnalysis" );
  if (m_useTile) {

     ATH_CHECK( detStore()->retrieve(m_tileMgr) );
     ATH_CHECK( detStore()->retrieve(m_tileID) );
  }
  ATH_CHECK(m_tileKey.initialize(m_useTile));
  ATH_CHECK(m_caloMgrKey.initialize());
  ATH_CHECK(m_caloKeys.initialize(m_useLAr));
  ATH_CHECK(m_caloCalibKeys.initialize(m_calib && m_useLAr));
  

  m_h_cell_e = new TH1D("h_Calo_cell_e", "cell_e", 100,0.,500.);
  m_h_cell_e->StatOverflows();
  ATH_CHECK(histSvc()->regHist( m_path+m_h_cell_e->GetName(), m_h_cell_e));

  m_h_cell_eta = new TH1D("h_Calo_cell_eta", "cell_eta", 50,-5.,5.);
  m_h_cell_eta->StatOverflows();
  ATH_CHECK(histSvc()->regHist( m_path+m_h_cell_eta->GetName(), m_h_cell_eta));

  m_h_cell_phi = new TH1D("h_Calo_cell_phi", "cell_phi", 50,-3.1416,3.1416);
  m_h_cell_phi->StatOverflows();
  ATH_CHECK(histSvc()->regHist( m_path+m_h_cell_phi->GetName(), m_h_cell_phi));

  m_h_cell_radius = new TH1D("h_Calo_cell_radius", "cell_radius", 100, 0., 6000.);
  m_h_cell_radius->StatOverflows();
  ATH_CHECK(histSvc()->regHist( m_path+m_h_cell_radius->GetName(), m_h_cell_radius));

  m_h_cell_layer = new TH1D("h_Calo_cell_layer", "cell_layer", 24, -0.5, 23.5);
  m_h_cell_layer->StatOverflows();
  CHECK(m_thistSvc->regHist( m_path+m_h_cell_layer->GetName(), m_h_cell_layer));

  m_h_cell_eta_Eweight = new TH1D("h_Calo_cell_eta_Eweight", "cell_eta_Eweight", 50,-5.,5.);
  m_h_cell_eta_Eweight->StatOverflows();
  CHECK(m_thistSvc->regHist( m_path+m_h_cell_eta_Eweight->GetName(), m_h_cell_eta_Eweight));

  m_h_cell_phi_Eweight = new TH1D("h_Calo_cell_phi_Eweight", "cell_phi_Eweight", 50,-3.1416,3.1416);
  m_h_cell_phi_Eweight->StatOverflows();
  CHECK(m_thistSvc->regHist( m_path+m_h_cell_phi_Eweight->GetName(), m_h_cell_phi_Eweight));

  m_h_cell_radius_Eweight = new TH1D("h_Calo_cell_radius_Eweight", "cell_radius_Eweight", 100, 0., 6000.);
  m_h_cell_radius_Eweight->StatOverflows();
  CHECK(m_thistSvc->regHist( m_path+m_h_cell_radius_Eweight->GetName(), m_h_cell_radius_Eweight));

  m_h_cell_layer_Eweight = new TH1D("h_Calo_cell_layer_Eweight", "cell_layer_Eweight", 24, -0.5, 23.5);
  m_h_cell_layer_Eweight->StatOverflows();
  CHECK(m_thistSvc->regHist( m_path+m_h_cell_layer_Eweight->GetName(), m_h_cell_layer_Eweight));

  m_h_xy = new TH2F("h_Calo_xy", "xy", 100,-4000,4000,100, -4000, 4000);
  m_h_xy->StatOverflows();
  ATH_CHECK(histSvc()->regHist( m_path+m_h_xy->GetName(), m_h_xy));

  m_h_zr = new TH2D("h_Calo_zr", "zr", 100,-7000.,7000.,100, 0., 6000.);
  m_h_zr->StatOverflows();
  ATH_CHECK(histSvc()->regHist( m_path+m_h_zr->GetName(), m_h_zr));

  m_h_etaphi = new TH2D("h_Calo_etaphi", "eta_phi", 50,-5.,5.,50, -3.1416, 3.1416);
  m_h_etaphi->StatOverflows();
  ATH_CHECK(histSvc()->regHist( m_path+m_h_etaphi->GetName(), m_h_etaphi));

  //These histograms will be filled only if expert mode is set on
  m_h_time_e = new TH2D("h_Calo_time_e", "energy vs time", 100, 0,50, 100,0,500);
  m_h_time_e->StatOverflows();

  m_h_eta_e = new TH2D("h_Calo_eta_e", "energy vs eta", 50, -5,5, 100,0,500);
  m_h_eta_e->StatOverflows();

  m_h_phi_e = new TH2D("h_Calo_phi_e", "energy vs phi", 50, -3.1416,3.1416, 100,0,500);
  m_h_phi_e->StatOverflows();

  m_h_r_e = new TH2D("h_Calo_r_e", "energy vs radius", 100, 0,6000, 100,0,500);
  m_h_r_e->StatOverflows();

  if (m_expert) {
    ATH_CHECK(histSvc()->regHist(m_path + m_h_time_e->GetName(), m_h_time_e));
    ATH_CHECK(histSvc()->regHist(m_path + m_h_eta_e->GetName(), m_h_eta_e));
    ATH_CHECK(histSvc()->regHist(m_path + m_h_phi_e->GetName(), m_h_phi_e));
    ATH_CHECK(histSvc()->regHist(m_path + m_h_r_e->GetName(), m_h_r_e));
  }

  //Histograms for calibrated hits
  m_h_calib_eta = new TH1D("h_calib_eta", "calib. hits eta", 50,-5,5);
  m_h_calib_eta->StatOverflows();

  m_h_calib_phi = new TH1D("h_calib_phi", "calib. hits phi", 50,-3.1416,3.1416);
  m_h_calib_phi->StatOverflows();

  m_h_calib_rz = new TH2D("h_calib_rz", "calib. hits r vs z", 100,-7000,7000,1000, 0,6000);
  m_h_calib_rz->StatOverflows();

  m_h_calib_etaphi = new TH2D("h_calib_etaphi", "calib. hits eta vs phi",50,-5.,5., 50,-3.1416,3.1416);
  m_h_calib_etaphi->StatOverflows();

  m_h_calib_eEM = new TH1D("h_calib_eEM", "calib. hits EM energy", 100,0,100);
  m_h_calib_eEM->StatOverflows();

  m_h_calib_eNonEM = new TH1D("h_calib_nonEM", "calib. hits non EM energy", 100,0,100);
  m_h_calib_eNonEM->StatOverflows();

  m_h_calib_eInv = new TH1D("h_calib_eInv", "calib. hits invisible energy", 100,0,100);
  m_h_calib_eInv->StatOverflows();

  m_h_calib_eEsc = new TH1D("h_calib_eEsc", "calib. hits escaped energy", 100,0,100);
  m_h_calib_eEsc->StatOverflows();

  m_h_calib_eTot = new TH1D("h_calib_eTot", "calib. hits energy", 100,0,100);
  m_h_calib_eTot->StatOverflows();

  m_h_calib_eTotpartID = new TH1D("h_calib_eTotpartID", "calib. hits partID weighted with energy",600,0,300000);
  m_h_calib_eTotpartID->StatOverflows();

  if (m_calib) {
    ATH_CHECK(histSvc()->regHist(m_path + m_h_calib_eta->GetName(), m_h_calib_eta));
    ATH_CHECK(histSvc()->regHist(m_path + m_h_calib_phi->GetName(), m_h_calib_phi));
    ATH_CHECK(histSvc()->regHist(m_path + m_h_calib_rz->GetName(), m_h_calib_rz));
    ATH_CHECK(histSvc()->regHist(m_path + m_h_calib_etaphi->GetName(), m_h_calib_etaphi));
    ATH_CHECK(histSvc()->regHist(m_path + m_h_calib_eEM->GetName(), m_h_calib_eEM));
    ATH_CHECK(histSvc()->regHist(m_path + m_h_calib_eNonEM->GetName(), m_h_calib_eNonEM));
    ATH_CHECK(histSvc()->regHist(m_path + m_h_calib_eInv->GetName(), m_h_calib_eInv));
    ATH_CHECK(histSvc()->regHist(m_path + m_h_calib_eEsc->GetName(), m_h_calib_eEsc));
    ATH_CHECK(histSvc()->regHist(m_path + m_h_calib_eTot->GetName(), m_h_calib_eTot));
    ATH_CHECK(histSvc()->regHist(m_path + m_h_calib_eTotpartID->GetName(), m_h_calib_eTotpartID));
  }

  /** now add branches and leaves to the tree */
  m_tree = new TTree("Calo", "Calo");
  std::string fullNtupleName =  "/" + m_ntupleFileName + "/";
  ATH_CHECK( histSvc()->regTree(fullNtupleName, m_tree) );

  m_tree->Branch("CellEta", &m_cell_eta);
  m_tree->Branch("CellPhi", &m_cell_phi);
  m_tree->Branch("CellX", &m_cell_x);
  m_tree->Branch("CellY", &m_cell_y);
  m_tree->Branch("CellZ", &m_cell_z);
  m_tree->Branch("CellE", &m_cell_e);
  m_tree->Branch("CellRadius", &m_cell_radius);
  m_tree->Branch("Time", &m_time);
  m_tree->Branch("CalibEta", &m_calib_eta);
  m_tree->Branch("CalibPhi", &m_calib_phi);
  m_tree->Branch("CalibRadius", &m_calib_radius);
  m_tree->Branch("CalibZ", &m_calib_z);
  m_tree->Branch("Calib_eEM", &m_calib_eEM);
  m_tree->Branch("Calib_eNonEM", &m_calib_eNonEM);
  m_tree->Branch("Calib_eInv", &m_calib_eInv);
  m_tree->Branch("Calib_eEsc", &m_calib_eEsc);
  m_tree->Branch("Calib_eTot", &m_calib_eTot);
  m_tree->Branch("Calib_partID", &m_calib_partID);

  return StatusCode::SUCCESS;
}


StatusCode CaloHitAnalysis::execute() {
  ATH_MSG_DEBUG( "In CaloHitAnalysis::execute()" );

  m_cell_eta->clear();
  m_cell_phi->clear();
  m_cell_e->clear();
  m_cell_x->clear();
  m_cell_y->clear();
  m_cell_z->clear();
  m_cell_radius->clear();
  m_cell_layer->clear();
  m_time->clear();
  m_calib_eta->clear();
  m_calib_phi->clear();
  m_calib_radius->clear();
  m_calib_z->clear();
  m_calib_eEM->clear();
  m_calib_eNonEM->clear();
  m_calib_eInv->clear();
  m_calib_eEsc->clear();
  m_calib_eTot->clear();
  m_calib_partID->clear();

  const EventContext& ctx{Gaudi::Hive::currentContext()};
  const TileHitVector* hitVec{nullptr};
  ATH_CHECK(SG::get(hitVec, m_tileKey, ctx));

  if (hitVec) {
      for (const auto& i_hit : *hitVec) {
        Identifier pmt_id = (i_hit).identify();
        Identifier cell_id = m_tileID->cell_id(pmt_id);
        const CaloDetDescrElement* ddElement = (m_tileID->is_tile_aux(cell_id)) ? 0 : m_tileMgr->get_cell_element(cell_id);
        if (ddElement) {
          double tot_e = 0.;
          double tot_time = 0.;
          for (int t=0; t<(i_hit).size(); ++t) tot_e += (i_hit).energy(t);
          for (int t=0; t<(i_hit).size(); ++t) tot_time += (i_hit).time(t);
          m_h_cell_e->Fill(tot_e);
          m_h_cell_eta->Fill(ddElement->eta());
          m_h_cell_phi->Fill(ddElement->phi()) ;
          m_h_cell_radius->Fill(ddElement->z());
          m_h_cell_layer->Fill(ddElement->getSampling());
          m_h_cell_eta_Eweight->Fill(ddElement->eta(),tot_e);
          m_h_cell_phi_Eweight->Fill(ddElement->phi(),tot_e) ;
          m_h_cell_radius_Eweight->Fill(ddElement->z(),tot_e);
          m_h_cell_layer_Eweight->Fill(ddElement->getSampling(),tot_e);
          m_h_xy->Fill(ddElement->x(), ddElement->y());
          m_h_zr->Fill(ddElement->r(), ddElement->r());
          m_h_etaphi->Fill(ddElement->eta(), ddElement->phi());

          if (m_expert) {
            m_h_time_e->Fill(tot_time, tot_e);
            m_h_eta_e->Fill(ddElement->eta(), tot_e);
            m_h_phi_e->Fill(ddElement->phi(), tot_e);
            m_h_r_e->Fill(ddElement->r(), tot_e);
          }
          m_cell_eta->push_back(ddElement->eta());
          m_cell_phi->push_back(ddElement->phi());
          m_cell_e->push_back(tot_e);
          m_cell_x->push_back(ddElement->x());
          m_cell_y->push_back(ddElement->y());
          m_cell_z->push_back(ddElement->z());
          m_cell_radius->push_back(ddElement->r());
          m_cell_layer->push_back(ddElement->getSampling());
          m_time->push_back(tot_time);
        }
      }
  } // DoTile

  if (m_useLAr) {
    
    const CaloDetDescrManager* caloMgr{nullptr};
    ATH_CHECK(SG::get(caloMgr, m_caloMgrKey, ctx));
 
    for (const auto& key : m_caloKeys) {
      const LArHitContainer* iter{nullptr};
      ATH_CHECK(SG::get(iter, key, ctx));
        for (auto hi : *iter ) {
          const CaloDetDescrElement *hitElement = caloMgr->get_element(hi->cellID());
          double energy = hi->energy();
          double time = hi->time();
          double eta = hitElement->eta();
          double phi = hitElement->phi();
          double radius = hitElement->r();
          int layer = hitElement->getSampling();
          float x = hitElement->x();
          float y = hitElement->y();
          double z = hitElement->z();

          m_h_cell_e->Fill( energy );
          m_h_cell_eta->Fill( eta );
          m_h_cell_phi->Fill( phi );
          m_h_cell_radius->Fill( radius );
          m_h_cell_layer->Fill( layer );
          m_h_cell_eta_Eweight->Fill( eta , energy );
          m_h_cell_phi_Eweight->Fill( phi , energy );
          m_h_cell_radius_Eweight->Fill( radius , energy );
          m_h_cell_layer_Eweight->Fill( layer , energy );
          m_h_xy->Fill(x,y);
          m_h_zr->Fill(z,radius);
          m_h_etaphi->Fill(eta, phi);
          if (m_expert) {
            m_h_time_e->Fill(time, energy);
            m_h_eta_e->Fill(eta, energy);
            m_h_phi_e->Fill(phi, energy);
            m_h_r_e->Fill(radius, energy);
          }
          m_cell_eta->push_back(eta);
          m_cell_phi->push_back(phi);
          m_cell_e->push_back(energy);
          m_cell_x->push_back(x);
          m_cell_y->push_back(y);
          m_cell_z->push_back(z);
          m_cell_radius->push_back(radius);
          m_cell_layer->push_back(layer);
          m_time->push_back(time);
        } // End while hits
    } // End detector type loop

    //For calibrated hits
    for (const auto& calibKey : m_caloCalibKeys){
      const CaloCalibrationHitContainer* iterator{nullptr};
      ATH_CHECK(SG::get(iterator,calibKey, ctx));
        //Not tested
        for (auto hit_i : *iterator) {
          GeoCaloCalibHit geoHit(*hit_i, calibKey.key(), caloMgr);
          if (!geoHit) continue;
          const CaloDetDescrElement* Element = geoHit.getDetDescrElement();
          double eta = Element->eta();
          double phi = Element->phi();
          double radius = Element->r();
          double z = Element->z();
          double emEnergy = geoHit.energyEM();
          double nonEmEnergy = geoHit.energyNonEM();
          double invEnergy = geoHit.energyInvisible();
          double escEnergy = geoHit.energyEscaped();
          double totEnergy = geoHit.energyTotal();
          double particleID = HepMC::barcode(*hit_i);

          m_h_calib_eta->Fill(eta);
          m_h_calib_phi->Fill(phi);
          m_h_calib_rz->Fill(z,radius);
          m_h_calib_etaphi->Fill(eta,phi);
          m_h_calib_eEM->Fill(emEnergy);
          m_h_calib_eNonEM->Fill(nonEmEnergy);
          m_h_calib_eInv->Fill(invEnergy);
          m_h_calib_eEsc->Fill(escEnergy);
          m_h_calib_eTot->Fill(totEnergy);
          m_h_calib_eTotpartID->Fill(particleID, totEnergy);

          m_calib_eta->push_back(eta);
          m_calib_phi->push_back(phi);
          m_calib_radius->push_back(radius);
          m_calib_z->push_back(z);
          m_calib_eEM->push_back(emEnergy);
          m_calib_eNonEM->push_back(nonEmEnergy);
          m_calib_eInv->push_back(invEnergy);
          m_calib_eEsc->push_back(escEnergy);
          m_calib_eTot->push_back(totEnergy);
          m_calib_partID->push_back(particleID);
        }
    }
  } // DoLAr


  if (m_tree) m_tree->Fill();

  return StatusCode::SUCCESS;
}
