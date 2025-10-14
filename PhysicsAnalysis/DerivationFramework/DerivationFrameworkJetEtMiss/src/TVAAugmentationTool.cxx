/*
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#include "TVAAugmentationTool.h"
#include "StoreGate/WriteDecorHandle.h"

namespace DerivationFramework {

  TVAAugmentationTool::TVAAugmentationTool(
      const std::string& t,
      const std::string& n,
      const IInterface* p):
    base_class(t, n, p)
  {
  }

  StatusCode TVAAugmentationTool::initialize()
  {
    ATH_MSG_DEBUG("Initialising TVAAugmentationTool " << name() );
    ATH_CHECK( m_trackName.initialize() );
    ATH_CHECK( m_vertexName.initialize() );
    ATH_CHECK(m_vtxDec_key.initialize());
    ATH_CHECK( m_tool.retrieve() );

    return StatusCode::SUCCESS;
  }

  StatusCode TVAAugmentationTool::addBranches() const
  {
    const EventContext& ctx = Gaudi::Hive::currentContext();
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, vtxLink_t> vtxDec_handle(m_vtxDec_key, ctx);

    SG::ReadHandle<xAOD::VertexContainer> vertices{m_vertexName, ctx};
    SG::ReadHandle<xAOD::TrackParticleContainer> tracks{m_trackName, ctx};

    xAOD::TrackVertexAssociationMap matchMap = m_tool->getMatchMap(*tracks, *vertices);

    for (const xAOD::Vertex* ivtx : *vertices)
      for (const xAOD::TrackParticle* itrk : matchMap[ivtx])
        vtxDec_handle(*itrk).toContainedElement(*vertices, ivtx);

    return StatusCode::SUCCESS;
  }
} //> end namespace DerivationFramework
