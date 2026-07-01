///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// DFlowAlg2.h 
// Header file for class DFlowAlg2
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef ATHVIEWS_ATHVIEWS_DFLOWALG2_H
#define ATHVIEWS_ATHVIEWS_DFLOWALG2_H 1

// STL includes
#include <string>

// FrameWork includes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/UpdateHandleKey.h"
#include "AthExHive/HiveDataObj.h"

namespace AthViews {

class DFlowAlg2
  : public ::AthReentrantAlgorithm
{ 
 public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  // Athena algorithm's Hooks
  virtual StatusCode  initialize() override;
  virtual StatusCode  execute(const EventContext& ctx) const override;
  virtual StatusCode  finalize() override;

 private:
  SG::ReadHandleKey<int>  m_r_int{this, "RIntFlow", "dflow_int", "Data flow of int"};
  SG::WriteHandleKey<std::vector<int> > m_ints{this, "IntsFlow", "dflow_ints", "Data flow of integers"};
  SG::UpdateHandleKey< HiveDataObj > m_testUpdate{this, "TestUpdate", "testUpdate", "Test update handle"};
}; 

} //> end namespace AthViews
#endif //> !ATHVIEWS_ATHVIEWS_DFLOWALG2_H
