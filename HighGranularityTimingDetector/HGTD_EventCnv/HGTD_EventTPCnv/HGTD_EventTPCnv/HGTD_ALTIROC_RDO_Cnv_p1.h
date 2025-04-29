/**
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_EventTPCnv/HGTD_ALTIROC_RDO_Cnv_p1.h
 * @author Alexander Leopold <alexander.leopold@cern.ch>
 * @author Rodrigo Estevam de Paula <rodrigo.estevam.de.paula@cern.ch>
 * @brief Transient/Persistent converter for the HGTD_ALTIROC_RDO class.
 *
 */

#ifndef HGTD_EVENTTPCNV_HGTD_ALTIROC_RDO_CNV_P1_H
#define HGTD_EVENTTPCNV_HGTD_ALTIROC_RDO_CNV_P1_H

#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"
#include "HGTD_EventTPCnv/HGTD_ALTIROC_RDO_p1.h"
#include "HGTD_RawData/HGTD_ALTIROC_RDO.h"

class MsgStream;

class HGTD_ALTIROC_RDO_Cnv_p1
    : public T_AthenaPoolTPCnvBase<HGTD_ALTIROC_RDO, HGTD_ALTIROC_RDO_p1> {
public:
  HGTD_ALTIROC_RDO_Cnv_p1() = default;

  void persToTrans(const HGTD_ALTIROC_RDO_p1* pers_obj, HGTD_ALTIROC_RDO* trans_obj,
                   MsgStream& log);

  void transToPers(const HGTD_ALTIROC_RDO* trans_obj, HGTD_ALTIROC_RDO_p1* pers_obj,
                   MsgStream& log);
};

#endif // HGTD_EVENTTPCNV_HGTD_ALTIROC_RDO_CNV_P1_H
