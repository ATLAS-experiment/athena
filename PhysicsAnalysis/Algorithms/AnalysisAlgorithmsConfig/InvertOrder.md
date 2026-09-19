Inverting the Algorithm Order
=============================

The declared data dependencies of the analysis algorithms are what let the
scheduler order them correctly.  If a dependency is missing, a job can still
produce the right answer purely because the algorithms happen to be added in a
sensible order.  This tool checks that the *declared* dependencies alone are
sufficient, by re-running the job with the algorithm order **maximally
inverted** — as reversed as the dependencies allow.

The idea is three steps:

1. **Capture** the declared data dependencies from a job log.
2. **Extract** the direct algorithm-to-algorithm dependency graph as JSON.
3. **Re-run** the job with a maximally-inverted order that still respects that
   graph, and check the output is unchanged.

If the inverted job produces the same output, the declared dependencies are
sufficient.  If it crashes or produces different output, the inverted order has
surfaced a missing dependency.

> **Important:** the inversion check must be run under the **sequential
> scheduler** (i.e. *without* `--threads`), because only then does the order of
> the algorithms in the sequence directly drive the execution order.  The
> capture step in (1), however, needs the multithreaded scheduler to emit the
> dependency dump — see the notes at the bottom.


The two building blocks
-----------------------

* `extract_alg_dependencies.py <log> -o alg_dependencies.json` — parses the
  scheduler's `Data Dependencies for Algorithms:` dump out of a log file (it
  tolerates the `Derivation_tf.py`/athena line prefixes) and writes the direct
  dependency graph as JSON.  An `INPUT` with no matching `OUTPUT` is assumed to
  come from the input file and is ignored.

* `AnalysisAlgorithmsConfig.InvertAlgOrder.invertAlgOrder(ca, deps,
  sequenceName=..., pinned=...)` — a reusable helper that reorders a sequence of
  a fully-assembled `ComponentAccumulator` in place.  It works recursively:
  sub-sequences are positioned as units by the aggregate dependencies of the
  algorithms they contain, concurrent sub-sequences are additionally inverted
  internally, and sequential sub-sequences keep their internal order.  Any names
  passed in `pinned` are kept at the front of the sequence.


CP Algorithms (`CPRun.py`)
--------------------------

`CPRun.py` has a built-in `--invert-alg-order` option, so no extra scripting is
needed.

```sh
# 1. Capture the data dependencies. The dump is emitted by the multithreaded
#    scheduler, so run once with a thread (--test-mt-dependencies 1).
CPRun.py --input-list $ASG_TEST_FILE_RUN3_MC \
  --text-config AnalysisAlgorithmsConfig/test_configuration_physlite_like.yaml \
  -e 1 --test-mt-dependencies 1 | tee dep_log.txt

# 2. Extract the dependency graph.
extract_alg_dependencies.py dep_log.txt -o alg_dependencies.json

# 3. Re-run (sequentially) with the inverted order.
CPRun.py --input-list $ASG_TEST_FILE_RUN3_MC \
  --text-config AnalysisAlgorithmsConfig/test_configuration_physlite_like.yaml \
  -e 1 --invert-alg-order alg_dependencies.json
```

The analysis algorithms live in their own `AthSequencer` sub-sequence, so
`--invert-alg-order` reorders exactly those and leaves the framework algorithms
untouched.  A clean run means the declared dependencies are sufficient.


PHYSLITE derivation (`Derivation_tf.py`)
----------------------------------------

`Derivation_tf.py` has no dedicated flag, but its `--postExec` snippets are
executed with the `ComponentAccumulator` bound to the variable `cfg`, right
before the job runs.  That is the hook used to invert the order.

The derivation algorithms live in `AthAlgSeq`.  `SGInputLoader` is a member of
that sequence and is the implicit producer of all the input-file data, but it
declares no dependencies of its own — so it is pinned to the front to keep the
input consumers after it.

```sh
export MY_TEST_FILE=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/data18/AOD/data18_13TeV.00357772.physics_Main.merge.AOD.r13286_p4910/1000events.AOD.27655096._000455.pool.root.1

# 1. Capture the data dependencies (Scheduler.ShowDataDeps needs the
#    multithreaded scheduler, so run with --threads=1).
Derivation_tf.py --CA --athenaopts '--threads=1 Scheduler.ShowDataDeps=True' \
  --inputAODFile $MY_TEST_FILE --outputDAODFile deps.pool.root \
  --formats PHYSLITE --maxEvents 10 | tee log-physlite

# 2. Extract the dependency graph.
extract_alg_dependencies.py log-physlite -o alg_dependencies.json

# 3a. Baseline run (sequential, no --threads).
Derivation_tf.py --CA --inputAODFile $MY_TEST_FILE \
  --outputDAODFile serial.pool.root --formats PHYSLITE --maxEvents 10

# 3b. Inverted run (sequential) via --postExec.
Derivation_tf.py --CA --inputAODFile $MY_TEST_FILE \
  --outputDAODFile invert.pool.root --formats PHYSLITE --maxEvents 10 \
  --postExec 'from AnalysisAlgorithmsConfig.InvertAlgOrder import invertAlgOrder; invertAlgOrder(cfg, "alg_dependencies.json", sequenceName="AthAlgSeq", pinned=["SGInputLoader"])'

# 4. Compare the two outputs.
acmd.py diff-root -t CollectionTree --order-trees --entries 0:10 --mode detailed \
  --error-mode resilient --nan-equal \
  DAOD_PHYSLITE.serial.pool.root DAOD_PHYSLITE.invert.pool.root
```


Interpreting the result
-----------------------

* **Pass:** `diff-root` reports `0 different leaves` (`all good.`), or the
  `CPRun.py` outputs match.  The declared dependencies enforce a correct
  ordering on their own.
* **Fail:** a scheduler/StoreGate error, a crash, or differing output means the
  inverted order ran an algorithm before one it actually depends on — a missing
  declared dependency.  The failing algorithm points at what needs its
  dependency fixed.

The inversion logs a line per reordered sequence, e.g.

```
Py:invertAlgOrder    INFO Inverted order in sequence 'AthAlgSeq' (172 members)
```

and warns if any algorithm in the sequence is absent from the dependency file
(a stale or mismatched `alg_dependencies.json` — regenerate it).


Notes
-----

* The **capture** step needs the multithreaded scheduler to emit the
  `Data Dependencies for Algorithms:` dump (`--test-mt-dependencies` for
  `CPRun.py`, or `--athenaopts '--threads=1 Scheduler.ShowDataDeps=True'` for
  `Derivation_tf.py`).  The **inversion check** itself must run sequentially.
* The dependency file is specific to a given job configuration; regenerate it
  whenever the configuration changes.
* `pinned` is not limited to `SGInputLoader`; pass any infrastructure
  algorithms that must stay at the front.
