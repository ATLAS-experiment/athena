# Pepper_i

Athena interface to the [Pepper](https://gitlab.com/spice-mc/pepper)
parton-level event generator from the Sherpa team.

## What this package does

Provides a thin Python wrapper that invokes the LCG-installed
`pepper_kokkos` binary from inside an AthGeneration `Gen_tf.py`
job. The architecture is:

```
Gen_tf.py
  └─ DSID JO file
       ├─ Pepper_Common.py
       │    ├─ run_pepper()      →  pepper (LCG)  →  pepper.lhef
       │    └─ arrange_output()  →  events.lhe  (+ TXT tarball)
       └─ Pythia8_i/Pythia8_LHEF (user-included)
            └─ Pythia8 + EvtGen reads events.lhe  →  EVNT.root
```

There is no C++ in this package. The `pepper` binary is consumed as
an opaque external tool, which keeps the interface stable across
Pepper releases and isolates Pepper's threading/MPI/Kokkos model
from AthenaMT.

`Pepper_Common.py` is **only** responsible for running Pepper and
handing the parton-level events to the transform. Composing it with
Pythia8, Herwig, or any other shower happens at the DSID JO level,
using the existing standard fragments. This keeps the Pepper
interface single-purpose and composable.

The same fragment serves LHE-only production: without shower
fragments in the JO, and with `--outputTXTFile` on the command line,
the events are written out as the transform's TXT output (see
"LHE-only jobs" below).

## Runtime requirements

### GPU node

`pepper_kokkos` is built against Kokkos with CUDA enabled. On a
CPU-only worker the binary deadlocks during Kokkos initialisation
waiting for a CUDA device.

**All Pepper jobs must run on a GPU-equipped node.** On lxplus, that
means `lxplus-gpu` rather than the default `lxplus`. On the grid,
the corresponding queue/site selection applies.

### LCG dependency

This package depends on `pepper_kokkos` from the LCG release. If it
isn't present in the `AtlasGenerationExternals` view (e.g. on
aarch64), the package is skipped at configure time with a CMake
warning.

Versions confirmed working:

| LCG release | Platform                  | pepper_kokkos |
|-------------|---------------------------|---------------|
| LCG_109a    | x86_64-el9-gcc13-opt      | 1.8.0-156a3   |
| LCG_109     | x86_64-el9-gcc13-opt      | 1.8.0-0f9ed   |
| LCG_109_cuda| x86_64-el9-gcc13-opt      | 1.8.0-89813   |

Older 1.1.1 builds (in plain LCG_107) have a hardcoded build path
bug that 1.8.x fixes; do not use them.

### PEPPER_DATA_PATH

The `pepper` binary has its data directory baked in at build time
(`${CMAKE_BINARY_DIR}` rather than `${CMAKE_INSTALL_PREFIX}`), so it
cannot find its process CSVs without an env-var override. The
wrapper sets `PEPPER_DATA_PATH` automatically; users do not need to.
This is undocumented in `pepper --help`; verified via `strings` on
the binary.

## Writing a JO file

Minimal DSID JO using Pepper + Pythia8 + EvtGen:

```python
# mc.PepperPy8EG_A14NNPDF23LO_dijet.py
evgenConfig.description    = "Pepper dijet + Pythia8 A14 NNPDF23LO + EvtGen"
evgenConfig.keywords       = ["SM", "QCD", "jets"]
evgenConfig.contact        = ["you@cern.ch"]
evgenConfig.generators     = ["Pepper", "Pythia8", "EvtGen"]
evgenConfig.nEventsPerJob  = 100

PEPPER_PROCESS = "ppjj"

include("Pepper_i/Pepper_Common.py")
include("Pythia8_i/Pythia8_A14_NNPDF23LO_EvtGen_Common.py")
include("Pythia8_i/Pythia8_LHEF.py")
```

The user JO composes Pepper with whatever shower / decay chain they
want. `Pepper_Common.py` makes the events available as `events.lhe`,
the name `Pythia8_LHEF.py` reads, exactly as for an LHE input given
through `--inputGeneratorFile`. The path of the file Pepper wrote is
also exported as the global `PEPPER_OUTPUT`.

### Filename and generators-list convention

- DSID filenames start with `mc.PepperPy8EG_<tune>_<process>.py`
  (or `mc.PepperHerwig_...`, etc. depending on shower).
- `evgenConfig.generators` includes `"Pepper"` as the first entry.

`"Pepper"` is a known generator in `GeneratorConfig` and in
`common/GeneratorList.txt` of the mcjoboptions repository.

### Number of events

Pepper's `--batch-size` and `--n-batches` options count *trial*
phase-space points. Only the points that survive unweighting are
written out (about 15% for `ppjj` at 13.6 TeV), so they do not say
how many events end up in the file. The wrapper therefore steers
Pepper with `--n-events`, the minimum number of written events, and
fails the job if the file holds fewer.

| JO variable            | Meaning                                         | Default |
|------------------------|-------------------------------------------------|---------|
| `PEPPER_NEVENTS`       | Minimum number of events written                | `PEPPER_SAFETY_FACTOR` x events the job delivers (`--maxEvents` if given, else `evgenConfig.nEventsPerJob`) |
| `PEPPER_SAFETY_FACTOR` | Head-room so the shower does not run out of events | 1.1 (1.0 for LHE-only) |
| `PEPPER_BATCH_SIZE`    | Trial points per batch; throughput tuning only  | 1000    |

Pepper stops at the first batch boundary after reaching the request,
so the file usually holds a little more than `PEPPER_NEVENTS` (up to
one batch's worth of accepted events).

`PEPPER_N_BATCHES` is no longer supported.

### LHE-only jobs

To produce parton-level events only, leave the shower out of the JO:

```python
# mc.Pepper_dijet_LHE.py
evgenConfig.description    = "Pepper dijet, LHE only"
evgenConfig.keywords       = ["SM", "QCD", "jets"]
evgenConfig.contact        = ["you@cern.ch"]
evgenConfig.generators     = ["Pepper"]
evgenConfig.nEventsPerJob  = 10000

PEPPER_PROCESS = "ppjj"

include("Pepper_i/Pepper_Common.py")
```

and ask the transform for a TXT output instead of an EVNT file:

```bash
Gen_tf.py \
    --ecmEnergy=13600. \
    --randomSeed=1234 \
    --jobConfig=/path/to/your/DSID/dir \
    --outputTXTFile=pepper_dijet.TXT.tar.gz
```

The tarball holds a single uncompressed LHE file,
`pepper_dijet.TXT.events`, following the MadGraphControl convention.
A later showering job reads it through `--inputGeneratorFile`. This
splits the chain into a GPU step (matrix elements) and a CPU step
(shower), which can then run on different resources.

For TXT-only jobs `Gen_tf.py` uses the `skel.GENtoTXT.py` skeleton,
which reports the number of LHE events in the job metadata and then
runs a single dummy Pythia8 event to complete the Athena job; that
part of the log can be ignored.

### Built-in Pepper processes

`PEPPER_PROCESS` accepts any of Pepper's built-in shortcuts:

| Process  | Description                                |
|----------|--------------------------------------------|
| `ppjj`   | Dijet (gg/qq/qg → 2 partons)               |
| `pp3j`   | Trijet                                     |
| `pp4j`   | 4-jet                                      |
| `ppz0j`  | Drell-Yan, Z + 0 jets                      |
| `ppz1j`  | Z + 1 jet                                  |
| `ppz2j`  | Z + 2 jets                                 |
| `pptt0j` | ttbar + 0 jets                             |
| `pptt1j` | ttbar + 1 jet                              |

Run `pepper --process X --help` (or look in
`<install>/share/pepper/data/`) for the full current list.

## Running locally

End-to-end test on `lxplus-gpu`:

```bash
ssh lxplus-gpu.cern.ch

# Standard ATLAS setup
setupATLAS
asetup AthGeneration,main,latest,here

# Place the JO directory on JOBOPTSEARCHPATH and run
mkdir -p run && cd run
Gen_tf.py \
    --ecmEnergy=13600. \
    --maxEvents=10 \
    --randomSeed=1234 \
    --jobConfig=/path/to/your/DSID/dir \
    --outputEVNTFile=test.EVNT.root
```

`run_pepper()` takes about 5 minutes on a Tesla T4 to optimise a fresh grid for
`ppjj` at 13.6 TeV. Subsequent runs reuse the `pepper_cache/`
directory in the run area and start producing events immediately.

Pepper runs once, as a single external process on the GPU, before
the Athena event loop starts. It is not steered by `--multiprocess`
/ `ATHENA_CORE_NUMBER`.

## Implementation notes

### Why an external-process wrapper rather than a `GenModule`?

A `GenModule`-derived C++ algorithm running inside the AthenaMT event
loop would be the conventional choice (see `Pythia8_i`,
`Sherpa_i`). For Pepper it's the wrong fit:

- Pepper parallelises events with Kokkos (GPU/OpenMP) and MPI; AthenaMT
  parallelises algorithms with TBB across event slots. The two
  threading layers don't compose without significant integration work.
- Pepper's standalone CLI is feature-complete and stable. Keeping
  Pepper out-of-process means the interface here doesn't need to
  track Pepper's C++ API across releases.
- Pepper is parton-level only — its output naturally flows into
  Pythia8 via LHE, which the existing `Pythia8_i` code already
  consumes well. There's no need for Pepper's events to live in
  StoreGate as a HepMC `GenEvent` until after the shower has touched
  them.

### How the events reach the transform

`"Pepper"` is registered as an LHE generator
(`GeneratorConfig.GenConfigHelpers.lhefGenerators`). For those the
`Gen_tf.py` skeletons require `runArgs.inputGeneratorFile`, locate
the matching uncompressed `<name>.events` file, link it to
`events.lhe`, and count its events for the job metadata.

`arrange_output()` in `PepperConfig.py` does what
`MadGraphControl.arrange_output()` does for MadGraph: it provides
`<name>.events` (a link to `pepper.lhef`), packs it into the tarball
given by `--outputTXTFile` if there is one, and sets
`runArgs.inputGeneratorFile` accordingly. Without a TXT output the
name is `pepper_LHE_events`.
