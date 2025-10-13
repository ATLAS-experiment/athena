/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HGTD_IDCNV_HGTD_IDDETDESCRCNV_H
# define HGTD_IDCNV_HGTD_IDDETDESCRCNV_H

#include "DetDescrCnvSvc/DetDescrConverter.h"

#include "HGTD_Identifier/HGTD_ID.h"

#include <memory>
#include <string>

class IOpaqueAddress;
class DataObject;
class ISvcLocator;

/**
 **  This class is a converter for the HGTD_ID an IdHelper which is
 **  stored in the detector store. This class derives from
 **  DetDescrConverter which is a converter of the DetDescrCnvSvc.
 **
 **/

class HGTD_IDDetDescrCnv: public DetDescrConverter {

public:

    virtual long int   repSvcType() const override final;
    virtual StatusCode initialize() override final;
    virtual StatusCode createObj(IOpaqueAddress* pAddr, DataObject*& pObj) override final;

    // Storage type and class ID (used by CnvFactory)
    static long storageType();
    static const CLID& classID();

    HGTD_IDDetDescrCnv(ISvcLocator* svcloc);

private:
    /// The helper - only will create it once
    std::unique_ptr<HGTD_ID>   m_hgtdId;

    /// File to be read for InDet ids
    std::string   m_inDetIDFileName;

    /// Tag of RDB record for InDet ids
    std::string   m_inDetIdDictTag;

    /// Internal InDet id tag
    std::string   m_inDetIDTag;

    /// Whether or not 
    bool          m_doChecks;

};


#endif // HGTD_IDCNV_HGTD_IDDETDESCRCNV_H
