/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// LArTPCnv includes
#include "LArTPCnv/LArTTL1ContainerCnv_p1.h"

// LArEventAthenaPool includes
#include "LArTTL1ContainerCnv.h"

LArTTL1Container_PERS* 
LArTTL1ContainerCnv::createPersistent( LArTTL1Container* transCont ) 
{
  LArTTL1ContainerCnv_p1 cnv;
  LArTTL1Container_PERS *persObj = cnv.createPersistent( transCont, msg() );

  return persObj;
}

LArTTL1Container* LArTTL1ContainerCnv::createTransient() {

  LArTTL1Container *transObj = 0;

  static const pool::Guid tr_guid("38FAECC7-D0C5-4DD8-8FAE-8D35F0542ECD");
  static const pool::Guid p1_guid("B859A463-2EA4-4902-B46A-89E5FBC20132");

  if ( compareClassGuid(tr_guid) ) {

    // regular object from before the T/P separation
    return poolReadObject<LArTTL1Container>();

  } else if ( compareClassGuid(p1_guid) ) {

    // using unique_ptr ensures deletion of the persistent object
    std::unique_ptr<LArTTL1Container_p1> persObj( poolReadObject<LArTTL1Container_p1>() );
    LArTTL1ContainerCnv_p1 cnv;
    transObj = cnv.createTransient( persObj.get(), msg() );
  } else {
    throw std::runtime_error("Unsupported persistent version of LArTTL1Container");
  }

  return transObj;
}
