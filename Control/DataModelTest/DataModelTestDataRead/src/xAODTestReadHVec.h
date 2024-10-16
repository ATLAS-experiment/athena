// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file src/xAODTestReadHVec.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Oct, 2016
 * @brief Algorithm to test reading xAOD data with schema evolution (HVec/HView)
 */


#ifndef DATAMODELTESTDATAREAD_XAODTESTREADHVEC_H
#define DATAMODELTESTDATAREAD_XAODTESTREADHVEC_H


#include "DataModelTestDataRead/HVec.h"
#include "DataModelTestDataRead/HView.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"


namespace DMTest {


/**
 * @brief Algorithm to test reading xAOD data with schema evolution (HVec/HView)
 */
class xAODTestReadHVec
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
  SG::ReadHandleKey<DMTest::HVec> m_hvecKey
  { this, "HVecKey", "hvec" };
  SG::ReadHandleKey<DMTest::HVec> m_hviewKey
  { this, "HViewKey", "hview" };
  SG::WriteHandleKey<DMTest::HVec> m_vecWriteKey
  { this, "VecWriteKey", "" };
  SG::WriteHandleKey<DMTest::HView> m_viewWriteKey
  { this, "ViewWriteKey", "" };
};


} // namespace DMTest


#endif // not DATAMODELTESTDATAREAD_XAODTESTREADHVEC_H
