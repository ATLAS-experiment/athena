/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETEVENTATHENAPOOL_INDETSIMDATACNV_P3_H
#define INDETEVENTATHENAPOOL_INDETSIMDATACNV_P3_H

/*
  Transient/Persistent converter for InDetSimData class
  Author: Davide Costanzo
*/

#include "InDetSimData/InDetSimData.h"
#include "InDetEventAthenaPool/InDetSimData_p3.h"

#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"
#include "GeneratorObjectsTPCnv/HepMcParticleLinkCnv_p3.h"

class MsgStream;
class IProxyDict;

class InDetSimDataCnv_p3  : public T_AthenaPoolTPCnvBase<InDetSimData, InDetSimData_p3>
{
public:

  InDetSimDataCnv_p3();
  virtual void          persToTrans(const InDetSimData_p3* persObj, InDetSimData* transObj, MsgStream &log);
  virtual void          transToPers(const InDetSimData* transObj, InDetSimData_p3* persObj, MsgStream &log);
  void setCurrentStore (IProxyDict* store);

private:
  IProxyDict* m_sg;
  HepMcParticleLinkCnv_p3 HepMcPLCnv;
};


#endif // INDETEVENTATHENAPOOL_INDETSIMDATACNV_P3_H


