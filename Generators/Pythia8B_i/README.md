# Pythia8B ComponentAccumulator configuration

Use `Pythia8B_i.Pythia8BConfig` from a `Sample(EvgenConfig).setupProcess`
method, and run the job option with `Gen_tf.py --CA True`:

```python
from Pythia8B_i.Pythia8BConfig import (
    Pythia8B_A14_CTEQ6L1_EvtGen_Common_Cfg,
    Pythia8B_exclusiveB_Common_Cfg,
)

return Pythia8B_exclusiveB_Common_Cfg(
    flags,
    ShowerCfg=Pythia8B_A14_CTEQ6L1_EvtGen_Common_Cfg,
    Commands=["HardQCD:gg2bbbar = on", "PhaseSpace:pTHatMin = 8."],
    NHadronizationLoops=1,
)
```

The module provides both A14 tunes, exclusive/inclusive B and anti-B production,
charmonium, bottomonium, and decay-selection fragments. Names follow the legacy
fragments with a `Cfg` suffix; `Pythia8BBaseCfg` replaces the base fragment.
`flags` supplies energy, seed and DSID. `name` defaults to `Pythia8B`; other
keyword arguments override generator properties. Pythia commands and Pythia8B
selection momenta use GeV, while `flags.Beam.Energy` uses Athena units.

Compose production/decay wrappers using `functools.partial` and `ShowerCfg`.
Nested wrappers collect commands into one process layer, inner commands first.
Commands execute in BASE, TUNE, PROCESS, USER order. Pythia8B preserves repeated
operations (including `onIfAny` and `addChannel`) using `GeneratorSettingsKeep.ALL`.
Pass user commands through `Commands` on the configuration function; raw
property assignments use the default LAST policy and cannot be merged with ALL.
Identical CA merges remain idempotent; distinct layers at the same precedence
remain an error. Legacy tune commands and B/anti-B conventions are preserved.

EvtGen tune wrappers accept `EvtGenOptions`, for example
`{"userDecayFile": "my.dec"}`. EvtGen uses `inclusiveP8DsDPlus.pdt` by default;
selected input files are fetched from DATAPATH unless already local. `whiteList`
extends the default species list and `auxfiles` adds auxiliary files.

`Pythia8B_Photospp_Cfg(flags, ShowerCfg=..., PhotosppOptions=...)` appends Photos++
and disables native lepton QED showering by default. It follows EvtGen when both
are configured; the radiation prescription needs sample-specific validation
because EvtGen also has an internal radiation model.

Run the `Pythia8BConfig` and `GeneratorSettingsSemantics` CTests in AthGeneration.
Configuration tests mock file retrieval; Gen_tf runtime and physics validation
must additionally use real inputs and compare against legacy generation.
