/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "G4RunToolEventQueue.h"

#include "AthenaKernel/ExtendedEventContext.h"

#include <chrono>
#include <future>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

namespace
{
  using namespace std::chrono_literals;
  using EventOutcome = G4EventSynchronizationInterface::EventOutcome;
  using SyncInterface = std::shared_ptr<G4EventSynchronizationInterface>;

  bool check(bool condition, const std::string& message)
  {
    if (!condition) {
      std::cerr << "FAIL: " << message << '\n';
    }
    return condition;
  }

  auto makeEvent(EventContext& context)
    -> G4RunToolEventQueue::UPEvent
  {
    Atlas::setExtendedEventContext(
      context, Atlas::ExtendedEventContext{});
    AtlasG4SyncEventUserInfo::EventFactoryFunction factory =
      [](G4Event&) { return StatusCode::SUCCESS; };
    return std::make_unique<AtlasG4SyncEventUserInfo>(
      nullptr, std::move(factory), context);
  }

  std::future<EventOutcome> waitForCompletion(SyncInterface interface)
  {
    return std::async(std::launch::async, [interface = std::move(interface)] {
      interface->WaitStatusDone();
      return interface->Outcome();
    });
  }

  bool checkWaitIsPending(std::future<EventOutcome>& result)
  {
    return check(result.wait_for(20ms) == std::future_status::timeout,
                 "event completion wait returned too early");
  }

  bool checkWaitIsReady(std::future<EventOutcome>& result)
  {
    return check(result.wait_for(1s) == std::future_status::ready,
                 "event completion wait was not notified");
  }

  bool testNormalPushAndPop()
  {
    G4RunToolEventQueue queue;
    EventContext context;
    auto event = makeEvent(context);
    auto interface = event->SyncInterface();

    queue.PushEvent(std::move(event));
    bool success = check(queue.Size() == 1, "pushed event was not queued");
    auto poppedEvent = queue.GetEvent();
    success &= check(poppedEvent != nullptr, "open queue returned no event");
    success &= check(queue.Size() == 0, "popped event remained queued");

    interface->Complete(EventOutcome::Success);
    interface->Complete(EventOutcome::RunTerminated);
    success &= check(interface->Outcome() == EventOutcome::Success,
                     "later completion overwrote the first event outcome");

    queue.Close();
    poppedEvent.reset();
    queue.CompleteOutstandingEvents();
    return success;
  }

  bool testCloseWakesAllConsumers()
  {
    G4RunToolEventQueue queue;
    auto first = std::async(std::launch::async,
                            [&queue] { return queue.GetEvent() == nullptr; });
    auto second = std::async(std::launch::async,
                             [&queue] { return queue.GetEvent() == nullptr; });

    bool success = check(first.wait_for(20ms) == std::future_status::timeout,
                         "first consumer did not block on an open queue");
    success &= check(second.wait_for(20ms) == std::future_status::timeout,
                     "second consumer did not block on an open queue");

    queue.Close();
    queue.Close();
    success &= check(first.wait_for(1s) == std::future_status::ready,
                     "queue close did not wake the first consumer");
    success &= check(second.wait_for(1s) == std::future_status::ready,
                     "queue close did not wake the second consumer");
    success &= first.get();
    success &= second.get();
    return success;
  }

  bool testOutstandingEventsCompleteAfterWorkerStop()
  {
    G4RunToolEventQueue queue;
    EventContext firstContext;
    EventContext secondContext;
    auto firstEvent = makeEvent(firstContext);
    auto secondEvent = makeEvent(secondContext);
    auto firstInterface = firstEvent->SyncInterface();
    auto secondInterface = secondEvent->SyncInterface();
    auto firstResult = waitForCompletion(firstInterface);
    auto secondResult = waitForCompletion(secondInterface);

    queue.PushEvent(std::move(firstEvent));
    queue.PushEvent(std::move(secondEvent));
    auto inFlightEvent = queue.GetEvent();
    queue.Close();

    bool success = checkWaitIsPending(firstResult);
    success &= checkWaitIsPending(secondResult);

    // Model run-manager destruction: the worker releases its current event
    // before the queue reports termination to Athena.
    inFlightEvent.reset();
    queue.CompleteOutstandingEvents();
    queue.CompleteOutstandingEvents();

    success &= checkWaitIsReady(firstResult);
    success &= checkWaitIsReady(secondResult);
    success &= check(firstResult.get() == EventOutcome::RunTerminated,
                     "in-flight event did not report run termination");
    success &= check(secondResult.get() == EventOutcome::RunTerminated,
                     "queued event did not report run termination");
    return success;
  }

  bool testPushAfterClose()
  {
    G4RunToolEventQueue queue;
    EventContext context;
    auto event = makeEvent(context);
    auto interface = event->SyncInterface();
    auto result = waitForCompletion(interface);

    queue.Close();
    queue.PushEvent(std::move(event));

    bool success = checkWaitIsReady(result);
    success &= check(result.get() == EventOutcome::RunTerminated,
                     "event pushed after close did not report run termination");
    success &= check(queue.Size() == 0,
                     "event pushed after close was added to the queue");
    return success;
  }

  bool testNullEventIsRejected()
  {
    G4RunToolEventQueue queue;
    bool rejected = false;
    try {
      queue.PushEvent(nullptr);
    }
    catch (const std::invalid_argument&) {
      rejected = true;
    }

    bool success = check(rejected, "null event was not rejected");
    success &= check(queue.Size() == 0,
                     "null event changed the queue contents");

    // Rejection is not a shutdown request: the queue must remain usable.
    EventContext context;
    auto event = makeEvent(context);
    queue.PushEvent(std::move(event));
    auto poppedEvent = queue.GetEvent();
    success &= check(poppedEvent != nullptr,
                     "null rejection closed the event queue");

    queue.Close();
    poppedEvent.reset();
    queue.CompleteOutstandingEvents();
    return success;
  }
}

int main()
{
  const bool success = testNormalPushAndPop() &&
                       testCloseWakesAllConsumers() &&
                       testOutstandingEventsCompleteAfterWorkerStop() &&
                       testPushAfterClose() &&
                       testNullEventIsRejected();
  return success ? 0 : 1;
}
