/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4RUNMANAGEMENT_G4RUNTOOLSTATESYNCHRONIZATION_H
#define G4RUNMANAGEMENT_G4RUNTOOLSTATESYNCHRONIZATION_H

#include <condition_variable>
#include <mutex>
#include <string>

/// Thread-safe lifecycle state shared by the Athena and Geant4 main threads.
///
/// The normal lifecycle is:
///
///   Starting --NotifyBeginRun()--> Running
///   Starting/Running --RequestFinalize()--> FinalizeRequested
///   FinalizeRequested --NotifyThreadExit()--> Stopped
///
/// A failure before normal thread exit instead transitions Starting, Running or
/// FinalizeRequested to Failed. An unexpected thread exit while Starting or
/// Running is also treated as a failure. Failed and Stopped are terminal states;
/// late BeginOfRun, finalize and failure notifications cannot leave them.
class G4RunToolStateSynchronization
{
 public:
  enum class Status {
    Starting,          ///< Waiting for the first Geant4 BeginOfRun notification.
    Running,           ///< BeginOfRun was observed; Athena may submit events.
    FinalizeRequested, ///< Athena requested termination of the Geant4 run loop.
    Failed,            ///< The Geant4 main thread failed; terminal state.
    Stopped            ///< The Geant4 main thread exited normally; terminal state.
  };

  /// Transition Starting to Running. Ignored in every other state.
  void NotifyBeginRun();
  /// Transition Starting or Running to FinalizeRequested.
  void RequestFinalize();
  /// Transition any non-terminal state to Failed and wake startup waiters.
  void Fail(std::string message);
  /// Record normal exit after finalization, or failure after an unexpected exit.
  void NotifyThreadExit();
  /// Wait until startup succeeds or the lifecycle reaches a terminal condition.
  bool WaitBeginRun(std::string& failureMessage);
  /// Return whether the Geant4 run loop must stop.
  bool StopRequested() const;

 private:
  Status m_status{Status::Starting};
  std::string m_failureMessage;
  mutable std::mutex m_mutex;
  std::condition_variable m_cv;
};

#endif
