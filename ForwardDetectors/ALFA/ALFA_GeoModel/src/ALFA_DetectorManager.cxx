/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "ALFA_GeoModel/ALFA_DetectorManager.h"

ALFA_DetectorManager::ALFA_DetectorManager()
{
  setName("ALFA");
}


ALFA_DetectorManager::~ALFA_DetectorManager() = default;


unsigned int ALFA_DetectorManager::getNumTreeTops() const
{
  return m_volume.size();
}

PVConstLink ALFA_DetectorManager::getTreeTop(unsigned int i) const
{
  return m_volume[i];
}

void  ALFA_DetectorManager::addTreeTop(PVLink vol)
{
  m_volume.push_back(vol);
}



