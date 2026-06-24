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
//---------------------------------------------------------------------------
// Author: M. Novak 
// Date:   October 2024
//
// Modified:    S. Diederichs 22.07.2025
//
// - a copy of the FTFP_BERT_ATL physics list from the Atlas version of
//   Geant4-11.2.2-atlaspatch
// - added the option of switching to use the local G4EmStandardPhysics_HepEm 
//   EM physics constructor (with a G4HepEm tracking manager for e-/e+ and gamma 
//   inside instead of the native Geant4 standard EM physics)
// - deactivated G4EmExtraPhysics as long there is no handling of gamma/lepton 
//   nuclear interactions in G4HepEm 
//----------------------------------------------------------------------------
//
#include "FTFP_BERT_ATL_AdePT.hh"

#include "globals.hh"
#include "G4ios.hh"

#include "G4DecayPhysics.hh"
#include "G4EmStandardPhysics_AdePT.hh"
#include "G4EmStandardPhysics.hh"
#include "G4EmExtraPhysics.hh"
#include "G4IonPhysics.hh"
#include "G4StoppingPhysics.hh"
#include "G4HadronElasticPhysics.hh"
#include "G4NeutronTrackingCut.hh"

#include "G4HadronPhysicsFTFP_BERT_ATL.hh"

#include "G4WarnPLStatus.hh"
#include "G4FTFTunings.hh"
#include "G4HadronicParameters.hh"

#include <iomanip>   

FTFP_BERT_ATL_AdePT::FTFP_BERT_ATL_AdePT(G4int ver)
{
  if(ver > 0) {
    G4cout << "<<< Geant4 Physics List simulation engine: FTFP_BERT_ATL_AdePT"<<G4endl;
    G4cout <<G4endl;
    G4WarnPLStatus exp;
    exp.Experimental("FTFP_BERT_ATL_AdePT");
  }
  defaultCutValue = 0.7*CLHEP::mm;  
  SetVerboseLevel(ver);

  // OUTDATED, needed only when athena is run with v11.2 not 11.3
  // // Use the 4th tunes of Fritiof (FTF) string model, meant to to overcome
  // // the problem of too optimistic (i.e. narrow) pion shower energy resolutions
  // // in ATLAS calorimeters with respect to test-beam data.
  // G4FTFTunings::Instance()->SetTuneApplicabilityState( 4, 1 );
  
  // Revert Bertini Cascade model to use behaviour as in Geant4 v11.2.X. This
  // gives settings compatible with 11.1.X. See:
  // - Discussion in ATLASSIM-7441
  // - Geant4 11.3.2 Release Notes: https://geant4-data.web.cern.ch/ReleaseNotes/Patch.11.3-2.txt
  //   - Section "Processes - Hadronic"
  G4HadronicParameters::Instance()->SetBertiniAs11_2(true);


  // EM Physics
  RegisterPhysics( new G4EmStandardPhysics_AdePT(ver));

  // Synchroton Radiation & GN Physics
 RegisterPhysics( new G4EmExtraPhysics(ver) );

  // Decays 
  RegisterPhysics( new G4DecayPhysics(ver) );

   // Hadron Elastic scattering
  RegisterPhysics( new G4HadronElasticPhysics(ver) );

   // Hadron Physics
  RegisterPhysics( new G4HadronPhysicsFTFP_BERT_ATL(ver) );

  // Stopping Physics
  RegisterPhysics( new G4StoppingPhysics(ver) );

  // Ion Physics
  RegisterPhysics( new G4IonPhysics(ver));
  
  // Neutron tracking cut
  RegisterPhysics( new G4NeutronTrackingCut(ver));
}

