# DerivationFrameworkJetEtMiss

This package contains the JETMX derivation formats needed for Jet/Etmiss performance studies.

## How to run:

```
Derivation_tf.py --inputAODFile aod.pool.root --outputDAODFile test.pool.root --formats JETM1
```

Test files can be found	in the relevant	[ART test scripts](https://gitlab.cern.ch/atlas/athena/-/tree/main/PhysicsAnalysis/DerivationFramework/DerivationFrameworkART/DerivationFrameworkJetEtMissART/test?ref_type=heads).

## JETMX formats

* `JETM1`: MC calibrations (MC-JES, GSC) and in situ calibrations (eta-intercalibration, MJB, JER), trigger jet studies
* `JETM2`: MC only for tagger developments and JetDef R&D
* `JETM3`: *in situ* Z+jets calibration
* `JETM4`: *in situ* gamma+jets
* `JETM5`: random cones in zero bias data
* `JETM7`: per-vertex jet reconstruction
* `JETM12`: E/p studies in W to tau + nu events
* `JETM42`: PHYS + clusters and towers, mostly for upgrade TDAQ studies

## Contacts

For questions regarding the derivation formats, feel free to reach out to the JSV conveners: atlas-cp-jetetmiss-jsv-conveners(at)cern.ch

Responsibles for specific derivation formats: 
- `JETM7`: Steven Schramm 
- `JETM42`: Dylan Rankin 