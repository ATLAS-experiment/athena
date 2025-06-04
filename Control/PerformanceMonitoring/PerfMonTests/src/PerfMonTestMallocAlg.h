/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// PerfMonTestMallocAlg.h 
// Header file for class PerfMonTest::MallocAlg
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef PERFMONTESTS_PERFMONTESTMALLOCALG_H 
#define PERFMONTESTS_PERFMONTESTMALLOCALG_H 

// STL includes
#include <string>


// FrameWork includes
#include "AthenaBaseComps/AthAlgorithm.h"

// Forward declaration

namespace PerfMonTest {

class MallocAlg : public AthAlgorithm
{ 

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
 public: 
  using AthAlgorithm::AthAlgorithm;

  virtual StatusCode execute() override;

 private: 

  virtual void isUsed(const void*) {}

  /// event number at which to actually do stuff
  Gaudi::Property<unsigned int> m_evtNbr{this, "EvtNbr", 10, "event number at which to actually do stuff"};

  /// current event number
  unsigned int m_currentEvtNbr{0};

  /// switch between using a C-array and a std::vector
  Gaudi::Property<bool> m_useStdVector{this, "UseStdVector", false, "switch between using a C-array and a std::vector"};

};

} //> end namespace PerfMonTest

#endif //> PERFMONTESTS_PERFMONTESTMALLOCALG_H
