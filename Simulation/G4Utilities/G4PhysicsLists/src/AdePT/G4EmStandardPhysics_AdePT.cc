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
// ClassName:   G4EmStandardPhysics_AdePT
//
// Author:      M. Novak 18.10.2023
//
// Modified:    S. Diederichs 22.07.2025
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
//----------------------------------------------------------------------------
//

#include "G4EmStandardPhysics_AdePT.hh"

#include "G4HepEmTrackingManager.hh"
#include "G4HepEmConfig.hh"
#include "G4HepEmParameters.hh"

#include <AdePT/g4integration/AdePTConfiguration.hh>
#include <AdePT/g4integration/AdePTTrackingManager.hh>

#include "G4SystemOfUnits.hh"
#include "G4ParticleDefinition.hh"
#include "G4EmParameters.hh"
#include "G4EmBuilder.hh"
#include "G4LossTableManager.hh"

#include "G4ComptonScattering.hh"
#include "G4KleinNishinaModel.hh"
#include "G4GammaConversion.hh"
#include "G4PhotoElectricEffect.hh"
#include "G4RayleighScattering.hh"
#include "G4LivermorePhotoElectricModel.hh"
#include "G4LivermorePolarizedRayleighModel.hh"
#include "G4PhotoElectricAngularGeneratorPolarized.hh"

#include "G4hMultipleScattering.hh"
#include "G4CoulombScattering.hh"
#include "G4eCoulombScatteringModel.hh"
#include "G4WentzelVIModel.hh"
#include "G4UrbanMscModel.hh"

#include "G4eIonisation.hh"
#include "G4eBremsstrahlung.hh"
#include "G4eplusAnnihilation.hh"

#include "G4hIonisation.hh"
#include "G4ionIonisation.hh"
#include "G4NuclearStopping.hh"

#include "G4Gamma.hh"
#include "G4Electron.hh"
#include "G4Positron.hh"
#include "G4GenericIon.hh"

#include "G4PhysicsListHelper.hh"
#include "G4BuilderType.hh"
#include "G4EmModelActivator.hh"
#include "G4GammaGeneralProcess.hh"
#include "G4WoodcockProcess.hh"

// factory
#include "G4PhysicsConstructorFactory.hh"
//
G4_DECLARE_PHYSCONSTR_FACTORY(G4EmStandardPhysics_AdePT);

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4EmStandardPhysics_AdePT::G4EmStandardPhysics_AdePT(G4int ver, const G4String&)
  : G4VPhysicsConstructor("G4EmStandard_AdePT")
{

  fAdePTConfiguration = new AdePTConfiguration();

  SetVerboseLevel(ver);
  G4EmParameters* param = G4EmParameters::Instance();
  param->SetDefaults();
  param->SetVerbose(ver);
  param->SetGeneralProcessActive(true);
  param->SetFluctuationType(fUrbanFluctuation);
  SetPhysicsType(bElectromagnetic);

//  param->SetMscRangeFactor(0.04);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4EmStandardPhysics_AdePT::~G4EmStandardPhysics_AdePT()
{
  // delete fAdePTConfiguration;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void G4EmStandardPhysics_AdePT::ConstructParticle()
{
  // minimal set of particles for EM physics
  G4EmBuilder::ConstructMinimalEmSet();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void G4EmStandardPhysics_AdePT::ConstructProcess()
{
  if(verboseLevel > 1) {
    G4cout << "### " << GetPhysicsName() << " Construct Processes " << G4endl;
  }
  G4EmBuilder::PrepareEMPhysics();

  G4PhysicsListHelper* ph = G4PhysicsListHelper::GetPhysicsListHelper();
  G4EmParameters* param = G4EmParameters::Instance();

  // processes used by several particles
  G4hMultipleScattering* hmsc = new G4hMultipleScattering("ionmsc");

  // nuclear stopping is enabled if th eenergy limit above zero
  G4double nielEnergyLimit = param->MaxNIELEnergy();
  G4NuclearStopping* pnuc = nullptr;
  if(nielEnergyLimit > 0.0) {
    pnuc = new G4NuclearStopping();
    pnuc->SetMaxKinEnergy(nielEnergyLimit);
  }

  std::cout << " ### AdePT is active: using the AdePTTrackingManager for e-/e+ and gamma." << std::endl;
  // NOTE: hardcode verboseLevel to 1 as silenced
  // TOD: remove this line below! 
  verboseLevel = 1;

  // Construct the G4HepEm tracking manager and configure: 
  fTrackingManager = new AdePTTrackingManager(fAdePTConfiguration, /*verbosity=*/0);

  if (!fAdePTConfiguration) {
    std::cout << " ADEPT CONFIGURATION SHOULD NOT BE ZERO " << std::endl;
    std::abort();
  }


  G4HepEmConfig* config = fTrackingManager->GetG4HepEmConfig();
  // Apply Woodcock tracking of photons in the EMEC and EMB
  config->SetWoodcockTrackingRegion("EMEC");
  config->SetWoodcockTrackingRegion("EMB");
  // Disable the e+ correction to the \theta0 in the Urban MSC
  // NOTE: this should be set explicitely here only when using g4-10.6
  //       (as the e+ correction to theta0 is turend off in the ATLAS version)
  //       but g4-11.x has the flag to control this. 
  //      G4HepEmParameters* hepEmPars = config->GetG4HepEmParameters(); 
  //      hepEmPars->fIsMSCPositronCor = false;

  // Turning off energy loss fluctuation for e-/e+
  // config->SetEnergyLossFluctuation(false);
  // (also possible for a given region: e.g. config->SetEnergyLossFluctuation(false, "EMEC");

  // Don't allow to make multiple steps in the combined MSC+transportation
  // NOTE: this might lead to some performance loss but keeps the physics validation the same as with Geant4
  config->SetMultipleStepsInMSCWithTransportation(false);

  // Attach the tracking manager to e-/e+ and gamma
  G4Electron::Definition()->SetTrackingManager(fTrackingManager);
  G4Positron::Definition()->SetTrackingManager(fTrackingManager);
  G4Gamma::Definition()->SetTrackingManager(fTrackingManager);


  // generic ion
  G4ParticleDefinition* particle = G4GenericIon::GenericIon();
  G4ionIonisation* ionIoni = new G4ionIonisation();
  ph->RegisterProcess(hmsc, particle);
  ph->RegisterProcess(ionIoni, particle);
  if(nullptr != pnuc) { ph->RegisterProcess(pnuc, particle); }

  // muons, hadrons ions
  G4EmBuilder::ConstructCharged(hmsc, pnuc);

  // extra configuration
  G4EmModelActivator mact(GetPhysicsName());
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
