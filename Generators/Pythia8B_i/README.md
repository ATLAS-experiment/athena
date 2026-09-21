# Pythia8B ComponentAccumulator configuration

Use `Pythia8B_i.Pythia8BConfig` and `Pythia8B_i.Pythia8BProcesses` from a
`Sample(EvgenConfig).setupProcess` method, and run the job option with
`Gen_tf.py --CA True`:

```python
from Pythia8B_i.Pythia8BConfig import Pythia8B_A14_CTEQ6L1_EvtGen_Common_Cfg
from Pythia8B_i.Pythia8BProcesses import Pythia8B_exclusiveB_Common_Cfg

return Pythia8B_exclusiveB_Common_Cfg(
    flags,
    ShowerCfg=Pythia8B_A14_CTEQ6L1_EvtGen_Common_Cfg,
    Commands=["HardQCD:gg2bbbar = on", "PhaseSpace:pTHatMin = 8."],
    NHadronizationLoops=1,
)
```

The modules provide both A14 tunes, exclusive/inclusive B and anti-B production,
charmonium, bottomonium, and decay-selection fragments. Names follow the legacy
fragments with a `Cfg` suffix; `Pythia8BBaseCfg` replaces the base fragment.
`flags` supplies energy, seed and DSID. `name` defaults to `Pythia8B`; other
keyword arguments override generator properties. Pythia commands and Pythia8B
selection momenta use GeV, while `flags.Beam.Energy` uses Athena units.

Compose production/decay wrappers using `functools.partial` and `ShowerCfg`.
Nested wrappers collect commands into one process layer, inner commands first.
Commands execute in BASE, TUNE, MATCHING, USER order using the shared generator
settings semantics. Pass user commands through `Commands` on the configuration
function. Legacy tune commands and B/anti-B conventions are preserved.

EvtGen tune wrappers accept `EvtGenOptions`, for example
`{"userDecayFile": "my.dec"}`. EvtGen uses `inclusiveP8DsDPlus.pdt` by default;
selected input files are fetched from DATAPATH. `whiteList` extends the default
species list and `auxfiles` adds auxiliary files.

`Pythia8B_Photospp_Cfg(flags, ShowerCfg=..., PhotosppOptions=...)` appends Photos++
and disables native lepton QED showering by default. It follows EvtGen when both
are configured; the radiation prescription needs sample-specific validation
because EvtGen also has an internal radiation model.

Run the `Pythia8BConfig` CTest in AthGeneration. Configuration tests mock file
retrieval; Gen_tf runtime and physics validation must additionally use real
inputs and compare against legacy generation.
