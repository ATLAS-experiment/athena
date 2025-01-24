/**
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_EventAthenaPool/src/HGTD_ALTIROC_RDO_ContainerCnv.h
 * @author Alexander Leopold <alexander.leopold@cern.ch>
 * @author Rodrigo Estevam de Paula <rodrigo.estevam.de.paula@cern.ch>
 * @brief
 *
 */

#ifndef HGTD_EVENTATHENAPOOL_HGTD_ALTIROC_RDO_CONTAINERCNV_H
#define HGTD_EVENTATHENAPOOL_HGTD_ALTIROC_RDO_CONTAINERCNV_H

#include "AthenaPoolCnvSvc/T_AthenaPoolCustomCnv.h"
#include "GaudiKernel/MsgStream.h"
#include "HGTD_RawData/HGTD_ALTIROC_RDO_Container.h"

#include "HGTD_EventTPCnv/HGTD_ALTIROC_RDO_ContainerCnv_p1.h"

// the latest persistent representation type of the container holding
// HGTD_RawData
typedef HGTD_ALTIROC_RDO_Container_p1 HGTD_ALTIROC_RDO_Container_PERS_t;

typedef HGTD_ALTIROC_RDO_ContainerCnv_p1 HGTD_ALTIROC_RDO_ContainerCNV_t;

typedef T_AthenaPoolCustomCnv<HGTD_ALTIROC_RDO_Container, HGTD_ALTIROC_RDO_Container_PERS_t>
  HGTD_ALTIROC_RDO_ContainerCnvBase_t;

class HGTD_ALTIROC_RDO_ContainerCnv : public HGTD_ALTIROC_RDO_ContainerCnvBase_t {

  friend class CnvFactory<HGTD_ALTIROC_RDO_ContainerCnv>;

public:

  HGTD_ALTIROC_RDO_ContainerCnv(ISvcLocator* svcloc);

protected:

  virtual HGTD_ALTIROC_RDO_Container_PERS_t*
  createPersistent(HGTD_ALTIROC_RDO_Container* trans) override;

  virtual HGTD_ALTIROC_RDO_Container* createTransient() override;

private:
  HGTD_ALTIROC_RDO_ContainerCNV_t m_converter;
};

#endif // HGTD_CLUSTERCONTAINERCNV_H
