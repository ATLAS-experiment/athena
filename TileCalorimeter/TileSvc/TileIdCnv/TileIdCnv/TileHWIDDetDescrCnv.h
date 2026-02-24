/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILEIDCNV_TILEHWIDDETDESCRCNV_H
# define TILEIDCNV_TILEHWIDDETDESCRCNV_H

#include "DetDescrCnvSvc/DetDescrConverter.h"


/**
 **  This class is a converter for the TileHWID an IdHelper which is
 **  stored in the detector store. This class derives from
 **  DetDescrConverter which is a converter of the DetDescrCnvSvc.
 **
 **/

class TileHWIDDetDescrCnv: public DetDescrConverter {

public:
    virtual StatusCode initialize() override;
    virtual StatusCode createObj(IOpaqueAddress* pAddr, DataObject*& pObj) override;

    // Storage type and class ID (used by CnvFactory)
    virtual long repSvcType() const override;
    static long storageType();
    static CLID classID();

    TileHWIDDetDescrCnv(ISvcLocator* svcloc);
};

#endif // TILEDETMGRDETDESCRCNV_TILEHWIDDETDESCRCNV_H

