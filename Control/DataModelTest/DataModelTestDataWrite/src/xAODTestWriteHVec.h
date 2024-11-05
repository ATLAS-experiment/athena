// This file's extension implies that it's C, but it's really -*- C++ -*-.

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file DataModelTestDataWrite/src/xAODTestWriteHVec.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Oct, 2016
 * @brief Algorithm to test writing xAOD classes for schema evolution (hvec/hview).
 */


#ifndef DATAMODELTESTDATAWRITE_XAODTESTWRITEHVEC_H
#define DATAMODELTESTDATAWRITE_XAODTESTWRITEHVEC_H


#include "DataModelTestDataWrite/HVec.h"
#include "DataModelTestDataWrite/HView.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadHandleKey.h"


namespace DMTest {


/**
 * @brief Algorithm to test writing xAOD classes for schema evolution (hvec/hview).
 */
class xAODTestWriteHVec
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
  SG::WriteHandleKey<DMTest::HVec> m_hvecKey
  { this, "HVecKey", "hvec", "" };
  SG::WriteHandleKey<DMTest::HView> m_hviewKey
  { this, "HViewKey", "hview", "" };
};


} // namespace DMTest


#endif // not DATAMODELTESTDATAWRITE_XAODTESTWRITEHVEC_H
