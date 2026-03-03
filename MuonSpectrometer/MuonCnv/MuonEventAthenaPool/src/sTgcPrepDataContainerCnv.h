/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONEVENTATHENAPOOL_sTgcPREPDATACONTAINERCNV_H
#define MUONEVENTATHENAPOOL_sTgcPREPDATACONTAINERCNV_H

#include "AthenaPoolCnvSvc/T_AthenaPoolCustomCnv.h"
#include "MuonPrepRawData/sTgcPrepDataCollection.h"
#include "MuonPrepRawData/sTgcPrepDataContainer.h"
#include "MuonEventTPCnv/MuonPrepRawData/sTgcPrepDataContainerCnv_p1.h"
#include "MuonEventTPCnv/MuonPrepRawData/sTgcPrepDataContainerCnv_p2.h"
#include "MuonEventTPCnv/MuonPrepRawData/sTgcPrepDataContainerCnv_p3.h"


// the latest persistent representation type of DataCollection:
typedef  Muon::sTgcPrepDataContainer_p3  sTgcPrepDataContainer_PERS;
typedef  T_AthenaPoolCustomCnv<Muon::sTgcPrepDataContainer, sTgcPrepDataContainer_PERS >  sTgcPrepDataContainerCnvBase;

/**
 ** Create derived converter to customize the saving of identifiable
 ** container
 **/
class sTgcPrepDataContainerCnv : 
    public sTgcPrepDataContainerCnvBase 
{
    
    friend class CnvFactory<sTgcPrepDataContainerCnv>;
    
public:
    sTgcPrepDataContainerCnv(ISvcLocator* svcloc);
    virtual ~sTgcPrepDataContainerCnv();
    
    virtual sTgcPrepDataContainer_PERS*   createPersistent (Muon::sTgcPrepDataContainer* transCont);
    virtual Muon::sTgcPrepDataContainer*  createTransient(const Token* token);

    virtual StatusCode initialize();
        
private:
    Muon::sTgcPrepDataContainerCnv_p1    m_converter_p1;
    Muon::sTgcPrepDataContainerCnv_p2    m_converter_p2;
    Muon::sTgcPrepDataContainerCnv_p3    m_converter_p3;
};

#endif
