/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETMGRDETDESCRCNV_TRT_IDDETDESCRCNV_H
# define INDETMGRDETDESCRCNV_TRT_IDDETDESCRCNV_H

#include "DetDescrCnvSvc/DetDescrConverter.h"

class TRT_ID;

/**
 **  This class is a converter for the TRT_ID an IdHelper which is
 **  stored in the detector store. This class derives from
 **  DetDescrConverter which is a converter of the DetDescrCnvSvc.
 **
 **/

class TRT_IDDetDescrCnv: public DetDescrConverter {

public:

    virtual long int   repSvcType() const override;
    virtual StatusCode initialize() override;
    virtual StatusCode createObj(IOpaqueAddress* pAddr, DataObject*& pObj) override;

    // Storage type and class ID (used by CnvFactory)
    static long storageType();
    static const CLID& classID();

    TRT_IDDetDescrCnv(ISvcLocator* svcloc);

private:
    /// The helper - only will create it once
    TRT_ID*       m_trtId;

    /// File to be read for InDet ids
    std::string   m_inDetIDFileName;

    /// Tag of RDB record for InDet ids
    std::string   m_inDetIdDictTag;

    /// Internal InDet id tag
    std::string   m_inDetIDTag;

    /// Whether or not 
    bool          m_doChecks;

};

#endif // INDETMGRDETDESCRCNV_TRT_IDDETDESCRCNV_H
