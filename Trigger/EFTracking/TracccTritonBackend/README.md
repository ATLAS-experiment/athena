# TracccTritonBackend

A [Triton Inference Server](https://github.com/triton-inference-server/backend)
*custom backend* that serves the Traccc GPU track-reconstruction chain.

Instead of embedding a self-contained traccc pipeline, this backend **embeds a
Gaudi/Athena kernel** and drives the same device algorithms that the offline /
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

## Layout

| Path | Purpose |
| --- | --- |
| `src/TracccTritionBackend.cc` | The `TRITONBACKEND_*` C API entry points. Thin: gathers the input tensor, calls the runner, scatters the output tensor. |
| `src/TracccTritonInitializer.{hpp,cpp}` | Embeds a Python interpreter, boots the Gaudi kernel through it (process-wide singleton) and caches the device-description service + algorithm handles. |
| `src/TracccTritonRunner.{hpp,cpp}` | Per-request: deserialize cells, record to StoreGate, `sysExecute()` the five device algorithms, copy the fitted tracks and their measurements back. |
| `python/TracccTritonBackendConfig.py` | `TracccTritonDeviceRecoCfg(flags)` — the ComponentAccumulator for the device chain. |
| `python/TracccTritonBootstrap.py` | `bootstrap()` — called by the embedded interpreter; builds the ComponentAccumulator and drives it up through `ApplicationMgr.initialize()`. |

## How the embedding works

The device algorithms are ordinary `AthReentrantAlgorithm` components: they read
and write StoreGate and depend on provider *tools* (`CUDAClusterizationAlgProviderTool`,
…) and DetectorStore *services* (`JSONDeviceDetectorDescriptionProviderSvc`,
`JSONDeviceMagFieldProviderSvc`).
They therefore cannot be constructed as plain C++ objects — a Gaudi kernel has to
be running.

`TracccTritonInitializer::initialize()` does, in order:

1. embed a Python interpreter (`Py_Initialize()`, once per process) and call
   `TracccTritonBootstrap.bootstrap()`, which builds the same
   `ComponentAccumulator` a normal athena job would get from
   `TracccTritonDeviceRecoCfg`, then drives it through
   `ApplicationMgr.configure()` + `initialize()` — this resolves the full
   provider-tool / memory-resource / copy / stream tool tree and instantiates
   the device algorithms. (An earlier version of this class tried to reach the
   same state by calling `Gaudi::createApplicationMgr()` directly in C++ and
   pointing `JobOptionsPath` at a jobOptions file — that doesn't work for a
   `ComponentAccumulator`-based configuration: `JobOptionsType=FILE` invokes
   Gaudi's classic declarative job-options parser, which cannot execute a
   `ComponentAccumulator` script, and there was no Python interpreter in the
   process to run it anyway.)
2. fetch the resulting kernel from C++ via `Gaudi::createApplicationMgr()`
   again — it returns the same process-wide singleton Python just configured,
   rather than creating a second one;
3. resolve `IDeviceDetectorDescriptionProviderSvc` and the algorithms through
   `IAlgManager`.

Neither the Python nor the C++ side ever calls `ApplicationMgr.start()`: the
device algorithms have `sysExecute()` called on them directly, once per
Triton request, the same way `AtlasTest/TestTools/src/initGaudi.cxx`'s
minimal `configure()+initialize()` bootstrap lets unit tests call algorithms
directly without ever starting an event loop.

`TracccTritonRunner::run()` then, per request:

1. deserializes the raw `CELLS` bytes into a host
   `traccc::edm::silicon_cell_collection`;
2. copies it to the device and records it under `Config::cellsKey`;
3. calls `sysExecute(ctx)` on the five algorithms;
4. copies the track container (tracks + track states) and the measurement
   collection its states index into back to the host; the backend translation
   unit then flattens those into the four output tensors.

## Constraints

* **One instance per process.** Gaudi's `ToolSvc`, `DetectorStore` and the
  device-description provider services are process singletons, so a second
  kernel cannot be brought up in the same process. The model config must set
  `instance_group { count: 1 }`. For more than one GPU, run one `tritonserver`
  per GPU with `CUDA_VISIBLE_DEVICES` pinned.
* **The server must run inside an ATLAS runtime.** The bootstrap is athena
  python executed through an embedded interpreter, so `tritonserver` needs the
  athena setup sourced: `PYTHONPATH` must contain the generated
  `TracccTritonBackendConfig.py` and `TracccTritonBootstrap.py`, and `DATAPATH`
  must resolve the `dev/ACTS/detray-itk/*` geometry, digitization, conditions
  and `athenaIdentifierToDetrayMap.txt` files.

## Running with `tritonserver`

Triton needs a **backend directory** and a **model repository**, laid out
like this:

```
<backend-dir>/traccc/libtraccc.so       # the built artifact -- a real file,
                                         # NOT a symlink resolving outside
                                         # <backend-dir>/traccc/ (Triton's
                                         # path-containment check rejects that)
<model-repo>/<model-name>/config.pbtxt
<model-repo>/<model-name>/1/            # empty version directory; still required
```

`config.pbtxt` needs the input/output spec (`CELLS` in, the four track
tensors out — see the comment at the top of `config.pbtxt`),
`instance_group { count: 1 }` (see Constraints), and one non-obvious field:

```
runtime: "libtraccc.so"
```

Triton's *default* backend-file naming convention is
`<backend-dir>/<backend>/libtriton_<backend>.so` (see
`TRITONSERVER_ServerOptionsSetBackendDirectory`'s doc comment in
`triton/core/tritonserver.h`), not `lib<backend>.so` — without this explicit
`runtime` override, Triton fails to load with "unable to find backend
library for backend 'traccc'".

Environment `tritonserver` itself needs before starting:

```sh
source <build-dir>/<platform>/setup.sh   # puts TracccTritonBackendConfig.py /
                                          # TracccTritonBootstrap.py on PYTHONPATH
export CUDA_VISIBLE_DEVICES=<N>          # one GPU per tritonserver process
export DATAPATH=<dir containing dev/ACTS/detray-itk/*>:$DATAPATH
```

The `dev/ACTS/detray-itk/*` files PathResolver needs to (via `DATAPATH`)
don't match the real filenames on the EOS ITk-data area
(`/eos/project/a/atlas-eftracking/GPU/ITk_data/ATLAS-P2-RUN4-03-00-01/`) out
of the box:

* `detray_detector_geometry-for-fun.json` is expected; the real file is
  `detray_detector_geometry.json` — symlink/rename to match.
* `ITk_digitization_config.json`, `athenaIdentifierToDetrayMap.txt`,
  `detray_detector_surface_grids.json`, `detray_detector_material_maps.json`
  and `ITk_bfield.cvf` already match by name. The last three are only needed
  once track finding runs (detray navigation, material effects and the
  propagator's field), but the geometry service and the magnetic-field
  service load them at initialize time regardless.
* `ITk_conditions_config.json` doesn't currently exist on that EOS area at
  all. This is harmless: `PathResolver` logs an `ERROR ... Do not let this
  propagate to a release!` (that message is normal/expected for anything
  resolved via `DATAPATH`'s dev-area convention, not fatal on its own) and
  the service tolerates the missing conditions file and proceeds.

Then:

```sh
tritonserver --backend-directory=<backend-dir> --model-repository=<model-repo>
```

## StoreGate key contract

The keys below are shared between three places and must stay in sync:

| Key | Default | Written by | Read by |
| --- | --- | --- | --- |
| cells | `TracccTritonCells` | `TracccTritonRunner` | `DeviceClusterizationAlg` |
| measurements | `TracccTritonMeasurements` | `DeviceClusterizationAlg` | `DeviceSPFormationAlg` |
| spacepoints | `TracccTritonSpacepoints` | `DeviceSPFormationAlg` | `DeviceTripletSeedingAlg` |
| seeds | `TracccTritonSeeds` | `DeviceTripletSeedingAlg` | `DeviceTrkParamEstimationAlg` |
| track parameters | `TracccTritonTrackParameters` | `DeviceTrkParamEstimationAlg` | `DeviceTrackFindingAlg` |
| tracks | `TracccTritonTracks` | `DeviceTrackFindingAlg` | `TracccTritonRunner` |

The measurement collection is read back by the runner too: the track container
carries only a *device* view of the measurements its states index into.

Defined in `TracccTritonInitializer::Config`, passed to
`TracccTritonDeviceRecoCfg` in `python/TracccTritonBackendConfig.py`.

