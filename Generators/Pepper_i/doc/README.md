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
       │    └─ run_pepper()  →  pepper (LCG)  →  pepper.events.gz
       └─ Pythia8_i/Pythia8_LHEF (user-included)
            └─ Pythia8 + EvtGen reads pepper.events.gz  →  EVNT.root
```

There is no C++ in this package. The `pepper` binary is consumed as
an opaque external tool, which keeps the interface stable across
Pepper releases and isolates Pepper's threading/MPI/Kokkos model
from AthenaMT.

`Pepper_Common.py` is **only** responsible for running Pepper and
producing the parton-level event file. Composing it with Pythia8,
Herwig, or any other shower happens at the DSID JO level, using the
existing standard fragments. This keeps the Pepper interface
single-purpose and composable.

## Runtime requirements

### GPU node

`pepper_kokkos` is built against Kokkos with CUDA enabled. On a
CPU-only worker the binary deadlocks during Kokkos initialisation
waiting for a CUDA device.

**All Pepper jobs must run on a GPU-equipped node.** On lxplus, that
means `lxplus-gpu` rather than the default `lxplus`. On the grid,
the corresponding queue/site selection applies.

### LCG dependency

This package depends on `pepper_kokkos` from the LCG release. The
build will fail at configure time if it isn't present in the
`AtlasGenerationExternals` view.

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

genSeq.Pythia8.Commands += [ "Beams:LHEF = " + PEPPER_OUTPUT ]
```

The user JO composes Pepper with whatever shower / decay chain they
want. `Pepper_Common.py` exports the produced event file as the
global `PEPPER_OUTPUT`, which the shower fragment then consumes.

### Filename and generators-list convention

- DSID filenames start with `mc.PepperPy8EG_<tune>_<process>.py`
  (or `mc.PepperHerwig_...`, etc. depending on shower).
- `evgenConfig.generators` includes `"Pepper"` as the first entry.

This requires `"Pepper"` to be in the allowed-generators whitelist
in `EvgenJobTransforms`. A follow-up MR adds it there.

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

`run_pepper()` consumes ~25 minutes on a fresh integration grid for
`ppjj` at 13.6 TeV. Subsequent runs reuse the `pepper_cache/`
directory in the run area and start producing events immediately.

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

### The `.events.gz` default output

`Gen_tf.py`'s post-job validator and `Pythia8_LHEF`'s reader both
expect input event files to match the glob `*.events*` (a convention
inherited from the MadGraph/Powheg workflow). Pepper's native
extension is `.lhef[.gz]`; we satisfy both by writing
`pepper.lhef.gz` under the hood and exposing `pepper.events.gz` as
the user-visible filename (via a symlink). Users and JO authors see
only the `.events.gz` form.
