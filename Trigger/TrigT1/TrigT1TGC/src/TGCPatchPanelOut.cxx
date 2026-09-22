/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigT1TGC/TGCPatchPanelOut.h"
#include "TrigT1TGC/TGCPatchPanel.h"
#include "TrigT1TGC/TGCHitPattern.h"
#include <cstdlib>
#include <iostream>

namespace LVL1TGCTrigger {



void
TGCPatchPanelOut::swap(TGCPatchPanelOut& other) noexcept
{
  using std::swap;
  swap(m_bid, other.m_bid);
  swap(m_origin, other.m_origin);
  swap(m_signalPattern, other.m_signalPattern);
}

TGCPatchPanelOut::TGCPatchPanelOut(const TGCPatchPanelOut& right)
  : m_bid{right.m_bid}, m_origin{right.m_origin}
{
  for (std::size_t i = 0; i < m_signalPattern.size(); ++i) {
    if (right.m_signalPattern[i]) {
      m_signalPattern[i] =
        std::make_unique<TGCHitPattern>(*right.m_signalPattern[i]);
    }
  }
}

TGCPatchPanelOut&
TGCPatchPanelOut::operator=(const TGCPatchPanelOut& right)
{
  TGCPatchPanelOut tmp{right};
  swap(tmp);
  return *this;
}

void TGCPatchPanelOut::deleteHitPattern(int i)
{
  m_signalPattern[i].reset();
}

void TGCPatchPanelOut::print() const
{
#ifdef TGCCOUT
  std::cout <<"PatchPanelOut: bid= "<<m_bid<<" PPID= "<<m_origin->getId()
       <<" PPType= "<<m_origin->getType()<<" PPRegion= "<<m_origin->getRegion()<<std::endl;
#endif
  int i;
  for( i=0; i<NumberOfConnectorPerPPOut; i++){
    if(m_signalPattern[i]!=0){
#ifdef TGCCOUT
      std::cout << "Connector"<<i<<std::endl;
#endif
      m_signalPattern[i]->print();
    }
  }
}



} //end of namespace bracket
