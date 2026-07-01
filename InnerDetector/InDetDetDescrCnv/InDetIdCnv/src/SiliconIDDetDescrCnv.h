/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETMGRDETDESCRCNV_SILICONIDDETDESCRCNV_H
# define INDETMGRDETDESCRCNV_SILICONIDDETDESCRCNV_H

#include "DetDescrCnvSvc/DetDescrConverter.h"

class SiliconID;


/**
 **  This class is a converter for the SiliconID an IdHelper which is
 **  stored in the detector store. This class derives from
 **  DetDescrConverter which is a converter of the DetDescrCnvSvc.
 **
 **/

class SiliconIDDetDescrCnv: public DetDescrConverter {

public:

    virtual long int   repSvcType() const override;
    virtual StatusCode initialize() override;
    virtual StatusCode createObj(IOpaqueAddress* pAddr, DataObject*& pObj) override;

    // Storage type and class ID (used by CnvFactory)
    static long storageType();
    static CLID classID();

    SiliconIDDetDescrCnv(ISvcLocator* svcloc);

private:
    /// The helper - only will create it once
    SiliconID*       m_siliconId;

};

#endif // INDETMGRDETDESCRCNV_SILICONIDDETDESCRCNV_H
