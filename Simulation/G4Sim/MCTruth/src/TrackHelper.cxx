/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MCTruth/TrackHelper.h"
#include "G4Track.hh"
#include "ISF_Event/ISFParticle.h"
#include "MCTruth/TrackInformation.h"
#include "GeneratorObjects/HepMcParticleLink.h"


TrackHelper::TrackHelper(const G4Track* t)
{
  m_trackInfo=static_cast<TrackInformation *>(t->GetUserInformation());
}
bool TrackHelper::IsPrimary() const
{
  if (m_trackInfo==0) return false;
  return m_trackInfo->GetClassification()==TrackInformation::Primary;
}
bool TrackHelper::IsRegeneratedPrimary() const
{
  if (m_trackInfo==0) return false;
  return m_trackInfo->GetClassification()==TrackInformation::RegeneratedPrimary;
}
bool TrackHelper::IsRegisteredSecondary() const
{
  if (m_trackInfo==0) return false;
  return m_trackInfo->GetClassification()==TrackInformation::RegisteredSecondary;
}
bool TrackHelper::IsSecondary() const
{
  if (m_trackInfo==0) return true;
  return m_trackInfo->GetClassification()==TrackInformation::Secondary;
}
int TrackHelper::GetBarcode() const  // TODO Drop this once UniqueID and Status are used instead
{
  if (m_trackInfo==0 || std::as_const(m_trackInfo)->GetCurrentGenParticle()==0) return 0;
  return m_trackInfo->GetParticleBarcode();
}

int TrackHelper::GetUniqueID() const
{
  if (m_trackInfo==0 || std::as_const(m_trackInfo)->GetCurrentGenParticle()==0) return 0;
  return m_trackInfo->GetParticleUniqueID();
}

int TrackHelper::GetStatus() const
{
  if (m_trackInfo==0 || std::as_const(m_trackInfo)->GetCurrentGenParticle()==0) return 0;
  return m_trackInfo->GetParticleStatus();
}

HepMcParticleLink TrackHelper::GetParticleLink()
{
  // FIXME update to use HepMcParticleLink::IS_POSITION ATLASSIM-6999
#if defined(HEPMC3)
  return HepMcParticleLink(this->GetUniqueID(), 0, HepMcParticleLink::IS_POSITION, HepMcParticleLink::IS_ID);
#else
  return HepMcParticleLink(this->GetBarcode(), 0, HepMcParticleLink::IS_POSITION, HepMcParticleLink::IS_BARCODE);
#endif
}
