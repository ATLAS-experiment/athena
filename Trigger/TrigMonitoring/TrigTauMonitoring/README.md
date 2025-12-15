# TrigTauMonitoring

Tau offline monitoring package.

To add or remove chains from the monitoring, add/remove the `tauMon:t0`, `tauMon:shifter`, or `tauMon:val` monitoring groups in the [Trigger Menu definition files](https://gitlab.cern.ch/atlas/athena/-/blob/main/Trigger/TriggerCommon/TriggerMenuMT/python/HLT/Menu/) as required. If the Monitoring framework is executed standalone on AOD files that don't contain Monitoring information for the used Menu in the Metadata, the manual chain list defined in [ManualChains.py](python/ManualChains.py) will be loaded.

## Developing

Any major change to the monitoring package should be thoroughly tested on both MC signal and EB background AODs, as they could affect signal or background events asymmetrically. You can use SampleA $`Z\to\tau\tau`$ or $`\gamma^*\to\tau\tau`$ MC AODs, and AODs from the latest EB HLT reprocessing.

## How to Run standalone:

To execute the tau monitoring locally excluding all other signatures, after setting up Athena, run

```
Run3DQTestingDriver.py --inputFiles=path/to/input/AOD.root --dqOffByDefault DQ.Steering.doHLTMon=True DQ.Steering.HLT.doBjet=False DQ.Steering.HLT.doBphys=False DQ.Steering.HLT.doCalo=False DQ.Steering.HLT.doEgamma=False DQ.Steering.HLT.doJet=False DQ.Steering.HLT.doMET=False DQ.Steering.HLT.doMinBias=False DQ.Steering.HLT.doMuon=False DQ.Steering.HLT.doInDet=False 
```

Specify the number of threads with `--threads` for multi-threaded operation.

### Local Installation (optional):

In case you would like to run with a modified version of the monitoring, you would need first to perform a partial check-out of the `TrigTauMonitoring` package, following the instructions from the [ATLAS Git tutorial](https://atlas-software.docs.cern.ch/athena/git/).

### Run on the GRID :

Similarly, to execute the tau monitoring on the GRID, after setting up Athena and `panda` (with `lsetup panda && voms-proxy-init -voms atlas`), use

```
pathena --trf 'Run3DQTestingDriver.py --threads=$ATHENA_CORE_NUMBER --dqOffByDefault --inputFiles=%IN Output.HISTFileName=%OUT.HIST.root DQ.Steering.doHLTMon=True DQ.Steering.HLT.doBjet=False DQ.Steering.HLT.doBphys=False DQ.Steering.HLT.doCalo=False DQ.Steering.HLT.doEgamma=False DQ.Steering.HLT.doJet=False DQ.Steering.HLT.doMET=False DQ.Steering.HLT.doMinBias=False DQ.Steering.HLT.doMuon=False DQ.Steering.HLT.doInDet=False' --inDS=user.myname.myxAODDataset --outDS=user.myname.myHIST

```

## Additional options

### Calculate total efficiencies

All HLT efficiencies are estimated by the Tau monitoring with respect to the L1 accepted RoIs. The total efficiencies, with respect to all offline objects, can also be estimated for single-tau and di-tau chains, by adding the following configuration:

```
--preExec 'from TrigTauMonitoring.TrigTauMonitoringConfig import TrigTauMonAlgBuilder; TrigTauMonAlgBuilder.do_total_efficiency=True'
```
**Be careful!** You shouldn't enable these options when running over data samples acquired with the usual data-taking conditions, since comparisons between chains can be meaningless due to using different prescales! This is mainly to be used on Enhanced Bias and Monte Carlo samples.
