/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloDetDescr/MbtsDetDescrManager.h"
#include "CaloDetDescr/CaloDetDescrElement.h"

#include "AthenaKernel/getMessageSvc.h"
#include "GaudiKernel/MsgStream.h"

MbtsDetDescrManager::MbtsDetDescrManager()
{
}

MbtsDetDescrManager::~MbtsDetDescrManager()
{
  for (auto& p : m_elements) {
    delete p.second;
  }
}

CaloDetDescrElement* MbtsDetDescrManager::get_element(const Identifier& elementId) const
{
  MbtsElements::const_iterator it = m_elements.find(elementId);
  if(it==m_elements.end())
    return nullptr;
  else
    return it->second;
}

void MbtsDetDescrManager::add(CaloDetDescrElement* element)
{
  MbtsElements::iterator it = m_elements.find(element->identify());
  if(it==m_elements.end()) 
    m_elements[element->identify()] = element;
  else {
    // Issue warning message and don't update map entry
    MsgStream log(Athena::getMessageSvc(), "MbtsDetDescrManager");
    if (log.level() <= MSG::DEBUG) {
      log << MSG::DEBUG << "Attempt to update map entry for ID=" << element->identify().getString() << ". Skipping" << endmsg;
    }
  }
}
