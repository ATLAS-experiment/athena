/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef SIHIT_P2_H
#define SIHIT_P2_H

#include "GeneratorObjectsTPCnv/HepMcParticleLink_p2.h"

class SiHit_p2 {
 public:
  float m_stX = 0, m_stY = 0, m_stZ = 0;
  float m_enX = 0, m_enY = 0, m_enZ = 0;
  float m_energyLoss = 0; // deposited energy
  float m_meanTime = 0; // time of energy deposition
  HepMcParticleLink_p2 m_partLink;
  unsigned int m_ID = 0;
};
#endif
