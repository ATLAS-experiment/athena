/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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
    EventStatus Status() const;
    void SetStatusDone() {SetStatus(EventStatus::Done);};
    void WaitStatusDone() {WaitStatus(EventStatus::Done);};
    bool EventAborted() const { return m_event_aborted; }
    void EventAborted(bool aborted) { m_event_aborted = aborted; }
    
  private:
    void SetStatus(const EventStatus&);
    void WaitStatus(const EventStatus&);

    bool m_event_aborted{false};
    EventStatus m_status{EventStatus::Ready};
    mutable std::mutex m_mutex;
    std::condition_variable m_cv;
};

// Extension of AtlasG4EventUserInfo to store Athena-G4 interface data necessary for synchronized event processing
class AtlasG4SyncEventUserInfo : public AtlasG4EventUserInfo
{
  public:
    using EventFactoryFunction = std::function<StatusCode(G4Event&, std::unique_ptr<AtlasG4SyncEventUserInfo>)>;
    using SPSyncInterface = std::shared_ptr<G4EventSynchronizationInterface>;

    AtlasG4SyncEventUserInfo(CLHEP::HepRandomEngine*, EventFactoryFunction&&, const EventContext&);

    event_number_t AthenaEventID() const {
      return GetEventContext().eventID().event_number();
    }

    CLHEP::HepRandomEngine* HepRandomEngine() {
      return m_rng_engine;
    }

    EventFactoryFunction EventFactory() const {
      return m_event_factory;
    }

    SPSyncInterface SyncInterface() const {
      return m_sync_interface;
    }

  private:
    // Random engine for this event seeded by Athena
    CLHEP::HepRandomEngine* m_rng_engine{nullptr};
    // Factory function for Athena to prepare the G4 event
    EventFactoryFunction m_event_factory;
    // Synchronization interface between Athena and Geant4 for this event
    // held in a shared pointer to allow access from Athena after G4Event is destroyed
    SPSyncInterface m_sync_interface;
};

#endif// G4RUNMANAGEMENT_AtlasG4SyncEventUserInfo_H
