/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "ALFA_EventTPCnv/ALFA_LocRecODEvCollectionCnv_p1.h"
#include "ALFA_LocRecODEvCollectionCnv.h"

 

ALFA_LocRecODEvCollection_PERS* ALFA_LocRecODEvCollectionCnv::createPersistent(ALFA_LocRecODEvCollection* transCont) {
    MsgStream mlog(msgSvc(), "ALFA_LocRecODEvCollectionConverter" );
    ALFA_LocRecODEvCollectionCnv_p1   TPConverter;
    ALFA_LocRecODEvCollection_PERS *persObj = TPConverter.createPersistent( transCont, mlog );
    return persObj;
}


ALFA_LocRecODEvCollection* ALFA_LocRecODEvCollectionCnv::createTransient(const Token* token) {
    MsgStream mlog(msgSvc(), "ALFA_LocRecODEvCollectionConverter" );
    
    ALFA_LocRecODEvCollectionCnv_p1   TPConverter_p1;

    ALFA_LocRecODEvCollection       *trans_cont(nullptr); // probably inicialization
    static const pool::Guid p1_guid ("D6688847-9903-4FDE-B709-879B0E470073");
    
    if( this->compareClassGuid(token, p1_guid)) {
         std::unique_ptr< ALFA_LocRecODEvCollection_p1 >   col_vect( this->poolReadObject< ALFA_LocRecODEvCollection_p1 >(token) );
        trans_cont = TPConverter_p1.createTransient( col_vect.get(), mlog );
    } else {
        throw std::runtime_error("Unsupported persistent version of Data container");
    }
    return trans_cont;
}
