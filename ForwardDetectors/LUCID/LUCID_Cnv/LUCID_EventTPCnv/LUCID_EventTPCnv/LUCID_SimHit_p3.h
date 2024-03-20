/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LUCID_EVENTTPCNV_LUCID_SIMHIT_P3_H
#define LUCID_EVENTTPCNV_LUCID_SIMHIT_P3_H

#include <vector>
#include "GeneratorObjectsTPCnv/HepMcParticleLink_p3.h"

class LUCID_SimHit_p3 {

 public:

  LUCID_SimHit_p3() {};

  friend class LUCID_SimHitCnv_p3;

 private:

  HepMcParticleLink_p3 m_partLink;

  short m_tubeID{0};
  int   m_pdgCode{0};
  int   m_genVolume{0};
  float m_stepStartPosX{0};
  float m_stepStartPosY{0};
  float m_stepStartPosZ{0};
  float m_stepEndPosX{0};
  float m_stepEndPosY{0};
  float m_stepEndPosZ{0};
  float m_preStepTime{0};
  float m_postStepTime{0};
  float m_wavelength{0};
  float m_energy{0};
};

#endif // LUCID_EVENTTPCNV_LUCID_SIMHIT_P3_H
