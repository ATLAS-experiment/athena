/**
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 *
 * @fileHGTD_EventTPCnv/src/HGTD_ALTIROC_RDO_Cnv_p1.cxx
 * @author Alexander Leopold <alexander.leopold@cern.ch>
 * @author Rodrigo Estevam de Paula <rodrigo.estevam.de.paula@cern.ch>
 * @date December, 2024
 * @brief
 */

#include "HGTD_EventTPCnv/HGTD_ALTIROC_RDO_Cnv_p1.h"

void HGTD_ALTIROC_RDO_Cnv_p1::transToPers(const HGTD_ALTIROC_RDO* trans_obj,
                                  HGTD_ALTIROC_RDO_p1* pers_obj,
                                  MsgStream& log) {
  log << MSG::VERBOSE << "calling HGTD_ALTIROC_RDO_Cnv_p1::transToPers" << endmsg;

  pers_obj->m_rdo_id = trans_obj->identify().get_compact();
  pers_obj->m_word = trans_obj->getWord();
}

void HGTD_ALTIROC_RDO_Cnv_p1::persToTrans(const HGTD_ALTIROC_RDO_p1* pers_obj,
                                  HGTD_ALTIROC_RDO* trans_obj,
                                  MsgStream& log) {
  log << MSG::VERBOSE << "calling HGTD_ALTIROC_RDO_Cnv_p1::persToTrans" << endmsg;
  *trans_obj = HGTD_ALTIROC_RDO(Identifier(pers_obj->m_rdo_id), pers_obj->m_word);
}
