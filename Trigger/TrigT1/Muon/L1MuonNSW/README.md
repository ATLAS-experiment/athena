# L1MuonNSW

Run 4 (Phase-II) simulation of the New Small Wheel (NSW) part of the L1Muon trigger.

## Content

- `NSWSimulation`: the algorithm. It reads the MM and sTGC digits, runs the MM trigger tool and encodes
  every MM candidate into one segment of the output `xAOD::L1NSWCandDataContainer` (key
  `L1NSWCandidates`). As in Run 3, all candidates are written: no duplicate removal and no limit on
  the number of segments per board and bunch crossing. The overflow flag marks the frames with more
  than 8 segments. The sTGC trigger is not implemented yet.
- `L1Muon::L1MM::MMTriggerTool` (interface `IL1MMTriggerTool`): emulation of the MM Trigger Processor.
  It is a port of the Run 3 `NSWL1::MMTriggerTool` of `Trigger/TrigT1/TrigT1NSWSimTools`, which is not
  modified. `MMT_Road` and `MMT_Diamond` (the Diamond Road algorithm) are copies of the Run 3 classes.
  `MMT_Hit` is ported to the Phase-II muon geometry (`MuonReadoutGeometryR4`).
- `L1MMDefinitions.h`: definitions shared by the MM trigger classes: the detector side and layer type
  enums and the electronics constants of an MM sector layer (VMMs, MMFE8 boards, ART ASICs).
- `NSWEncoding.h`: quantisation into the Run 4 NSW-TP to Sector Logic segment word (L1Muon interface
  document, EDMS 2377909, Section 4.2): global eta (14 bits), global phi (10 bits), Delta theta
  (5 bits, sign and magnitude) and quality (3 bits).

## Running

```
python -m L1MuonNSW.L1MuonNSWConfig
```

runs the Phase-II geometry test job on `MuonPhaseIITestDefaults.RDO_R3` (20 events): RDO to digits for
MM and sTGC, then `NSWSimulation`. This is also the unit test of the package (`ctest`).
`L1MuonNSWSimCfg` is used by the full chain test `python -m L1MuonValidation.FullChainTrigger`.
