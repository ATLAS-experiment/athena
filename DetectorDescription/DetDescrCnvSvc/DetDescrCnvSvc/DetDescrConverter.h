/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 Detector description conversion service package
 -----------------------------------------------
 ***************************************************************************/

#ifndef DETDESCRCNVSVC_DETDESCRCONVERTER_H
#define DETDESCRCNVSVC_DETDESCRCONVERTER_H

#include "GaudiKernel/Converter.h"
#include "GaudiKernel/ServiceHandle.h"

#include "AthenaBaseComps/AthMessaging.h"
#include "AthenaBaseComps/AthCheckMacros.h"
#include "StoreGate/StoreGateSvc.h"

class DetDescrCnvSvc;
class DetDescrAddress;
class DataObject;


class DetDescrConverter : public Converter, public AthMessaging {
   public:
    virtual StatusCode createObj(IOpaqueAddress* pAddr, DataObject*& pObj) = 0;
    virtual StatusCode fillObjRefs(IOpaqueAddress* pAddr, DataObject* pObj);
    virtual StatusCode createRep(DataObject* pObj, IOpaqueAddress*& pAddr);
    virtual StatusCode fillRepRefs(IOpaqueAddress* pAddr, DataObject* pObj);

    /// Handle to DetectorStore
    const ServiceHandle<StoreGateSvc>& detStore() const { return m_detStore; }

    /// Storage type for all DetDescrConverters
    static long storageType();

   protected:
    DetDescrConverter(const CLID& myCLID, ISvcLocator* svcloc, const char* name = nullptr);

   private:
    ServiceHandle<StoreGateSvc> m_detStore;
};

#endif  // DETDESCRCNVSVC_DETDESCRCONVERTER_H
