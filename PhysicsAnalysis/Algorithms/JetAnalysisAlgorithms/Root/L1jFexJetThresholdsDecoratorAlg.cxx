/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "JetAnalysisAlgorithms/L1jFexJetThresholdsDecoratorAlg.h"

#include <AthContainers/ConstAccessor.h>
#include <AsgDataHandles/ReadHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>

#include <cstdint>
#include <exception>
#include <string>
#include <vector>

#include "TrigConfData/L1Menu.h"
#include "TrigConfData/L1Threshold.h"

namespace CP
{

  L1jFexJetThresholdsDecoratorAlg::L1jFexJetThresholdsDecoratorAlg(
      const std::string& name, ISvcLocator* svcLoc)
    : EL::AnaAlgorithm(name, svcLoc) {}


  StatusCode L1jFexJetThresholdsDecoratorAlg::initialize()
  {
    ANA_CHECK(m_l1JetsKey.initialize());
    ANA_CHECK(m_trigConfigTool.retrieve());

    m_thresholdsDecorKey = m_l1JetsKey.key() + "." + m_decorationName.value();
    ANA_CHECK(m_thresholdsDecorKey.initialize());
    return StatusCode::SUCCESS;
  }

  StatusCode L1jFexJetThresholdsDecoratorAlg::rebuildJfexThresholdTable(
      const EventContext& ctx) {
    const TrigConf::L1Menu* l1menu = nullptr;
    try {
      l1menu = &m_trigConfigTool->l1Menu(ctx);
    } catch (const std::exception& e) {
      ANA_MSG_ERROR("Could not read the L1 menu in execute(): "
                    << e.what()
                    << ". The Phase-I jFEX threshold table "
                       "cannot be built. Ensure TrigConf::xAODConfigSvc has "
                       "loaded the L1 menu for this input file.");
      return StatusCode::FAILURE;
    }

    if (m_thresholdNamesLoaded && l1menu->name() == m_cachedL1MenuName) {
      return StatusCode::SUCCESS;
    }
    if (m_thresholdNamesLoaded) {
      ANA_MSG_INFO("L1 menu changed from '"
                   << m_cachedL1MenuName << "' to '" << l1menu->name()
                   << "' - rebuilding jFEX threshold table");
    }
    const auto& thresholds = l1menu->thresholds(m_l1ThresholdType.value());
    if (thresholds.empty()) {
      ANA_MSG_ERROR("L1 menu '" << l1menu->name()
                                << "' has no thresholds of type '"
                                << m_l1ThresholdType.value() << "'");
      return StatusCode::FAILURE;
    }
    m_jfexThresholdNames.clear();
    for (const auto& thr : thresholds) {
      const unsigned int bit = thr->mapping();
      if (bit >= m_jfexThresholdNames.size())
        m_jfexThresholdNames.resize(bit + 1);
      m_jfexThresholdNames[bit] = thr->name();
    }
    ANA_MSG_INFO("Loaded " << m_jfexThresholdNames.size()
                           << " jFEX threshold names from L1 menu '"
                           << l1menu->name() << "'");
    m_cachedL1MenuName = l1menu->name();
    m_thresholdNamesLoaded = true;
    return StatusCode::SUCCESS;
  }

  StatusCode L1jFexJetThresholdsDecoratorAlg::execute(const EventContext& ctx) {
    ANA_CHECK(rebuildJfexThresholdTable(ctx));

    SG::ReadHandle<xAOD::jFexSRJetRoIContainer> l1Jets(m_l1JetsKey, ctx);
    if (!l1Jets.isValid()) {
      ANA_MSG_ERROR("Failed to retrieve " << m_l1JetsKey.key());
      return StatusCode::FAILURE;
    }

    static const SG::AuxElement::ConstAccessor<uint64_t>
        thrPatternsAcc("thresholdPatterns");
    SG::WriteDecorHandle<xAOD::jFexSRJetRoIContainer, std::vector<std::string>>
        thresholdsDec(m_thresholdsDecorKey, ctx);

    for (const xAOD::jFexSRJetRoI* roi : *l1Jets) {
      std::vector<std::string> passed;
      if (thrPatternsAcc.isAvailable(*roi)) {
        const uint64_t pat = thrPatternsAcc(*roi);
        passed.reserve(m_jfexThresholdNames.size());
        for (size_t b = 0; b < m_jfexThresholdNames.size(); ++b) {
          if (((pat >> b) & 1ULL) && !m_jfexThresholdNames[b].empty()) {
            passed.push_back(m_jfexThresholdNames[b]);
          }
        }
      }
      thresholdsDec(*roi) = std::move(passed);
    }

    return StatusCode::SUCCESS;
  }
}
