/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MMSimHitCollectionCnv.h"

// Gaudi
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/MsgStream.h"

// Athena
#include "StoreGate/StoreGateSvc.h"

#include "MuonSimEvent/MMSimHitCollection.h"
#include "MuonSimEventTPCnv/MMSimHitCollection_p1.h"
#include "MuonSimEventTPCnv/MMSimHitCollection_p2.h"
#include "MuonSimEventTPCnv/MMSimHitCollection_p3.h"
#include "HitManagement/AthenaHitsVector.h" //for back-compatibility


MMSimHitCollectionCnv::MMSimHitCollectionCnv(ISvcLocator* svcloc) :
    MMSimHitCollectionCnvBase(svcloc)
{
}

MMSimHitCollectionCnv::~MMSimHitCollectionCnv() = default;

MMSimHitCollection_PERS*    MMSimHitCollectionCnv::createPersistent (MMSimHitCollection* transCont) {
    MsgStream log(msgSvc(), "MMSimHitCollectionCnv" );
    ATH_MSG_DEBUG("createPersistent(): main converter");
    MMSimHitCollection_PERS *pixdc_p= m_TPConverter_p2.createPersistent( transCont, log );
    return pixdc_p;
}

MMSimHitCollection* MMSimHitCollectionCnv::createTransient() {
    MsgStream log(msgSvc(), "MMSimHitCollectionCnv" );
    static const pool::Guid   p1_guid("AC0B677C-FE08-11E8-B174-02163E018187");
    static const pool::Guid   p2_guid("B9BDD436-FE08-11E8-A40F-02163E018187");
    static const pool::Guid   p3_guid("018E2DAC-18EB-7EAA-A141-F0FD2A6E1E06");
    ATH_MSG_DEBUG("createTransient(): main converter");
    MMSimHitCollection* p_collection(nullptr);
    if( compareClassGuid(p3_guid) ) {
      ATH_MSG_DEBUG("createTransient(): T/P version 3 detected");
      std::unique_ptr< Muon::MMSimHitCollection_p3 >   col_vect( this->poolReadObject< Muon::MMSimHitCollection_p3 >() );
      p_collection = m_TPConverter_p3.createTransient( col_vect.get(), log );
    } else if( compareClassGuid(p2_guid) ) {
      ATH_MSG_DEBUG("createTransient(): T/P version 2 detected");
      std::unique_ptr< Muon::MMSimHitCollection_p2 >   col_vect( this->poolReadObject< Muon::MMSimHitCollection_p2 >() );
      p_collection = m_TPConverter_p2.createTransient( col_vect.get(), log );
    } else if( compareClassGuid(p1_guid) ) {
      ATH_MSG_DEBUG("createTransient(): T/P version 1 detected");
      std::unique_ptr< Muon::MMSimHitCollection_p1 >   col_vect( this->poolReadObject< Muon::MMSimHitCollection_p1 >() );
      p_collection = m_TPConverter_p1.createTransient( col_vect.get(), log );
    }
  //----------------------------------------------------------------
    else {
        throw std::runtime_error("Unsupported persistent version of MMSimHitCollection");

    }
    return p_collection;
}
