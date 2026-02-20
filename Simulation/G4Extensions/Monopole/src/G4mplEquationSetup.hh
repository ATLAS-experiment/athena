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
// $Id: $
//
// G4mplEquationSetup is responsible for
//  - creating the stepper, equation of motion and chord finder for one
//     or more fields (& managers) in a setup;
//  - switching to use the equation of motion for monopole tracks;
//  - providing the way to restore the original chord finder, when a
//     monopole track moves to a different field-mgr, or ends its tracking.
//
// =======================================================================
// Modified:  2 Nov 2022, J. Apostolakis:
//           - correct description of ownership/lifetimes of objects
//           - added ResetIntegration method that restore 'original' chordfinder
//              object (for pure electic charge)  after end of monopole track
//              stepping, or when crossing a boundary to a new field manager.
//           - reduce churn of G4ChordFinder objects
//           
// Modified: 19 May 2019, M. Bandieramonte: introduced MT mode. The class
// 	     was a Singleton and it was not thread-safe.
//           Added the #ifdef G4MULTITHREADED directive to handle
//           the multithreaded case. One instance of the class will be created
//           per each thread and stored in a tbb::concurrent_unordered_map that
//           is hashed with the threadID number.
//
// Modified: 28 August 2013, W. Taylor: adapted for ATLAS
// Created:  23 May 2013, J. Apostolakis
//            Adapted from G4MonopoleFieldSetup by B. Bozsogi
// =======================================================================

#ifndef MONOPOLE_G4mplEquationSetup_H
#define MONOPOLE_G4mplEquationSetup_H

#include <thread>
#include <memory>
#include "G4Types.hh"
#ifdef G4MULTITHREADED
#  include "tbb/concurrent_unordered_map.h"
#endif

// Geant4 headers
#include "G4MagneticField.hh"

#include "CxxUtils/checker_macros.h"

class G4FieldManager;
class G4ChordFinder;
class G4Mag_UsualEqRhs;
class G4MagIntegratorStepper;
class G4mplEqMagElectricField;
class G4MagneticField;

class G4mplEquationSetup
{
public:

  void InitialiseForField(G4FieldManager* fieldManager );
  // Prepare classes needed integration of 'Monopole' transport, but do not put it in place
  // Must be called every time the field manager can change (=> for each monopole track in application with many field Mgrs! )

  void SwitchStepperAndChordFinder(G4bool val, G4FieldManager* fieldManager );
  // Switch between mode for 'Monopole' and ordinary propagation
  //  0 / false - ordinary
  //  1 / true  - monopole
  // Uses the original 'ordinary' ChordFinder or 'our' monolope ChordFinder

  void ResetIntegration(G4FieldManager* fieldManager);
  //  Restore the original integration classes (ChordFinder+dependents) to current field Manager.
  //  Checks (asserts) that the fieldManager was the original one (when replacing.)

  void CheckAndUpdateField( G4FieldManager* fieldManager); 
  //  Ensure that the field of this fieldManager is the one in the equation - else update it to be so.
   
  static G4mplEquationSetup* GetInstance();

  ~G4mplEquationSetup() ;

private: 

#ifdef G4MULTITHREADED
     // Thread-to-EquationSetup concurrent map type
     using ESThreadMap_t = tbb::concurrent_unordered_map< std::thread::id, G4mplEquationSetup*, std::hash<std::thread::id> >;
     // Concurrent map of EquationsSetup, one for each thread
     static ESThreadMap_t m_ESThreadMap ATLAS_THREAD_SAFE;
     //@brief Search inside m_ESThreadMap the element with the current threadID 
     // and return it or return a null pointer if the element is not found
     static G4mplEquationSetup* getES();
     // @brief Insert the current ES in m_ESThreadMap and 
     // associate it with the current threadID
     static G4mplEquationSetup* setES();
 #endif
     //

  G4mplEquationSetup();

  // Internal method

  // 1. to recreate all key objects - stepper, equation & chord-finder for monopole
  void CreateStepperToChordFinder(G4MagneticField* magFieldNC);
  // 2. to check whether the field object was changed -- by a field manager reconfiguring itself
  void CheckAndUpdateField( G4MagneticField* magField );
   
  //
  // Invariants - constant during tracking
  // ----------

  // Parameters - invariant during stepping
  G4double                fMinStep ;
  G4bool                  fVerbose;

  //
  // State - changed during tracking
  // -----

  // Objects owned -- created during tracking 
  //                  and transiently associated with the current FieldManager object.
  std::unique_ptr<G4mplEqMagElectricField>  fMonopoleEquation;
  std::unique_ptr<G4MagIntegratorStepper>   fMonopoleStepper;
  std::unique_ptr<G4ChordFinder>            fMonopoleChordFinder;

  // Cache of objects related to the current state - for consistency
  G4FieldManager*   fCurrentFieldManager = nullptr; // Memory of this needed to ensure integrity
  G4ChordFinder*    fOriginalChordFinder = nullptr; // ChordFinder object (for ordinary charged tracks) of current FM
};

#endif
