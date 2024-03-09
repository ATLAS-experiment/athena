/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TrkVertexWeightCalculators/DecorateVertexScoreAlg.h"

#include "AsgDataHandles/WriteDecorHandle.h"
#include "xAODTracking/Vertex.h"
#include "xAODTracking/VertexContainer.h"

DecorateVertexScoreAlg::DecorateVertexScoreAlg(const std::string& name,
                                               ISvcLocator* svcLoc)
    : EL::AnaAlgorithm(name, svcLoc) {}

StatusCode DecorateVertexScoreAlg::initialize() {
  ATH_CHECK(m_vertexContainer_key.initialize());
  ATH_CHECK(m_vertexScoreDecor_key.initialize());
  ATH_CHECK(m_vertexWeightCalculator.retrieve());
  return StatusCode::SUCCESS;
}

StatusCode DecorateVertexScoreAlg::execute() {
  SG::ReadHandle<xAOD::VertexContainer> vertices{m_vertexContainer_key};
  if (!vertices.isValid()) {
    ATH_MSG_ERROR(m_vertexContainer_key.key() << " not found");
    return StatusCode::FAILURE;
  }

  auto vertexScoreDecor = SG::makeHandle<float>(m_vertexScoreDecor_key);

  ATH_MSG_DEBUG("Decorating " << vertices->size() << " vertices with scores");
  for (const auto* vertex : *vertices) {
    const float score = m_vertexWeightCalculator->estimateSignalCompatibility(*vertex);
    vertexScoreDecor(*vertex) = score;
    ATH_MSG_DEBUG("Decorated vertex with score: " << score);
  }
  return StatusCode::SUCCESS;
}
