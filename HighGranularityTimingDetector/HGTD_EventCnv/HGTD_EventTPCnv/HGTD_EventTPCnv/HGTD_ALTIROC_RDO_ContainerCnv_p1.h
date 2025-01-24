/**
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_EventTPCnv/HGTD_ALTIROC_RDO_ContainerCnv_p1.h
 * @author Alexander Leopold <alexander.leopold@cern.ch>
 * @brief
 *
 */

#ifndef HGTD_EVENTTPCNV_HGTD_ALTIROC_RDO_CONTAINERCNV_P1_H
#define HGTD_EVENTTPCNV_HGTD_ALTIROC_RDO_CONTAINERCNV_P1_H

#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"
#include "HGTD_EventTPCnv/HGTD_ALTIROC_RDO_Container_p1.h"
#include "HGTD_RawData/HGTD_ALTIROC_RDO_Container.h"

class HGTD_ID;
class StoreGateSvc;

class HGTD_ALTIROC_RDO_ContainerCnv_p1
  : public T_AthenaPoolTPCnvBase<HGTD_ALTIROC_RDO_Container,
                                 HGTD_ALTIROC_RDO_Container_p1> {

public:
  typedef HGTD_ALTIROC_RDO_Container_p1 Pers_t;
  typedef HGTD_ALTIROC_RDO_Container Trans_t;

  HGTD_ALTIROC_RDO_ContainerCnv_p1() = default;

  virtual void persToTrans(const Pers_t* persistent_container,
                           Trans_t* transient_container, MsgStream& log);
  virtual void transToPers(const Trans_t* transient_container,
                           Pers_t* persistent_container, MsgStream& log);

  virtual Trans_t* createTransient(const Pers_t* persistent_container,
                                   MsgStream& log);

private:
  StatusCode initialize(MsgStream& log);

  const HGTD_ID* m_hgtd_idhelper{nullptr};

  bool m_is_initialized{false};
};

#endif // HGTD_EVENTTPCNV_HGTD_ALTIROC_RDO_CONTAINERCNV_P1_H
