///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// DFlowAlg3.h 
// Header file for class DFlowAlg3
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef ATHVIEWS_ATHVIEWS_DFLOWALG3_H
#define ATHVIEWS_ATHVIEWS_DFLOWALG3_H 1

// STL includes
#include <string>

// FrameWork includes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "AthExHive/HiveDataObj.h"
#include "AthExHive/CondDataObj.h"

namespace AthViews {

class DFlowAlg3
  : public ::AthReentrantAlgorithm
{ 
 public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  // Athena algorithm's Hooks
  virtual StatusCode  initialize() override;
  virtual StatusCode  execute(const EventContext& ctx) const override;
  virtual StatusCode  finalize() override;

 private:
  SG::ReadHandleKey<int>  m_r_int{this, "RIntFlow", "dflow_int", "Data flow of int (read)"};
  SG::ReadHandleKey<std::vector<int> > m_r_ints{this, "RIntsFlow", "dflow_ints", "Data flow of integers (read)"};
  SG::WriteHandleKey<int> m_w_dflowDummy{this, "DFlowDummy", "dflow_dummy", "Dummy object to fix dependencies"};
  SG::ReadHandleKey<HiveDataObj> m_testUpdate{this, "TestUpdate", "testUpdate", "Test update handle"};
  SG::ReadCondHandleKey<CondDataObj> m_condKeyTest{ this, "TestConditionsData", "testConditionsData", "" };

}; 

} //> end namespace AthViews
#endif //> !ATHVIEWS_ATHVIEWS_DFLOWALG3_H
