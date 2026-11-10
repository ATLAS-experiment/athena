// This file's extension implies that it's C, but it's really -*- C++ -*-.

/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */

// $Id$
/**
 * @file DataModelTestDataCommon/src/xAODTestReadSymlinkTool.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Jun, 2017
 * @brief Test reading via a symlink, as a tool.
 */


#ifndef DATAMODELTESTDATACOMMON_XAODTESTREADSYMLINKTOOL_H
#define DATAMODELTESTDATACOMMON_XAODTESTREADSYMLINKTOOL_H


#include "DataModelTestDataCommon/ITestTool.h"
#include "DataModelTestDataCommon/S1.h"
#include "DataModelTestDataCommon/S2.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "StoreGate/ReadHandleKey.h"
#include "AthContainers/AuxElement.h"


namespace DMTest {


class xAODTestReadSymlinkTool
  : public extends<AthAlgTool, ITestTool>
{
public:
  /**
   * @brief Constructor.
   * @param type The tool type.
   * @param name The tool name.
   * @param parent The tool's parent.
   */
  xAODTestReadSymlinkTool (const std::string& type,
                           const std::string& name,
                           const IInterface* parent);


  /**
   * @brief Tool initialization; called at the beginning of the job.
   */
  virtual StatusCode initialize() override;


  /**
   * @brief Tool event processing.
   */
  virtual StatusCode doit (const EventContext& ctx) const override;


private:
  SG::ReadHandleKey<SG::AuxElement> m_objKey{this, "Key", "", "Object key"};

  SG::ReadHandleKey<DMTest::S1> m_s1Key{this, "S1Key", "S2", "S1 key"};
  SG::ReadHandleKey<DMTest::S2> m_s2Key{this, "S2Key", "S2", "S2 key"};
  SG::ReadHandleKey<DMTest::S2> m_aliasKey{this, "AliasKey", "S2alias", "S2 alias key"};
};


} // namespace DMTest


#endif // not DATAMODELTESTDATACOMMON_XAODTESTREADSYMLINKTOOL_H
