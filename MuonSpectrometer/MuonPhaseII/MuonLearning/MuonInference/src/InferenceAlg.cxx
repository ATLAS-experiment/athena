/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "InferenceAlg.h"

#include "MuonInferenceInterfaces/GraphData.h"

namespace MuonML{
    StatusCode InferenceAlg::initialize() {
        if (m_inferenceTools.empty()) {
            ATH_MSG_ERROR("Provide at least one inference tool");
            return StatusCode::FAILURE;
        }
        ATH_CHECK(m_inferenceTools.retrieve());
        return StatusCode::SUCCESS;
    }
    StatusCode InferenceAlg::execute(const EventContext& ctx) const {
        GraphRawData graphData{};
        for (const auto& infTool : m_inferenceTools) {
            ATH_CHECK(infTool->runGraphInference(ctx, graphData));
        }
        return StatusCode::SUCCESS;
    }
}