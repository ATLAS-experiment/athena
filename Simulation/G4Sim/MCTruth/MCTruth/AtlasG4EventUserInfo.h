/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MCTRUTH_ATLASG4EVENTUSERINFO_H
#define MCTRUTH_ATLASG4EVENTUSERINFO_H

#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenParticle.h"
#include "TruthUtils/MagicNumbers.h"
#include "G4ThreeVector.hh"
#include "G4VUserEventInformation.hh"

/** @class AtlasG4EventUserInfo

 * @brief This class is attached to G4Event objects as
 * UserInformation. It holds a pointer to the HepMC::GenEvent which
 * was used to create the G4Event.
 * NB As with VTrackInformation, the GenParticlePtr held by the
 * AtlasG4EventUserInfo object can change during simulation (i.e. each
 * time the track undergoes a non-destructive interaction).
 */
class AtlasG4EventUserInfo: public G4VUserEventInformation {
public:
  AtlasG4EventUserInfo()
    : G4VUserEventInformation()
  {}

  /**
   * @brief return a pointer to the HepMC::GenEvent used to create the
   * G4Event. (Never called. Remove?)
   */
  HepMC::GenEvent* GetHepMCEvent() ;
  /**
   * @brief set m_theEvent, the pointer to the HepMC::GenEvent used to
   * create the G4Event. Only called in ISF::InputConverter::ISF_to_G4Event(...).
   */
  void SetHepMCEvent(HepMC::GenEvent*);

  int GetNrOfPrimaryParticles() const; // Never called. TODO Remove
  void SetNrOfPrimaryParticles(int nr); // Only called in ISF::InputConverter::ISF_to_G4Event(...). TODO Remove

  int GetNrOfPrimaryVertices() const; // Never called. TODO Remove
  void SetNrOfPrimaryVertices(int nr); // Only called in ISF::InputConverter::ISF_to_G4Event(...). TODO Remove

  void SetVertexPosition(const G4ThreeVector&); // Never called. TODO Remove
  const G4ThreeVector GetVertexPosition() const; // Never called. TODO Remove

  /**
   * @brief return a pointer to the HepMC::GenParticle used to create
   * the current G4PrimaryParticle. (Used in G4VFastSimulationModel
   * implementations and Sensitive Detectors which record
   * CaloCalibrationHits.) TODO Rename
   */
  HepMC::ConstGenParticlePtr GetCurrentPrimary() const {return m_currentPrimary;}
  /**
   * @brief set m_currentPrimary, the pointer to the
   * HepMC::GenParticle used to create the current
   * G4PrimaryParticle. This pointer is updated each time there is a
   * new G4PrimaryParticle. Called from
   * (AthenaTrackingAction/TrackProcessorUserActionBase)::
   * PreUserTrackingAction(...). TODO Rename
   */
  void SetCurrentPrimary(HepMC::ConstGenParticlePtr p) {m_currentPrimary=p;}

  /**
   * @brief return a pointer to the GenParticle corresponding to the
   * current G4Track (if there is one). TODO Rename
   */
  HepMC::GenParticlePtr GetCurrentlyTraced() {return m_currentlyTraced;}
  HepMC::ConstGenParticlePtr GetCurrentlyTraced() const {return m_currentlyTraced;}
  /**
   * @brief set m_currentlyTraced, the pointer to the GenParticle
   * corresponding to the current G4Track. This will be updated each
   * time an interaction of the G4Track is recorded to the
   * HepMC::GenEvent. TODO Rename
   */
  void SetCurrentlyTraced(HepMC::GenParticlePtr p) {m_currentlyTraced=p;}

  /**
   * @brief return the value of G4Track::GetTrackID() for the last
   * G4Step processed by a CaloCalibrationHit Sensitive Detector. Used
   * in CalibrationDefaultProcessing::UserSteppingAction(...) to
   * ensure that unprocessed G4Steps are passed to the default
   * CaloCalibrationHit sensitive detector. TODO Rename
   */
  int GetLastProcessedBarcode() const { return m_last_processed_barcode; }
  /**
   * @brief record the value of G4Track::GetTrackID() for the current
   * G4Step. Should be called by all CaloCalibrationHit Sensitive
   * Detectors after they process a G4Step. TODO Check this. TODO
   * Rename
   */
  void SetLastProcessedBarcode(int b) { m_last_processed_barcode = b; }

  /**
   * @brief return the value of the G4Track::GetCurrentStepNumber()
   * for the last G4Step processed by a CaloCalibrationHit Sensitive
   * Detector. Used in
   * CalibrationDefaultProcessing::UserSteppingAction(...) to ensure
   * that unprocessed G4Steps are passed to the default
   * CaloCalibrationHit sensitive detector.
   */
  int GetLastProcessedStep() const { return m_last_processed_step; }
  /**
   * @brief record value of the G4Track::GetCurrentStepNumber() for
   * the current G4Step. Should be called by all CaloCalibrationHit
   * Sensitive Detectors after they process a G4Step. TODO Check this
   * is done.
   */
  void SetLastProcessedStep(int s) { m_last_processed_step = s; }

  void Print() const {}

private:
  G4ThreeVector m_vertexPosition; // TODO Remove
  int m_nrOfPrimaryParticles{0}; // TODO Remove
  int m_nrOfPrimaryVertices{0}; // TODO Remove
  HepMC::GenEvent *m_theEvent{};
  HepMC::ConstGenParticlePtr m_currentPrimary{};
  HepMC::GenParticlePtr m_currentlyTraced{};
  // These next two variables are used by the CaloCalibrationHit
  // recording code as event-level flags They correspond to the Track
  // ID and step number of the last G4Step processed by a
  // CaloCalibrationHit SD Both are needed, because a particle might
  // have only one step
  int m_last_processed_barcode{};
  int m_last_processed_step{};
};

#endif // MCTRUTH_ATLASG4EVENTUSERINFO_H
