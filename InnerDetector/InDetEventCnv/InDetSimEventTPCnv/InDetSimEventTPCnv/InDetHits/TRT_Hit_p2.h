/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRT_HIT_P2_H
#define TRT_HIT_P2_H

#include "GeneratorObjectsTPCnv/HepMcParticleLink_p2.h"

class TRT_Hit_p2 {
 public:
  int hitID = 0; // To identify the hit
  HepMcParticleLink_p2 m_partLink; // link to the particle generating the hit
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
