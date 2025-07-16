/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// ParticleHypothesis.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKEXUTILS_PARTICLEHYPOTHESIS_H
#define TRKEXUTILS_PARTICLEHYPOTHESIS_H

// Gaudi
#include "GaudiKernel/SystemOfUnits.h"

#include "TruthUtils/ParticleConstants.h"

// define the particle hypotheses
#define PARTICLEHYPOTHESES 11

namespace Trk {

  /** @enum ParticleHypothesis
   Enumeration for Particle hypothesis respecting the
   interaction with material

   @author Andreas.Salzburger@cern.ch
   */
   enum ParticleHypothesis { nonInteracting  = 0,
                             geantino        = 0,
                             electron        = 1,
                             muon            = 2,                            
                             pion            = 3,                             
                             kaon            = 4,
                             proton          = 5,
                             photon          = 6,  // for Fatras usage
                             neutron         = 7,  // for Fatras usage 
                             pi0             = 8, // for Fatras usage 
                             k0              = 9, // for Fatras usage 
                             nonInteractingMuon = 10, // For material collection
                             noHypothesis    = 99,
                             undefined       = 99};
  
  /** @struct ParticleMasses
   Mass declaration of particles covered 
   in the ParticleHypothesis.
   Masses are given in MeV and taken from:

   Review of Particle Physics (2010)
   K. Nakamura et al. (Particle Data Group), J. Phys. G 37, 075021 (2010)

   @author Andreas.Salzburger@cern.ch
   */
    
   namespace ParticleMasses {
      /** the array of masses */
     constexpr double mass[PARTICLEHYPOTHESES] =
     { (0.*Gaudi::Units::MeV),// non interacting mass
       (ParticleConstants::electronMassInMeV), // electron mass
       (ParticleConstants::muonMassInMeV), // muon mass
       (ParticleConstants::chargedPionMassInMeV), // charged pion mass
       (ParticleConstants::chargedKaonMassInMeV),    // kaon mass
       (ParticleConstants::protonMassInMeV), // proton mass
       (ParticleConstants::photonMassInMeV),         // photon rest mass
       (ParticleConstants::neutronMassInMeV), // neutron rest mass
       (ParticleConstants::piZeroMassInMeV),  // pi0 rest mass
       (ParticleConstants::KZeroMassInMeV),    // K0 rest mass
       (ParticleConstants::muonMassInMeV) // muon mass
     }; 
   };
   
  /** @struct ParticleSwitcher
   Simple struct to translate an integer in the ParticleHypothesis type
     
   @author Andreas.Salzburger@cern.ch
   */
    
   namespace ParticleSwitcher {
      /** the array  of masses */
   constexpr ParticleHypothesis particle[PARTICLEHYPOTHESES] =
   { (Trk::nonInteracting), 
     (Trk::electron),
     (Trk::muon),          
     (Trk::pion),             
     (Trk::kaon),          
     (Trk::proton),         
     (Trk::photon),         
     (Trk::neutron),         
     (Trk::pi0),         
     (Trk::k0),
     (Trk::nonInteractingMuon)
   };
   };
    
   
}

#endif
