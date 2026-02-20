/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MCTruth/TrackInformation.h"
#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenParticle.h"

TrackInformation::TrackInformation()
  : m_regenerationNr(0)
  , m_currentGenParticle(nullptr)
  , m_theBaseISFParticle(nullptr)
  , m_returnedToISF(false)
{
}

TrackInformation::TrackInformation(HepMC::GenParticlePtr p, ISF::ISFParticle* baseIsp)
  : m_regenerationNr(0)
  , m_currentGenParticle(std::move(p))
  , m_theBaseISFParticle(baseIsp)
  , m_returnedToISF(false)
{
}

int TrackInformation::GetParticleBarcode() const
{
  if (m_barcode != HepMC::INVALID_PARTICLE_BARCODE) return m_barcode;
  if (m_currentGenParticle) {
    m_barcode = HepMC::barcode(m_currentGenParticle);
    return m_barcode;
  }
  return HepMC::UNDEFINED_ID;
}

int TrackInformation::GetParticleUniqueID() const
{
  if (m_uniqueID != HepMC::INVALID_PARTICLE_BARCODE) return m_uniqueID;
  if (m_currentGenParticle) {
    HepMC::ConstGenParticlePtr particle = m_currentGenParticle;
    m_uniqueID = HepMC::uniqueID(particle);
    return m_uniqueID;
  }
  return HepMC::UNDEFINED_ID;
}

int TrackInformation::GetParticleStatus() const
{
  if (m_currentGenParticle) {
    return m_currentGenParticle->status();
  }
  return 0;
}

void TrackInformation::SetCurrentGenParticle(HepMC::GenParticlePtr p)
{
  m_currentGenParticle = std::move(p);
  m_barcode = HepMC::INVALID_PARTICLE_BARCODE;
  m_uniqueID = HepMC::INVALID_PARTICLE_BARCODE;
}

void TrackInformation::SetBaseISFParticle(ISF::ISFParticle* p)
{
  m_theBaseISFParticle = p;
}
