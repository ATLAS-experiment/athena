/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "LArReadoutGeometry/LArDetectorManager.h"
#include <algorithm>

LArDetectorManager::LArDetectorManager (const EMBDetectorManager* emb
					, const EMECDetectorManager* emec
					, const HECDetectorManager* hec
					, const FCALDetectorManager* fcal)
  : m_embManager(emb)
  , m_emecManager(emec)
  , m_hecManager(hec)
  , m_fcalManager(fcal)
  , m_isTestBeam(true)
{
  setName("LArMgr");
}

PVConstLink LArDetectorManager::getTreeTop (unsigned int i) const
{
  return m_treeTop[i];
}

unsigned int LArDetectorManager::getNumTreeTops () const
{
  return m_treeTop.size();
}

void LArDetectorManager::addTreeTop (PVConstLink treeTop)
{
  if (std::find(m_treeTop.begin(),m_treeTop.end(),treeTop)!=m_treeTop.end())  return;
  m_treeTop.push_back(treeTop);
}

