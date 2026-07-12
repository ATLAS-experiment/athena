/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

  FatrasG4 implementation of the ACTS PhotonConversion class
  https://github.com/acts-project/acts/blob/v45.0.0/Fatras/include/ActsFatras/Physics/ElectroMagnetic/PhotonConversion.hpp
*/

#include "FatrasG4PhotonConversion.h"

// Geant4 particle definitions
#include "G4Gamma.hh"
#include "G4Electron.hh"
#include "G4Positron.hh"

// Geant4 classes
#include "G4FastTrack.hh"
#include "G4FastStep.hh"
#include "G4DynamicParticle.hh"

// CLHEP
#include "CLHEP/Random/RandomEngine.h"
#include "CLHEP/Random/RandFlat.h"

#include <cmath>
#include <numbers>

// #define FATRASG4PHOTONCONVERSION_DEBUG

inline double FatrasG4PhotonConversion::screenFunction1(double delta) const {
  // Compute the value of the screening function 3*PHI1(delta) - PHI2(delta)
  return (delta > 1.4) ? 42.038 - 8.29 * std::log(delta + 0.958)
                       : 42.184 - delta * (7.444 - 1.623 * delta);
}

inline double FatrasG4PhotonConversion::screenFunction2(double delta) const {
  // Compute the value of the screening function 1.5*PHI1(delta)
  // +0.5*PHI2(delta)
  return (delta > 1.4) ? 42.038 - 8.29 * std::log(delta + 0.958)
                       : 41.326 - delta * (5.848 - 0.902 * delta);
}

std::pair<double, double> FatrasG4PhotonConversion::generatePathLimits(
    CLHEP::HepRandomEngine& generator, const G4FastTrack& fastTrack) const {
  // This method is based upon the Athena class PhotonConversionTool

  // Get the primary track information
  const G4Track * track = fastTrack.GetPrimaryTrack();
  const double p = track -> GetMomentum().mag();
  const G4ParticleDefinition * definition = track -> GetDefinition();
  
  // Fast exit if not a photon or if not enough momentum
  const bool isPhoton = definition == G4Gamma::Definition();
  if (!isPhoton ||
      p < (2. * m_electronMass)){
    return {std::numeric_limits<double>::infinity(),
            std::numeric_limits<double>::infinity()};
  }

  // Use for the moment only Al data - Yung Tsai - Rev.Mod.Particle Physics Vol.
  // 46, No.4, October 1974 optainef from a fit given in the momentum range 100
  // 10 6 2 1 0.6 0.4 0.2 0.1 GeV

  //// Quadratic background function
  //  Double_t fitFunction(Double_t *x, Double_t *par) {
  //  return par[0] + par[1]*pow(x[0],par[2]);
  // }
  // EXT PARAMETER                                   STEP         FIRST
  // NO.   NAME      VALUE            ERROR          SIZE      DERIVATIVE
  //  1  p0          -7.01612e-03   8.43478e-01   1.62766e-04   1.11914e-05
  //  2  p1           7.69040e-02   1.00059e+00   8.90718e-05  -8.41167e-07
  //  3  p2          -6.07682e-01   5.13256e+00   6.07228e-04  -9.44448e-07
  constexpr double p0 = -7.01612e-03;
  constexpr double p1 = 7.69040e-02;
  constexpr double p2 = -6.07682e-01;

  // Calculate xi
  const double xi = p0 + p1 * std::pow(p, p2);

  double u = generator.flat();
  // This is a transformation of eq. 3.75
  return {-9. / 7. *
              std::log(m_conversionProbScaleFactor *
                       (1 - u)) /
              (1. - xi),
          std::numeric_limits<double>::infinity()};
}

double FatrasG4PhotonConversion::generateFirstChildEnergyFraction(
    CLHEP::HepRandomEngine& generator, double gammaMom) const {
  // This method is based upon the Geant4 class G4PairProductionRelModel
  // This method is from the Geant4 class G4Element
  
  //  Compute Coulomb correction factor (Phys Rev. D50 3-1 (1994) page 1254)
  constexpr double k1 = 0.0083;
  constexpr double k2 = 0.20206;
  constexpr double k3 = 0.0020;  // This term is missing in Athena
  constexpr double k4 = 0.0369;
  constexpr double alphaEM = 1. / 137.;
  constexpr double Z = 13.;  // Aluminium
  constexpr double az2 = (alphaEM * Z) * (alphaEM * Z);
  constexpr double az4 = az2 * az2;
  constexpr double coulombFactor =
      (k1 * az4 + k2 + 1. / (1. + az2)) * az2 - (k3 * az4 + k4) * az4;

  const double logZ13 = std::log(Z) * 1. / 3.;
  const double FZ = 8. * (logZ13 + coulombFactor);
  const double deltaMax = std::exp((42.038 - FZ) * 0.1206) - 0.958;

  const double deltaPreFactor = 136. / std::pow(Z, 1. / 3.);
  const double eps0 = m_electronMass / gammaMom;
  const double deltaFactor = deltaPreFactor * eps0;
  const double deltaMin = 4. * deltaFactor;

  // Compute the limits of eps
  const double epsMin =
      std::max(eps0, 0.5 - 0.5 * std::sqrt(1. - deltaMin / deltaMax));
  const double epsRange = 0.5 - epsMin;

  // Sample the energy rate (eps) of the created electron (or positron)
  const double F10 = screenFunction1(deltaMin) - FZ;
  const double F20 = screenFunction2(deltaMin) - FZ;
  const double NormF1 = F10 * epsRange * epsRange;
  const double NormF2 = 1.5 * F20;

  // We will need 3 uniform random number for each trial of sampling
  double greject = 0.;
  double eps = 0.;
  do{
    if(NormF1 > generator.flat() * (NormF1 + NormF2)){
      eps = 0.5 - epsRange * std::pow(generator.flat(), 1./3.);
      const double delta = deltaFactor / (eps * (1. - eps));
      greject = (screenFunction1(delta) - FZ) / F10;
    }else{
      eps = epsMin + epsRange * generator.flat();
      const double delta = deltaFactor / (eps * (1. - eps));
      greject = (screenFunction2(delta) - FZ) / F20;
    }
  }while(greject < generator.flat());
  //  End of eps sampling
  return eps * m_childEnergyScaleFactor;
}

G4ThreeVector FatrasG4PhotonConversion::generateChildDirection(
    CLHEP::HepRandomEngine& generator, const G4FastTrack& fastTrack) const {
  /// This method is based upon the Athena class PhotonConversionTool

  // Following the Geant4 approximation from L. Urban
  // the azimutal angle
  double p = fastTrack.GetPrimaryTrack() -> GetMomentum().mag();
  double theta = m_electronMass / p;

  const double rand = - std::log(generator.flat() * generator.flat()) * 1.6;

  theta *= (generator.flat() < 0.25)
               ? rand
               : rand * 1. / 3.;  // 9./(9.+27) = 0.25

  // draw the random orientation angle
  const auto psi=(2. * std::numbers::pi * generator.flat() - std::numbers::pi);

  G4ThreeVector direction = fastTrack.GetPrimaryTrack()->GetMomentumDirection();

  // rotate by psi around the original direction
  direction.rotate(psi, direction); // rotation around itself (does nothing, can skip)

  // rotate by theta around an axis perpendicular to direction
  G4ThreeVector u = direction.orthogonal(); // returns a vector orthogonal to 'direction'
  direction.rotate(theta, u);
  return direction;
}

inline std::pair<G4DynamicParticle, G4DynamicParticle> FatrasG4PhotonConversion::generateChildren(
    const G4FastTrack& fastTrack, double childEnergy,
    const G4ThreeVector& childDirection) const {

  // Calculate the child momentum
  const double massChild = m_electronMass;
  const double momentum1 =
      std::sqrt(childEnergy * childEnergy - massChild * massChild);

  const G4ThreeVector motherVec = fastTrack.GetPrimaryTrack() -> GetMomentum();
  const G4ThreeVector firstChildVec = momentum1 * childDirection;

  // Use energy conservation to get the second child direction
  // and momentum
  const G4ThreeVector secondChildVec = motherVec - firstChildVec;


  std::pair<G4DynamicParticle, G4DynamicParticle> children = {
    G4DynamicParticle(G4Electron::Definition(), firstChildVec),
    G4DynamicParticle(G4Positron::Definition(), secondChildVec)
  };
  return children;
}

void FatrasG4PhotonConversion::run(
    CLHEP::HepRandomEngine& generator,
    const G4FastTrack& fastTrack,
    G4FastStep& fastStep) const {
  
  // Get primary track information
  const G4Track * track = fastTrack.GetPrimaryTrack();
  const double p = track -> GetMomentum().mag();
  const double currentTime = track -> GetGlobalTime();
  const G4ThreeVector currentPos = track -> GetPosition();
  
  // Get first child momentum and energy
  const double firstChildMomentum = p * generateFirstChildEnergyFraction(generator, p);
  const double firstChildEnergy = std::sqrt(firstChildMomentum * firstChildMomentum
                                            + m_electronMass * m_electronMass);

  // Get first child direction
  const G4ThreeVector firstChildDirection = generateChildDirection(generator, fastTrack);

  // Get children as pair of G4DynamicParticle
  const std::pair<G4DynamicParticle, G4DynamicParticle> children = 
    generateChildren(fastTrack, firstChildEnergy, firstChildDirection);

  // Add 2 secondary tracks to the fastStep
  fastStep.SetNumberOfSecondaryTracks(2);

  // Add secondary tracks to fastStep
  #ifdef FATRASG4PHOTONCONVERSION_DEBUG
    G4cout << "[FatrasG4PhotonConversion::run] ";
    G4cout << "Creating secondary electron track at (";
    G4cout << currentPos.x() << ", ";
    G4cout << currentPos.y() << ", ";
    G4cout << currentPos.z() << ")" << G4endl;
  #endif
  fastStep.CreateSecondaryTrack(children.first, currentPos, currentTime, false);

  #ifdef FATRASG4PHOTONCONVERSION_DEBUG
    G4cout << "[FatrasG4PhotonConversion::run] ";
    G4cout << "Creating secondary positron track at (";
    G4cout << currentPos.x() << ", ";
    G4cout << currentPos.y() << ", ";
    G4cout << currentPos.z() << ")" << G4endl;
  #endif
  fastStep.CreateSecondaryTrack(children.second, currentPos, currentTime, false);
  
  // Kill primary track
  #ifdef FATRASG4PHOTONCONVERSION_DEBUG
    G4cout << "[FatrasG4PhotonConversion::run] ";
    G4cout << "Killing photon track." << G4endl;
  #endif
  fastStep.KillPrimaryTrack();
  fastStep.ProposePrimaryTrackPathLength(0.0);
}
