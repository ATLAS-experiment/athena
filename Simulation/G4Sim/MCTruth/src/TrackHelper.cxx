/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MCTruth/TrackHelper.h"
#include "G4Track.hh"
#include "ISF_Event/ISFParticle.h"
#include "MCTruth/TrackInformation.h"


TrackHelper::TrackHelper(const G4Track* t)
{
  m_trackInfo=static_cast<TrackInformation *>(t->GetUserInformation());
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
