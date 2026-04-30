/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagInference/PassThroughModelSvc.h"
#include "FlavorTagInference/PassThroughSaltModel.h"
#include "FlavorTagInference/ISaltModel.h"
#include "PathResolver/PathResolver.h"

#include "nlohmann/json.hpp"
#include <fstream>
#include <stdexcept>

namespace FlavorTagInference {

  StatusCode PassThroughModelSvc::initialize() {
    if (m_jsonFile.value().empty()) {
      ATH_MSG_ERROR("JsonFile property is empty");
      return StatusCode::FAILURE;
    }

    // Resolve the JSON file path
    std::string resolved = PathResolverFindCalibFile(m_jsonFile);
    if (resolved.empty()) {
      ATH_MSG_ERROR("Could not resolve JSON file: " << m_jsonFile);
      return StatusCode::FAILURE;
    }

    // Load and parse the JSON
    std::ifstream file(resolved);
    if (!file.is_open()) {
      ATH_MSG_ERROR("Could not open JSON file: " << resolved);
      return StatusCode::FAILURE;
    }

    nlohmann::json config;
    try {
      file >> config;
    } catch (const nlohmann::json::parse_error& e) {
      ATH_MSG_ERROR("JSON parse error in " << resolved << ": " << e.what());
      return StatusCode::FAILURE;
    }

    // Create the PassThroughSaltModel
    try {
      auto model = std::make_shared<const PassThroughSaltModel>(config);
      ISaltModelPtr salt_model = model;

      // Create the GNN wrapper with variable remapping so that
      // TracksLoader uses the correct aux variable names (e.g.
      // GhostTrack instead of BTagTrackToJetAssociator)
      GNNOptions opts;
      opts.variable_remapping = m_variableRemapping;
      m_gnn = std::make_shared<const GNN>(salt_model, opts);
    } catch (const std::exception& e) {
      ATH_MSG_ERROR("Failed to create PassThroughSaltModel: " << e.what());
      return StatusCode::FAILURE;
    }

    ATH_MSG_INFO("PassThroughModelSvc initialized from " << resolved);
    return StatusCode::SUCCESS;
  }

  std::shared_ptr<const GNN> PassThroughModelSvc::get(
    const std::string& /*nn_name*/,
    const GNNOptions& opts)
  {
    // If options differ from defaults, create a new GNN sharing
    // the underlying model but with new options
    GNNOptions default_opts;
    if (!(opts == default_opts)) {
      return std::make_shared<const GNN>(*m_gnn, opts);
    }
    return m_gnn;
  }

} // namespace FlavorTagInference
