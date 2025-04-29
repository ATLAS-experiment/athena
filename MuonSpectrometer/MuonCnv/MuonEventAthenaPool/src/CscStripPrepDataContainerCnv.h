/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONEVENTATHENAPOOL_CSCSTRIPPREPDATACONTAINERCNV_H
#define MUONEVENTATHENAPOOL_CSCSTRIPPREPDATACONTAINERCNV_H

#include "AthenaPoolCnvSvc/T_AthenaPoolCustomCnv.h"
#include "MuonPrepRawData/CscStripPrepDataCollection.h"
#include "MuonPrepRawData/CscStripPrepDataContainer.h"
#include "MuonEventTPCnv/CscStripPrepDataContainerCnv_tlp1.h"

class StoreGateSvc;
class CscStripIdHelper;

// the latest persistent representation type of DataCollection:
typedef  Muon::CscStripPrepDataContainer_tlp1  CscStripPrepDataContainer_PERS;
typedef  T_AthenaPoolCustomCnv<Muon::CscStripPrepDataContainer, CscStripPrepDataContainer_PERS >  CscStripPrepDataContainerCnvBase;

/**
 ** Create derived converter to customize the saving of identifiable
 ** container
 **/
class CscStripPrepDataContainerCnv :  public CscStripPrepDataContainerCnvBase
{
    
    friend class CnvFactory<CscStripPrepDataContainerCnv>;
    
public:
    CscStripPrepDataContainerCnv(ISvcLocator* svcloc);
    virtual ~CscStripPrepDataContainerCnv();
    
    virtual CscStripPrepDataContainer_PERS*   createPersistent (Muon::CscStripPrepDataContainer* transCont);
    virtual Muon::CscStripPrepDataContainer*  createTransient ();

    // Must initialize ID helpers
    virtual StatusCode initialize();
    virtual AthenaPoolTopLevelTPCnvBase*  getTopLevelTPCnv() { return &m_TPConverter; }
        
private:
    StoreGateSvc*                   m_storeGate{};
    CscStripPrepDataContainerCnv_tlp1    m_TPConverter;
    
};

#endif
