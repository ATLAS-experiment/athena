///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// PerfMonTestManyLeaksAlg.h 
// Header file for class PerfMonTest::ManyLeaksAlg
// Author: R.Sesuter
/////////////////////////////////////////////////////////////////// 
#ifndef PERFMONTESTS_PERFMONTESTMANYLEAKSALG_H 
#define PERFMONTESTS_PERFMONTESTMANYLEAKSALG_H 

// STL includes
#include <string>
#include <list>
#include <vector>


// FrameWork includes
#include "AthenaBaseComps/AthAlgorithm.h"
#include "CxxUtils/checker_macros.h"

namespace PerfMonTest {

class ManyLeaksAlg : public AthAlgorithm
{
 public:
  /// Constructor:
  using AthAlgorithm::AthAlgorithm;

  // Athena algorithm's Hooks
  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;

 private:

  long* stillReachableFct(long** array);
  
  /// this one's possible lost
  long* possibleLostFct(long** array);
  
  /// this one's indirectly lost
  long* indirectlyLostFct(long** array);
  
  /// this one's definitely lost
  long* definitelyLostFct(long** array);
  
  /// this one's definitely lost
  void leakAll();
  
  /// Property to setup the size of the leak
  Gaudi::Property<int> m_leakSize{this, "LeakSize", 10, "Number of longs to be leaked just once"};
  
  /// Property to setup the location of the leak, in initialize (true) or execute (false)
  Gaudi::Property<bool> m_leakInInit{this, "LeakInInit", false, "Where it will leak: initialize or execute(default)"};
  
 private:
  
  // this one's still reachable
  long* m_stillReachable{};
  
  // this one's possible lost
  long* m_possibleLost{};
  
  // this one's indirectly lost
  long* m_indirectlyLost{};
  
  // this one's definitely lost
  long* m_definitelyLost{};
  
  // we still need to reference some pointers,
  // otherwise vagrind labels everything definitely lost
  static long **m_pointers ATLAS_THREAD_SAFE;
}; 
  
} //> end namespace PerfMonTest

#endif //> PERFMONTESTS_PERFMONTESTMANYLEAKSALG_H
