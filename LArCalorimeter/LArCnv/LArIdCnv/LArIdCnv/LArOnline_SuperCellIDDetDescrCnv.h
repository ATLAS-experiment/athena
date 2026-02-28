/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARIDCNV_LARONLINE_SUPERCELLIDDETDESCRCNV_H
# define LARIDCNV_LARONLINE_SUPERCELLIDDETDESCRCNV_H

#include "DetDescrCnvSvc/DetDescrConverter.h"

/**
 **  This class is a converter for the LArOnline_SuperCellID an IdHelper which is
 **  stored in the detector store. This class derives from
 **  DetDescrConverter which is a converter of the DetDescrCnvSvc.
 **
 **/

class LArOnline_SuperCellIDDetDescrCnv: public DetDescrConverter {

public:
    virtual long int   repSvcType() const override;
    virtual StatusCode initialize() override;
    virtual StatusCode createObj(IOpaqueAddress* pAddr, DataObject*& pObj) override;

    // Storage type and class ID (used by CnvFactory)
    static long  storageType();
    static CLID classID();

    LArOnline_SuperCellIDDetDescrCnv(ISvcLocator* svcloc);
};

#endif // LARIDCNV_LARONLINE_SUPERCELLIDDETDESCRCNV_H
