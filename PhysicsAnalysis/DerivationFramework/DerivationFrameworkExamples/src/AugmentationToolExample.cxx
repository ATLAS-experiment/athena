/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// AugmentationToolExample.cxx
///////////////////////////////////////////////////////////////////
// Author: Thomas Gillam (thomas.gillam@cern.ch)
//
// This is a trivial example of tool that creates extra information
// and places it in the StoreGate.
// The example shows how to do it with (a) simple vectors and (b) dressing
// of objects (tracks in this case).
// The same information is written into both.

#include "AugmentationToolExample.h"
#include "StoreGate/WriteDecorHandle.h"
#include <vector>
#include <string>

namespace DerivationFramework {

  StatusCode AugmentationToolExample::initialize()
  {
    ATH_CHECK(m_vertexContainerKey.initialize());
    ATH_CHECK(m_trackPartContainerKey.initialize());
    ATH_CHECK(m_exampleDecorKey.initialize());
    ATH_CHECK(m_decisionKey.initialize());
    return StatusCode::SUCCESS;
  }

  StatusCode AugmentationToolExample::addBranches(const EventContext& ctx) const
  {
    // Set up the vector
    std::unique_ptr<std::vector<float> > track_z0_PV(new std::vector<float>());

    // Set up the decorators
    SG::WriteDecorHandle< xAOD::TrackParticleContainer, float > decorator(m_exampleDecorKey, ctx);

    // CALCULATION OF THE NEW VARIABLE
    // Get Primary vertex
    SG::ReadHandle<xAOD::VertexContainer> vertices(m_vertexContainerKey, ctx);
    if (!vertices.isValid()) {
      ATH_MSG_ERROR ("Couldn't retrieve VertexContainer with key PrimaryVertices");
      return StatusCode::FAILURE;
    }

    const xAOD::Vertex* pv{};
    for (const xAOD::Vertex* vx : *vertices) {
      if (vx->vertexType() == xAOD::VxType::PriVtx) {
        pv = vx;
        break;
      }
    }

    // Get the track container
    SG::ReadHandle<xAOD::TrackParticleContainer> tracks(m_trackPartContainerKey, ctx);
    if (!tracks.isValid()) {
      ATH_MSG_ERROR ("Couldn't retrieve TrackParticleContainer with key InDetTrackParticles");
      return StatusCode::FAILURE;
    }

    // Get track z0 w.r.t PV: this is what we're adding
    for (const auto *trackParticle : *tracks) {
      if (pv) {
        float z0wrtPV = trackParticle->z0() + trackParticle->vz() - pv->z(); // CALCULATE THE QUANTITY
        track_z0_PV->push_back(z0wrtPV); // ADD TO VECTOR
        decorator(*trackParticle) = z0wrtPV; // DECORATE THE TRACK
      } else {
        track_z0_PV->push_back(999.);
        decorator(*trackParticle) = 999.;
      }
    }

    // Write decision to SG for access by downstream algs
    SG::WriteHandle<std::vector<float> > decision(m_decisionKey, ctx);
    if (!decision.isValid()) {
      ATH_MSG_ERROR("Tool is attempting to write StoreGate keys which already exists. Please use a different key");
      return StatusCode::FAILURE;
    } else {
      decision = std::move(track_z0_PV);
    }

    return StatusCode::SUCCESS;
  }
}
