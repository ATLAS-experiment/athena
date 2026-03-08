/*
 * Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#ifndef MUONEVENTATHENAPOOL_NSW_TRIGRAWDATACONTAINERCNV_H
#define MUONEVENTATHENAPOOL_NSW_TRIGRAWDATACONTAINERCNV_H

#include "MuonRdoContainerTPCnv.h"
#include "MuonRDO/NSW_TrigRawDataContainer.h"
#include "MuonEventTPCnv/MuonRDO/NSW_TrigRawDataContainerCnv_p1.h"

typedef Muon::NSW_TrigRawDataContainer_p1 NSW_TrigRawDataContainer_PERS;
typedef T_AthenaPoolCustomCnv<Muon::NSW_TrigRawDataContainer, NSW_TrigRawDataContainer_PERS> NSW_TrigRawDataContainerCnvBase;

class NSW_TrigRawDataContainerCnv : public NSW_TrigRawDataContainerCnvBase {
  public:
    NSW_TrigRawDataContainerCnv(ISvcLocator* svcLocator);
    virtual ~NSW_TrigRawDataContainerCnv()=default;
    virtual NSW_TrigRawDataContainer_PERS* createPersistent(Muon::NSW_TrigRawDataContainer* transCont) override;
    virtual Muon::NSW_TrigRawDataContainer* createTransient(const Token* token) override;

  private:
    Muon::NSW_TrigRawDataContainerCnv_p1 m_TPConverter_p1;
};

#endif
