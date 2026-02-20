/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENAPOOLEXAMPLEALGORITHMS_REWRITEDATA_H
#define ATHENAPOOLEXAMPLEALGORITHMS_REWRITEDATA_H

/** @file ReWriteData.h
 *  @brief This file contains the class definition for the ReWriteData class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 *  $Id: ReWriteData.h,v 1.1 2008-12-10 21:28:11 gemmeren Exp $
 **/

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaPoolExampleData/ExampleHitContainer.h"
#include "AthenaPoolExampleData/ExampleTrackContainer.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

namespace AthPoolEx {

/** @class AthPoolEx::ReWriteData
 *  @brief This class provides an example for reading and writing data objects from/to Pool.
 **/
class ReWriteData : public AthReentrantAlgorithm {
 public:
  ReWriteData(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~ReWriteData() = default;

 public:
  /// Gaudi Service Interface method implementations:
  virtual StatusCode initialize() override final;
  virtual StatusCode execute(const EventContext& ctx) const override final;
  virtual StatusCode finalize() override final;

 private:
  SG::ReadHandleKey<ExampleHitContainer> m_exampleHitKey{this, "ExampleHitKey", "MyHits"};
  SG::WriteHandleKey<ExampleTrackContainer> m_exampleTrackKey{this, "ExampleTrackKey", "MyTracks"};
};

}  // namespace AthPoolEx

#endif
