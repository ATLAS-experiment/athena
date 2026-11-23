/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


#ifndef LARRECCONDITIONS_LARFCALDEGFACTORS
#define LARRECCONDITIONS_LARFCALDEGFACTORS

#include <map>
#include <vector>

class CondAttrListCollection;
class MsgStream;

class LArFcalDegFactors {

 public:

  static constexpr unsigned short nBins   = 63;  // FCAL1 ieta bins
  static constexpr unsigned short nValues = 10;  // mu bins per ieta bin

  LArFcalDegFactors() = delete;
  LArFcalDegFactors(const CondAttrListCollection* attrList,  MsgStream &msg);

  typedef std::map<unsigned short, std::vector<float> >  payload_t;
  //map key: FCAL1 ieta bin number, value degradation factors for 10 mu bins

  const payload_t &get() const { return m_fcalDegradationMap; } 

 private:
  payload_t m_fcalDegradationMap;
};

#include "AthenaKernel/CLASS_DEF.h" 
CLASS_DEF(LArFcalDegFactors, 96802586 ,1)
#include "AthenaKernel/CondCont.h"
CONDCONT_DEF(LArFcalDegFactors, 44852944);

#endif
