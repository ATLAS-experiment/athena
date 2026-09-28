/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETSIMDATACNV_P2_H
#define INDETSIMDATACNV_P2_H

/*
  Transient/Persistent converter for InDetSimData class
  Author: Davide Costanzo
*/

#include "InDetSimData/InDetSimData.h"
#include "InDetEventAthenaPool/InDetSimData_p2.h"

#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"
#include "GeneratorObjectsTPCnv/HepMcParticleLinkCnv_p2.h"

class MsgStream;
class EventContext;

class InDetSimDataCnv_p2  : public T_AthenaPoolTPCnvBase<InDetSimData, InDetSimData_p2>
{
public:

  InDetSimDataCnv_p2 (const EventContext& ctx);
  virtual void          persToTrans(const InDetSimData_p2* persObj, InDetSimData* transObj, MsgStream &log);
  virtual void          transToPers(const InDetSimData* transObj, InDetSimData_p2* persObj, MsgStream &log);

private:
  const EventContext& m_ctx;
  HepMcParticleLinkCnv_p2 HepMcPLCnv;
};


#endif


