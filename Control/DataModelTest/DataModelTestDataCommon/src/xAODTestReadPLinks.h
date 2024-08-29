// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file DataModelTestDataCommon/xAODTestReadPLinks.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Dec, 2023
 * @brief Test reading packed links.
 */


#ifndef DATAMODELTESTDATACOMMON_XAODTESTREADPLINKS_H
#define DATAMODELTESTDATACOMMON_XAODTESTREADPLINKS_H


#include "DataModelTestDataCommon/CVec.h"
#include "DataModelTestDataCommon/PLinks.h"
#include "DataModelTestDataCommon/PLinksContainer.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"


namespace DMTest {


/**
 * @brief Test reading packed links.
 */
class xAODTestReadPLinks
  : public AthReentrantAlgorithm
{
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;


  typedef ElementLink<DMTest::CVec> EL;
  typedef DataLink<DMTest::CVec> DL;


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
   * @brief Dump a PLinks object.
   */
  StatusCode dumpPLinks (const PLinks& plinks) const;


  /**
   * @brief Dump decorations from a PLinks object.
   */
  StatusCode dumpDecor (const EventContext& ctx,
                        const PLinks& plinks) const;


  /**
   * @brief Dump decorations from a standalone PLinks object.
   */
  StatusCode dumpInfoDecor (const EventContext& ctx,
                            const PLinks& plinks) const;


  SG::ReadHandleKey<DMTest::PLinksContainer> m_plinksContainerKey
  { this, "PLinksContainerKey", "plinksContainer", "PLinks container key" };

  SG::ReadHandleKey<DMTest::PLinks> m_plinksInfoKey
  { this, "PLinksInfoKey", "plinksInfo", "Standalone PLinks object key" };

  SG::ReadDecorHandleKey<DMTest::PLinksContainer> m_plinksDecorLinkKey
  { this, "PLinksDecorLinkKey", "plinksContainer.decorLink", "" };

  SG::ReadDecorHandleKey<DMTest::PLinksContainer> m_plinksDecorVLinksKey
  { this, "PLinksDecorVLinksKey", "plinksContainer.decorVLinks", "" };

  SG::ReadDecorHandleKey<DMTest::PLinks> m_plinksInfoDecorLinkKey
  { this, "PLinksInfoDecorLinkKey", "plinksInfo.decorLink", "" };

  SG::ReadDecorHandleKey<DMTest::PLinks> m_plinksInfoDecorVLinksKey
  { this, "PLinksInfoDecorVLinksKey", "plinksInfo.decorVLinks", "" };
};


} // namespace DMTest


#endif // not DATAMODELTESTDATACOMMON_XAODTESTREADPLINKS_H
