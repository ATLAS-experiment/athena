///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// PerfMonTestErroneousAlg.h 
// Header file for class PerfMonTest::ErroneousAlg
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef PERFMONTESTS_PERFMONTESTERRONEOUSALG_H 
#define PERFMONTESTS_PERFMONTESTERRONEOUSALG_H 

// STL includes
#include <string>
#include <list>
#include <vector>


// FrameWork includes
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"

namespace PerfMonTest {

class ErroneousAlg : public AthAlgorithm
{ 

 public:
  using AthAlgorithm::AthAlgorithm;

  virtual StatusCode execute() override;

 private:

  /// three member functions which will exhibit faulty behaviour
  bool jumpOnUninitializedValue();
  bool invalidRead();
  bool mismatchedFree();
  
  bool shouldIJump(bool shouldIJump)
  {
    return not shouldIJump;
  }

}; 


} //> end namespace PerfMonTest

#endif //> PERFMONTESTS_PERFMONTESTERRONEOUSALG_H
