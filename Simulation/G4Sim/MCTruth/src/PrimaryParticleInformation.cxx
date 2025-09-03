/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MCTruth/PrimaryParticleInformation.h"
#include "AtlasHepMC/GenParticle.h"

PrimaryParticleInformation::PrimaryParticleInformation()
{
}

PrimaryParticleInformation::PrimaryParticleInformation(HepMC::GenParticlePtr p, ISF::ISFParticle* isp)
  : m_theParticle(std::move(p))
  , m_theISFParticle(isp)
{
}


int PrimaryParticleInformation::GetParticleUniqueID() const
{
  if (m_uniqueID !=  HepMC::INVALID_PARTICLE_ID) return m_uniqueID;
  if (m_theParticle) {
    HepMC::ConstGenParticlePtr particle = m_theParticle;
    m_uniqueID = HepMC::uniqueID(particle);
    return m_uniqueID;
  }
  return 0;
}

void PrimaryParticleInformation::SetISFParticle(ISF::ISFParticle* p)
{
  m_theISFParticle=p;
}
