/*
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#include "TVAAugmentationTool.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "StoreGate/WriteDecorHandle.h"

namespace DerivationFramework {

  TVAAugmentationTool::TVAAugmentationTool(
      const std::string& t,
      const std::string& n,
      const IInterface* p):
    base_class(t, n, p)
  {
    declareProperty("LinkName", m_linkName, "The name of the output links");
    declareProperty("TrackName", m_trackName="InDetTrackParticles");
    declareProperty("VertexName", m_vertexName="PrimaryVertices");
    declareProperty("TVATool", m_tool);
  }

  StatusCode TVAAugmentationTool::initialize()
  {
    ATH_MSG_INFO("Initialising TVAAugmentationTool " << name() );
    ATH_CHECK( m_tool.retrieve() );

    m_vtxDec_key = m_trackName + "." + m_linkName;
    ATH_CHECK(m_vtxDec_key.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode TVAAugmentationTool::addBranches() const
  {
    const EventContext& ctx = Gaudi::Hive::currentContext();
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, vtxLink_t> vtxDec_handle(m_vtxDec_key, ctx);

    const xAOD::VertexContainer* vertices{};
    ATH_CHECK(evtStore()->retrieve(vertices, m_vertexName) ); // FIXME Use Handles
    const xAOD::TrackParticleContainer* tracks{};
    ATH_CHECK(evtStore()->retrieve(tracks, m_trackName) ); // FIXME Use Handles

    xAOD::TrackVertexAssociationMap matchMap = m_tool->getMatchMap(*tracks, *vertices);

    for (const xAOD::Vertex* ivtx : *vertices)
      for (const xAOD::TrackParticle* itrk : matchMap[ivtx])
        vtxDec_handle(*itrk).toContainedElement(*vertices, ivtx);

    return StatusCode::SUCCESS;
  }
} //> end namespace DerivationFramework
