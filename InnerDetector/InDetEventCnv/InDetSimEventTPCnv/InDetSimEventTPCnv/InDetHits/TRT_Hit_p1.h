/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRT_HIT_P1_H
#define TRT_HIT_P1_H

#include "GeneratorObjectsTPCnv/HepMcParticleLink_p1.h"

class TRT_Hit_p1 {
 public:
  int hitID = 0; // To identify the hit
  HepMcParticleLink_p1 m_partLink; // link to the particle generating the hit
  int particleEncoding = 0;         // PDG id
  float kineticEnergy = 0;          // kin energy of the particle
  float energyDeposit = 0;          // energy deposit by the hit
  float preStepX = 0;
  float preStepY = 0;
  float preStepZ = 0;
  float postStepX = 0;
  float postStepY = 0;
  float postStepZ = 0;
  float globalTime = 0;
};
#endif
