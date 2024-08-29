// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 */
/**
 * @file DataModelTestDataCommon/src/xAODTestWritePLinks.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2023
 * @brief Test writing packed links.
 */


#ifndef DATAMODELTESTDATACOMMON_XAODTESTWRITEPLINKS_H
#define DATAMODELTESTDATACOMMON_XAODTESTWRITEPLINKS_H


#include "DataModelTestDataCommon/CVec.h"
#include "DataModelTestDataCommon/PLinks.h"
#include "DataModelTestDataCommon/PLinksContainer.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"


namespace DMTest {


/**
 * @brief Test writing packed links.
 */
class xAODTestWritePLinks
  : public AthReentrantAlgorithm
{
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;


  using EL = ElementLink<CVec>;


  /**
   * @brief Algorithm initialization; called at the beginning of the job.
   */
  virtual StatusCode initialize() override;


  /**
   * @brief Algorithm event processing.
   */
  virtual StatusCode execute (const EventContext& ctx) const override;


private:
  EL makeLink (const std::string& key1,
               const CVec& cvec1,
               const std::string& key2,
               const CVec& cvec2,
               size_t ndx) const;


  StatusCode fillPLinks (const std::string& key1,
                         const CVec& cvec1,
                         const std::string& key2,
                         const CVec& cvec2,
                         size_t ndx,
                         PLinks& plinks) const;

  StatusCode decorPLinks (const EventContext& ctx,
                          const std::string& key1,
                          const CVec& cvec1,
                          const std::string& key2,
                          const CVec& cvec2) const;

  SG::ReadHandleKey<DMTest::CVec> m_cvecKey
  { this, "CVecKey", "cvec", "CVec object to target by links" };
  SG::ReadHandleKey<DMTest::CVec> m_ctrigKey
  { this, "CTrigKey", "ctrig", "Another CVec object to target by links" };

  SG::WriteHandleKey<DMTest::PLinksContainer> m_plinksContainerKey
  { this, "PLinksContainerKey", "plinksContainer", "PLinks container object to create" };

  SG::WriteHandleKey<DMTest::PLinks> m_plinksInfoKey
  { this, "PLinksInfoKey", "plinksInfo", "Standalone PLinks object to create" };

  SG::WriteDecorHandleKey<DMTest::PLinksContainer> m_plinksDecorLinkKey
  { this, "PLinksDecorLinkKey", "plinksContainer.decorLink", "" };

  SG::WriteDecorHandleKey<DMTest::PLinksContainer> m_plinksDecorVLinksKey
  { this, "PLinksDecorVLinksKey", "plinksContainer.decorVLinks", "" };

  SG::WriteDecorHandleKey<DMTest::PLinks> m_plinksInfoDecorLinkKey
  { this, "PLinksInfoDecorLinkKey", "plinksInfo.decorLink", "" };

  SG::WriteDecorHandleKey<DMTest::PLinks> m_plinksInfoDecorVLinksKey
  { this, "PLinksInfoDecorVLinksKey", "plinksInfo.decorVLinks", "" };
};


} // namespace DMTest


#endif // not DATAMODELTESTDATACOMMON_XAODTESTWRITEPLINKS_H
