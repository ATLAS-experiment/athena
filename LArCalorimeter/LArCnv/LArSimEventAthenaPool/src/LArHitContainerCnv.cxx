/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "LArSimEventTPCnv/LArHitContainerCnv_p1.h"
#include "LArSimEventTPCnv/LArHitContainerCnv_p2.h"
#include "LArHitContainerCnv.h"
#include "LArSimEventTPCnv/LArHit_p1.h"

LArHitContainer_PERS* LArHitContainerCnv::createPersistent(LArHitContainer* transCont) {
    MsgStream mlog(msgSvc(), "LArHitContainerConverter" );
    LArHitContainerCnv_p2   converter;
    LArHitContainer_PERS *persObj = converter.createPersistent( transCont, mlog );
    return persObj;
}


LArHitContainer* LArHitContainerCnv::createTransient(const Token* token) {
    MsgStream mlog(msgSvc(), "LArHitContainerConverter" );
    LArHitContainerCnv_p1   converter_p1;
    LArHitContainerCnv_p2   converter_p2;

    LArHitContainer       *trans_cont(0);

    static const Guid   p2_guid("1F1DE705-E0CE-4F0E-941A-C405CB2CD137");
    static const Guid   p1_guid("ED1ECB80-B38C-46DE-94BF-22F9379796DB");
    static const Guid   p0_guid("32703AED-CAA5-45ED-B804-8556900CA6B5");

    if( this->compareClassGuid(token, p2_guid)) {
        std::unique_ptr< LArHitContainer_p2 >   col_vect( this->poolReadObject< LArHitContainer_p2 >(token) );
        trans_cont = converter_p2.createTransient( col_vect.get(), mlog );
    }
    else if( this->compareClassGuid(token, p1_guid)) {
        std::unique_ptr< LArHitContainer_p1 >   col_vect( this->poolReadObject< LArHitContainer_p1 >(token) );
        trans_cont = converter_p1.createTransient( col_vect.get(), mlog );
    }
    else if( this->compareClassGuid(token, p0_guid)) {
        // old version from before TP separation, just return it
        trans_cont = this->poolReadObject<LArHitContainer>(token);
    }  else {
        throw std::runtime_error("Unsupported persistent version of Data container");
    }
    return trans_cont;
}
