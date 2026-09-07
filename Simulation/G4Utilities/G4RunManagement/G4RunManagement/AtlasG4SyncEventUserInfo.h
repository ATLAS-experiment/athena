/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4RUNMANAGEMENT_AtlasG4SyncEventUserInfo_H
#define G4RUNMANAGEMENT_AtlasG4SyncEventUserInfo_H

#include <functional>
#include <memory>
#include <mutex>
#include <condition_variable>

#include "GaudiKernel/EventContext.h"

#include "AthenaKernel/IEvtIdModifierSvc.h"
#include "MCTruth/AtlasG4EventUserInfo.h"

// Forward declaration
namespace CLHEP{
  class HepRandomEngine;
}

class G4Event;

// Synchronization variables stored in a separate structure
// to allow access from Athena thread after G4EventInfo are destroyed
class G4EventSynchronizationInterface
{
  enum class EventStatus {
      Ready,
      Done,
      Size
    };
  public:
    enum class EventOutcome {
      Success,
      Aborted,
      PreparationFailed,
      RunTerminated
    };

    EventStatus Status() const;
    /// Record the first completion outcome and wake Athena.
    void Complete(EventOutcome outcome) noexcept;
    void WaitStatusDone() {WaitStatus(EventStatus::Done);};
    EventOutcome Outcome() const;
    
  private:
    void WaitStatus(const EventStatus&);

    EventOutcome m_outcome{EventOutcome::Success};
    EventStatus m_status{EventStatus::Ready};
    mutable std::mutex m_mutex;
    std::condition_variable m_cv;
};

// Extension of AtlasG4EventUserInfo to store Athena-G4 interface data necessary for synchronized event processing
class AtlasG4SyncEventUserInfo : public AtlasG4EventUserInfo
{
  public:
    using EventFactoryFunction = std::function<StatusCode(G4Event&)>;
    using SPSyncInterface = std::shared_ptr<G4EventSynchronizationInterface>;

    AtlasG4SyncEventUserInfo(CLHEP::HepRandomEngine*, EventFactoryFunction&&, const EventContext&);

    event_number_t AthenaEventID() const {
      return GetEventContext().eventID().event_number();
    }

    CLHEP::HepRandomEngine* HepRandomEngine() {
      return m_rng_engine;
    }

    const EventFactoryFunction& EventFactory() const {
      return m_event_factory;
    }

    void SetEventPreparationFailed() {
      m_event_preparation_failed = true;
    }

    bool EventPreparationFailed() const {
      return m_event_preparation_failed;
    }

    SPSyncInterface SyncInterface() const {
      return m_sync_interface;
    }

  private:
    // Random engine for this event seeded by Athena
    CLHEP::HepRandomEngine* m_rng_engine{nullptr};
    // Factory function for Athena to prepare the G4 event
    EventFactoryFunction m_event_factory;
    // Set on the Geant4 worker if the event factory returns failure.
    bool m_event_preparation_failed{false};
    // Synchronization interface between Athena and Geant4 for this event
    // held in a shared pointer to allow access from Athena after G4Event is destroyed
    SPSyncInterface m_sync_interface;
};

#endif // G4RUNMANAGEMENT_AtlasG4SyncEventUserInfo_H
