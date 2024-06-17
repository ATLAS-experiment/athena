/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "CosTriggerTimeAlg.h"

StatusCode CosTriggerTimeAlg::initialize(){
  ATH_CHECK(m_larHitKeys.initialize());
  ATH_CHECK(m_timeKey.initialize());
  return StatusCode::SUCCESS;  
} 


StatusCode CosTriggerTimeAlg::execute(const EventContext& ctx) const {

  double te = 0; 
  double e = 0; 
  int n=0; 

  SG::WriteHandle<CosTrigTime> out(m_timeKey,ctx);
  
  for (auto& key : m_larHitKeys) {
    SG::ReadHandle<LArHitContainer> hits(key,ctx);
    if (!hits.isValid()) return StatusCode::FAILURE;

    for ( const LArHit* hit : *hits) {
      e += hit->energy(); 
      te += hit->energy() * hit->time() ; 
      ++n; 
    } 
  } 

  double t=0;
  if (n==0) {
    ATH_MSG_INFO( "no LArHit in this event" ); 
  } 
  if (e==0) { 
    ATH_MSG_INFO( "no LArHit energy in this event" ); 
  }
  else { 
    t = te/e;  
    ATH_MSG_DEBUG( "average time from LArHit =  " <<t ); 
  }
  auto cosTime=std::make_unique<CosTrigTime>(t);
  ATH_CHECK(out.record(std::move(cosTime)));
  
  return StatusCode::SUCCESS;
}
