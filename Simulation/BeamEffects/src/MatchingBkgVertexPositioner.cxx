/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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

  ATH_CHECK(m_vertexContainerKey.initialize());

  return StatusCode::SUCCESS;
}

CLHEP::HepLorentzVector *MatchingBkgVertexPositioner::generate(
    const EventContext &ctx) const {
  // we can currently not reliably determine if the timing information is
  // available
  bool enableTime{};

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
}

}  // namespace Simulation
