/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENASERVICES_TEST_METADATATOOLSTUB_H
#define ATHENASERVICES_TEST_METADATATOOLSTUB_H

#include "AthenaKernel/IMetaDataTool.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include <string>
#include <vector>

/**
 * @brief IMetaDataTool stub for MetaDataSvc_test.
 *
 * Records every call as "begin:<sid>", "end:<sid>" or "stop" so that the test
 * can check which tools MetaDataSvc called, and in which order. Failures are
 * steered by properties. The stub does not log ERRORs itself, so that all
 * ERRORs in the test log come from MetaDataSvc.
 */
class MetaDataToolStub : public extends<AthAlgTool, IMetaDataTool> {
public:
  using base_class::base_class;

  virtual StatusCode beginInputFile(const SG::SourceID& sid) override {
    m_calls.push_back("begin:" + sid);
    return sid == m_failSourceID.value() ? StatusCode::FAILURE : StatusCode::SUCCESS;
  }
  virtual StatusCode endInputFile(const SG::SourceID& sid) override {
    m_calls.push_back("end:" + sid);
    return sid == m_failSourceID.value() ? StatusCode::FAILURE : StatusCode::SUCCESS;
  }
  virtual StatusCode metaDataStop() override {
    m_calls.push_back("stop");
    return m_failMetaDataStop ? StatusCode::FAILURE : StatusCode::SUCCESS;
  }

  /// Calls received so far, in order.
  const std::vector<std::string>& calls() const { return m_calls; }

private:
  Gaudi::Property<std::string> m_failSourceID{this, "FailSourceID", "badGuid",
    "beginInputFile/endInputFile fail for this source ID"};
  Gaudi::Property<bool> m_failMetaDataStop{this, "FailMetaDataStop", false,
    "metaDataStop fails"};
  std::vector<std::string> m_calls;
};

#endif
