/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// PerfMonTestPolyVectorAlg.h 
/// Example for the memory optimization tutorial
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef PERFMONTESTS_PERFMONTESTPOLYVECTORALG_H 
#define PERFMONTESTS_PERFMONTESTPOLYVECTORALG_H 

#include <map>
#include "AthenaBaseComps/AthAlgorithm.h"

namespace PerfMonTest {
class IHit;
class PolyVectorAlg : public AthAlgorithm
{ 

 public:
  using AthAlgorithm::AthAlgorithm;

  virtual StatusCode execute() override;

 private: 
  /// Property to setup the size of the Hit container
  Gaudi::Property<int> m_vectorSize{this, "VectorSize", 1024*1024, "the size of the Hit container"};
  /// Property to setup the amount of elements to reserve
  Gaudi::Property<int> m_2bReserved{this, "ToBeReserved", 1024*1024, "the number of elements to be reserved"};
  /// Property to set DHIT/FHIT ratio
  Gaudi::Property<int> m_mixture{this, "Mixture", 1, "equal to the ratio DHIT/FHIT - 1 (default 1 == all DHits)"};
  /// Property to introduce some fragmentation
  Gaudi::Property<bool> m_mapIt{this, "MapIt", false, "add current hit to a map"};
  std::map<int,IHit*> m_mixMap;
};
} //> end namespace PerfMonTest

#endif //> PERFMONTESTS_PERFMONTESTPPOLYVECTORALG_H

