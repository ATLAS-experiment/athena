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

Run the `Pythia8BConfig` CTest in an AthGeneration build. Physics validation
should compare its output with legacy DSID 801918.
