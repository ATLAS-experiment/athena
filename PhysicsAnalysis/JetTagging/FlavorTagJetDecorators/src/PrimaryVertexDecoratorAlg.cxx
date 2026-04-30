/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "PrimaryVertexDecoratorAlg.h"

#include "StoreGate/WriteDecorHandle.h"

#include "xAODTracking/VertexContainer.h"
#include "xAODTracking/TrackingPrimitives.h"

#include <cmath>
#include <limits>

namespace FlavorTagJetDecorators {

  PrimaryVertexDecoratorAlg::PrimaryVertexDecoratorAlg(
    const std::string& name, ISvcLocator* loc)
    : AthReentrantAlgorithm(name, loc) {}

  StatusCode PrimaryVertexDecoratorAlg::initialize() {
    ATH_MSG_DEBUG("Initializing " << name() << "...");

    ATH_CHECK(m_vertexContainerKey.initialize());
    ATH_CHECK(m_eventInfoKey.initialize());
    ATH_CHECK(m_dec_nPrimaryVertices.initialize());
    ATH_CHECK(m_dec_primaryVertexZ.initialize());

    return StatusCode::SUCCESS;
  }

  StatusCode PrimaryVertexDecoratorAlg::execute(
      const EventContext& ctx) const {
    ATH_MSG_DEBUG("Executing " << name() << "...");

    constexpr float NaN = std::numeric_limits<float>::quiet_NaN();

    SG::ReadHandle<xAOD::VertexContainer> vertices(
      m_vertexContainerKey, ctx);
    ATH_CHECK(vertices.isValid());

    auto eventInfo = SG::makeHandle(m_eventInfoKey, ctx);
    ATH_CHECK(eventInfo.isValid());

    SG::WriteDecorHandle<xAOD::EventInfo, int> dec_nPV(
      m_dec_nPrimaryVertices, ctx);
    SG::WriteDecorHandle<xAOD::EventInfo, float> dec_pvZ(
      m_dec_primaryVertexZ, ctx);

    if (vertices->empty()) {
      ATH_MSG_WARNING("Empty PrimaryVertices container — writing defaults");
      dec_nPV(*eventInfo) = 0;
      dec_pvZ(*eventInfo) = NaN;
      return StatusCode::SUCCESS;
    }

    // Find the primary vertex: first with vertexType == PriVtx,
    // fallback to front().  Matches TDD's primary() function exactly.
    const xAOD::Vertex* pv = nullptr;
    for (const auto* vertex : *vertices) {
      if (vertex->vertexType() == xAOD::VxType::PriVtx) {
        pv = vertex;
        break;
      }
    }
    if (!pv) {
      pv = vertices->front();
    }

    dec_nPV(*eventInfo) = static_cast<int>(vertices->size());
    dec_pvZ(*eventInfo) = static_cast<float>(pv->z());

    ATH_MSG_DEBUG("nPrimaryVertices=" << vertices->size()
                  << " primaryVertexZ=" << pv->z());

    return StatusCode::SUCCESS;
  }

}  // namespace FlavorTagJetDecorators
