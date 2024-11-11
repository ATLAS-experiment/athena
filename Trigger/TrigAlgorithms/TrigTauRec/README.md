# TrigTauRec package

This package contains the HLT-specific algorithms required for the TauJet reconstruction.

## Tau trigger configuration flags

All configuration flags used by the Tau trigger signature are defined in [`TrigTauConfigFlags.py`](python/TrigTauConfigFlags.py).
The structure of the offline Tau CP flags is copied in the `Trigger.Offline.Tau` directory, used to configure some general properties, e.g. `Trigger.Offline.Tau.MvaTESConfig`.

Each HLT TauID inference algorithm uses a dedicated sub-folder for its specific flags, e.g. `Trigger.Offline.Tau.GNTau`. Algorithms with a `ONNXConfig` flag (where the inference `.onnx` configuration file is indicated) are treated as ONNX-based algorithms, using the `tauRecTools/TauGNN` infrastructure. Otherwise, the algorithm is asumed to be LVNN-based, and the `tauRecTools/TauJetRNN` infrastructure is used instead.

## TauJet reconstruction algorithm

Following the usual convention in Athena, the bulk of the HLT TauJet reconstruction work is perfomed by a set of tools, shared with the offline TauJet reconstruction (and configured in [TrigTauRecToolsConfig.py](python/TrigTauRecToolsConfig.py)). Their execution is handled by the HLT-specific `TrigTauRecMerged` algorithm, defined in this package. This algorithm is responsible for the retrieval of the required input objects (depending on the type of reconstruction that will be handled), the execution of the reconstruction tools, and monitoring of relevant kinematic and calculated variables.

Following the structure of the tau trigger, the `TrigTauRecMerged` algorithm can be configured and executed in two main modes:
 * **Calorimeter-reconstruction:** an `xAOD::Jet` is first built from the calo-clusters reconstructed by the preceeding Calo sequences in the CaloMVA step of the tau trigger (first step of all tau chains). The jet is then used to seed a new `xAOD::TauJet`, stored in the output containers. The `TrigTauRecMerged` instance used in the _CaloMVA_ step, and all the required reconstruction tools, are configured by the [`trigTauRecMergedCaloMVACfg`](python/TrigTauRecConfig.py) function.
 * **TauJet-seeded reconstruction:** an `xAOD::TauJet` in the input collection is used as the starting object to be processed by the reconstruction tools. Any associated `xAOD::TauTrack`s to the input TauJet are removed, allowing for new tracks to be linked. The `TrigTauRecMerged` instance used in the _Precision_ step, and all the required reconstruction tools, are configured by the [`trigTauRecMergedPrecisionCfg`](python/TrigTauRecConfig.py) function.

The configuration for the monitoring histograms is available in [`TrigTauRecMonitoring.py`](python/TrigTauRecMonitoring.py).

## RoI updater algorithms

To optimize the execution time of the tau trigger chains, the tracking is done in multiple stages, with RoIs updated according to the contents of the previously reconstructed calorimeter clusters and tracks.

### TrigTauCaloRoiUpdater

The [`TrigTauCaloRoiUpdater`](src/TrigTauCaloRoiUpdater.h) algorithm is used in the CaloMVA step (first step of the Tau HLT chains), to update the RoI position based on the detector-axis of the reconstructed calorimeter clusters.
The configuration for this algorithm is provided by the [`tauCaloRoiUpdaterCfg`](python/TrigTauRoIToolsConfig.py) function.

### TrigTauTrackRoiUpdater

The [`TrigTauTrackRoiUpdater`](src/TrigTauTrackRoiUpdater.h) algorithm is used in the fast-tracking steps (right now only in the `tauCore` and `tauLRT` FTF steps), to update the RoI position to that of the leading reaconstructed track (if any). If a track was found, the RoI size is also updated to that specified in the algorithm properties. If no tracks are found, the RoI is left as-is. The configuration is handled by the [`tauTrackRoiUpdaterCfg`](python/TrigTauRoIToolsConfig.py) and [`tauLRTRoiUpdaterCfg`](python/TrigTauRoIToolsConfig.py) functions.
