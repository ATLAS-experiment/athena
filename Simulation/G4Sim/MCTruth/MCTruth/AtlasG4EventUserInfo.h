/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MCTRUTH_ATLASG4EVENTUSERINFO_H
#define MCTRUTH_ATLASG4EVENTUSERINFO_H


#include <GaudiKernel/EventContext.h>
#include "AthenaKernel/ExtendedEventContext.h"
#include "AthenaKernel/IProxyDict.h"
#include "AtlasHepMC/GenEvent_fwd.h"
#include "HitManagement/HitCollectionMap.h"

#include <G4EventManager.hh>
#include "G4VUserEventInformation.hh"

#include <memory>
/** @class AtlasG4EventUserInfo

 * @brief This class is attached to G4Event objects as
 * UserInformation. It holds event-wide state such as the HepMC::GenEvent
 * used to create the G4Event and the hit collection map. Per-track truth
 * particles are stored on TrackInformation.
 */
class AtlasG4EventUserInfo: public G4VUserEventInformation {
public:
  AtlasG4EventUserInfo(const EventContext& ctx)
    : G4VUserEventInformation()
    , m_eventContext(ctx)
    , m_eventStore(Atlas::getExtendedEventContext(ctx).proxy())
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

  /**
   * @brief return the value of G4Track::GetTrackID() for the last
   * G4Step processed by a CaloCalibrationHit Sensitive Detector. Used
   * in CalibrationDefaultProcessing::UserSteppingAction(...) to
   * ensure that unprocessed G4Steps are passed to the default
   * CaloCalibrationHit sensitive detector. TODO Rename
   */
  int GetLastProcessedTrackID() const { return m_lastProcessedTrackID; }
  /**
   * @brief record the value of G4Track::GetTrackID() for the current
   * G4Step. Should be called by all CaloCalibrationHit Sensitive
   * Detectors after they process a G4Step. TODO Check this. TODO
   * Rename
   */
  void SetLastProcessedTrackID(int trackID) { m_lastProcessedTrackID = trackID; }

  /**
   * @brief return the value of the G4Track::GetCurrentStepNumber()
   * for the last G4Step processed by a CaloCalibrationHit Sensitive
   * Detector. Used in
   * CalibrationDefaultProcessing::UserSteppingAction(...) to ensure
   * that unprocessed G4Steps are passed to the default
   * CaloCalibrationHit sensitive detector.
   */
  int GetLastProcessedStep() const { return m_lastProcessedStep; }
  /**
   * @brief record value of the G4Track::GetCurrentStepNumber() for
   * the current G4Step. Should be called by all CaloCalibrationHit
   * Sensitive Detectors after they process a G4Step. TODO Check this
   * is done.
   */
  void SetLastProcessedStep(int stepNumber) { m_lastProcessedStep = stepNumber; }

  /**
   * @brief Get the HitCollectionMap object with shared ownership. Geant4 deleting this UserInfo
   * object will not delete the HitCollectionMap.
   */
  std::shared_ptr<HitCollectionMap> GetHitCollectionMap() const { return m_hitCollectionMap; }

  /**
   * @brief Set the HitCollectionMap object
   */
  void SetHitCollectionMap(std::shared_ptr<HitCollectionMap> hitCollections) {  m_hitCollectionMap = hitCollections; }

  const EventContext& GetEventContext() const { return m_eventContext; }

  IProxyDict* GetEventStore() { return m_eventStore; }

  void Print() const {}

  // Static helper method to get the AtlasG4EventUserInfo from G4EventManager
  static AtlasG4EventUserInfo* GetEventUserInfo()
  {
      // Event manager may be null in unit tests.
      G4EventManager* eventManager = G4EventManager::GetEventManager();
      if (!eventManager) {
          return nullptr;
      }
      // User info may be null
      return static_cast<AtlasG4EventUserInfo*>(eventManager->GetUserInformation());
  }


private:
  const EventContext& m_eventContext;
  IProxyDict* m_eventStore{};
  HepMC::GenEvent *m_theEvent{};

  std::shared_ptr<HitCollectionMap> m_hitCollectionMap{std::make_shared<HitCollectionMap>()};

  // These next two variables are used by the CaloCalibrationHit
  // recording code as event-level flags They correspond to the Track
  // ID and step number of the last G4Step processed by a
  // CaloCalibrationHit SD Both are needed, because a particle might
  // have only one step
  int m_lastProcessedTrackID{0};
  int m_lastProcessedStep{0};
};

#endif // MCTRUTH_ATLASG4EVENTUSERINFO_H
