/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

//Dear emacs, this is -*-c++-*- 
#ifndef LARATHENAPOOL_LARDIGITCONTAINERCNV_H
#define LARATHENAPOOL_LARDIGITCONTAINERCNV_H

#include "AthenaPoolCnvSvc/T_AthenaPoolCustomCnv.h"
#include "LArRawEvent/LArDigitContainer.h"
#include "LArTPCnv/LArDigitContainer_p3.h"
#include "StoreGate/StoreGateSvc.h"

class LArOnlineID_Base;
class StoreGateSvc;

typedef LArDigitContainer_p3 LArDigitContainerPERS;

typedef T_AthenaPoolCustomCnv<LArDigitContainer,LArDigitContainerPERS> LArDigitContainerCnvBase;

class LArDigitContainerCnv : public LArDigitContainerCnvBase 
{
public:
  LArDigitContainerCnv(ISvcLocator*);
  StatusCode initialize();
protected:
  virtual LArDigitContainer* createTransient();
  virtual LArDigitContainerPERS* createPersistent(LArDigitContainer*);
 private:
  const LArOnlineID_Base* m_idHelper = nullptr;
  const LArOnlineID_Base* m_idSCHelper = nullptr;
  ServiceHandle<StoreGateSvc> m_storeGateSvc;
};

#endif
