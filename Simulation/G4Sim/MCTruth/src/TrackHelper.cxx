/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <utility>

#include "MCTruth/TrackHelper.h"
#include "AtlasHepMC/GenParticle.h"
#include "G4Track.hh"
#include "MCTruth/TrackInformation.h"
#include "MCTruth/VTrackInformation.h"
#include "TruthUtils/MagicNumbers.h"

namespace
{
  HepMC::ConstGenParticlePtr outputAttributionParticle(const TrackInformation* trackInfo)
  {
    if (!trackInfo) {
      return nullptr;
    }
    // Output links should remain anchored to the pre-regeneration particle.
    // Older TrackInformation instances may only have the current particle.
    HepMC::ConstGenParticlePtr particle = trackInfo->GetGenerationZeroGenParticle();
    return particle ? particle : trackInfo->GetCurrentGenParticle();
  }
}

TrackHelper::TrackHelper(const G4Track* t)
{
  G4VUserTrackInformation* userInfo = t ? t->GetUserInformation() : nullptr;
  m_trackInfo = dynamic_cast<VTrackInformation*>(userInfo);
}

TrackInformation* TrackHelper::GetTrackInformation()
{
  return dynamic_cast<TrackInformation*>(m_trackInfo);
}
bool TrackHelper::IsPrimary() const
{
  if (m_trackInfo==0) return false;
  return m_trackInfo->GetClassification()==VTrackInformation::Primary;
}
bool TrackHelper::IsRegeneratedPrimary() const
{
  if (m_trackInfo==0) return false;
  return m_trackInfo->GetClassification()==VTrackInformation::RegeneratedPrimary;
}
bool TrackHelper::IsRegisteredSecondary() const
{
  if (m_trackInfo==0) return false;
  return m_trackInfo->GetClassification()==VTrackInformation::RegisteredSecondary;
}
bool TrackHelper::IsSecondary() const
{
  if (m_trackInfo==0) return true;
  return m_trackInfo->GetClassification()==VTrackInformation::Secondary;
}

int TrackHelper::GetUniqueID() const
{
  if (const TrackInformation* concreteInfo = dynamic_cast<const TrackInformation*>(m_trackInfo)) {
    HepMC::ConstGenParticlePtr particle = outputAttributionParticle(concreteInfo);
    return particle ? HepMC::uniqueID(particle) : 0;
  }
  return m_trackInfo ? m_trackInfo->GetParticleUniqueID() : 0;
}

int TrackHelper::GetStatus() const
{
  if (const TrackInformation* concreteInfo = dynamic_cast<const TrackInformation*>(m_trackInfo)) {
    HepMC::ConstGenParticlePtr particle = outputAttributionParticle(concreteInfo);
    return particle ? particle->status() : 0;
  }
  return m_trackInfo ? m_trackInfo->GetParticleStatus() : 0;
}

HepMC::GenParticlePtr TrackHelper::GetPrimaryGenParticle()
{
  return m_trackInfo ? m_trackInfo->GetPrimaryGenParticle() : nullptr;
}

HepMC::ConstGenParticlePtr TrackHelper::GetPrimaryGenParticle() const
{
  return m_trackInfo ? std::as_const(m_trackInfo)->GetPrimaryGenParticle() : nullptr;
}
