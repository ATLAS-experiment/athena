/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/********************************************************************
NAME:     MissingEtCaloCnv_p3.cxx
PURPOSE:  Transient/Persisten converter for MissingEtCalo class
********************************************************************/

// AthenaPoolCnvSvc includes
#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"

// MissingETEvent includes
#include "MissingETEvent/MissingET.h"
#include "MissingETEvent/MissingEtCalo.h"

// RecTPCnv includes
#include "RecTPCnv/MissingEtCaloCnv_p3.h"
#include "RecTPCnv/MissingETCnv_p3.h"
#include <bit>
#include <cstdint>

// MissingET converter
static const MissingETCnv_p3 metCnv;


void MissingEtCaloCnv_p3::persToTrans(  const MissingEtCalo_p3* pers, MissingEtCalo* trans,  MsgStream& /* msg */ ) const {
  std::vector<float>::const_iterator i=pers->m_allTheData.begin();	
  //float version;
  //version = (*i); 
  ++i; // as there is just one it is not used.
  static_assert(sizeof(float) == sizeof(std::uint32_t));
  int size= static_cast<int>(*i);  ++i;
  for (int vi=0;vi<size;++vi){
    auto ci = static_cast<MissingEtCalo::CaloIndex>(vi);
    trans->setExCalo(ci, *i); ++i;
    trans->setEyCalo(ci, *i); ++i;
    trans->setEtSumCalo(ci, *i); ++i;
    trans->setNCellCalo(ci, std::bit_cast<std::uint32_t>(*i)); 
    ++i;
  }
  if ( i != pers->m_allTheData.end()) {
    metCnv.persToTrans(trans, i);
  }

return;
}

void MissingEtCaloCnv_p3::transToPers(  const MissingEtCalo* trans, MissingEtCalo_p3* pers, MsgStream& /* msg */ )const {	
  static_assert(sizeof(float) == sizeof(std::uint32_t));
  pers->m_allTheData.push_back(3.0); // storing version number
  pers->m_allTheData.reserve(MissingEtCalo::Size * 4 + 1);
  pers->m_allTheData.push_back(MissingEtCalo::Size);
  for (int vi=0;vi<MissingEtCalo::Size;++vi){        
      auto ci = static_cast<MissingEtCalo::CaloIndex>(vi);
    pers->m_allTheData.push_back(trans->exCalo(ci));
    pers->m_allTheData.push_back(trans->eyCalo(ci));
    pers->m_allTheData.push_back(trans->etSumCalo(ci));	
    pers->m_allTheData.push_back(std::bit_cast<float>(trans->ncellCalo(ci)));
  }
  if( trans->getSource() >= 0 && trans->getSource() < 1000 ) 
    metCnv.transToPers( trans, pers->m_allTheData );
  return;
}
