/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "CBNTAA_TBScint.h"

#include "LArG4TBSimEvent/LArG4H6FrontHitCollection.h"
#include <fstream>
#include <string>
#include "PathResolver/PathResolver.h"


CBNTAA_TBScint::CBNTAA_TBScint(const std::string & name, ISvcLocator * pSvcLocator) : CBNT_TBRecBase(name, pSvcLocator)
{}

StatusCode CBNTAA_TBScint::CBNT_initialize() {
  ATH_MSG_DEBUG ( "in initialize()"  );

  // Add scint parameters
  addBranch("bm_scint_num", m_scint_num);
  addBranch("bm_scint_en", m_scint_en);
  addBranch("bm_scint_trackid", m_scint_trackid);
  addBranch("bm_scint_pdg", m_scint_pdg);

  return StatusCode::SUCCESS; 
}

StatusCode CBNTAA_TBScint::CBNT_execute() {
  ATH_MSG_DEBUG ( "in execute()" );
  
  double edep;
  int scnum, pcode, trid;
  unsigned i;
  
  // Get scint. hits
  const LArG4H6FrontHitCollection *frontcoll = nullptr;
  const LArG4H6FrontHitCollection *movecoll = nullptr;
  StatusCode sc = evtStore()->retrieve(frontcoll,"Front::Hits");
  if (sc.isSuccess()){
     for (const LArG4H6FrontHit* hit : *frontcoll) {
       scnum = hit->GetSC();
       if(scnum  <= 0) continue; // not a scintilator hit
       edep = hit->GetEdep();
       pcode = hit->GetPcode();
       trid = hit->GetTrackID();
       // Check if we have already such combination
       for(i=0; i<m_scint_num->size(); ++i){
          if( (scnum == (*m_scint_num)[i]) && (pcode == (*m_scint_pdg)[i]) &&
              (trid  == (*m_scint_trackid)[i]) ) { //add energy
             
            (*m_scint_en)[i] += edep;
            break;
          }  
       }
       if(i == m_scint_num->size()) { // new combination
          m_scint_num->push_back(scnum);
          m_scint_pdg->push_back(pcode);
          m_scint_trackid->push_back(trid);
          m_scint_en->push_back(edep);
       }
     }
  } else {
    ATH_MSG_WARNING ( "Retrieval of Front Hits failed..." );
    return StatusCode::FAILURE;
  }

  sc = evtStore()->retrieve(movecoll,"Movable::Hits");
  if (sc.isSuccess()){
     for (const LArG4H6FrontHit* hit : *movecoll) {
       scnum = hit->GetSC();
       if(scnum  <= 0) continue; // not a scintilator hit
       edep = hit->GetEdep();
       pcode = hit->GetPcode();
       trid = hit->GetTrackID();
       // Check if we have already such combination
       for(i=0; i<m_scint_num->size(); ++i){
          if( (scnum == (*m_scint_num)[i]) && (pcode == (*m_scint_pdg)[i]) &&
              (trid  == (*m_scint_trackid)[i]) ) { //add energy
             
            (*m_scint_en)[i] += edep;
            break;
          }  
       }
       if(i == m_scint_num->size()) { // new combination
          m_scint_num->push_back(scnum);
          m_scint_pdg->push_back(pcode);
          m_scint_trackid->push_back(trid);
          m_scint_en->push_back(edep);
       }
     }
  } else {
    ATH_MSG_WARNING ( "Retrieval of Movable Hits failed" );
  }
       
  return StatusCode::SUCCESS;
}

StatusCode CBNTAA_TBScint::CBNT_finalize() {
  ATH_MSG_DEBUG ( "in finalize()" );
  return StatusCode::SUCCESS;
}


StatusCode CBNTAA_TBScint::CBNT_clear()
{
   if(m_scint_num) m_scint_num->clear();
   if(m_scint_en) m_scint_en->clear();
   if(m_scint_trackid) m_scint_trackid->clear();
   if(m_scint_pdg) m_scint_pdg->clear();
  return StatusCode::SUCCESS;
}
