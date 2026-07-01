// SPDX-FileCopyrightText: 2022 CERN
// SPDX-License-Identifier: Apache-2.0

#include "G4ios.hh"

#include "G4DecayPhysics.hh"
#include "G4EmStandardPhysics.hh"
#include "G4EmExtraPhysics.hh"
#include "G4IonPhysics.hh"
#include "G4StoppingPhysics.hh"
#include "G4HadronElasticPhysics.hh"
#include "G4NeutronTrackingCut.hh"
#include "G4HadronPhysicsFTFP_BERT_ATL.hh"

#include "G4WarnPLStatus.hh"
#include "G4HadronicParameters.hh"

#include <accel/AlongStepFactory.hh>
#include <accel/CartMapMagneticField.hh>
#include "accel/TrackingManagerConstructor.hh"
#include <accel/TrackingManagerIntegration.hh>
#include <accel/SetupOptions.hh>

#include "FTFP_BERT_ATL_Celer.hh"

FTFP_BERT_ATL_Celer::FTFP_BERT_ATL_Celer(G4int ver)
{
  if(ver > 0) {
    G4cout << "<<< Geant4 Physics List simulation engine: FTFP_BERT_ATL_Celer"<<G4endl;
    G4cout << G4endl;
    G4WarnPLStatus exp;
    exp.Experimental("FTFP_BERT_ATL_Celer");
  }
  defaultCutValue = 0.7*CLHEP::mm;  
  SetVerboseLevel(ver);

  G4HadronicParameters::Instance()->SetBertiniAs11_2(true);

  // EM Physics
  RegisterPhysics(new G4EmStandardPhysics());

  // Register the Celeritas physics
  auto& tmi = celeritas::TrackingManagerIntegration::Instance();
  celeritas::SetupOptions opts;

  opts.make_along_step = celeritas::CartMapFieldAlongStepFactory([] {
    celeritas::CartMapFieldGridParams params;

    params.x.min = -15000;  // grid min in mm
    params.x.max = 15000;   // grid max in mm
    params.x.num = 64;      // number of grid points

    params.y.min = -15000;
    params.y.max = 15000;
    params.y.num = 64;
    
    params.z.min = -25000;
    params.z.max = 25000;
    params.z.num = 128;
    return celeritas::MakeCartMapFieldInput(params);
  });

  opts.ignore_processes = {"CoulombScat"};
  tmi.SetOptions(std::move(opts));
  RegisterPhysics(new celeritas::TrackingManagerConstructor(&tmi));

  // Synchroton Radiation & GN Physics
  // comenting out to remove gamma- and lepto-nuclear processes
  // as Celeritas doesn't support these yet (NB: DIFFERENT from FTFP_BERT_ATL_AdePT!!)
  // RegisterPhysics( new G4EmExtraPhysics(ver) );

  // Decays
  RegisterPhysics(new G4DecayPhysics(ver));

  // Hadron Elastic scattering
  RegisterPhysics(new G4HadronElasticPhysics(ver));

  // Hadron Physics
  RegisterPhysics(new G4HadronPhysicsFTFP_BERT_ATL(ver));

  // Stopping Physics
  RegisterPhysics(new G4StoppingPhysics(ver));

  // Ion Physics
  RegisterPhysics(new G4IonPhysics(ver));

  // Neutron tracking cut
  RegisterPhysics(new G4NeutronTrackingCut(ver));
}
