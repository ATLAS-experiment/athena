/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "G4RunToolEventQueue.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

std::size_t G4RunToolEventQueue::Size() const
{
  std::scoped_lock lock(m_mutex);
  return m_events.size();
}

void G4RunToolEventQueue::PushEvent(UPEvent event)
{
  if (!event) {
    throw std::invalid_argument(
      "G4RunToolEventQueue::PushEvent requires a non-null event");
  }

  auto syncInterface = event->SyncInterface();
  bool eventQueued = false;
  {
    std::scoped_lock lock(m_mutex);
    if (!m_closed) {
      m_outstandingEvents.erase(
        std::remove_if(m_outstandingEvents.begin(),
                       m_outstandingEvents.end(),
                       [](const WeakSyncInterface& interface) {
                         return interface.expired();
                       }),
        m_outstandingEvents.end());
      if (syncInterface) {
        m_outstandingEvents.emplace_back(syncInterface);
      }
      m_events.push(std::move(event));
      eventQueued = true;
    }
  }

  if (eventQueued) {
    m_cv.notify_one();
  }
  else if (syncInterface) {
    // The event was never exposed to Geant4, so its owned state can be safely
    // released before the Athena thread is notified.
    event.reset();
    syncInterface->Complete(
      G4EventSynchronizationInterface::EventOutcome::RunTerminated);
  }
}

auto G4RunToolEventQueue::GetEvent() -> UPEvent
{
  std::unique_lock lock(m_mutex);
  m_cv.wait(lock, [this] { return m_closed || !m_events.empty(); });
  if (m_closed) {
    return nullptr;
  }

  UPEvent event = std::move(m_events.front());
  m_events.pop();
  return event;
}

void G4RunToolEventQueue::Close() noexcept
{
  {
    std::scoped_lock lock(m_mutex);
    m_closed = true;
  }
  m_cv.notify_all();
}

void G4RunToolEventQueue::CompleteOutstandingEvents() noexcept
{
  std::vector<WeakSyncInterface> outstandingEvents;
  std::queue<UPEvent> abandonedEvents;
  {
    std::scoped_lock lock(m_mutex);
    outstandingEvents.swap(m_outstandingEvents);
    abandonedEvents.swap(m_events);
  }

  // Destroy queued event information before waking Athena. In-flight event
  // information has already been destroyed as part of worker termination.
  while (!abandonedEvents.empty()) {
    abandonedEvents.pop();
  }

  for (const WeakSyncInterface& weakInterface : outstandingEvents) {
    if (auto syncInterface = weakInterface.lock()) {
      syncInterface->Complete(
        G4EventSynchronizationInterface::EventOutcome::RunTerminated);
    }
  }
}
