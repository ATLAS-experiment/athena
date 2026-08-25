/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "G4RunManagement/AtlasG4SyncEventUserInfo.h"

#include <memory>
#include <mutex>

//---------------------------------------------------------------------------
// AtlasG4SyncEventUserInfo
//---------------------------------------------------------------------------

AtlasG4SyncEventUserInfo::AtlasG4SyncEventUserInfo(CLHEP::HepRandomEngine* rng_engine,
                                                  AtlasG4SyncEventUserInfo::EventFactoryFunction&& event_factory,
                                                  const EventContext& ctx)
: AtlasG4EventUserInfo(ctx)
  , m_rng_engine(rng_engine)
  , m_event_factory(std::move(event_factory))
  , m_sync_interface(std::make_shared<G4EventSynchronizationInterface>())
{
}

//---------------------------------------------------------------------------
// G4EventSynchronizationInterface
//---------------------------------------------------------------------------

auto G4EventSynchronizationInterface::Status() const -> EventStatus
{
  std::scoped_lock lk(m_mutex);
  return m_status;
}

auto G4EventSynchronizationInterface::Outcome() const -> EventOutcome
{
  std::scoped_lock lk(m_mutex);
  return m_outcome;
}

void G4EventSynchronizationInterface::Complete(EventOutcome outcome)
{
  {
    std::scoped_lock lk(m_mutex);
    m_outcome = outcome;
    m_status = EventStatus::Done;
  }
  m_cv.notify_all();
}

void G4EventSynchronizationInterface::WaitStatus(const EventStatus& status) 
{
  std::unique_lock lk(m_mutex);
  m_cv.wait(lk, [this,status]{ return m_status==status; });
}
