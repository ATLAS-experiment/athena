/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 */

#include "OverlayVertexSkimmingAlg.h"

#include "EventBookkeeperTools/FilterReporter.h"

OverlayVertexSkimmingAlg::OverlayVertexSkimmingAlg(const std::string& name,
                                                   ISvcLocator* pSvcLocator)
    : ::AthReentrantAlgorithm(name, pSvcLocator) {}

StatusCode OverlayVertexSkimmingAlg::initialize() {
  ATH_CHECK(m_filterParams.initialize());
  ATH_CHECK(m_vertexContainerKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode OverlayVertexSkimmingAlg::finalize() {
  ATH_MSG_INFO(m_filterParams.summary());
  return StatusCode::SUCCESS;
}

StatusCode OverlayVertexSkimmingAlg::execute(const EventContext& ctx) const {
  FilterReporter filter(m_filterParams, false, ctx);

  SG::ReadHandle<xAOD::VertexContainer> vertices(m_vertexContainerKey, ctx);
  if (!vertices.isValid()) {
    ATH_MSG_ERROR("Couldn't retrieve xAOD::VertexContainer with key: "
                  << m_vertexContainerKey);
    return StatusCode::FAILURE;
  }

  for (const xAOD::Vertex* vx : *(vertices.cptr())) {
    if (vx->vertexType() == xAOD::VxType::PriVtx) {
      filter.setPassed(true);
      ATH_MSG_VERBOSE("Found primary vertex, passing event");
      break;
    }
  }

  return StatusCode::SUCCESS;
}
