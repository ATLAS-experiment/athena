///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// PerfMonTestLeakyAlg.h 
// Header file for class PerfMonTest::LeakyAlg
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef PERFMONTESTS_PERFMONTESTLEAKYALG_H 
#define PERFMONTESTS_PERFMONTESTLEAKYALG_H 

// STL includes
#include <string>
#include <list>
#include <vector>


// FrameWork includes
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"

namespace PerfMonTest {

class LeakyAlg : public AthAlgorithm
{ 

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
 public: 
  using AthAlgorithm::AthAlgorithm;

  /// Destructor: 
  virtual ~LeakyAlg(); 

  // Athena algorithm's Hooks
  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;


  /////////////////////////////////////////////////////////////////// 
  // Private data: 
  /////////////////////////////////////////////////////////////////// 
 private: 

  /// Property to setup the size of the leak
  Gaudi::Property<int> m_leakSize{this, "LeakSize", 10, "Size of 'Leak' objects to be leaked each event"};

  struct Leak {
    std::vector<int> m_data;
  };

  /// nbr of Leak objects
  Gaudi::Property<int> m_nbrLeaks{this, "NbrLeaks", 1, "Number of 'Leak' objects to be leaked each event"};

  /// container to hold the leaked objects
  std::list<Leak*> m_leaks;
}; 


} //> end namespace PerfMonTest

#endif //> PERFMONTESTS_PERFMONTESTLEAKYALG_H
