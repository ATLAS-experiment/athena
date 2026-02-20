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
- `G4RunManagement/AtlasG4SyncEventUserInfo.h` and
  `src/AtlasG4SyncEventUserInfo.cxx`: event user information that holds the
  `EventContext`, RNG engine, event factory functor, and a `G4EventSynchronizationInterface`.
- `src/Sync*.[h,cxx]` and corresponding `*Tool` classes: run, event and primary
  generator actions that interact with `IG4RunTool` to coordinate with Athena.

## Thread & Synchronization Model

1. During `G4RunAlg::initialize` the public `G4RunTool` is retrieved.
   `G4RunTool::initialize` already spawned the Geant4 main thread, so `G4RunAlg` calls
   `WaitBeginRun()` to block until the Geant4 run manager reports that it has
   reached `BeginOfRun` through `G4UA::SyncRunAction`.
2. `G4RunTool` keeps a `StateSynchronization` structure with condition
   variables.  The Geant4 thread calls `NotifyBeginRun()` once the master run
   action executes, unblocking the Athena side.  A subsequent `AthenaFinalize`
   status is used to stop the Geant4 run loop.
3. Per event, `G4RunAlg` prepares a `std::unique_ptr<AtlasG4SyncEventUserInfo>`
   that owns the RNG engine seeded by Athena and a factory functor that will
   populate a `G4Event`.  The user info also stores a shared
   `G4EventSynchronizationInterface` so that the Athena thread can wait for the
   EndOfEvent notification even after Geant4 has destroyed its `G4Event`.
4. `G4RunAlg` hands the prepared user info to `G4RunTool::PushEvent()`.  The
   Geant4 primary generator action pulls entries via `GetEvent()` and invokes
   the stored factory to convert the HepMC event to G4 primaries.
5. `G4UA::SyncEventAction` checks that each `G4Event` really carried the user
   info (otherwise it aborts the run), and on `EndOfEvent` toggles the status in
   the synchronization interface (and flags aborted events).
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
     sets the `G4Event` ID and invokes `ISF::IInputConverter::convertHepMCToG4Event`.
   - The `EventContext` so that `AtlasG4EventUserInfo` can still expose it to
     user actions after transport.
3. Call `BeginOfAthenaEvent` on the user action, sensitive detector and fast
   simulation services, then `PushEvent` into the run tool.
4. Wait until the synchronization interface reports that the Geant4 event has
   been completed.  `SyncEventAction::EndOfEventAction` sets the status and
   forwards the `event->IsAborted()` flag.
5. Handle aborted events according to `FlagAbortedEvents` and
   `KillAbortedEvents`.  Close out the Athena-side services with
   `EndOfAthenaEvent` and ask the truth service to release the event.
6. (Optional) undo any quasi-stable particle workarounds before leaving execute.

## Key Components

### G4RunTool

- Retrieves the detector construction, physics list, user limits and user action
  services and initializes them in the Geant4 main thread.
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
  `WaitStatusDone()` / `SetStatusDone()` and the `EventAborted` flag.
- Holds the RNG engine and the event factory functor used by
  `SyncPrimaryGeneratorAction` to materialize the `G4Event` inside the Geant4
  worker thread.

### Synchronization Actions

- `SyncRunAction` (master only): calls `IG4RunTool::NotifyBeginRun()` so that
  `G4RunAlg` knows when it is safe to continue initialization.
- `SyncPrimaryGeneratorAction`: owns a pointer to `IG4RunTool`, fetches
  `AtlasG4SyncEventUserInfo` objects from the queue, seeds Geant4’s RNG and
  invokes the stored factory to build primaries.
- `SyncEventAction`: verifies that every event carries user info and signals the
  synchronization interface when Geant4 is done transporting the event.

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
