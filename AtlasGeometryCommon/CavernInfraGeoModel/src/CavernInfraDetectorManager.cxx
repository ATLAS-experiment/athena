/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "CavernInfraDetectorManager.h"

CavernInfraDetectorManager::CavernInfraDetectorManager()
{
  setName("CavernInfra");
}


CavernInfraDetectorManager::~CavernInfraDetectorManager() = default;


unsigned int CavernInfraDetectorManager::getNumTreeTops() const
{
  return m_treeTops.size();
}

PVConstLink CavernInfraDetectorManager::getTreeTop(unsigned int i) const
{
  if(i<m_treeTops.size())
    return m_treeTops[i];
  else
    return nullptr;
}

void  CavernInfraDetectorManager::addTreeTop(PVLink link) 
{
  m_treeTops.push_back(link);
}

