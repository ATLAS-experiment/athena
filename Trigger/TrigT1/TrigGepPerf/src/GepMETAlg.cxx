/*
 *   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 */

#include "./GepMETAlg.h"
#include <xAODTrigger/EnergySumRoIAuxInfo.h>

GepMETAlg::GepMETAlg( const std::string& name, ISvcLocator* pSvcLocator ) :
  AthReentrantAlgorithm( name, pSvcLocator ){
}


GepMETAlg::~GepMETAlg() {}


StatusCode GepMETAlg::initialize() {
  ATH_MSG_INFO ("Initializing " << name() << "...");
  CHECK(m_caloClustersKey.initialize());
  CHECK(m_outputMETKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode GepMETAlg::finalize() {
  ATH_MSG_INFO ("Finalizing " << name() << "...");
  return StatusCode::SUCCESS;
}

StatusCode GepMETAlg::execute(const EventContext& context) const {
  ATH_MSG_DEBUG ("Executing " << name() << "...");
  setFilterPassed(false, context);

  	
  // read in clusters
  auto h_caloClusters = SG::makeHandle(m_caloClustersKey, context);
  CHECK(h_caloClusters.isValid());
  ATH_MSG_DEBUG("Read in " << h_caloClusters->size() << " clusters");

  const auto& caloClusters = *h_caloClusters;
  
  float Ex = 0.;
  float Ey = 0.;
  float totalEt =0.; 

  for ( const auto cluster : caloClusters ) {
    float et = cluster->et();
    float phi = cluster->phi();

    Ex -= et * TMath::Cos(phi);
    Ey -= et * TMath::Sin(phi);
    totalEt += et; 
  }

  ATH_MSG_DEBUG( "Calculated MET Ex,Ey: " << Ex << "," << Ey); 

  // write out the MET object
  auto h_outputMET = SG::makeHandle(m_outputMETKey, context);

  auto METObj = std::make_unique<xAOD::EnergySumRoI>();
  METObj->setStore(new xAOD::EnergySumRoIAuxInfo());
  METObj->setEnergyX(Ex);
  METObj->setEnergyY(Ey);
  METObj->setEnergyT(totalEt);
    
  h_outputMET = std::move(METObj);
 
  setFilterPassed(true, context);
  ATH_MSG_DEBUG("No of MET objects: 1");
  
  return StatusCode::SUCCESS; 
}


