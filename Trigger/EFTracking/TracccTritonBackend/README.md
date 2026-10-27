# TracccTritonBackend

A [Triton Inference Server](https://docs.nvidia.com/deeplearning/triton-inference-server/user-guide/docs/index.html)
custom backend that serves the Traccc GPU track-reconstruction chain.

Instead of embedding a self-contained traccc pipeline, this backend embeds a
full Athena application and drives the same device algorithms that the offline /
HLT-athena GPU chain uses:

```
CELLS (uint8 tensor) + EVENT_IOV (int64 tensor)
  -> conditions (AthCondSeq: beamspot, field) for the event's IOV
  -> ActsTrk::DeviceClusterizationAlg
  -> ActsTrk::DeviceSPFormationAlg
  -> ActsTrk::DeviceTripletSeedingAlg (or ActsTrk::DeviceGBTSSeedingAlg)
  -> ActsTrk::DeviceTrkParamEstimationAlg
  -> ActsTrk::DeviceTrackFindingAlg
MEASUREMENTS / CLUSTERS / TRACKS / TRACK_STATES (uint8 tensors)
```

## Structure of the server

Two Tools are created to first initialize the server, and then second to run the reco algs once per event. 
The `traccc` GPU algorithms are taken from existing work in the Acts repo and are ordinary 
`AthReentrantAlgorithm` components: they read from
and write to StoreGate and depend on provider tools and DetectorStore services.

The initializer `TracccTritonInitializer::initialize()` at server intialization:

1. embeds a Python interpreter (`Py_Initialize()`, once per process) and calls
   `TracccTritonBootstrap.bootstrap()`, which schedules the device chain of the
   offline primary pass, `InDetConfig.ITkActsDeviceTrackRecoConfig.ITkActsDeviceTrackRecoCfg`,
   with `flags.Acts.Device.tritonMode = DeviceTritonMode.Server` (no RDO conversion, no
   conversion back to the host) inside the AthSequencer the runner executes, then
   drives it through `ApplicationMgr.configure()` + `initialize()` + `start()`;
2. fetches the resulting application manager from C++ via
   `Gaudi::createApplicationMgr()`, again returning the same process-wide
   singleton Python just configured,
   rather than creating a second one;
3. resolves `IDeviceDetectorDescriptionProviderSvc` and the algorithms through
   `IAlgManager`.

The runner `TracccTritonRunner::run()` then, per request:

1. deserializes the `CELLS` bytes into a host
   `traccc::edm::silicon_cell_collection`;
2. makes its own event slot current (`IHiveWhiteBoard::selectStore`), then
   copies the cells to the device and records them under `Config::cellsKey`;
3. sets up the event context from `EVENT_IOV` as the event loop manager
   would (event id and conditions run), fires `BeginRun` whenever the run
   changes, and runs `AthCondSeq`, so the conditions the device algorithms
   read are those of the client's event;
4. calls `sysExecute(ctx)` on the five algorithms, with `ctx.slot()` set to
   that slot;
5. copies the track container (tracks + track states), the measurement
   collection its states index into, and the clusters (cell indices) the
   measurements were built from back to the host; the backend translation
   unit then serializes each collection into its own output tensor.

All UINT8 tensors, the `CELLS` input included, are traccc EDM collections
serialized with `TracccTritonClient/TracccEdmSerialization.h`. `EVENT_IOV`
holds the five numbers the conditions are looked up with: run number,
lumiblock, time stamp [s], time stamp ns offset and conditions run.

## Running with `tritonserver`

To run the server, all that is needed is:

```sh
tritonserver --model-repository=${WorkDir_DIR:-$Athena_DIR}/data/TracccTritonBackend/models
```

Flags of the embedded job can be overridden from the server environment, `;`-separated, e.g.

```sh
export TRACCC_TRITON_FLAGS="Acts.Device.seedingStrategy=SeedingStrategy.Gbts"
```

A successful start ends with

```
+------------+---------+--------+
| Model      | Version | Status |
+------------+---------+--------+
| traccc-gpu | 1       | READY  |
+------------+---------+--------+
```

### TracccTritonClient

The [TracccTritonClient](https://gitlab.cern.ch/atlas/athena/-/tree/main/Trigger/EFTracking/TracccTritonClient?ref_type=heads) 
can then be run as before, using the setup server (instructions in [client README](https://gitlab.cern.ch/atlas/athena/-/blob/main/Trigger/EFTracking/TracccTritonClient/README.md?ref_type=heads)).

