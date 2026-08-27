/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4RUNMANAGEMENT_G4RUNTOOLEVENTQUEUE_H
#define G4RUNMANAGEMENT_G4RUNTOOLEVENTQUEUE_H

#include "G4RunManagement/AtlasG4SyncEventUserInfo.h"

#include <condition_variable>
#include <cstddef>
#include <memory>
#include <mutex>
#include <queue>
#include <vector>

/// Thread-safe event handoff between Athena and the Geant4 worker threads.
///
/// Events submitted while the queue is open are tracked until their Athena
/// waiters have been notified. Closing the queue wakes every blocked Geant4
/// worker; subsequent GetEvent() calls return nullptr so that SyncEventAction
/// aborts the run. Once the workers have stopped, CompleteOutstandingEvents()
/// releases any abandoned events and wakes their Athena waiters.
class G4RunToolEventQueue
{
 public:
  using UPEvent = std::unique_ptr<AtlasG4SyncEventUserInfo>;

  std::size_t Size() const;
  /// Submit a non-null event. Throws std::invalid_argument for null events.
  void PushEvent(UPEvent event);
  UPEvent GetEvent();

  /// Stop accepting events and wake all Geant4 workers. Idempotent.
  void Close() noexcept;

  /// Release abandoned events and report run termination to their waiters.
  /// Must be called only after all Geant4 workers have stopped. Idempotent.
  void CompleteOutstandingEvents() noexcept;

 private:
  using WeakSyncInterface =
    std::weak_ptr<G4EventSynchronizationInterface>;

  std::queue<UPEvent> m_events;
  std::vector<WeakSyncInterface> m_outstandingEvents;
  bool m_closed{false};
  mutable std::mutex m_mutex;
  std::condition_variable m_cv;
};

#endif
