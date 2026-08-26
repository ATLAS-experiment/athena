/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "G4RunToolStateSynchronization.h"

#include <utility>

void G4RunToolStateSynchronization::NotifyBeginRun()
{
  {
    std::scoped_lock lk(m_mutex);
    if (m_status == Status::Starting) {
      m_status = Status::Running;
    }
  }
  m_cv.notify_all();
}

void G4RunToolStateSynchronization::RequestFinalize()
{
  {
    std::scoped_lock lk(m_mutex);
    if (m_status == Status::Starting || m_status == Status::Running) {
      m_status = Status::FinalizeRequested;
    }
  }
  m_cv.notify_all();
}

void G4RunToolStateSynchronization::Fail(std::string message)
{
  {
    std::scoped_lock lk(m_mutex);
    // Preserve the first terminal outcome and its diagnostic message.
    if (m_status != Status::Failed && m_status != Status::Stopped) {
      m_failureMessage = std::move(message);
      m_status = Status::Failed;
    }
  }
  m_cv.notify_all();
}

void G4RunToolStateSynchronization::NotifyThreadExit()
{
  {
    std::scoped_lock lk(m_mutex);
    if (m_status == Status::Starting || m_status == Status::Running) {
      if (m_failureMessage.empty()) {
        m_failureMessage =
          "Geant4 main thread exited without completing its lifecycle";
      }
      m_status = Status::Failed;
    }
    else if (m_status == Status::FinalizeRequested) {
      m_status = Status::Stopped;
    }
  }
  m_cv.notify_all();
}

bool G4RunToolStateSynchronization::WaitBeginRun(
  std::string& failureMessage)
{
  std::unique_lock lk(m_mutex);
  m_cv.wait(lk, [this]{ return m_status != Status::Starting; });
  if (m_status == Status::Running) {
    return true;
  }

  failureMessage = m_failureMessage;
  if (failureMessage.empty()) {
    failureMessage = "Geant4 stopped before beginning a run";
  }
  return false;
}

bool G4RunToolStateSynchronization::StopRequested() const
{
  std::scoped_lock lk(m_mutex);
  return m_status == Status::FinalizeRequested ||
         m_status == Status::Failed ||
         m_status == Status::Stopped;
}
