/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 IdDictDetDescr package
 -----------------------------------------
 ***************************************************************************/


#include "IdDictDetDescr/IdDictManager.h"
#include "Identifier/IdHelper.h"


IdDictManager::IdDictManager()
    :
    m_mgr(0)
{}

IdDictManager::IdDictManager(const IdDictMgr& mgr)
    :
    m_mgr(&mgr)
{}


IdDictManager::~IdDictManager()
{}

const IdDictMgr*
IdDictManager::manager			(void) const
{
    return (m_mgr);
}


int
IdDictManager::initializeHelper	(IdHelper& helper) const
{
    if (m_mgr) return (helper.initialize_from_dictionary(*m_mgr));
    return (1); // otherwise error
}


