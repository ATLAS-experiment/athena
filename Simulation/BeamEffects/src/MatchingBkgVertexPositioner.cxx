/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MatchingBkgVertexPositioner.h"

#include "CLHEP/Vector/LorentzVector.h"
#include "StoreGate/ReadHandle.h"

namespace Simulation {

MatchingBkgVertexPositioner::MatchingBkgVertexPositioner(const std::string &t,
                                                         const std::string &n,
                                                         const IInterface *p)
    : base_class(t, n, p) {}

StatusCode MatchingBkgVertexPositioner::initialize() {
  ATH_MSG_VERBOSE("Initializing ...");

  ATH_CHECK(m_vertexContainerKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_mcEventKey.initialize(SG::AllowEmpty));

  if ( m_vertexContainerKey.key().empty() && m_mcEventKey.key().empty()) {
    ATH_MSG_ERROR("Both, vertex and McEventCollection keys are empty, no source of vertex position");
    return StatusCode::FAILURE;
  }
  if ( !m_vertexContainerKey.key().empty() && !m_mcEventKey.key().empty()) {
    ATH_MSG_ERROR("Both, vertex and McEventCollection keys are not empty, ambiguous source of vertex position");
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

CLHEP::HepLorentzVector *MatchingBkgVertexPositioner::generate(
    const EventContext &ctx) const {
  // we can currently not reliably determine if the timing information is
  // available
  constexpr bool enableTime{false};
  if(!m_vertexContainerKey.key().empty()) {
    SG::ReadHandle<xAOD::VertexContainer> vertices(m_vertexContainerKey, ctx);
    if (!vertices.isValid()) {
      ATH_MSG_ERROR("Couldn't retrieve xAOD::VertexContainer with key: "
                    << m_vertexContainerKey);
      return nullptr;
    }

    for (const xAOD::Vertex *vx : *(vertices.cptr())) {
      if (vx->vertexType() == xAOD::VxType::PriVtx) {
        ATH_MSG_INFO("Using primary vertex with position and time: "
                    << vx->position().x() << " " << vx->position().y() << " "
                    << vx->position().z() << " "
                    << (enableTime && vx->hasValidTime() ? vx->time() : 0));
        return new CLHEP::HepLorentzVector(
            vx->position().x(), vx->position().y(), vx->position().z(),
            enableTime && vx->hasValidTime() ? vx->time() : 0);
      }
    }

    ATH_MSG_ERROR("No primary vertex found in xAOD::VertexContainer with key: "
                  << m_vertexContainerKey);
    return nullptr;
  } else if (!m_mcEventKey.key().empty()) {
    SG::ReadHandle<McEventCollection> mcEvent(m_mcEventKey, ctx);
    for ( auto event: *mcEvent) {
      for ( auto vertex: event->vertices()) {
        const float x = vertex->position().x();  
        const float y = vertex->position().y();
        const float z = vertex->position().z();

        ATH_MSG_DEBUG("bkg gen vertex position " << vertex->position() << " " << vertex->particles_in_size());
        return new CLHEP::HepLorentzVector(x, y, z, 0);
      }
    }
  }
  return nullptr;
}

}  // namespace Simulation
