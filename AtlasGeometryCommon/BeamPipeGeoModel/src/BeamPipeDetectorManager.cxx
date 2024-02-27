/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "BeamPipeGeoModel/BeamPipeDetectorManager.h"

BeamPipeDetectorManager::BeamPipeDetectorManager()
{
  setName("BeamPipe");
}


BeamPipeDetectorManager::~BeamPipeDetectorManager() = default;

unsigned int BeamPipeDetectorManager::getNumTreeTops() const
{
  return m_volume.size();
}

PVConstLink BeamPipeDetectorManager::getTreeTop(unsigned int i) const
{
  return m_volume[i];
}

void  BeamPipeDetectorManager::addTreeTop(PVConstLink vol)
{
  m_volume.push_back(vol);
}



