/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHVIEWS_ATHVIEWS_VIEWMERGEALG_H
#define ATHVIEWS_ATHVIEWS_VIEWMERGEALG_H 1

// STL includes
#include <string>
#include <vector>

// FrameWork includes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "AthViews/View.h"

namespace AthViews {

class ViewMergeAlg
  : public ::AthReentrantAlgorithm
{ 
 public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  // Athena algorithm's Hooks
  virtual StatusCode  initialize() override;
  virtual StatusCode  execute(const EventContext& ctx) const override;
  virtual StatusCode  finalize() override;

 private:
  SG::WriteHandleKey< std::vector<int> > m_w_ints{ this, "MergedInts", "mergedOutput", "Data flow of ints" };
  SG::ReadHandleKey< std::vector<int> > m_r_ints{"dflow_ints"}; //This is not guaranteed to be created, so can't be declared as property
  SG::ReadHandleKey< ViewContainer > m_r_views{ this, "AllViews", "all_views", "All views" };
}; 
} //> end namespace AthViews
#endif //> !ATHVIEWS_ATHVIEWS_VIEWMERGEALG_H
