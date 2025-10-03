///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// PerfMonTestCpuCruncherAlg.h 
// Header file for class PerfMonTest::CpuCruncherAlg
// Author: S.Binet<binet@cern.ch>, A. S. Mete<amete@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef PERFMONTESTS_PERFMONTESTCPUCRUNCHERALG_H 
#define PERFMONTESTS_PERFMONTESTCPUCRUNCHERALG_H 

// STL includes
#include <string>
#include <random>

// FrameWork includes
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"

namespace PerfMonTest {

class CpuCruncherAlg : public AthAlgorithm
{ 

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
 public: 
  using AthAlgorithm::AthAlgorithm;

  // Athena algorithm's Hooks
  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;

  // Perform math operations to burn CPU for a number of iterations
  double burn(unsigned long nIterations);

  /////////////////////////////////////////////////////////////////// 
  // Private data: 
  /////////////////////////////////////////////////////////////////// 
 private: 

  /// Property to setup the mean (in ms) of CPU time to consume
  Gaudi::Property<float> m_meanCpuTime{this, "MeanCpu", 100., "Mean (in ms) of CPU time to consume."};

  /// Property to setup the RMS  (in ms) of CPU time to consume
  Gaudi::Property<float> m_rmsCpuTime{this, "RmsCpu", 5., "RMS (in ms) of CPU time to consume."};

  /// Random number setup
  std::default_random_engine m_random;
  std::normal_distribution<double> m_distribution;

}; 

} //> end namespace PerfMonTest

#endif //> PERFMONTESTS_PERFMONTESTCPUCRUNCHERALG_H
