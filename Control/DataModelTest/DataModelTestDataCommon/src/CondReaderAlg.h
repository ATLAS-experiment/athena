// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/*
 */
// $Id$
/**
 * @file DataModelTestDataCommon/src/CondReaderAlg.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Jul, 2017
 * @brief Testing conditions handling.
 */


#ifndef DATAMODELTESTDATACOMMON_CONDREADERALG_H
#define DATAMODELTESTDATACOMMON_CONDREADERALG_H


#include "DataModelTestDataCommon/S1Cond.h"
#include "DataModelTestDataCommon/S3Cond.h"
#include "xAODEventInfo/EventInfo.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaPoolUtilities/AthenaAttributeList.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/IChronoStatSvc.h"
#include "Gaudi/Property.h"


namespace DMTest {


class CondReaderAlg
  : public AthReentrantAlgorithm
{
public:
  /**
   * @brief Constructor.
   * @param name The algorithm name.
   * @param pSvcLocator The service locator.
   */
  CondReaderAlg (const std::string &name, ISvcLocator *pSvcLocator);


  /**
   * @brief Algorithm initialization; called at the beginning of the job.
   */
  virtual StatusCode initialize() override;


  /**
   * @brief Algorithm event processing.
   */
  virtual StatusCode execute (const EventContext& ctx) const override;


private:
  ServiceHandle<IChronoStatSvc> m_chronoSvc{this, "ChronoSvc", "ChronoStatSvc", "Chrono service"};
  SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{this, "EvtInfo", "EventInfo", "EventInfo name"};
  SG::ReadCondHandleKey<AthenaAttributeList> m_attrListKey{this, "AttrListKey", "/DMTest/TestAttrList", "Attribute list key"};
  SG::ReadCondHandleKey<DMTest::S1> m_scondKey{this, "SCondKey", "scond", "Conditions object key"};
  SG::ReadCondHandleKey<DMTest::S1> m_s2Key{this, "S2Key", "/DMTest/S2", "S2 conditions object key"};
  SG::ReadCondHandleKey<DMTest::S3> m_s3Key{this, "S3Key", "", "S3 conditions object key"};
  SG::ReadCondHandleKey<AthenaAttributeList> m_rltestKey{this, "RLTestKey", "", "Run-lumi attribute list key"};
  SG::ReadCondHandleKey<AthenaAttributeList> m_tstestKey{this, "TSTestKey", "", "Timestamp attribute list key"};
  Gaudi::Property<size_t> m_spins{this, "Spins", 0, "Number of extra spins over conditions access"};
};


} // namespace DMTest


#endif // not DATAMODELTESTDATACOMMON_CONDREADERALG_H
