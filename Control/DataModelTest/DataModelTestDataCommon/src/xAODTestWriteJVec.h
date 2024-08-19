// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file DataModelTestDataCommon/src/xAODTestWriteJVec.h
 * @author scott snyder <snyder@bnl.gov>
 * @date May, 2024
 * @brief For testing jagged vectors.
 */


#ifndef DATAMODELTESTDATACOMMON_XAODTESTWRITEJVEC_H
#define DATAMODELTESTDATACOMMON_XAODTESTWRITEJVEC_H


#include "DataModelTestDataCommon/JVec.h"
#include "DataModelTestDataCommon/JVecContainer.h"
#include "DataModelTestDataCommon/CVec.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"


namespace DMTest {


/**
 * @brief Test writing jagged vectors.
 */
class xAODTestWriteJVec
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
  StatusCode fillJVec (const std::string& key1, const CVec& cvec1,
                       size_t ndx, JVec& jvec) const;


  StatusCode decorJVec (const EventContext& ctx) const;

  SG::ReadHandleKey<DMTest::CVec> m_cvecKey
  { this, "CVecKey", "cvec", "CVec object to target by links" };

  SG::WriteHandleKey<DMTest::JVecContainer> m_jvecContainerKey
  { this, "JVecContainerKey", "jvecContainer", "JVec container object to create" };

  SG::WriteHandleKey<DMTest::JVec> m_jvecInfoKey
  { this, "JVecInfoKey", "jvecInfo", "Standalone JVec object to create" };

  SG::WriteDecorHandleKey<DMTest::JVecContainer> m_jvecDecorKey
  { this, "JVecDecorKey", "jvecContainer.decorJVec", "" };

  SG::WriteDecorHandleKey<DMTest::JVec> m_jvecInfoDecorKey
  { this, "JVecInfoDecorKey", "jvecInfo.decorJVec", "" };
};


} // namespace DMTest


#endif // not DATAMODELTESTDATACOMMON_XAODTESTWRITEJVEC_H
