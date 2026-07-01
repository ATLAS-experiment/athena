/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 Calo Identifier Manager converter package
 -----------------------------------------
 ***************************************************************************/

#ifndef CALOIDMGRDETDESCRCNV_CALOIDMGRDETDESCRCNV_H
#define CALOIDMGRDETDESCRCNV_CALOIDMGRDETDESCRCNV_H

#include "DetDescrCnvSvc/DetDescrConverter.h"


/**
 *  @brief This class is a converter for the CaloIdManager which is
 *  stored in the detector store. 
 *
 *  This class derives from
 *  DetDescrConverter which is a converter of the DetDescrCnvSvc. This
 *  converter creates a manager object and adds descriptors and
 *  detector elements to the manager. This objects are either created
 *  or accessed from the detector store.
 *
 * @author RD Schaffer
 * @author maintained by F. Ledroit
 */

class CaloIdMgrDetDescrCnv: public DetDescrConverter {

public:
    virtual long int   repSvcType() const override;
    virtual StatusCode initialize() override;
    virtual StatusCode createObj(IOpaqueAddress* pAddr, DataObject*& pObj) override;

    /** Storage type (used by CnvFactory) */
    static long storageType();
    /** class ID (used by CnvFactory) */
    static CLID classID();

    CaloIdMgrDetDescrCnv(ISvcLocator* svcloc);

private:

};


#endif // CALOIDMGRDETDESCRCNV_CALOIDMGRDETDESCRCNV_H
