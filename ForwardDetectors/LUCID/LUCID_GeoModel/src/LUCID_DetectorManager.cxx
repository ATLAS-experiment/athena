/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "LUCID_GeoModel/LUCID_DetectorManager.h"

LUCID_DetectorManager::LUCID_DetectorManager() {

  setName("LUCID");
}

LUCID_DetectorManager::~LUCID_DetectorManager() = default;

unsigned int LUCID_DetectorManager::getNumTreeTops() const {

  return m_volume.size();
}

PVConstLink LUCID_DetectorManager::getTreeTop(unsigned int i) const {

  return m_volume[i];
}

void  LUCID_DetectorManager::addTreeTop(PVLink vol) {

  m_volume.push_back(vol);
}
