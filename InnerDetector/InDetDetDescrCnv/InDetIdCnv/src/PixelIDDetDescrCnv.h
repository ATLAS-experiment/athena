/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETMGRDETDESCRCNV_PIXELIDDETDESCRCNV_H
# define INDETMGRDETDESCRCNV_PIXELIDDETDESCRCNV_H

#include "DetDescrCnvSvc/DetDescrConverter.h"

class PixelID;


/**
 **  This class is a converter for the PixelID an IdHelper which is
 **  stored in the detector store. This class derives from
 **  DetDescrConverter which is a converter of the DetDescrCnvSvc.
 **
 **/

class PixelIDDetDescrCnv: public DetDescrConverter {

public:

    virtual long int   repSvcType() const override;
    virtual StatusCode initialize() override;
    virtual StatusCode createObj(IOpaqueAddress* pAddr, DataObject*& pObj) override;

    // Storage type and class ID (used by CnvFactory)
    static long storageType();
    static const CLID& classID();

    PixelIDDetDescrCnv(ISvcLocator* svcloc);

private:
    /// The helper - only will create it once
    PixelID*      m_pixelId;

    /// File to be read for InDet ids
    std::string   m_inDetIDFileName;

    /// Tag of RDB record for InDet ids
    std::string   m_inDetIdDictTag;

    /// Internal InDet id tag
    std::string   m_inDetIDTag;

    /// Whether or not 
    bool          m_doChecks;

};

#endif // INDETMGRDETDESCRCNV_PIXELIDDETDESCRCNV_H
