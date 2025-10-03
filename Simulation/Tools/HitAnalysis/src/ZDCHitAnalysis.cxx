/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "ZDCHitAnalysis.h"


#include "ZDC_SimEvent/ZDC_SimFiberHit.h"
#include "CaloSimEvent/CaloCalibrationHit.h"
#include "StoreGate/ReadHandle.h"

StatusCode ZDCHitAnalysis::initialize() {
  ATH_MSG_DEBUG( "Initializing ZDCHitAnalysis" );

  ATH_CHECK(detStore()->retrieve(m_ZdcID));
  ATH_CHECK(m_readKey.initialize());
  ATH_CHECK(m_readCalibKey.initialize());
 
  for(int side : {0,1}){
    for(int module = 0; module < 5; module++){
      std::string name = Form("%s%d", (side==1) ? "a" : "c", module);
      m_h_zdc_photons[side][module] = new TH1I( ("m_edep_module_" + name).c_str(), ("edep_module_" + name).c_str(), 100, 0, 20000);
      ATH_CHECK(histSvc()->regHist(m_path + m_h_zdc_photons[side][module]->GetName(), m_h_zdc_photons[side][module]));
    }
  }

  for(int side : {0,1}){
    for(int module = 0; module < 5; module++){
      std::string name = Form("%s%d", (side==1) ? "a" : "c", module);
      m_h_zdc_calibTot[side][module] = new TH1D( ("m_calibTot_module_" + name).c_str(), ("calibTot_module_" + name).c_str(), 100, 1e-2, 1e7);
      m_h_zdc_calibEM[side][module] = new TH1D( ("m_calibEM_module_" + name).c_str(), ("calibEM_module_" + name).c_str(), 100, 1e-2, 1e7);
      m_h_zdc_calibNonEM[side][module] = new TH1D( ("m_calibNonEM_module_" + name).c_str(), ("calibNonEM_module_" + name).c_str(), 100, 1e-2, 1e7);
      
      ATH_CHECK(histSvc()->regHist(m_path + m_h_zdc_calibTot[side][module]->GetName(), m_h_zdc_calibTot[side][module]));
      ATH_CHECK(histSvc()->regHist(m_path + m_h_zdc_calibEM[side][module]->GetName(), m_h_zdc_calibEM[side][module]));
      ATH_CHECK(histSvc()->regHist(m_path + m_h_zdc_calibNonEM[side][module]->GetName(), m_h_zdc_calibNonEM[side][module]));
    }
  }
 
  /** now add branches and leaves to the tree */
  m_tree = new TTree("ZDC","ZDC");
  std::string fullNtupleName =  "/" + m_ntupleFileName + "/";
  ATH_CHECK(histSvc()->regTree(fullNtupleName,m_tree));
  
  m_tree->Branch("fiber_side", &m_zdc_fiber_side);
  m_tree->Branch("fiber_mod", &m_zdc_fiber_mod);
  m_tree->Branch("fiber_channel", &m_zdc_fiber_channel);
  m_tree->Branch("fiber_nphotons", &m_zdc_fiber_photons);

  m_tree->Branch("calib_side", &m_zdc_calib_side);
  m_tree->Branch("calib_mod", &m_zdc_calib_mod);
  m_tree->Branch("calib_channel", &m_zdc_calib_channel);
  m_tree->Branch("calib_total", &m_zdc_calib_Total);
  m_tree->Branch("calib_em", &m_zdc_calib_EM);
  m_tree->Branch("calib_nonem", &m_zdc_calib_NonEM);

 
  return StatusCode::SUCCESS;
}


StatusCode ZDCHitAnalysis::execute() {
  ATH_MSG_DEBUG( "In ZDCHitAnalysis::execute()" );
  
  m_zdc_fiber_side->clear();
  m_zdc_fiber_mod->clear();
  m_zdc_fiber_channel->clear();
  m_zdc_fiber_photons->clear();

  double photons_fiber = -1;
  int side_fiber = -1;
  int mod_fiber = -1;
  int channel_fiber = -1;

  ZDC_SimFiberHit_ConstIterator fiberhi;
  const EventContext& ctx{Gaudi::Hive::currentContext()};
  const ZDC_SimFiberHit_Collection* fiberiter{nullptr};
  ATH_CHECK(SG::get(fiberiter, m_readKey, ctx));
  for (fiberhi=(*fiberiter).begin(); fiberhi != (*fiberiter).end(); ++fiberhi) {
    ZDC_SimFiberHit ghit(*fiberhi);
    Identifier id = ghit.getID();
    photons_fiber = ghit.getNPhotons();
    side_fiber = (m_ZdcID->side(id)==-1) ? 0 : 1;
    mod_fiber = m_ZdcID->module(id);
    channel_fiber = m_ZdcID->channel(id);

    m_h_zdc_photons[side_fiber][mod_fiber]->Fill(photons_fiber);

    m_zdc_fiber_side->push_back(side_fiber);
    m_zdc_fiber_mod->push_back(mod_fiber);
    m_zdc_fiber_channel->push_back(channel_fiber);
    m_zdc_fiber_photons->push_back(photons_fiber);
  }


  m_zdc_calib_side->clear();
  m_zdc_calib_mod->clear();
  m_zdc_calib_channel->clear();
  m_zdc_calib_Total->clear();
  m_zdc_calib_EM->clear();
  m_zdc_calib_NonEM->clear();

  int side_calib = -1;
  int mod_calib = -1;
  int channel_calib = -1;
  float calib_eTot = -999.;
  float calib_eEM = -999.;
  float calib_eNonEM = -999.;
  
  const CaloCalibrationHitContainer* calibiter{nullptr};
  ATH_CHECK(SG::get(calibiter, m_readCalibKey, ctx));
  for (auto hit : *calibiter) {
    Identifier id = hit->cellID();
    side_calib = (m_ZdcID->side(id)==-1) ? 0 : 1;
    mod_calib = m_ZdcID->module(id);
    channel_calib = m_ZdcID->channel(id);
    calib_eTot = hit->energyTotal();
    calib_eEM = hit->energyEM();
    calib_eNonEM = hit->energyNonEM();

    m_h_zdc_calibTot[side_calib][mod_calib]->Fill(calib_eTot);
    m_h_zdc_calibEM[side_calib][mod_calib]->Fill(calib_eEM);
    m_h_zdc_calibNonEM[side_calib][mod_calib]->Fill(calib_eNonEM);

    m_zdc_calib_side->push_back(side_calib);
    m_zdc_calib_mod->push_back(mod_calib);
    m_zdc_calib_channel->push_back(channel_calib);
    m_zdc_calib_Total->push_back(calib_eTot);
    m_zdc_calib_EM->push_back(calib_eEM);
    m_zdc_calib_NonEM->push_back(calib_eNonEM);
  }

  if (m_tree) m_tree->Fill();
  
  return StatusCode::SUCCESS;
}
