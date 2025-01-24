/**
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_EventAthenaPool/src/HGTD_ALTIROC_RDO_ContainerCnv.cxx
 * @author Alexander Leopold <alexander.leopold@cern.ch>
 * @author Rodrigo Estevam de Paula <rodrigo.estevam.de.paula@cern.ch>
 * @brief
 */

#include "HGTD_ALTIROC_RDO_ContainerCnv.h"
#include "GaudiKernel/MsgStream.h"

#include <memory>

HGTD_ALTIROC_RDO_ContainerCnv::HGTD_ALTIROC_RDO_ContainerCnv(ISvcLocator* svcloc)
  : HGTD_ALTIROC_RDO_ContainerCnvBase_t(svcloc) {}

HGTD_ALTIROC_RDO_Container* HGTD_ALTIROC_RDO_ContainerCnv::createTransient() {

  static const pool::Guid p1_guid(
    "2EE1819F-7EFE-ACB8-5D5F-9EFBFAEBC11E"); // with HGTD_ALTIROC_RDO_p1
  ATH_MSG_DEBUG("createTransient(): main converter");

  HGTD_ALTIROC_RDO_Container* trans_cont(0);
  if (compareClassGuid(p1_guid)) {
    ATH_MSG_DEBUG("createTransient(): T/P version 1 detected");
    std::unique_ptr<HGTD_ALTIROC_RDO_Container_PERS_t> pers_cont(
      poolReadObject<HGTD_ALTIROC_RDO_Container_PERS_t>());

    trans_cont = m_converter.createTransient(pers_cont.get(), msg());

  } else {
    throw std::runtime_error(
      "Unsupported persistent version of HGTD_ALTIROC_RDO_Container");
  }
  return trans_cont;
}

HGTD_ALTIROC_RDO_Container_PERS_t*
HGTD_ALTIROC_RDO_ContainerCnv::createPersistent(HGTD_ALTIROC_RDO_Container* trans_cont) {

  HGTD_ALTIROC_RDO_Container_PERS_t* pers_cont =
    m_converter.createPersistent(trans_cont, msg());

  return pers_cont;
}
