//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
//
//---------------------------------------------------------------------------
//
// ClassName:   G4EmStandardPhysics_HepEm
//
// Author:      M. Novak 18.10.2023
//
// Modified:
//
//----------------------------------------------------------------------------
//
// This class is:
// - a copy of the G4EmStandardPhysics EM CTR from the Atlas version of
//   Geant4-11.2.2-atlaspatch
// - but the native Geant4 e-/e+ and gamma processes can here be replaced with 
//   G4HepEm using a its tracking manager instead depending on the G4EmParameters
//   configuartion
// - if the G4EmParameters::Instance()->UseG4HepEm() is False (default):
//   ===> the original G4EmStandardPhysics is used
// - if the G4EmParameters::Instance()->UseG4HepEm() is True:
//   ===> a G4HepEm tracking manager is used for e-/e+ and gamma instead
//        The tracking manager is the G4HepEmTrackingManager that includes 
//        now eveything that an ATLAS Athena simulation require (even the  
//        ATLAS specific TRTTransitionRatiation process for e-/e+ when used 
//        inside Athena)
// - NOTE:
//   1. this G4EmParameters has been added to the the special geant4-11.2.2 branch
//   2. this EM constructor is used then in a dedicated FTFP_BERT_ATL_HepEm physics
//      list instead, which is the same as the FTFP_BERT_ATL physics list but:
//      - the original G4EmStandardPhysics is replace with this G4EmStandardPhysics_HepEm
//      - the G4EmExtraPhysics constructor is deactivated (as long there is no gamma/lepton
//        nuclear interactions in G4HepEm)
//

#ifndef G4EmStandardPhysics_HepEm_h
#define G4EmStandardPhysics_HepEm_h 1

#include "G4VPhysicsConstructor.hh"
#include "globals.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class G4EmStandardPhysics_HepEm : public G4VPhysicsConstructor
{
public:

  explicit G4EmStandardPhysics_HepEm(G4int ver=1, const G4String& name="G4EmStandard_HepEm", G4bool multipleStepsInMSCTransport=false);

  ~G4EmStandardPhysics_HepEm() override;

  void ConstructParticle() override;
  void ConstructProcess() override;

private:
  G4bool fMultipleStepsInMSCTransport;

};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
