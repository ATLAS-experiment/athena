/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "G4FatrasTransportTool.h"

#include <memory>

// Geant4 includes for for particle extrapolation
#include "G4FieldTrack.hh"
#include "G4FieldTrackUpdator.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4Navigator.hh"
#include "G4PVPlacement.hh"
#include "G4PathFinder.hh"
#include "G4TransportationManager.hh"

thread_local std::unique_ptr<G4PropagatorInField, G4FatrasTransportTool::Deleter> G4FatrasTransportTool::s_propagator;

void G4FatrasTransportTool::Deleter::operator()(G4PropagatorInField* ptr) const {
  if (!ptr) {
     return;
  }
  delete ptr->GetNavigatorForPropagating();
  delete ptr;
}


StatusCode G4FatrasTransportTool::finalize() {

  // Delete the world volume if we created it
  if (m_useSimplifiedGeo)
    delete m_worldVolume;

  return StatusCode::SUCCESS;
}

StatusCode G4FatrasTransportTool::initializePropagator() {
  ATH_MSG_INFO("Initializing G4PropagatorInField for thread "
               << G4Threading::G4GetThreadId());

  if (!m_worldVolume) {
    // If not set, get either the simplified or full world volume
    m_worldVolume = getWorldVolume();

    if (!m_worldVolume) {
      G4Exception("G4FatrasTransportTool", "FailedToGetWorldVolume",
                  FatalException,
                  "G4FatrasTransportTool: Failed to get world volume.");
      abort();
    }
    ATH_MSG_INFO("Using world volume: " << m_worldVolume->GetName());
    ATH_MSG_INFO("Transport will be stopped at volume: "
                 << m_transportLimitVolume.value());
    ATH_MSG_INFO("Maximum allowed number of steps in particle transport: "
                 << m_maxSteps.value());
  }

  // Check if we already have propagator set up for the current thread
  // If not, we create one
  if (!s_propagator) {
    s_propagator = std::unique_ptr<G4PropagatorInField, Deleter>(makePropagator());
  } else {
    ATH_MSG_ERROR(
        "G4FatrasTransportTool::initializePropagator() Propagator already "
        "initialized!");
  }

  return StatusCode::SUCCESS;
}

G4VPhysicalVolume* G4FatrasTransportTool::getWorldVolume() {

  if (m_useSimplifiedGeo) {

    ATH_MSG_INFO("Creating simplified world volume for particle transport");
    // Get the logical world volume of the simplified geometry by name
    G4LogicalVolume* logVol = G4LogicalVolumeStore::GetInstance()->GetVolume(
        m_simplifiedWorldLogName.value());

    // Create the physical volume of the simplified world
    return new G4PVPlacement(
        nullptr,                   // no rotation
        G4ThreeVector(0, 0, 0),    // world centre at (0,0,0)
        logVol,                    // logical volume
        "simplifiedWorldPhysVol",  // name of physical volume
        nullptr,                   // mother volume
        false,                     // not used
        999,                       // copy number
        false);                    // overlap check

  } else {
    ATH_MSG_INFO("Using full geometry for particle transport");
    return G4TransportationManager::GetTransportationManager()
        ->GetNavigatorForTracking()
        ->GetWorldVolume();
  }
}

G4PropagatorInField* G4FatrasTransportTool::makePropagator() {
  // Create a new navigator
  G4Navigator* navigator = new G4Navigator();
  // Set world volume in which the navigator will operate
  navigator->SetWorldVolume(m_worldVolume);
  // Get the global field manager
  G4FieldManager* fieldMgr =
      G4TransportationManager::GetTransportationManager()->GetFieldManager();
  // Create a new magnetic field propagator
  G4PropagatorInField* propagator =
      new G4PropagatorInField(navigator, fieldMgr);
  return propagator;
}

void G4FatrasTransportTool::doStep(G4FieldTrack& fieldTrack) const {

  // Get the propagator and navigator for the current thread
  auto navigator = s_propagator->GetNavigatorForPropagating();

  G4double retSafety = -1.0;
  G4double currentMinimumStep = 10.0 * CLHEP::m;

  G4VPhysicalVolume* currentPhysVol =
      navigator->LocateGlobalPointAndSetup(fieldTrack.GetPosition(), nullptr);

  G4ThreeVector direction = fieldTrack.GetMomentumDirection();
  // Must be called before calling the computeStep method
  navigator->LocateGlobalPointAndSetup(fieldTrack.GetPosition(), &direction);

  if (fieldTrack.GetCharge() == 0) {
    /* Neutral particles: transport with navigator */

    // Compute the step length
    G4double stepLength = navigator->ComputeStep(
        fieldTrack.GetPosition(), fieldTrack.GetMomentumDirection(),
        currentMinimumStep, retSafety);

    // Update the position of the track from the computed step length
    fieldTrack.SetPosition(fieldTrack.GetPosition() +
                           stepLength *
                               fieldTrack.GetMomentumDirection().unit());

  } else {
    /* Charged particles: transport with magnetic field propagator */
    s_propagator->ComputeStep(fieldTrack, currentMinimumStep, retSafety,
                            currentPhysVol);
  }

  return;
}

std::vector<G4FieldTrack> G4FatrasTransportTool::transport(
    const G4Track& G4InputTrack) const {

  // Get the PDG ID of the particle
  int pdgId = G4InputTrack.GetDefinition()->GetPDGEncoding();

  // Get the navigator for the current thread
  auto navigator = s_propagator->GetNavigatorForPropagating();

  // Create a vector to store the output steps
  std::vector<G4FieldTrack> outputStepVector;

  // Initialize the tmpFieldTrack with the input track
  G4FieldTrack tmpFieldTrack('0');
  G4FieldTrackUpdator::Update(&tmpFieldTrack, &G4InputTrack);
  // Fill with the initial particle position
  outputStepVector.push_back(tmpFieldTrack);

  // Iterate until we reach the maximum number of steps or the requested volume
  for (unsigned int iStep = 0; iStep < m_maxSteps; iStep++) {
    // Save preStep information
    G4ThreeVector preStepPos = tmpFieldTrack.GetPosition();
    G4ThreeVector preStepMom = tmpFieldTrack.GetMomentum();

    // Perform a single Geant4 step
    doStep(tmpFieldTrack);

    // Fill the output vector with the updated track
    outputStepVector.push_back(tmpFieldTrack);

    // Get the name of the volume in which the particle is located
    auto volume = navigator->LocateGlobalPointAndSetup(
        tmpFieldTrack.GetPosition(), nullptr);

    if (volume != nullptr) {
      // Get the name of the current volume
      std::string volName = volume->GetName();

      // We stop the track navigation once we have reached the provided volume
      if (volName.find(m_transportLimitVolume) != std::string::npos) {
        break;
      }
    } else {
      G4ExceptionDescription description;
      description
          << "Transport failure for particle PID: " << pdgId << " at step "
          << iStep << "/" << m_maxSteps << G4endl
          << " - PreStep position: " << preStepPos << G4endl
          << " - PreStep momentum: " << preStepMom << G4endl
          << " - PostStep position: " << tmpFieldTrack.GetPosition() << G4endl
          << " - PostStep momentum: " << tmpFieldTrack.GetMomentum() << G4endl
          << "Possible cause: The transport is likely outside the world volume."
          << G4endl
          << "Check if an envelope volume is defined and properly set up."
          << G4endl << "This issue should not occur during normal operation.";
      G4Exception("G4FatrasTransportTool::transport",
                  "LocateGlobalPointAndSetup failed: Particle may be "
                  "transported outside the world volume.",
                  JustWarning, description);
      break;
    }
  }

  return outputStepVector;
}
