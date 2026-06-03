/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/********************************************************************
NAME:     MissingEtCaloCnv_p2.cxx
PURPOSE:  Transient/Persisten converter for MissingEtCalo class
********************************************************************/

// AthenaPoolCnvSvc includes
#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"

// MissingETEvent includes
#include "MissingETEvent/MissingET.h"

// RecTPCnv includes
#include "RecTPCnv/MissingEtCaloCnv_p2.h"
#include "RecTPCnv/MissingETCnv_p2.h"
#include <vector>
#include <bit>
#include <cstdint>

// MissingET converter
static const MissingETCnv_p2 metCnv;


void MissingEtCaloCnv_p2::persToTrans(  const MissingEtCalo_p2* pers, MissingEtCalo* trans,  MsgStream& /* msg */ ) const {
  std::vector<float>::const_iterator i=pers->m_AllTheData.begin();
  int size=(int)(*i);++i;
  trans->setExCaloVec (std::vector<double> (i, i+size));  i+= size;
  trans->setEyCaloVec (std::vector<double> (i, i+size));  i+= size;
  trans->setEtSumCaloVec (std::vector<double> (i, i+size));  i+= size;
  static_assert(sizeof(float) == sizeof(std::uint32_t));
  for (int w = 0; w < size; ++w) {
    const auto ncell = std::bit_cast<std::uint32_t>(*i);
    trans->setNCellCalo(static_cast<MissingEtCalo::CaloIndex>(w), ncell);
    ++i;
  }
	metCnv.persToTrans(trans,i);
  return;
}

void MissingEtCaloCnv_p2::transToPers(  const MissingEtCalo* trans, MissingEtCalo_p2* pers, MsgStream& /* msg */ ) const {
  pers->m_AllTheData.push_back((float)trans->exCaloVec().size());
  std::copy(trans->exCaloVec().begin(), trans->exCaloVec().end(), std::back_inserter(pers->m_AllTheData) );
  std::copy(trans->eyCaloVec().begin(),trans->eyCaloVec().end(), std::back_inserter(pers->m_AllTheData) );
  std::copy(trans->etSumCaloVec().begin(),trans->etSumCaloVec().end(), std::back_inserter(pers->m_AllTheData) );
  static_assert(sizeof(float) == sizeof(std::uint32_t));
  for (unsigned int w = 0; w < trans->exCaloVec().size(); ++w) {
    const auto ncell = static_cast<std::uint32_t>(
      trans->ncellCalo(static_cast<MissingEtCalo::CaloIndex>(w))
    );
    pers->m_AllTheData.push_back(std::bit_cast<float>(ncell));
  }
	metCnv.transToPers( trans, pers->m_AllTheData );
	
 return;
}
