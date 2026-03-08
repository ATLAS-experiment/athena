/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 InDet DetDescrCnv package
 -----------------------------------------
 ***************************************************************************/

#ifndef INDETMGRDETDESCRCNV_ATLASIDDETDESCRCNV_H
# define INDETMGRDETDESCRCNV_ATLASIDDETDESCRCNV_H

#include "DetDescrCnvSvc/DetDescrConverter.h"

/**
 **  This class is a converter for the AtlasID an IdHelper which is
 **  stored in the detector store. This class derives from
 **  DetDescrConverter which is a converter of the DetDescrCnvSvc.
 **
 **/

class AtlasIDDetDescrCnv: public DetDescrConverter {

public:
    virtual long int   repSvcType() const override;
    virtual StatusCode initialize() override;
    virtual StatusCode createObj(IOpaqueAddress* pAddr, DataObject*& pObj) override;

    // Storage type and class ID (used by CnvFactory)
    static long int    storageType();
    static CLID classID();

    AtlasIDDetDescrCnv(ISvcLocator* svcloc);
};


#endif // INDETMGRDETDESCRCNV_ATLASIDDETDESCRCNV_H
