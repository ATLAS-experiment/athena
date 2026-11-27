/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


#ifndef LARRECCONDITIONS_LARFCALDEGFACTORS
#define LARRECCONDITIONS_LARFCALDEGFACTORS

#include <array>
#include <map>
#include <vector>
#include "LArRecConditions/FCalAmplitudeDegradation.h"

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

  /// mu points of the columns of the table (L. Kurchaninov, Note-FCalSig)
  static constexpr std::array<double, nValues> muGrid{20., 40., 60., 80., 100., 120., 140., 160., 180., 200.};

  /// true if the table was read correctly
  bool isValid() const { return m_degradation.nChannels() > 0; }
  /// number of FCal1 channels (eta bins) in the table
  size_t nChannels() const { return m_degradation.nChannels(); }
  /// amplitude factor D for FCal1 eta bin and mu (linear interpolation in mu)
  double factor(size_t channel, double mu) const { return m_degradation.factor(channel, mu); }

 private:
  payload_t m_fcalDegradationMap;
  FCalAmplitudeDegradation m_degradation; // same table, with the interpolation used by the tools
};

#include "AthenaKernel/CLASS_DEF.h" 
CLASS_DEF(LArFcalDegFactors, 96802586 ,1)
#include "AthenaKernel/CondCont.h"
CONDCONT_DEF(LArFcalDegFactors, 44852944);

#endif
