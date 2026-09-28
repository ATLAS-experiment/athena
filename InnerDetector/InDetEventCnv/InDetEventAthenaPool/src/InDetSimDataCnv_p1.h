/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETSIMDATACNV_P1_H
#define INDETSIMDATACNV_P1_H

/*
Transient/Persistent converter for InDetSimData class
Author: Davide Costanzo
*/

#include "InDetSimData/InDetSimData.h"
#include "InDetEventAthenaPool/InDetSimData_p1.h"

#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"
#include "GeneratorObjectsTPCnv/HepMcParticleLinkCnv_p1.h"

class MsgStream;
class EventContext;

class InDetSimDataCnv_p1  : public T_AthenaPoolTPCnvBase<InDetSimData, InDetSimData_p1>
{
public:

  InDetSimDataCnv_p1 (const EventContext& ctx);
  virtual void          persToTrans(const InDetSimData_p1* persObj, InDetSimData* transObj, MsgStream &log);
  virtual void          transToPers(const InDetSimData* transObj, InDetSimData_p1* persObj, MsgStream &log);

private:
  const EventContext& m_ctx;
  HepMcParticleLinkCnv_p1 HepMcPLCnv;
};


#endif


