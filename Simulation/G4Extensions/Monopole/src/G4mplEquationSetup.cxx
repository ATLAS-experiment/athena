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
/// \file Derived from example exoticphysics G4MonopolFieldSetup.cc
/// \brief Implementation of the G4mplEquationSetup class
//
//
//
// G4mplEquationSetup is responsible for switching between two different
// equation of motions, one for monopoles and another for the rest of the
// particles.
//
// This updated version is designed to cope with:
//   - multiple G4FieldManager objects being defined in the setup
//   - many G4MagneticField objects (i.e a global and one or more local fiedls)
//   - a G4FieldManager which switches the G4Field object to which it points
//       during the simulation (e.g. to null)

// =======================================================================
// Modified: 19 May 2019, M. Bandieramonte: introduced MT mode. The class
//           was a Singleton and it was not thread-safe.
//           Added the #ifdef G4MULTITHREADED directive to handle
//           the multithreaded case. One instance of the class will be created
//           per each thread and stored in a tbb::concurrent_unordered_map that
//           is hashed with the threadID number.
// Modified: 28 August 2013, W. Taylor: adapted for ATLAS
// Created:  23 May 2013,    J. Apostolakis
//            Adapted from G4MonopoleFieldSetup by B. Bozsogi
// =======================================================================

// class header
#include "G4mplEquationSetup.hh"
// package headers
#include "G4mplEqMagElectricField.hh"
// Geant4 headers
#include "G4MagneticField.hh"
#include "G4UniformMagField.hh"
#include "G4FieldManager.hh"
#include "G4TransportationManager.hh"
#include "G4Mag_UsualEqRhs.hh"
#include "G4MagIntegratorStepper.hh"
#include "G4ChordFinder.hh"
#include "G4MagIntegratorDriver.hh"
#include "G4AtlasRK4.hh" // Used for non-magnetic particles
#include "G4ClassicalRK4.hh" // Used for monopoles
// NOTE:
// Only Steppers which can integrate over any number of variables can be used
//  to integrate the equation of motion of a monopole.
// The following cannot be used:
//    Helix...  - these integrate over 6 variables only; also assume helix as 'base'
//    AtlasRK4, NystromRK4 - these have the equation of motion embedded inside
#include "G4SystemOfUnits.hh"
#include "G4UniformMagField.hh"

#ifdef G4MULTITHREADED
G4mplEquationSetup::ESThreadMap_t G4mplEquationSetup::m_ESThreadMap;
#endif

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4mplEquationSetup::G4mplEquationSetup():
  fMinStep(0.01*CLHEP::mm), // minimal step of 1 mm is default
  fVerbose(false)
{
  if( fVerbose )
    G4cout << "!!! G4mplEquationSetup constructor" <<  G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4mplEquationSetup* G4mplEquationSetup::GetInstance()
{
#ifdef G4MULTITHREADED
    auto es = getES();
    if (!es) //nullpointer if it is not found
      return setES();
    else return es;
#else
    //Standard implementation of a Singleton Pattern
    static G4mplEquationSetup instance;
    return &instance;
#endif

}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4mplEquationSetup::~G4mplEquationSetup()
{
  if( fVerbose )   
    G4cout << "!!! G4mplEquationSetup destructor" << G4endl;

  delete fMonopoleEquation;
  delete fMonopoleStepper;
  
  delete fMonopoleChordFinder;
  //  JA: Is protection below still needed ?
  // WJT: Avoid segmentation violation in G4FieldManager destructor
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#ifdef G4MULTITHREADED
  G4mplEquationSetup* G4mplEquationSetup::getES()
  {
   // Get current thread-ID
   const auto tid = std::this_thread::get_id();
   auto esPair = m_ESThreadMap.find(tid);
   if(esPair == m_ESThreadMap.end())
      return nullptr; //if not found return null pointer
   else return esPair->second;
  }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......   

   G4mplEquationSetup* G4mplEquationSetup::setES()
   {
      G4mplEquationSetup* instance = new G4mplEquationSetup;
      const auto tid = std::this_thread::get_id();
      auto inserted = m_ESThreadMap.insert( std::make_pair(tid, instance)).first;
      return (G4mplEquationSetup*) inserted->second;
   }
#endif

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

// The ownership model of the current implementation is the following:
//   An instance of G4mplEquationSetup owns
//    - the equation fMonopoleEquation
//    - the stepper   fMonopoleStepper
//    - the chord-finder fChordFinder
// They are transiently associated with the current FieldManager during the
//    'stepping' loop of the one monopole track.

// InitialiseForField must be called either
//  - at the start of the simulation (when the global field manager has been assigned, or else
//  - at the start of tracking of the first monopole track (and never later than this!)

void
G4mplEquationSetup::InitialiseForField(G4FieldManager* fieldManager )
{
  if (fieldManager == nullptr) {
    return;
  }

  // Store the 'original ChordFinder of the volume - to enable us to restore it,
  //  when either we
  //     i. enter a volume with a different field manager or
  //    ii. have finished tracking the current track.           Fix: J.A. 27/10/2022
  fOriginalChordFinder = fieldManager->GetChordFinder();
  fCurrentFieldManager = fieldManager;
  // How to ensure that two/multiple calls do not overwrite it?
  // Consider whether it can be null  
  
  const G4MagneticField* magField = dynamic_cast<const G4MagneticField*>(fieldManager->GetDetectorField());
  G4MagneticField* magFieldNC ATLAS_THREAD_SAFE = const_cast<G4MagneticField*>(magField);

  CreateStepperToChordFinder(magFieldNC);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void
G4mplEquationSetup::CreateStepperToChordFinder(G4MagneticField* magFieldNC)
{
  if(magFieldNC == nullptr)
  {
     static G4ThreadLocal G4UniformMagField nullField( G4ThreeVector(0.0, 0.0, 0.0));
     magFieldNC= &nullField;
  }

  delete fMonopoleEquation;
  fMonopoleEquation = new G4mplEqMagElectricField(magFieldNC);

  delete fMonopoleStepper;
  fMonopoleStepper = new G4ClassicalRK4( fMonopoleEquation, 8 ); // for time information..
  
  if ( !fMonopoleStepper )
  {
      G4ExceptionDescription ermsg;       
      ermsg << "The creation of a stepper for Monopoles failed - trying to use G4ClassicalRK4";
      G4Exception("G4mplEquationSetup::InitialiseForField",
                  "FailureToCreateObject", FatalException, ermsg);
  }
  
  delete fMonopoleChordFinder;

  auto integrDriver = new G4MagInt_Driver( fMinStep, fMonopoleStepper,
                                           fMonopoleStepper->GetNumberOfVariables() );
  fMonopoleChordFinder = new G4ChordFinder( integrDriver );
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

// This method must be called before 

void G4mplEquationSetup::SwitchStepperAndChordFinder(G4bool useMonopoleEq,
                                                     G4FieldManager* fieldManager)
{
  // For the existing version to work, InitialiseForField must be called 
  //   - for *this* field manager (not another) before this call
  //   - potentially after a change of 'field' object in this field manager (tbc)
  const G4MagneticField* magField = dynamic_cast<const G4MagneticField*>(fieldManager->GetDetectorField());
  G4MagneticField* magFieldNC ATLAS_THREAD_SAFE = const_cast<G4MagneticField*>(magField);  // fieldManager only allow const access despite being non-const

  // New code is needed to cope with the following cases:
  //  - a different field manager is encountered (i.e. a local one or second local one)
  //  - the 'field' object to which the G4FieldManager is pointing was changed
  
  if ( magFieldNC )
  {
     // First check whether the field in equation of motion has changed!
     CheckAndUpdateField( magFieldNC );
     
     if (useMonopoleEq) {
        fieldManager->SetChordFinder( fMonopoleChordFinder );
     } else {
        fieldManager->SetChordFinder( fOriginalChordFinder );
     }
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void G4mplEquationSetup::ResetIntegration(G4FieldManager* fieldManager)
// Restore the original ChordFinder to either
//  - allow non-monopole tracks to work seamlessly
//  - prepare for move of monopole to volume/region with different fieldManager
{
  if( fieldManager != fCurrentFieldManager){
    G4ExceptionDescription ermsg;       
    ermsg << "This method expects to be called with the chordfinder it knows"
          << "  yet its is called with " << fieldManager
          << "  and expected " << fCurrentFieldManager << G4endl;
    G4Exception("G4mplEquationSetup::ResetIntegration",
                "InvalidState", FatalException, ermsg);
  }
  
  if( fCurrentFieldManager ) {
    fCurrentFieldManager->SetChordFinder(fOriginalChordFinder);

    if ( fVerbose )
      G4cout << " G4mplEquationSetup: Reset Chord-Finder to original one (for pure electric charge): "
             << fOriginalChordFinder << G4endl;

    fOriginalChordFinder  = nullptr; // Now Forget it!
    fCurrentFieldManager = nullptr;
  }
  // Since these objects referred to the outgoing field manager, clean them up!  
  delete fMonopoleEquation;    fMonopoleEquation= nullptr;
  delete fMonopoleStepper;     fMonopoleStepper=  nullptr;
  delete fMonopoleChordFinder; fMonopoleChordFinder= nullptr;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void G4mplEquationSetup::CheckAndUpdateField( G4MagneticField* magFieldNC )
//  Ensure that the field of this fieldManager is the one in the equation - else update it to be so.
{
  if( magFieldNC != fMonopoleEquation->GetFieldObj() )
  {
    // fMonopoleEquation->SetFieldObj( magFieldNC );  // Dangerous method -- but types are both G4MagneticField
    // A cleaner version will call again Initialise ...
    CreateStepperToChordFinder(magFieldNC);
  }
}

