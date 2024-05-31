/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MCTruth/VTrackInformation.h"

VTrackInformation::VTrackInformation(TrackClassification tc)
  : m_classify(tc)
{
}

void VTrackInformation::SetPrimaryGenParticle(HepMC::GenParticlePtr p)
{
  m_primaryGenParticle = std::move(p);
}

bool VTrackInformation::GetReturnedToISF() const
{
  return false;
}

void VTrackInformation::SetCurrentGenParticle(HepMC::GenParticlePtr /*p*/)
{
  // you should not call this, perhaps throw an exception?
  std::cerr<<"ERROR  VTrackInformation::SetCurrentGenParticle() not supported  "<<std::endl;
}

void VTrackInformation::SetBaseISFParticle(ISF::ISFParticle* /*p*/)
{
  // you should not call this, perhaps throw an exception?
  std::cerr<<"ERROR  VTrackInformation::SetBaseISFParticle() not supported  "<<std::endl;
}

void VTrackInformation::SetReturnedToISF(bool)
{
  std::cerr<<"ERROR  VTrackInformation::SetReturnedToISF() not supported  "<<std::endl;
}
