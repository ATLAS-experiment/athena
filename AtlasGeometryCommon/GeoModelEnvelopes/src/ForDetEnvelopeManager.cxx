/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "GeoModelEnvelopes/ForDetEnvelopeManager.h"

ForDetEnvelopeManager::ForDetEnvelopeManager()
{
  setName("ForDetEnvelope");
}


ForDetEnvelopeManager::~ForDetEnvelopeManager() = default;


unsigned int ForDetEnvelopeManager::getNumTreeTops() const
{
  return m_volume.size();
}

PVConstLink ForDetEnvelopeManager::getTreeTop(unsigned int i) const
{
  return m_volume[i];
}

void  ForDetEnvelopeManager::addTreeTop(const PVLink& vol)
{
  m_volume.push_back(vol);
}



