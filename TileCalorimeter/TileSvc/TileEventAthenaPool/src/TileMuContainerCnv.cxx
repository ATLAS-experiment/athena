///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

// TileMuContainerCnv.cxx 
// Implementation file for class TileMuContainerCnv
// Author: Aranzazu Ruiz <aranzazu.ruiz.martinez@cern.ch>
// Date:   July 2008
/////////////////////////////////////////////////////////////////// 

// Framework includes
#include "GaudiKernel/MsgStream.h"

// TileTPCnv includes
#include "TileTPCnv/TileMuContainerCnv_p1.h"

// TileEventAthenaPool includes
#include "TileMuContainerCnv.h"

TileMuContainer_PERS* 
TileMuContainerCnv::createPersistent( TileMuContainer* transCont ) 
{
  MsgStream msg( msgSvc(), "TileMuContainerCnv" );

  TileMuContainerCnv_p1 cnv;
  TileMuContainer_PERS *persObj = cnv.createPersistent( transCont, msg );

  if (msg.level()<=MSG::DEBUG)
    msg << MSG::DEBUG << "::createPersistent [Success]" << endmsg;
  return persObj; 
}

TileMuContainer* TileMuContainerCnv::createTransient(const Token* token) {

  MsgStream msg( msgSvc(), "TileMuContainerCnv" );

  TileMuContainer *transObj = 0;

  static const Guid tr_guid("FC0456E4-912B-425B-9AA2-4DDD0C6B2275");
  static const Guid p1_guid("DE8904EB-25FD-495A-8DD5-E31B05E397C6");

  if ( compareClassGuid(token, tr_guid) ) {

    // regular object from before the T/P separation
    return poolReadObject<TileMuContainer>(token);

  } else if ( compareClassGuid(token, p1_guid) ) {

    // using unique_ptr ensures deletion of the persistent object
    std::unique_ptr<TileMuContainer_p1> persObj( poolReadObject<TileMuContainer_p1>(token) );
    TileMuContainerCnv_p1 cnv;
    transObj = cnv.createTransient( persObj.get(), msg );
  } else {
    throw std::runtime_error("Unsupported persistent version of TileMuContainer");
  }

  return transObj;
}
