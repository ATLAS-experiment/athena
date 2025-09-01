/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TGC_EVENT_H
#define TGC_EVENT_H

#include <vector>
#include "TrigT1TGC/TGCReadoutIndex.h"

namespace LVL1TGCTrigger {

class TGCASDOut;

class TGCEvent {
 protected:
  int m_eventNumber{0};
  std::vector<TGCASDOut*> m_vecASDOut;

 public:
  TGCEvent() = default;
  ~TGCEvent() = default;
 
  TGCEvent(const TGCEvent& right)
  {
    *this= right;
  }
 
  const TGCEvent& operator=(const TGCEvent& right)
  {
    m_eventNumber= right.m_eventNumber;
    m_vecASDOut= right.m_vecASDOut;

    return *this;
  }
 
  // set functions
  void SetEventNumber(int num) { m_eventNumber= num; }

  TGCASDOut* NewASDOut(const TGCReadoutIndex& tgcindex, 
		       TGCSignalType sigtype=WIRE, int id=-1, 
		       double tof=0);

  // get functions
  int GetEventNumber() const { return m_eventNumber; }

  // ASDOut ...
  int GetNASDOut() const { return m_vecASDOut.size(); }

  const TGCASDOut* GetASDOut(int index) const
  {
    if(index<=0 || index> std::ssize(m_vecASDOut) ) return 0;
    else return m_vecASDOut[index-1];
  }

  const std::vector<TGCASDOut*>& GetASDOutVector() const
  {
    return m_vecASDOut;
  }

  // operations
  void Clear();          // clear event
  void Print() const;    // print out event information
};


} //end of namespace bracket

#endif
