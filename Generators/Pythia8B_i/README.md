# Pythia8B ComponentAccumulator configuration

This package currently supports the configuration needed to migrate legacy
Pythia8B DSID 801918:

- A14 CTEQ6L1 tune;
- exclusive B-hadron production;
- Photos++ final-state radiation;
- sample-specific Pythia commands and Pythia8B selection properties.

All public fragments are in `Pythia8B_i.Pythia8BConfig`. See
`EvgenJobTransforms/share/EvgenTest/801918/` for the CA rewrite of the real
production job option.

The CA tune explicitly sets `SpaceShower:rapidityOrderMPI = on` using the
shared Pythia8 helper. The legacy Pythia8B tune fragment leaves this setting
implicit.

After building and setting up AthGeneration, run these tests from the build
directory:

```sh
ctest --output-on-failure -R '^Pythia8B_i_Pythia8BConfig_ctest$'
ctest --output-on-failure -R '^CITest_Generation_CA_P8B_13p6TeV_ctest$'
```

The configuration test loads the actual 801918 job option and checks cuts,
decay-command order, metadata, and generator/Photos sequencing. It also checks
custom algorithm names and user overrides of base, tune, and process settings.

The generation test follows the other CA generator workflows: it generates
10 events at 13.6 TeV and runs the standard workflow checks. CI tests must be
enabled with `ATLAS_ENABLE_CI_TESTS=TRUE` (the default for WorkDir builds).
To run the same workflow directly in a configured release with CVMFS access:

```sh
RunWorkflowTests_Run3.py --CI -g --dsid Test801918 -e '--CA True'
```

Use `Test801918` to select the CA job option installed with Athena; `801918`
selects the production job option. The smoke test does not establish physics
equivalence: compare decay content, selection efficiency, cross-section
reporting, and EVNT metadata with legacy DSID 801918 in the same release.
