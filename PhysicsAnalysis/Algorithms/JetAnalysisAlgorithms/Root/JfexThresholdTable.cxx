/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "JetAnalysisAlgorithms/JfexThresholdTable.h"

#include <AthContainers/ConstAccessor.h>

#include <cstdint>
#include <exception>

#include "TrigConfData/L1Menu.h"
#include "TrigConfData/L1Threshold.h"

namespace CP
{
  StatusCode JfexThresholdTable::update(
      ToolHandle<TrigConf::ITrigConfigTool>& trigConfigTool,
      const std::string& thresholdType, const EventContext& ctx,
      MsgStream& msg) {
    const TrigConf::L1Menu* l1menu = nullptr;
    try {
      l1menu = &trigConfigTool->l1Menu(ctx);
    } catch (const std::exception& e) {
      msg << MSG::ERROR << "Could not read the L1 menu in execute(): "
          << e.what()
          << ". The Phase-I jFEX threshold table "
             "cannot be built. Ensure TrigConf::xAODConfigSvc has "
             "loaded the L1 menu for this input file." << endmsg;
      return StatusCode::FAILURE;
    }

    if (m_loaded && l1menu->name() == m_menuName) {
      return StatusCode::SUCCESS;
    }
    if (m_loaded) {
      msg << MSG::INFO << "L1 menu changed from '"
          << m_menuName << "' to '" << l1menu->name()
          << "' - rebuilding jFEX threshold table" << endmsg;
    }
    const auto& thresholds = l1menu->thresholds(thresholdType);
    if (thresholds.empty()) {
      msg << MSG::ERROR << "L1 menu '" << l1menu->name()
          << "' has no thresholds of type '" << thresholdType << "'"
          << endmsg;
      return StatusCode::FAILURE;
    }
    m_names.clear();
    for (const auto& thr : thresholds) {
      const unsigned int bit = thr->mapping();
      if (bit >= m_names.size())
        m_names.resize(bit + 1);
      m_names[bit] = thr->name();
    }
    msg << MSG::INFO << "Loaded " << m_names.size()
        << " jFEX threshold names from L1 menu '"
        << l1menu->name() << "'" << endmsg;
    m_menuName = l1menu->name();
    m_loaded = true;
    return StatusCode::SUCCESS;
  }

  std::vector<std::string> JfexThresholdTable::decode(
      const xAOD::jFexSRJetRoI& roi) const {
    std::vector<std::string> passed;
    static const SG::ConstAccessor<uint64_t> thrPatternsAcc("thresholdPatterns");
    if (!thrPatternsAcc.isAvailable(roi)) return passed;
    const uint64_t pat = thrPatternsAcc(roi);
    passed.reserve(m_names.size());
    for (size_t b = 0; b < m_names.size(); ++b) {
      if (((pat >> b) & 1ULL) && !m_names[b].empty()) {
        passed.push_back(m_names[b]);
      }
    }
    return passed;
  }
}
