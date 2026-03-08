/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PHASEII_STRIPRAWDATACONTAINERCNV_H
#define PHASEII_STRIPRAWDATACONTAINERCNV_H

#include "AthenaPoolCnvSvc/T_AthenaPoolCustomCnv.h"

#include "InDetRawData/PhaseIIStripRawDataContainer.h"
#include "InDetEventAthenaPool/SCT_RawDataContainer_p4.h"
#include "StoreGate/StoreGateSvc.h"


class SCT_ID;

using StripRDO_Container_PERS = SCT_RawDataContainer_p4;
using PhaseIIStripRawDataContainerCnvBase =  T_AthenaPoolCustomCnv<PhaseIIStripRawDataContainer, StripRDO_Container_PERS >;

/// @brief Converter to convert persistent SCT raw data into an PhaseIIStripRawDataContainer.
class PhaseIIStripRawDataContainerCnv : public PhaseIIStripRawDataContainerCnvBase {
   friend class CnvFactory<PhaseIIStripRawDataContainerCnv >;

public:
   PhaseIIStripRawDataContainerCnv(ISvcLocator* svcloc)
      : PhaseIIStripRawDataContainerCnvBase(svcloc, "PhaseIIStripRawDataContainerCnv")
   {}
protected:
   virtual StripRDO_Container_PERS*   createPersistent (PhaseIIStripRawDataContainer* transCont) override;
   virtual PhaseIIStripRawDataContainer* createTransient(const Token* token) override;

   virtual StatusCode initialize() override;

   const SCT_ID *m_idHelper = nullptr;
};

#endif
