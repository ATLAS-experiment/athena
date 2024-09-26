// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file DataModelTestDataCommon/xAODTestWritePVec.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Sep, 2024
 * @brief Algorithm to test writing xAOD data with packed containers.
 */


#ifndef DATAMODELTESTDATACOMMON_XAODTESTWRITEPVEC_H
#define DATAMODELTESTDATACOMMON_XAODTESTWRITEPVEC_H


#include "DataModelTestDataCommon/PVec.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/WriteHandleKey.h"


namespace DMTest {


/**
 * @brief Algorithm to test writing xAOD data with packed containers.
 */
class xAODTestWritePVec
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
  SG::WriteHandleKey<DMTest::PVec> m_pvecKey
  { this, "PVecKey", "pvec", };
};


} // namespace DMTest


#endif // not DATAMODELTESTDATACOMMON_XAODTESTWRITEPVEC_H
