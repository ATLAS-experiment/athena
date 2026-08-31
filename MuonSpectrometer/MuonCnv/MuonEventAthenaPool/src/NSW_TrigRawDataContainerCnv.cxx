/*
 * Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#include "NSW_TrigRawDataContainerCnv.h"

NSW_TrigRawDataContainerCnv::NSW_TrigRawDataContainerCnv(ISvcLocator* svcLocator) : NSW_TrigRawDataContainerCnvBase(svcLocator) { }

NSW_TrigRawDataContainer_PERS* NSW_TrigRawDataContainerCnv::createPersistent(Muon::NSW_TrigRawDataContainer* transCont) {
  MsgStream log(msgSvc(), "NSW_TrigRawDataContainerCnv");
  if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << "NSW_TrigRawDataContainerCnv::createPersistent()" << endmsg;
  return m_TPConverter_p1.createPersistent(transCont, log);
}

Muon::NSW_TrigRawDataContainer* NSW_TrigRawDataContainerCnv::createTransient(const Token* token) {
  MsgStream log(msgSvc(), "NSW_TrigRawDataContainerCnv");
  if (log.level() <= MSG::DEBUG) log << MSG::DEBUG << "NSW_TrigRawDataContainerCnv::createTransient(const Token* token)" << endmsg;
  // UUID of the NSW_TrigRawDataContainer_p1 representation, created by uuidgen command
  static const Guid p1_guid("5D25FB79-BFE3-44DC-9EEC-8A93CE7776B3");

  Muon::NSW_TrigRawDataContainer *transCont = nullptr;
  if(compareClassGuid(token, p1_guid)) {
    std::unique_ptr<Muon::NSW_TrigRawDataContainer_p1> pContainer( this->poolReadObject<Muon::NSW_TrigRawDataContainer_p1>(token) );
    transCont = m_TPConverter_p1.createTransient(pContainer.get(), log);
  } else throw std::runtime_error("No persistent version match for GUID NSW_TrigRawData RDO container");
  return transCont;
}
