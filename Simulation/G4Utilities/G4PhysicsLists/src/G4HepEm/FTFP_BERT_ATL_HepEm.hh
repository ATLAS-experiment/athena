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
// - a copy of the FTFP_BERT_ATL physics list from the Atlas version of
//   Geant4-11.2.2-atlaspatch
// - added the option of switching to use the local G4EmStandardPhysics_HepEm 
//   EM physics constructor (with a G4HepEm tracking manager for e-/e+ and gamma 
//   inside instead of the native Geant4 standard EM physics)
// - deactivated G4EmExtraPhysics as long there is no handling of gamma/lepton 
//   nuclear interactions in G4HepEm 
//----------------------------------------------------------------------------
//
#ifndef FTFP_BERT_ATL_HepEm_h
#define FTFP_BERT_ATL_HepEm_h 1

#include <CLHEP/Units/SystemOfUnits.h>

#include "globals.hh"
#include "G4VModularPhysicsList.hh"



class FTFP_BERT_ATL_HepEm: public G4VModularPhysicsList
{
public:
FTFP_BERT_ATL_HepEm(G4int ver = 1, G4bool multipleStepsInMSCTransport = false);
  virtual ~FTFP_BERT_ATL_HepEm()=default;

  FTFP_BERT_ATL_HepEm(const FTFP_BERT_ATL_HepEm &) = delete;
  FTFP_BERT_ATL_HepEm & operator=(const FTFP_BERT_ATL_HepEm &)=delete;
};

#endif

