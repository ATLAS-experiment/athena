/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TrigParticleTable.h, (c) ATLAS Detector software
// LVL2 adaptation of the offline code ParticleHypothesis.h 
// by A.Salzburger
///////////////////////////////////////////////////////////////////

#ifndef __TRIG_PARTICLE_TABLE__
#define __TRIG_PARTICLE_TABLE__

#include <array>
#include "GaudiKernel/SystemOfUnits.h"
#include "TruthUtils/ParticleConstants.h"

namespace TrigVtx
{
  enum TrigParticleName 
    { 
      electron=0,			 
      muon=1,
      pion=2,
      kaon=3,
      proton=4,
      gamma=5
    };
  
  struct TrigParticleMasses 
  {
    constexpr TrigParticleMasses() = default;
    std::array<double,6> mass{
      ParticleConstants::electronMassInMeV, // electron mass
      ParticleConstants::muonMassInMeV, // muon mass
      ParticleConstants::chargedPionMassInMeV, // charged pion mass
      493.67700*Gaudi::Units::MeV, // charged kaon mass
      938.27203*Gaudi::Units::MeV, // proton mass
      0                            // photon mass
    };
  };
}

#endif

