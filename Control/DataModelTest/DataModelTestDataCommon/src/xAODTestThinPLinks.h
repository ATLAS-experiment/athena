// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file DataModelTestDataCommon/src/xAODTestThinPLinks.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Mar, 2024
 * @brief Thin PLinksContainer objects.
 */


#ifndef DATAMODELTESTDATACOMMON_XAODTESTTHINPLINKS_H
#define DATAMODELTESTDATACOMMON_XAODTESTTHINPLINKS_H


#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "DataModelTestDataCommon/PLinksContainer.h"
#include "StoreGate/ThinningHandleKey.h"


namespace DMTest {


/**
 * @brief Thin PLinksContainer objects.
 */
class xAODTestThinPLinks
  : public AthReentrantAlgorithm
{
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;


  /**
   * @brief Algorithm initialization; called at the beginning of the job.
   */
  virtual StatusCode initialize() override;


  /**
   * @brief Algorithm event processing.
   */
  virtual StatusCode execute (const EventContext& ctx) const override;


private:
  SG::ThinningHandleKey<DMTest::PLinksContainer> m_plinksContainerKey
  { this, "PLinksContainerKey", "plinksContainer", "Object being thinned" };

  StringProperty m_stream
  { this, "Stream", "STREAM", "Stream for which to apply thinning" };

  UnsignedIntegerProperty m_mask
  { this, "Mask", 0, "Mask to apply to event number when thinning" };
};


} // namespace DMTest



#endif // not DATAMODELTESTDATACOMMON_XAODTESTTHINPLINKS_H
