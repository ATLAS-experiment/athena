/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "HIEventUtils/ExtractCaloGeoConstants.h"

ExtractCaloGeoConstants::ExtractCaloGeoConstants(const std::string& name, ISvcLocator* pSvcLocator) : AthAlgorithm(name, pSvcLocator)
{
}

StatusCode ExtractCaloGeoConstants::initialize()
{
  ATH_CHECK(m_tower_container_key.initialize());
  ATH_CHECK(m_cell_container_key.initialize());
  
  CHECK( m_thistSvc.retrieve() );

  m_h3_w=new TH3F("h3_w","; #it{#eta}; #it{phi}; Sampling",100,-5,5,64,-TMath::Pi(),TMath::Pi(),24,-0.5,23.5);
  m_h3_eta=new TH3F("h3_eta","; #it{#eta}; #it{phi}; Sampling",100,-5,5,64,-TMath::Pi(),TMath::Pi(),24,-0.5,23.5);
  m_h3_phi=new TH3F("h3_phi","; #it{#eta}; #it{phi}; Sampling",100,-5,5,64,-TMath::Pi(),TMath::Pi(),24,-0.5,23.5);
  m_h3_R=new TH3F("h3_R","; #it{#eta}; #it{phi}; Sampling",100,-5,5,64,-TMath::Pi(),TMath::Pi(),24,-0.5,23.5);
  m_h1_events=new TH1F("h1_events","; Processed Events",1,0,1);
  CHECK(m_thistSvc->regHist("/" + m_hist_stream + "/" + m_h3_w->GetName(), m_h3_w));
  CHECK(m_thistSvc->regHist("/" + m_hist_stream + "/" + m_h3_eta->GetName(), m_h3_eta));
  CHECK(m_thistSvc->regHist("/" + m_hist_stream + "/" + m_h3_phi->GetName(), m_h3_phi));
  CHECK(m_thistSvc->regHist("/" + m_hist_stream + "/" + m_h3_R->GetName(), m_h3_R));
  CHECK(m_thistSvc->regHist("/" + m_hist_stream + "/" + m_h1_events->GetName(), m_h1_events));

  return StatusCode::SUCCESS;
}

StatusCode ExtractCaloGeoConstants::execute()
{
  // retrieve the tower container
  SG::ReadHandle<CaloTowerContainer> navInColl(m_tower_container_key);
  if (!navInColl.isValid()) {
    ATH_MSG_ERROR("Could not find CaloTowerContainer " << m_tower_container_key);
    return(StatusCode::FAILURE);
  }

  // retrieve cell container
  SG::ReadHandle<CaloCellContainer> cellColl(m_cell_container_key);
  if (!cellColl.isValid()) {
    ATH_MSG_ERROR("Could not find CaloCellContainer " << m_cell_container_key);
    return(StatusCode::FAILURE);
  }

  // record number of processed events
  m_h1_events->Fill(0.5);

  // loop on towers
  for(auto towerItr : *navInColl)
  {
    // navigate back to cells
    // Default is to sort the cells by either pointer values leading to irreproducible output
    // CaloCellIDFcn ensures cells are ordered by their IDs
    NavigationToken<CaloCell,double,CaloCellIDFcn> cellToken;
    towerItr->fillToken(cellToken,double(1.));
    if ( cellToken.size() == 0 ) continue;
    for(NavigationToken<CaloCell,double,CaloCellIDFcn>::const_iterator cellItr = cellToken.begin();
	cellItr != cellToken.end(); ++cellItr )
    {      
      double geoWeight = cellToken.getParameter(*cellItr); 
      int layer = (*cellItr)->caloDDE()->getSampling();
      double cell_x=(*cellItr)->caloDDE()->x();
      double cell_y=(*cellItr)->caloDDE()->y();
      double cell_z=(*cellItr)->caloDDE()->z();
      double cell_r2=cell_x*cell_x+cell_y*cell_y+cell_z*cell_z;
      float deta=(*cellItr)->caloDDE()->deta();
      float dphi=(*cellItr)->caloDDE()->dphi();
      float area= std::abs(deta*dphi)*geoWeight;
      m_h3_w->Fill(towerItr->eta(),towerItr->phi(),layer,area);
      m_h3_eta->Fill(towerItr->eta(),towerItr->phi(),layer,area*(*cellItr)->eta());
      m_h3_phi->Fill(towerItr->eta(),towerItr->phi(),layer,area*(*cellItr)->phi());
      m_h3_R->Fill(towerItr->eta(),towerItr->phi(),layer,area*area*cell_r2);

    }//end cell loop
  }//end tower loop
  return StatusCode::SUCCESS;
}

StatusCode ExtractCaloGeoConstants::finalize()
{
  return StatusCode::SUCCESS;
}


