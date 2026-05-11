///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// DFlowAlg1.h 
// Header file for class DFlowAlg1
// Author: S.Binet<binet@cern.ch>
/////////////////////////////////////////////////////////////////// 
#ifndef ATHVIEWS_ATHVIEWS_DFLOWALG1_H
#define ATHVIEWS_ATHVIEWS_DFLOWALG1_H 1

// STL includes
#include <string>

// FrameWork includes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "AthExHive/HiveDataObj.h"

namespace AthViews {

class DFlowAlg1
  : public ::AthReentrantAlgorithm
{ 
 public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  // Athena algorithm's Hooks
  virtual StatusCode  initialize() override;
  virtual StatusCode  execute(const EventContext& ctx) const override;
  virtual StatusCode  finalize() override;

 private:
  SG::ReadHandleKey<int> m_r_int{this, "IntFlow", "view_start", "Data flow of int"};
  SG::WriteHandleKey<int> m_w_int{this, "ViewStart", "dflow_int", "Seed data of view"};
  SG::WriteHandleKey<HiveDataObj> m_testUpdate{this, "TestUpdate", "testUpdate", "Test update handle"};
}; 

} //> end namespace AthViews
#endif //> !ATHVIEWS_ATHVIEWS_DFLOWALG1_H
