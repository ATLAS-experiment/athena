/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "InferenceAlg.h"

StatusCode MuonML::InferenceAlg::initialize() {
    ATH_CHECK(m_inferenceTools.retrieve());
    return StatusCode::SUCCESS;
}

StatusCode MuonML::InferenceAlg::execute(const EventContext& ctx) const {
  // Fresh, per-event graph workspace for THIS alg instance
    MuonML::GraphRawData graphData{};

    // This alg has one tool, but loop is fine.
    for (const auto& tool : m_inferenceTools) {
        ATH_CHECK(tool->runGraphInference(ctx, graphData));
    }
    return StatusCode::SUCCESS;
}