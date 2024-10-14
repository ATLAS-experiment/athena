// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file src/xAODTestWrite.h
 * @author scott snyder <snyder@bnl.gov>
 * @date May, 2014
 * @brief Algorithm to test writing xAOD classes with auxiliary data.
 */


#ifndef DATAMODELTESTDATAWRITE_XAODTESTWRITE_H
#define DATAMODELTESTDATAWRITE_XAODTESTWRITE_H


#include "DataModelTestDataCommon/CVec.h"
#include "DataModelTestDataCommon/CVecWithData.h"
#include "DataModelTestDataWrite/GVec.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"


namespace DMTest {


/**
 * @brief Algorithm for creating test aux data.
 */
class xAODTestWrite
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
  /// Test writing container with additional data.
  StatusCode write_cvec_with_data (unsigned int count,
                                   const EventContext& ctx) const;

  SG::ReadHandleKey<DMTest::CVec> m_cvecKey
    { this, "CVecKey", "cvec", "" };
  SG::WriteHandleKey<DMTest::CVec> m_ctrigKey
    { this, "CTrigKey", "ctrig", "" };
  SG::WriteHandleKey<DMTest::GVec> m_gvecKey
    { this, "GVecKey", "gvec", "" };
  SG::WriteHandleKey<DMTest::CVecWithData> m_cvecWDKey
    { this, "CVecWDKey", "cvecWD", "" };
};


} // namespace DMTest


#endif // not DATAMODELTESTDATAWRITE_XAODTESTWRITE_H
