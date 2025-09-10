/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef LARDEADOTXFROMSC_H
#define LARDEADOTXFROMSC_H

#include "Identifier/HWIdentifier.h"
#include "AthenaKernel/CLASS_DEF.h"

#include <vector>
#include <map>

class LArDeadOTXFromSC
{
  public:
  void clear(){m_FEBFromSC.clear();}

  bool isThisOTXdead(HWIdentifier febid) const{
    return m_FEBFromSC.find(febid)!=m_FEBFromSC.end() ;
  }
  const std::vector<float>& correctionFromThisOTXdead(HWIdentifier febid) const{
    return m_FEBFromSC.at(febid);
  }
  void addFEB(HWIdentifier febid, const std::vector<float> & vec){
     if (!this->isThisOTXdead(febid)) m_FEBFromSC[febid]=vec;
     return;
  }

  private:
  std::map<HWIdentifier, std::vector<float> > m_FEBFromSC;
};

CLASS_DEF( LArDeadOTXFromSC , 192664128, 0 )
#endif
