/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "TrkVertexWeightCalculators/GNNVertexWeightCalculator.h"

#include "xAODTracking/Vertex.h"
#include "xAODTracking/VertexContainer.h"
#include "StoreGate/ReadDecorHandle.h"

// class constructor implementation
GNNVertexWeightCalculator::GNNVertexWeightCalculator(const std::string& t,
                                                     const std::string& n,
                                                     const IInterface* p)
  : AthAlgTool(t, n, p)
{
  declareInterface<IVertexWeightCalculator>(this);
}

GNNVertexWeightCalculator::~GNNVertexWeightCalculator() = default;

StatusCode GNNVertexWeightCalculator::initialize() {
  ATH_MSG_DEBUG("Initializing " << name() << "...");

  ATH_CHECK(m_gnnScoreKey.initialize());

  return StatusCode::SUCCESS;
}

double GNNVertexWeightCalculator::estimateSignalCompatibility(
    const xAOD::Vertex &vertex) const {

  const EventContext &ctx = Gaudi::Hive::currentContext();
  SG::ReadDecorHandle<xAOD::VertexContainer, float> acc_gnnScore(m_gnnScoreKey,
                                                                 ctx);

  if (acc_gnnScore.isAvailable()) {
    return acc_gnnScore(vertex);
  } else {
    return -1;
  }

}
