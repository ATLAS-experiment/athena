/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
MC Truth object associated with an TrackParticle object

***************************************************************************

 ***************************************************************************/
#ifndef PARTICLETRUTH_TRACKPARTICLETRUTH_H
#define PARTICLETRUTH_TRACKPARTICLETRUTH_H
#include "GeneratorObjects/HepMcParticleLink.h"

class TrackParticleTruth {

 public:
  TrackParticleTruth();
  TrackParticleTruth(const HepMcParticleLink& particleLink, float probability);
  TrackParticleTruth(const TrackParticleTruth& other);
  TrackParticleTruth & operator=(const TrackParticleTruth& ) = default;
  virtual ~TrackParticleTruth();
  const HepMcParticleLink& particleLink() const;
  float probability() const;

 private:
  HepMcParticleLink m_particleLink;
  float m_probability; //probability of being m_barcode the truth information
};

inline const HepMcParticleLink& TrackParticleTruth::particleLink() const
{
  return m_particleLink;
}

inline float TrackParticleTruth::probability() const 
{
  return m_probability;
}

#endif // PARTICLE_TRACKPARTICLETRUTH_H

