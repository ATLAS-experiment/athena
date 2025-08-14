/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "G4SimTPCnv/TrackRecordCollectionCnv_p1.h"
#include "G4SimTPCnv/TrackRecordCollectionCnv_p2.h"
#include "G4SimTPCnv/TrackRecordCollectionCnv_p3.h"
#include "TrackRecordCollectionCnv.h"

TrackRecordCollection_PERS* TrackRecordCollectionCnv::createPersistent(TrackRecordCollection* transCont) {
   MsgStream mlog(msgSvc(), "TrackRecordCollectionConverter" );
   TrackRecordCollectionCnv_p3   converter;
   TrackRecordCollection_PERS *persObj = converter.createPersistent( transCont, mlog );
   return persObj;
}


TrackRecordCollection* TrackRecordCollectionCnv::createTransient() {

    MsgStream mlog(msgSvc(), "TrackRecordCollectionConverter" );
    TrackRecordCollectionCnv_p1   converter;
    TrackRecordCollectionCnv_p2   converter_p2;
    TrackRecordCollectionCnv_p3   converter_p3;
    TrackRecordCollection       *trans_cont{};
    
   // GUIDs are here:
    static const pool::Guid   p1_guid("1B1EEE3B-4647-41B4-B1D4-495DF77F0D3C");
    static const pool::Guid   p2_guid("22D044AD-A13A-42BF-B2A4-BDAF5BE2D819");
    static const pool::Guid   p3_guid("C17DB0DC-C53D-4993-A222-B1F4E01C9004");

   if( this->compareClassGuid(p1_guid))  {
      std::unique_ptr< TrackRecordCollection_p1 >   col_vect( this->poolReadObject< TrackRecordCollection_p1 >() );
      trans_cont = converter.createTransient( col_vect.get(), mlog );
   }
   // New _p2 version faster and smaller
   else  if( this->compareClassGuid(p2_guid))  {
      std::unique_ptr< TrackRecordCollection_p2 >   col_vect( this->poolReadObject< TrackRecordCollection_p2 >() );
      trans_cont = converter_p2.createTransient( col_vect.get(), mlog );
   }
   else  if( this->compareClassGuid(p3_guid))  {
      std::unique_ptr< TrackRecordCollection_p3 >   col_vect( this->poolReadObject< TrackRecordCollection_p3 >() );
      trans_cont = converter_p3.createTransient( col_vect.get(), mlog );
   }
   else 
   {
      throw std::runtime_error("Unsupported persistent version of Data container");
   }
   return trans_cont;
}


