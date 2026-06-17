/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Author: James Catmore (James.Catmore@cern.ch)
// Removes all ID tracks which do not pass a user-defined cut

#include "DerivationFrameworkInDet/TrackParticleThinningPHYS.h"
#include "StoreGate/ReadDecorHandle.h"

// Athena initialize and finalize
StatusCode DerivationFramework::TrackParticleThinningPHYS::initialize()
{
  ATH_CHECK( TrackParticleThinningBase::initialize () );
  ATH_CHECK( m_trackZ0PVKey.initialize() );
  ATH_CHECK( m_tightPrimaryKey.initialize() );
  return StatusCode::SUCCESS;
}


std::vector<int> DerivationFramework::TrackParticleThinningPHYS::updateMask(const EventContext& ctx, const xAOD::TrackParticleContainer* trackParticles) const
{
  SG::ReadDecorHandle<xAOD::TrackParticleContainer, bool> tightPrimaryHandle(m_tightPrimaryKey, ctx); // TODO CHECK TYPE
  SG::ReadDecorHandle<xAOD::TrackParticleContainer, float> z0AtPVHandle(m_trackZ0PVKey, ctx);
  unsigned int index{0};
  std::vector<int> entries;
  entries.reserve(trackParticles->size());
  for (const auto* trackParticle : *trackParticles) {
    // Inner detector group recommendations for indet tracks in analysis
    // https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/DaodRecommendations
    entries.push_back( (tightPrimaryHandle(*trackParticle) && (std::abs(z0AtPVHandle(*trackParticle)) * sin(trackParticle->theta()) < 3.0 * Gaudi::Units::mm) && (trackParticle->pt() > 10 * Gaudi::Units::GeV)) ? 1 : 0);
    ++index;
  }
  return entries;
}
