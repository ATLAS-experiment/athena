/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "TrkVertexWeightCalculators/VertexWeightCalculator.h"

#include "xAODTracking/Vertex.h"
#include "xAODTracking/VertexContainer.h"
#include "StoreGate/ReadDecorHandle.h"

// class constructor implementation
VertexWeightCalculator::VertexWeightCalculator(const std::string& t,
                                                     const std::string& n,
                                                     const IInterface* p)
  : AthAlgTool(t, n, p)
{
  declareInterface<IVertexWeightCalculator>(this);
}

VertexWeightCalculator::~VertexWeightCalculator() = default;

StatusCode VertexWeightCalculator::initialize() {
  ATH_MSG_DEBUG("Initializing " << name() << "...");

  ATH_CHECK(m_ScoreKey.initialize());

  return StatusCode::SUCCESS;
}

double VertexWeightCalculator::estimateSignalCompatibility(
    const xAOD::Vertex &vertex) const {

  const EventContext &ctx = Gaudi::Hive::currentContext();
  SG::ReadDecorHandle<xAOD::VertexContainer, float> acc_score(m_ScoreKey,
                                                                 ctx);
  return acc_score(vertex);
}
