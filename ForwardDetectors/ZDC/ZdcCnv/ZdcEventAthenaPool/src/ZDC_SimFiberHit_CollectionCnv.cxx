/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file ZDC_SimFiberHit_CollectionCnv.cxx
 * @brief Generated implementation file which includes header files needed by ZDC_SimFiberHit_CollectionCnv
 * @author RD Schaffer <R.D.Schaffer@cern.ch>
 */

#include "ZDC_SimFiberHit_CollectionCnv.h"
#include "ZdcEventTPCnv/ZDC_SimFiberHit_CollectionCnv_p1.h"

#include "GaudiKernel/MsgStream.h"

ZDC_SimFiberHit_Collection_PERS* ZDC_SimFiberHit_CollectionCnv::createPersistent(ZDC_SimFiberHit_Collection* transCont) {

    MsgStream mlog(msgSvc(), "ZDC_SimFiberHit_CollectionConverter::createPersistent" );
    ZDC_SimFiberHit_CollectionCnv_p1   converter;
    ZDC_SimFiberHit_Collection_PERS *persObj = converter.createPersistent( transCont, mlog );


    return persObj;
}

ZDC_SimFiberHit_Collection* ZDC_SimFiberHit_CollectionCnv::createTransient(const Token* token) {


    MsgStream mlog(msgSvc(), "ZDC_SimFiberHit_CollectionConverter::createTransient" );
    ZDC_SimFiberHit_CollectionCnv_p1   converter_p1;

    ZDC_SimFiberHit_Collection       *trans_cont(nullptr);

    static const Guid   p1_guid("92374D8F-1A24-4A38-86B4-611AAFA89CFB");
    if( this->compareClassGuid(token, p1_guid)) {
      std::unique_ptr< ZDC_SimFiberHit_Collection_PERS >   col_vect( this->poolReadObject< ZDC_SimFiberHit_Collection_PERS >(token) );
      trans_cont = converter_p1.createTransient(col_vect.get(), mlog );
    }
    else {
      throw std::runtime_error("Unsupported persistent version of Data collection");
    }


    return trans_cont;
}

