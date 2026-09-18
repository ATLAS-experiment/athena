/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

  FatrasG4 implementation of the ACTS PhotonConversion class
  https://github.com/acts-project/acts/blob/v45.0.0/Fatras/include/ActsFatras/Physics/ElectroMagnetic/PhotonConversion.hpp
*/

#pragma once

#ifndef G4FASTSIMULATION_FATRASG4PHOTONCONVERSION_H
#define G4FASTSIMULATION_FATRASG4PHOTONCONVERSION_H

// Geant 4 particle definitions
#include "G4Gamma.hh"
#include "G4Electron.hh"
#include "G4Positron.hh"

//Geant4
#include "G4FastTrack.hh"
#include "G4FastStep.hh"
#include "G4ThreeVector.hh"
#include "G4ParticleTable.hh"
#include "Randomize.hh"

// CLHEP
#include "CLHEP/Random/RandomEngine.h"
#include "CLHEP/Random/RandFlat.h"


class FatrasG4PhotonConversion
{
 public:
  // Method for evaluating the distance after which the photon
  // conversion will occur.
  std::pair<double, double> generatePathLimits(CLHEP::HepRandomEngine& generator,
                                               const G4FastTrack& fastTrack) const;

  // This method evaluates the final state due to the photon conversion.
  void run(CLHEP::HepRandomEngine& generator,
           const G4FastTrack& fastTrack,
           G4FastStep& fastStep) const;

 private:
  // Helper methods for momentum evaluation
  // These methods are taken from the Geant4 class
  // G4PairProductionRelModel
  double screenFunction1(double delta) const;
  double screenFunction2(double delta) const;

  // Generate the energy fraction of the first child particle.
  double generateFirstChildEnergyFraction(CLHEP::HepRandomEngine& generator,
                                          double gammaMom) const;

  // Generate the direction of the child particles.
  G4ThreeVector generateChildDirection(CLHEP::HepRandomEngine& generator,
                                       const G4FastTrack& fastTrack) const;

  // This method constructs and returns the child particles.
  std::pair<G4DynamicParticle, G4DynamicParticle> generateChildren(
      const G4FastTrack& fastTrack, double childEnergy,
      const G4ThreeVector& childDirection) const;

  // Scaling factor of children energy
  double m_childEnergyScaleFactor = 2.0;
  // Scaling factor for photon conversion probability
  double m_conversionProbScaleFactor = 0.98;
  // Electron mass in MeV
  const double m_electronMass = 0.511;

};

#endif //G4FASTSIMULATION_FATRASG4PHOTONCONVERSION_H