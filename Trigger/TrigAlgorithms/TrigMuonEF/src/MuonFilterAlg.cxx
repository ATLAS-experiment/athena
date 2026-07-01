/*
  Filter algorithms for muon trigger
  
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonFilterAlg.h"

MuonFilterAlg::MuonFilterAlg(const std::string& name, ISvcLocator* pSvcLocator )
:AthReentrantAlgorithm(name, pSvcLocator)
{
}

StatusCode MuonFilterAlg::initialize(){

  ATH_CHECK(m_muonContainerKey.initialize());
  return StatusCode::SUCCESS;
}

StatusCode MuonFilterAlg::execute(const EventContext& ctx) const
{

  bool pass = false;
  //Get muon container
  SG::ReadHandle<xAOD::MuonContainer> rh_muons(m_muonContainerKey, ctx);
  if(!rh_muons.isValid()){
    ATH_MSG_ERROR("Could not find muons with name: "<<m_muonContainerKey.key());
    return StatusCode::FAILURE;
  }

  //if we find no muons, pass
  int nCBmuons=0;
  for(auto mu : *rh_muons){
    if(mu->muonType()== xAOD::Muon::MuonType::Combined &&
       mu->author() != xAOD::Muon::Author::STACO) {
        nCBmuons++; //count only combined muons
      }
  }
  if(nCBmuons==0) pass = true;

  ATH_MSG_DEBUG("Found: "<<nCBmuons<<" muons; pass="<<pass);
  setFilterPassed(pass, ctx);

  return StatusCode::SUCCESS;
}
