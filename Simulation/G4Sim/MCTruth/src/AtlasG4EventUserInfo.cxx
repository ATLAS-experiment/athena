/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MCTruth/AtlasG4EventUserInfo.h"

HepMC::GenEvent* AtlasG4EventUserInfo::GetHepMCEvent()
{
  return m_theEvent;
}

void AtlasG4EventUserInfo::SetHepMCEvent(HepMC::GenEvent* ev)
{
  m_theEvent=ev;
}
