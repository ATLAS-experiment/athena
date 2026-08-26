# G4RunManagement

Author julien.esseiva@cern.ch

## Introduction

`G4RunManagement` provides the infrastructure that allows the `G4RunAlg` Athena
algorithm to drive a Geant4 run in a separate thread pool.  The package
implements the queue shared between Athena and the Geant4 worker threads, the
user actions that synchronize the two frameworks, and the user information
objects that carry per-event state across the boundary.

```
Athena thread (G4RunAlg)  ⇄  G4RunTool ⇄  Geant4 main & worker threads
```

## Package Layout

- `src/G4RunTool.[h,cxx]`: public tool that owns the Geant4 run manager, spawns
  the Geant4 main thread, and exposes the event queue (`PushEvent`, `GetEvent`,
  `Size`, `NotifyBeginRun`, `WaitBeginRun`).
- `src/G4RunToolWorkerRunManager.[h,cxx]`: worker run manager and worker
  factory that notify Athena after Geant4 has terminated an event.
- `G4RunManagement/AtlasG4SyncEventUserInfo.h` and
  `src/AtlasG4SyncEventUserInfo.cxx`: event user information that holds the
  `EventContext`, RNG engine, event factory functor, and a `G4EventSynchronizationInterface`.
- `src/Sync*.[h,cxx]` and corresponding `*Tool` classes: run, event and primary
  generator actions that interact with `IG4RunTool` to coordinate with Athena.

## Thread & Synchronization Model

1. During `G4RunAlg::initialize` the public `G4RunTool` is retrieved.
   `G4RunTool::initialize` already spawned the Geant4 main thread, so `G4RunAlg` calls
   `WaitBeginRun()` to block until the Geant4 run manager reports that it has
   reached `BeginOfRun` through `G4UA::SyncRunAction`. If the Geant4 main thread
   fails or exits before that point, the wait returns failure instead of
   blocking indefinitely.
2. `G4RunTool` keeps a mutex-protected lifecycle state with condition
   variables. The Geant4 thread calls `NotifyBeginRun()` once the master run
   action executes, unblocking the Athena side. Explicit finalize, failure and
   stopped states provide terminal conditions for all waits and run-loop
   checks.
3. Per event, `G4RunAlg` prepares a `std::unique_ptr<AtlasG4SyncEventUserInfo>`
   that owns the RNG engine seeded by Athena and a factory functor that will
   populate a `G4Event`.  The user info also stores a shared
   `G4EventSynchronizationInterface` so that the Athena thread can wait for the
   event-completion notification even after Geant4 has destroyed its `G4Event`.
4. `G4RunAlg` hands the prepared user info to `G4RunTool::PushEvent()`.  The
   Geant4 primary generator action pulls entries via `GetEvent()`, attaches the
   user info to the `G4Event`, and invokes the stored factory to convert the
   HepMC event to G4 primaries. Attaching the user info first ensures Athena is
   notified even if event preparation fails. Exceptions from the factory are
   caught and handled as preparation failures.
5. `G4UA::SyncEventAction` checks that each `G4Event` really carried the user
   info (otherwise it aborts the run). If preparation failed, it aborts only
   that event. After all event actions, analysis, scoring and event cleanup
   have finished, `G4RunToolWorkerRunManager` updates the synchronization
   interface and wakes the Athena thread, which performs its own cleanup before
   propagating the preparation failure.
6. The Athena thread waits on `syncInterface->WaitStatusDone()` before running
   the end-of-event hooks (`Begin/EndOfAthenaEvent` for the sensitive detector,
   fast simulation and user action services).
7. When `G4RunTool::finalize()` is called, it sets the state to
   `AthenaFinalize` and pushes one `nullptr` per worker thread so that workers
   leave the queue wait, abort the Geant4 run and allow the thread to join.

## Event Lifecycle with `G4RunAlg`

1. Read the event context, truth collections and seed the RNG (`G4RunAlg::execute`).
2. Build `AtlasG4SyncEventUserInfo` with:
   - The seeded `CLHEP::HepRandomEngine`.
   - A lambda that calls `ISF::ITruthSvc::initializeTruthCollection`,
     then invokes `ISF::IInputConverter::convertHepMCToG4Event`.
   - The `EventContext` so that `AtlasG4EventUserInfo` can still expose it to
     user actions after transport.
3. Call `BeginOfAthenaEvent` on the user action, sensitive detector and fast
   simulation services, then `PushEvent` into the run tool.
4. Wait until the synchronization interface reports that the Geant4 event has
   been completed. `G4RunToolWorkerRunManager::TerminateOneEvent` captures the
   event outcome, invokes Geant4's event termination, then sets the completion
   status.
5. Handle aborted events according to `FlagAbortedEvents` and
   `KillAbortedEvents`.  Close out the Athena-side services with
   `EndOfAthenaEvent` and ask the truth service to release the event. Event
   preparation failures are propagated after this cleanup.
6. (Optional) undo any quasi-stable particle workarounds before leaving execute.

## Key Components

### G4RunTool

- Retrieves the detector construction, physics list, user limits and user action
  services and initializes them in the Geant4 main thread.
- Converts initialization failures and exceptions in the Geant4 main thread
  into a lifecycle failure that is returned by `WaitBeginRun()`.
- Accepts a list of additional action tools (`UserActionTools` property) that
  will be registered with the `G4UA::UserActionSvc`.
- Sends all configured Geant4 UI commands (`G4Commands`, `Dll`, `Physics`,
  `FieldMap`) before calling `runManager->Initialize()`.
- Applies additional physics initialization via
  `IPhysicsInitializationTool::initializePhysics()` after the run manager has
  been created.

### AtlasG4SyncEventUserInfo

- Inherits from `AtlasG4EventUserInfo` so that existing code can still
  retrieve the event context, hit collection map and truth links.
- Owns a shared `G4EventSynchronizationInterface` object that exposes
  `WaitStatusDone()` / `Complete()`, the aborted-event flag and the event
  preparation status.
- Holds the RNG engine and the event factory functor used by
  `SyncPrimaryGeneratorAction` to materialize the `G4Event` inside the Geant4
  worker thread.

### Synchronization Actions

- `SyncRunAction` (master only): calls `IG4RunTool::NotifyBeginRun()` so that
  `G4RunAlg` knows when it is safe to continue initialization.
- `SyncPrimaryGeneratorAction`: owns a pointer to `IG4RunTool`, fetches
  `AtlasG4SyncEventUserInfo` objects from the queue, seeds Geant4’s RNG and
  invokes the stored factory to build primaries.
- `SyncEventAction`: verifies that every event carries user info, aborting the
  run when a shutdown sentinel is received or aborting only the current event
  when its preparation failed.
- `G4RunToolWorkerRunManager`: completes the synchronization interface only
  after Geant4 has finished all processing and termination for the event.

All three actions are instantiated via their `*Tool` companions that plug into
`G4AtlasTools::UserActionToolBase`.  `G4RunTool` injects its own pointer into
the tools by calling their `G4RunTool(IG4RunTool*)` override once the tool has
been constructed.

## Configuration

Useful properties on `G4RunTool`:

- `G4Commands`: arbitrary UI commands to run after Geant4 initialization.
- `NG4threads`: Geant4 worker configuration (defaults to the number of
  Athena slots).
- `Physics`, `Dll`, `FieldMap`: request alternate physics lists or load extra
  shared libraries/maps.
- `PhysicsInitializationTools`: tools that run after `runManager->Initialize`
  to configure fast calorimeter extensions, punch-through etc.
- `ActivateParallelWorlds`: toggles the Geant4 parallel world machinery.

When adding new synchronization logic, keep the following in mind:

- Anything that touches the Geant4 run manager must happen in the thread created
  by `G4RunTool::Geant4main()`.
- Event objects are owned by Geant4, so data that must survive after
  `G4Event` deletion has to live in `AtlasG4SyncEventUserInfo` or in structures
  referenced through shared pointers.
