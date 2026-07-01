/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Author: Tomoe Kishimoto (Tomoe.Kishimoto@cern.ch)
// Wrapper around the passSelection() method of xAOD egamma
// Writes result to SG for later selection by string parser

#include "DerivationFrameworkInDet/TrackParametersAtPV.h"

#include <StoreGate/ReadHandle.h>
#include <StoreGate/WriteDecorHandle.h>

#include <string>

// Athena initialize and finalize
StatusCode DerivationFramework::TrackParametersAtPV::initialize()
{
   if (m_collTrackKey.key().empty() || m_collVertexKey.key().empty()) {
    ATH_MSG_ERROR("No selection variables for the TrackParametersAtPV tool!");
    return StatusCode::FAILURE;
  }
  ATH_CHECK( m_collTrackKey.initialize() );
  ATH_CHECK( m_collVertexKey.initialize() );
  ATH_CHECK( m_trackZ0PVDecoKey.initialize() );

  ATH_MSG_VERBOSE("initialize() ...");
  return StatusCode::SUCCESS;
}

// Augmentation
StatusCode DerivationFramework::TrackParametersAtPV::addBranches(const EventContext& ctx) const
{
  SG::WriteDecorHandle<xAOD::TrackParticleContainer, float> track_z0_PV(m_trackZ0PVDecoKey, ctx);

  // Get Primary vertex
  SG::ReadHandle<xAOD::VertexContainer> vertices(m_collVertexKey,ctx);
  if(!vertices.isValid()) {
    ATH_MSG_ERROR ("Couldn't retrieve VertexContainer with key: " << m_collVertexKey.key());
    return StatusCode::FAILURE;
  }

  const xAOD::Vertex* pv(nullptr);
  for (const xAOD::Vertex* vx : *vertices) {
    if (vx->vertexType() == xAOD::VxType::PriVtx) {
      pv = vx;
      break;
    }
  }

  // Get the track container
  SG::ReadHandle<xAOD::TrackParticleContainer> tracks(m_collTrackKey,ctx);
  if(!tracks.isValid()) {
    ATH_MSG_ERROR ("Couldn't retrieve TrackParticleContainer with key: " << m_collTrackKey.key());
    return StatusCode::FAILURE;
  }

  // Get track z0 w.r.t PV
  for (const auto *trackIt : *tracks) {
    track_z0_PV(*trackIt) = pv ? trackIt->z0() + trackIt->vz() - pv->z() : 999.;
  }

  return StatusCode::SUCCESS;
}

