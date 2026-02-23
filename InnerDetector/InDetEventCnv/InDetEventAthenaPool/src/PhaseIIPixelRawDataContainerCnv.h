/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PHASEII_PIXELRAWDATACONTAINERCNV_H
#define PHASEII_PIXELRAWDATACONTAINERCNV_H

#include "AthenaPoolCnvSvc/T_AthenaPoolCustomCnv.h"

#include "InDetRawData/PhaseIIPixelRawDataContainer.h"
#include "InDetEventAthenaPool/InDetRawDataContainer_p2.h"
#include "StoreGate/StoreGateSvc.h"


class PixelID;

using PixelRDO_Container_PERS = InDetRawDataContainer_p2;
using PhaseIIPixelRawDataContainerCnvBase =  T_AthenaPoolCustomCnv<PhaseIIPixelRawDataContainer, PixelRDO_Container_PERS >;

/// @brief Converter to convert persistent raw data into an PhaseIIPixelRawDataContainer.
class PhaseIIPixelRawDataContainerCnv : public PhaseIIPixelRawDataContainerCnvBase {
   friend class CnvFactory<PhaseIIPixelRawDataContainerCnv >;

public:
   PhaseIIPixelRawDataContainerCnv(ISvcLocator* svcloc)
      : PhaseIIPixelRawDataContainerCnvBase(svcloc, "PhaseIIPixelRawDataContainerCnv")
   {}
protected:
   virtual PixelRDO_Container_PERS*   createPersistent (PhaseIIPixelRawDataContainer* transCont) override;
   virtual PhaseIIPixelRawDataContainer* createTransient () override;

   virtual StatusCode initialize() override;

   const PixelID *m_idHelper = nullptr;
};

#endif
