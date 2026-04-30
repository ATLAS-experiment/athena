/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARIDCNV_LARHVLINEIDDETDESCRCNV_H
# define LARIDCNV_LARHVLINEIDDETDESCRCNV_H

#include "DetDescrCnvSvc/DetDescrConverter.h"


/**
 **  This class is a converter for the LArHVLineID an IdHelper which is
 **  stored in the detector store. This class derives from
 **  DetDescrConverter which is a converter of the DetDescrCnvSvc.
 **
 **/

class LArHVLineIDDetDescrCnv: public DetDescrConverter {

public:
    virtual long int   repSvcType() const override;
    virtual StatusCode initialize() override;
    virtual StatusCode createObj(IOpaqueAddress* pAddr, DataObject*& pObj) override;

    // Storage type and class ID (used by CnvFactory)
    static long  storageType();
    static CLID classID();

    LArHVLineIDDetDescrCnv(ISvcLocator* svcloc);
};

#endif // LARIDCNV_LARHVLINEIDDETDESCRCNV_H
