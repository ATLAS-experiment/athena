/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MDTSimHitCollectionCnv.h"

// Gaudi
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/MsgStream.h"

// Athena
#include "StoreGate/StoreGateSvc.h"

#include "MuonSimEvent/MDTSimHitCollection.h"
#include "MuonSimEventTPCnv/MDTSimHitCollection_p1.h"
#include "MuonSimEventTPCnv/MDTSimHitCollection_p2.h"
#include "MuonSimEventTPCnv/MDTSimHitCollection_p3.h"
#include "MuonSimEventTPCnv/MDTSimHitCollection_p4.h"
#include "HitManagement/AthenaHitsVector.h" //for back-compatibility


MDTSimHitCollectionCnv::MDTSimHitCollectionCnv(ISvcLocator* svcloc) :
    MDTSimHitCollectionCnvBase(svcloc)
{
}

MDTSimHitCollectionCnv::~MDTSimHitCollectionCnv() = default;

MDTSimHitCollection_PERS*    MDTSimHitCollectionCnv::createPersistent (MDTSimHitCollection* transCont) {
    MsgStream log(msgSvc(), "MDTSimHitCollectionCnv" );
    ATH_MSG_DEBUG("createPersistent(): main converter");
    MDTSimHitCollection_PERS *pixdc_p= m_TPConverter_p3.createPersistent( transCont, log );
    return pixdc_p;
}

MDTSimHitCollection* MDTSimHitCollectionCnv::createTransient() {
    MsgStream log(msgSvc(), "MDTSimHitCollectionCnv" );
    static const pool::Guid   p0_guid("D76D06CC-C15F-43E6-BBC3-480DE5DA065D"); // before t/p split
    static const pool::Guid   p1_guid("EA781971-65C5-4B30-9D22-EEFB764BA0B3");
    static const pool::Guid   p2_guid("92880B97-75BB-4C5D-8183-577338059FCC");
    static const pool::Guid   p3_guid("0E9EEEE2-304F-44B8-B1DF-E75297183A02");
    static const pool::Guid   p4_guid("018E2DAC-18EB-7BBC-92AF-EFE3F0E400C2");
    ATH_MSG_DEBUG("createTransient(): main converter");
    MDTSimHitCollection* p_collection(nullptr);
    if( compareClassGuid(p4_guid) ) {
      ATH_MSG_DEBUG("createTransient(): T/P version 4 detected");
      std::unique_ptr< Muon::MDTSimHitCollection_p4 >   col_vect( this->poolReadObject< Muon::MDTSimHitCollection_p4 >() );
      p_collection = m_TPConverter_p4.createTransient( col_vect.get(), log );
    } else if( compareClassGuid(p3_guid) ) {
      ATH_MSG_DEBUG("createTransient(): T/P version 3 detected");
      std::unique_ptr< Muon::MDTSimHitCollection_p3 >   col_vect( this->poolReadObject< Muon::MDTSimHitCollection_p3 >() );
      p_collection = m_TPConverter_p3.createTransient( col_vect.get(), log );
    } else if( compareClassGuid(p2_guid) ) {
      ATH_MSG_DEBUG("createTransient(): T/P version 2 detected");
      std::unique_ptr< Muon::MDTSimHitCollection_p2 >   col_vect( this->poolReadObject< Muon::MDTSimHitCollection_p2 >() );
      p_collection = m_TPConverter_p2.createTransient( col_vect.get(), log );
    } else if( compareClassGuid(p1_guid) ) {
        ATH_MSG_DEBUG("createTransient(): T/P version 1 detected");
        // poolReadObject< MDTSimHitCollection_PERS >( m_TPConverter );
        // p_collection = m_TPConverter.createTransient( log );
        std::unique_ptr< Muon::MDTSimHitCollection_p1 >   col_vect( this->poolReadObject< Muon::MDTSimHitCollection_p1 >() );
        p_collection = m_TPConverter_p1.createTransient( col_vect.get(), log );
    }
  //----------------------------------------------------------------
    else if( compareClassGuid(p0_guid) ) {
        if (log.level() <= MSG::DEBUG) log<<MSG::DEBUG<<"createTransient(): Old input file"<<std::endl;
        AthenaHitsVector<MDTSimHit>* oldColl = this->poolReadObject< AthenaHitsVector<MDTSimHit> >();
        size_t size = oldColl->size();
        p_collection=new MDTSimHitCollection("DefaultCollectionName",size);
        p_collection->reserve(size);
        //do the copy
        for (const MDTSimHit* hit : *oldColl) {
            p_collection->push_back(*hit); 
        }
        delete oldColl;
    }
    else {
        throw std::runtime_error("Unsupported persistent version of MDTSimHitCollection");

    }
    return p_collection;
}
