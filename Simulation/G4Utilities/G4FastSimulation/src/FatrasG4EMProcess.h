/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4FASTSIMULATION_FATRASG4EMPROCESS_H
#define G4FASTSIMULATION_FATRASG4EMPROCESS_H

#include "G4EmProcessSubType.hh"
#include "G4VDiscreteProcess.hh"

#include <limits>


// Never attached to a particle: it only labels a FatrasG4 step for the truth.
// G4EmProcessSubType:
//   fCoulombScattering = 1, fIonisation = 2, fBremsstrahlung = 3, fPairProdByCharged = 4,
//   fAnnihilation = 5, fAnnihilationToMuMu = 6, fAnnihilationToHadrons = 7,
//   fNuclearStopping = 8, fMultipleScattering = 10, fComptonScattering = 13,
//   fGammaConversion = 14, fGammaConversionToMuMu = 15

class FatrasG4EMProcess : public G4VDiscreteProcess
{
 public:
  FatrasG4EMProcess(const G4String& name, G4int subType) : G4VDiscreteProcess(name, fElectromagnetic) {
    SetProcessSubType(subType);
  }

 protected:
  G4double GetMeanFreePath(const G4Track&, G4double, G4ForceCondition*) override {
    return std::numeric_limits<G4double>::max();
  }
};

#endif //G4FASTSIMULATION_FATRASG4EMPROCESS_H
