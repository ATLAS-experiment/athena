/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 Detector description conversion service package
 -----------------------------------------------
 ***************************************************************************/

#include "DetDescrCnvSvc/DetDescrConverter.h"

#include "DetDescrCnvSvc/DetDescrAddress.h"
#include "DetDescrCnvSvc/DetDescrCnvSvc.h"

StatusCode DetDescrConverter::fillObjRefs(IOpaqueAddress* /*pAddr*/,
                                          DataObject* /*pObj*/) {
    return StatusCode::SUCCESS;
}

StatusCode DetDescrConverter::createRep(DataObject* /*pObj*/,
                                        IOpaqueAddress*& /*pAddr*/) {
    return StatusCode::SUCCESS;
}

StatusCode DetDescrConverter::fillRepRefs(IOpaqueAddress* /*pAddr*/,
                                          DataObject* /*pObj*/) {
    return StatusCode::SUCCESS;
}

long DetDescrConverter::storageType() {
    return DetDescr_StorageType;
}

DetDescrConverter::DetDescrConverter(const CLID& myCLID,
                                     ISvcLocator* svcloc,
                                     const char* name /*= nullptr*/)
  : Converter(DetDescr_StorageType, myCLID, svcloc),
    AthMessaging(svcloc != nullptr ? msgSvc() : nullptr,
                 name ? name : "DetDescrConverter"),
    m_detStore("DetectorStore", name ? name : "DetDescrConverter")
{}
