// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file DataModelTestDataCommon/src/xAODTestThinJVec.h
 * @author scott snyder <snyder@bnl.gov>
 * @date May, 2024
 * @brief Thin JVecContainer objects.
 */


#ifndef DATAMODELTESTDATACOMMON_XAODTESTTHINJVEC_H
#define DATAMODELTESTDATACOMMON_XAODTESTTHINJVEC_H


#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "DataModelTestDataCommon/JVecContainer.h"
#include "StoreGate/ThinningHandleKey.h"


namespace DMTest {


/**
 * @brief Thin JVecContainer objects.
 */
class xAODTestThinJVec
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
  SG::ThinningHandleKey<DMTest::JVecContainer> m_jvecContainerKey
  { this, "JVecContainerKey", "jvecContainer", "Object being thinned" };

  StringProperty m_stream
  { this, "Stream", "STREAM", "Stream for which to apply thinning" };

  UnsignedIntegerProperty m_mask
  { this, "Mask", 0, "Mask to apply to event number when thinning" };
};


} // namespace DMTest



#endif // not DATAMODELTESTDATACOMMON_XAODTESTTHINJVEC_H
