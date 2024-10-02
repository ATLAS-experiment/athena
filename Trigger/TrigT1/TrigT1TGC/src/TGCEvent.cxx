/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <iostream> 

#include "TrigT1TGC/TGCEvent.h"
#include "TrigT1TGC/TGCASDOut.h"

namespace LVL1TGCTrigger {

TGCASDOut* TGCEvent::NewASDOut(const TGCReadoutIndex& tgcrindex, 
			       TGCSignalType sigtype, int id, double tof)
{
  TGCASDOut* asdout= new TGCASDOut(tgcrindex, sigtype, id, tof);
  m_vecASDOut.push_back(asdout);
  return asdout;
}


void TGCEvent::Clear()
{
  // delete ASDOut
  for (const TGCASDOut* p : m_vecASDOut) {
    delete p;
  }
  m_vecASDOut.clear();
}


void TGCEvent::Print() const
{
  std::cout << " Event#= " << m_eventNumber << std::endl;
}


}   // end of namespace
