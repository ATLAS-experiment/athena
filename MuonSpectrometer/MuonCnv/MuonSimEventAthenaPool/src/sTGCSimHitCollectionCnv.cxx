/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "sTGCSimHitCollectionCnv.h"

// Gaudi
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/MsgStream.h"

// Athena
#include "StoreGate/StoreGateSvc.h"

#include "MuonSimEvent/sTGCSimHitCollection.h"
#include "MuonSimEventTPCnv/sTGCSimHitCollection_p1.h"
#include "MuonSimEventTPCnv/sTGCSimHitCollection_p2.h"
#include "MuonSimEventTPCnv/sTGCSimHitCollection_p3.h"
#include "MuonSimEventTPCnv/sTGCSimHitCollection_p4.h"
#include "HitManagement/AthenaHitsVector.h" //for back-compatibility


sTGCSimHitCollectionCnv::sTGCSimHitCollectionCnv(ISvcLocator* svcloc) :
    sTGCSimHitCollectionCnvBase(svcloc)
{
}

sTGCSimHitCollectionCnv::~sTGCSimHitCollectionCnv() = default;

sTGCSimHitCollection_PERS*    sTGCSimHitCollectionCnv::createPersistent (sTGCSimHitCollection* transCont) {
    MsgStream log(msgSvc(), "sTGCSimHitCollectionCnv" );
    ATH_MSG_DEBUG("createPersistent(): main converter");
    sTGCSimHitCollection_PERS *pixdc_p= m_TPConverter_p3.createPersistent( transCont, log );
    return pixdc_p;
}

sTGCSimHitCollection* sTGCSimHitCollectionCnv::createTransient() {
    MsgStream log(msgSvc(), "sTGCSimHitCollectionCnv" );
    static const pool::Guid   p1_guid("F8B975D2-8130-11E8-ABF4-4B4A6A2B6EE5");
    static const pool::Guid   p2_guid("B9521CC6-6E3B-11E8-ADBB-02163E01BDDD");
    static const pool::Guid   p3_guid("8F3FFD1C-C9A0-4DA7-B99E-A3828B6AC789");
    static const pool::Guid   p4_guid("018E2DAC-18EB-79C4-B562-FD7C035C92C1");

    ATH_MSG_DEBUG("createTransient(): main converter");
    sTGCSimHitCollection* p_collection(nullptr);
    if( compareClassGuid(p4_guid) ) {
      ATH_MSG_DEBUG("createTransient(): T/P version 4 detected");
      std::unique_ptr< Muon::sTGCSimHitCollection_p4 >   col_vect( this->poolReadObject< Muon::sTGCSimHitCollection_p4 >() );
      p_collection = m_TPConverter_p4.createTransient( col_vect.get(), log );
    } else if( compareClassGuid(p3_guid) ) {
      ATH_MSG_DEBUG("createTransient(): T/P version 3 detected");
      std::unique_ptr< Muon::sTGCSimHitCollection_p3 >   col_vect( this->poolReadObject< Muon::sTGCSimHitCollection_p3 >() );
      p_collection = m_TPConverter_p3.createTransient( col_vect.get(), log );
    } else if( compareClassGuid(p2_guid) ) {
      ATH_MSG_DEBUG("createTransient(): T/P version 2 detected");
      std::unique_ptr< Muon::sTGCSimHitCollection_p2 >   col_vect( this->poolReadObject< Muon::sTGCSimHitCollection_p2 >() );
      p_collection = m_TPConverter_p2.createTransient( col_vect.get(), log );
    } else if( compareClassGuid(p1_guid) ) {
      ATH_MSG_DEBUG("createTransient(): T/P version 1 detected");
      std::unique_ptr< Muon::sTGCSimHitCollection_p1 >   col_vect( this->poolReadObject< Muon::sTGCSimHitCollection_p1 >() );
      p_collection = m_TPConverter_p1.createTransient( col_vect.get(), log );
    }
  //----------------------------------------------------------------
    else {
        throw std::runtime_error("Unsupported persistent version of sTGCSimHitCollection");

    }
    return p_collection;
}
