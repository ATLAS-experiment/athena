/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FORWARDIDCNV_ZDCIDDETDESCRCNV_H
# define FORWARDIDCNV_ZDCIDDETDESCRCNV_H

#include "DetDescrCnvSvc/DetDescrConverter.h"

class ZdcID;

/**
 **  This class is a converter for the ZdcID an IdHelper which is
 **  stored in the detector store. This class derives from
 **  DetDescrConverter which is a converter of the DetDescrCnvSvc.
 **
 **/
class ZdcIDDetDescrCnv: public DetDescrConverter {

public:

    virtual long int   repSvcType() const override;
    virtual StatusCode initialize() override;
    virtual StatusCode createObj(IOpaqueAddress* pAddr, DataObject*& pObj) override;

    // Storage type and class ID (used by CnvFactory)
    static long storageType();
    static const CLID& classID();

    ZdcIDDetDescrCnv(ISvcLocator* svcloc);

private:
    /// The helper - only will create it once
    ZdcID*      m_zdcId;

    /// File to be read for InDet ids
    std::string   m_inDetIDFileName;

    /// Tag of RDB record for InDet ids
    std::string   m_inDetIdDictTag;

    /// Internal InDet id tag
    std::string   m_inDetIDTag;

    /// Whether or not 
    bool          m_doChecks;

};

#endif // FORWARDIDCNV_ZDCIDDETDESCRCNV_H
