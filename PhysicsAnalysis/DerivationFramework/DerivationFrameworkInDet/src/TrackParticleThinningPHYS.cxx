/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Author: James Catmore (James.Catmore@cern.ch)
// Removes all ID tracks which do not pass a user-defined cut

#include "DerivationFrameworkInDet/TrackParticleThinningPHYS.h"

// Athena initialize and finalize
StatusCode DerivationFramework::TrackParticleThinningPHYS::initialize()
{
  ATH_CHECK( TrackParticleThinningBase::initialize () );
  ATH_CHECK( m_trackZ0PVKey.initialize() );
  return StatusCode::SUCCESS;
}


std::vector<int> DerivationFramework::TrackParticleThinningPHYS::updateMask(const xAOD::TrackParticleContainer* trackParticles) const
{
  const EventContext& ctx = Gaudi::Hive::currentContext();
  static const SG::ConstAccessor< bool > tightPrimaryAcc( "DFCommonTightPrimary" ); // TODO CHECK TYPE
  SG::ReadHandle<std::vector<float>> z0AtPV(m_trackZ0PVKey, ctx);
  unsigned int index{0};
  std::vector<int> entries;
  if (z0AtPV->size() != trackParticles->size()) {
    ATH_MSG_ERROR("z0AtPV->size() != trackParticles->size() - the job bail out now.");
    return entries;
  }
  entries.reserve(trackParticles->size());
  for (const auto* trackParticle : *trackParticles) {
    // Inner detector group recommendations for indet tracks in analysis
    // https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/DaodRecommendations
    entries.push_back( (tightPrimaryAcc(*trackParticle) && (std::abs(z0AtPV->at(index)) * sin(trackParticle->theta()) < 3.0 * Gaudi::Units::mm) && (trackParticle->pt() > 10 * Gaudi::Units::GeV)) ? 1 : 0);
    ++index;
  }
  return entries;
}
