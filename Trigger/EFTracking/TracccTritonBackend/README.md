# TracccTritonBackend

A [Triton Inference Server](https://docs.nvidia.com/deeplearning/triton-inference-server/user-guide/docs/index.html)
custom backend that serves the Traccc GPU track-reconstruction chain.

Instead of embedding a self-contained traccc pipeline, this backend embeds a
Gaudi/Athena kernel and drives the same device algorithms that the offline /
HLT-athena GPU chain uses:

```
CELLS (uint8 tensor)
  -> ActsTrk::DeviceClusterizationAlg
  -> ActsTrk::DeviceSPFormationAlg
  -> ActsTrk::DeviceTripletSeedingAlg
  -> ActsTrk::DeviceTrkParamEstimationAlg
  -> ActsTrk::DeviceTrackFindingAlg
TRK_PARAMS / MEASUREMENTS / COVARIANCES / GEOMETRY_IDS
```

## Structure of the server

Two Tools are created to first initialize the server, and then second to run the reco algs once per event. 
The `traccc` GPU algorithms are taken from existing work in the Acts repo and are ordinary 
`AthReentrantAlgorithm` components: they read from
and write to StoreGate and depend on provider tools and DetectorStore services.

The initializer `TracccTritonInitializer::initialize()` at server intialization:

1. embeds a Python interpreter (`Py_Initialize()`, once per process) and calls
   `TracccTritonBootstrap.bootstrap()`, which builds the same
   `ComponentAccumulator` a normal athena job would get from
   `TracccTritonDeviceRecoCfg`, then drives it through
   `ApplicationMgr.configure()` + `initialize()`;
2. fetches the resulting kernel from C++ via `Gaudi::createApplicationMgr()`
   again returning the same process-wide singleton Python just configured,
   rather than creating a second one;
3. resolves `IDeviceDetectorDescriptionProviderSvc` and the algorithms through
   `IAlgManager`.

The runner `TracccTritonRunner::run()` then, per request:

1. deserializes the raw `CELLS` bytes into a host
   `traccc::edm::silicon_cell_collection`;
2. makes its own event slot current (`IHiveWhiteBoard::selectStore`), then
   copies the cells to the device and records them under `Config::cellsKey`;
3. calls `sysExecute(ctx)` on the five algorithms, with `ctx.slot()` set to
   that slot;
4. copies the track container (tracks + track states) and the measurement
   collection its states index into back to the host; the backend translation
   unit then flattens those into the four output tensors.

## Running with `tritonserver`

To run the server, all that is needed is:

```sh
tritonserver --model-repository=${WorkDir_DIR:-$Athena_DIR}/data/TracccTritonBackend/models
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

