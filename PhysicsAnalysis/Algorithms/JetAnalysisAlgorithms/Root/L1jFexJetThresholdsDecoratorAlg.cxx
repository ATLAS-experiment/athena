/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "JetAnalysisAlgorithms/L1jFexJetThresholdsDecoratorAlg.h"

#include <AsgDataHandles/ReadHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>

#include <string>
#include <vector>

namespace CP
{

  StatusCode L1jFexJetThresholdsDecoratorAlg::initialize()
  {
    ANA_CHECK(m_l1JetsKey.initialize());
    ANA_CHECK(m_trigConfigTool.retrieve());

    m_thresholdsDecorKey = m_l1JetsKey.key() + "." + m_decorationName.value();
    ANA_CHECK(m_thresholdsDecorKey.initialize());
    return StatusCode::SUCCESS;
  }

  StatusCode L1jFexJetThresholdsDecoratorAlg::execute(const EventContext& ctx) {
    ANA_CHECK(m_jfexThresholdTable.update(m_trigConfigTool,
                                          m_l1ThresholdType.value(), ctx,
                                          msg()));

    SG::ReadHandle<xAOD::jFexSRJetRoIContainer> l1Jets(m_l1JetsKey, ctx);
    if (!l1Jets.isValid()) {
      ANA_MSG_ERROR("Failed to retrieve " << m_l1JetsKey.key());
      return StatusCode::FAILURE;
    }

    SG::WriteDecorHandle<xAOD::jFexSRJetRoIContainer, std::vector<std::string>>
        thresholdsDec(m_thresholdsDecorKey, ctx);

    for (const xAOD::jFexSRJetRoI* roi : *l1Jets) {
      thresholdsDec(*roi) = m_jfexThresholdTable.decode(*roi);
    }

    return StatusCode::SUCCESS;
  }
}
