// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file DataModelTestDataCommon/src/xAODTestReadJVec.h
 * @author scott snyder <snyder@bnl.gov>
 * @date May, 2024
 * @brief For testing jagged vectors.
 */


#ifndef DATAMODELTESTDATACOMMON_XAODTESTREADJVEC_H
#define DATAMODELTESTDATACOMMON_XAODTESTREADJVEC_H


#include "DataModelTestDataCommon/JVec.h"
#include "DataModelTestDataCommon/JVecContainer.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"


namespace DMTest {


/**
 * @brief Test reading jagged vectors.
 */
class xAODTestReadJVec
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
  /**
   * @brief Dump a JVec object.
   */
  StatusCode dumpJVec (const JVec& jvec) const;


  /**
   * @brief Dump decorations from a JVec object.
   */
  StatusCode dumpDecor (const EventContext& ctx,
                        const JVec& jvec) const;


  /**
   * @brief Dump decorations from a standalone JVec object.
   */
  StatusCode dumpInfoDecor (const EventContext& ctx,
                            const JVec& jvec) const;


  SG::ReadHandleKey<DMTest::JVecContainer> m_jvecContainerKey
  { this, "JVecContainerKey", "jvecContainer", "JVec container key" };

  SG::ReadHandleKey<DMTest::JVec> m_jvecInfoKey
  { this, "JVecInfoKey", "jvecInfo", "Standalone JVec object key" };

  SG::ReadDecorHandleKey<DMTest::JVecContainer> m_jvecDecorKey
  { this, "JVecDecorKey", "jvecContainer.decorJVec", "" };

  SG::ReadDecorHandleKey<DMTest::JVec> m_jvecInfoDecorKey
  { this, "JVecInfoDecorKey", "jvecInfo.decorJVec", "" };
};


} // namespace DMTest


#endif // not DATAMODELTESTDATACOMMON_XAODTESTREADJVEC_H
