/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

//=================================================
// LArTBWarmTCHit class
//
// information stored
//=================================================

#ifndef LArTBWarmTCHit_h
#define LArTBWarmTCHit_h 1

#include "CLHEP/Vector/ThreeVector.h"
#include <vector>

class LArTBWarmTCHit 
{
 public:
     LArTBWarmTCHit() = default;
     LArTBWarmTCHit(int ad, double en) : m_addr(ad), m_energy(en) {};

//Set- Get- methods
 // energy deposit
     inline void SetEnergy(double ed) { m_energy = ed; }
     inline void AddEnergy(double ed) { m_energy += ed; }
     inline double GetEnergy() { return m_energy; }

 // address
     inline void SetAddr(int d) { m_addr = d; }
     inline int GetAddr() { return m_addr; }

 private:
     int     m_addr{};   // Address:
     double  m_energy{};
};

typedef std::vector<LArTBWarmTCHit> LArTBWarmTCHitsCollection;

#endif
