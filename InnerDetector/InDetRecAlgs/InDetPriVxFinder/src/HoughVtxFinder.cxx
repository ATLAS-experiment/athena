/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetPriVxFinder/HoughVtxFinder.h"


StatusCode HoughVtxFinder::initialize() {
  ATH_MSG_DEBUG("Inside HoughVtxFinder::initialize()");

  ATH_CHECK(m_inputPixelSpacePoints.initialize());
  ATH_CHECK(m_outputHoughVtx.initialize());

  ATH_CHECK(m_VertexFinderTool.retrieve());

  return StatusCode::SUCCESS;
}

StatusCode HoughVtxFinder::execute(const EventContext &ctx) const {
  const xAOD::SpacePointContainer* inputPixelSP{};
  ATH_CHECK(SG::get(inputPixelSP, m_inputPixelSpacePoints, ctx));

  auto HoughVtx = m_VertexFinderTool->findVertex(ctx, *inputPixelSP);

  SG::WriteHandle<xAOD::VertexContainer> vtxContainer(m_outputHoughVtx, ctx);
  ATH_CHECK(vtxContainer.record(std::move(HoughVtx.first),
                                std::move(HoughVtx.second)));

  return StatusCode::SUCCESS;
}
