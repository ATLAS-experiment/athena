/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "G4RunToolStateSynchronization.h"

#include <chrono>
#include <future>
#include <iostream>
#include <string>

namespace
{
  using namespace std::chrono_literals;

  struct WaitResult
  {
    bool success;
    std::string failureMessage;
  };

  std::future<WaitResult> waitForBeginRun(
    G4RunToolStateSynchronization& synchronization)
  {
    return std::async(std::launch::async, [&synchronization] {
      WaitResult result;
      result.success =
        synchronization.WaitBeginRun(result.failureMessage);
      return result;
    });
  }

  bool check(bool condition, const char* message)
  {
    if (!condition) {
      std::cerr << "FAIL: " << message << '\n';
    }
    return condition;
  }

  bool checkWaitIsPending(std::future<WaitResult>& result)
  {
    return check(result.wait_for(20ms) == std::future_status::timeout,
                 "WaitBeginRun returned while the lifecycle was Starting");
  }

  bool checkWaitIsReady(std::future<WaitResult>& result)
  {
    return check(result.wait_for(1s) == std::future_status::ready,
                 "WaitBeginRun was not notified of a lifecycle transition");
  }

  bool testBeginRun()
  {
    G4RunToolStateSynchronization synchronization;
    auto result = waitForBeginRun(synchronization);
    bool success = checkWaitIsPending(result);

    synchronization.NotifyBeginRun();
    success &= checkWaitIsReady(result);
    const WaitResult waitResult = result.get();
    success &= check(waitResult.success,
                     "BeginRun did not complete WaitBeginRun successfully");
    success &= check(waitResult.failureMessage.empty(),
                     "successful WaitBeginRun returned a failure message");
    success &= check(!synchronization.StopRequested(),
                     "BeginRun requested that the Geant4 loop stop");
    return success;
  }

  bool testStartupFailure()
  {
    G4RunToolStateSynchronization synchronization;
    auto result = waitForBeginRun(synchronization);
    bool success = checkWaitIsPending(result);

    synchronization.Fail("initialization failed");
    success &= checkWaitIsReady(result);
    const WaitResult waitResult = result.get();
    success &= check(!waitResult.success,
                     "failure completed WaitBeginRun successfully");
    success &= check(waitResult.failureMessage == "initialization failed",
                     "WaitBeginRun did not return the failure message");
    success &= check(synchronization.StopRequested(),
                     "failure did not request that the Geant4 loop stop");
    return success;
  }

  bool testFinalizeDuringStartup()
  {
    G4RunToolStateSynchronization synchronization;
    auto result = waitForBeginRun(synchronization);
    bool success = checkWaitIsPending(result);

    synchronization.RequestFinalize();
    success &= checkWaitIsReady(result);
    const WaitResult waitResult = result.get();
    success &= check(!waitResult.success,
                     "finalization completed WaitBeginRun successfully");
    success &= check(synchronization.StopRequested(),
                     "finalization did not request that the Geant4 loop stop");

    synchronization.NotifyBeginRun();
    std::string failureMessage;
    success &= check(!synchronization.WaitBeginRun(failureMessage),
                     "BeginRun overwrote an earlier finalization request");
    return success;
  }

  bool testUnexpectedThreadExit()
  {
    G4RunToolStateSynchronization synchronization;
    auto result = waitForBeginRun(synchronization);
    bool success = checkWaitIsPending(result);

    synchronization.NotifyThreadExit();
    success &= checkWaitIsReady(result);
    const WaitResult waitResult = result.get();
    success &= check(!waitResult.success,
                     "thread exit completed WaitBeginRun successfully");
    success &= check(!waitResult.failureMessage.empty(),
                     "thread exit did not provide a failure message");
    success &= check(synchronization.StopRequested(),
                     "thread exit did not request that the Geant4 loop stop");
    return success;
  }

  bool testRunningToStopped()
  {
    G4RunToolStateSynchronization synchronization;
    synchronization.NotifyBeginRun();
    synchronization.RequestFinalize();
    synchronization.NotifyThreadExit();
    return check(synchronization.StopRequested(),
                 "stopped lifecycle did not keep the Geant4 loop stopped");
  }
}

int main()
{
  const bool success = testBeginRun() &&
                       testStartupFailure() &&
                       testFinalizeDuringStartup() &&
                       testUnexpectedThreadExit() &&
                       testRunningToStopped();
  return success ? 0 : 1;
}
