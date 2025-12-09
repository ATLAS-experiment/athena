# Introduction
This *InDetTrackPerfMon* (or *IDTPM*) package is design to produce a user-defined set of tracking performance validation histograms.
It can perform the tracking validation for both offline tracking and Inner Detector trigger tracking.
It is used in Physics Validation, for automatised monitoring, but also for user-level performance checks during tracking development work.

More information about the design structure and the various use-cases can be found in the following [documentation page](https://codimd.web.cern.ch/XklVEplsSo-aNroVTrvIiw).

# Running the code 
Currently, it is possible to run this package "out of the box" without needing to be checked out or built by hand.
Building by hand is needed only when modifying the code.
The tracking performance validation runs on already existing ESD or AOD (or custom DAOD) files, without a reconstruction stes, by calling a set of standalone job options, which are explained in more details below, along with all the possible flags/configurations.

# Running IDTPM

Currently, it is possible to run this package "out of the box" without needing to be checked out or built by hand.
Building by hand is needed only when modifying the code.
The tracking performance validation runs on already existing ESD or AOD (or custom DAOD) files, without a reconstruction step, by calling a set of standalone job options, which are explained in more details below, along with all the possible flags/configurations.

The InDetTrackPerfMon package requires the full athena release. 
In `main`, you can set this up using for example: 
``` 
asetup Athena,main,latest 
```  
to get the most up-to date nightly or:
```
asetup Athena,25.0.2
``` 
to get a stable release. Recent releases can be found [like this](https://gitlab.cern.ch/atlas/athena/-/tags?sort=updated_desc&search=release) and the search will filter by name.
Release numbering is explained [here](https://atlas-software.docs.cern.ch/athena/developers/releases/).

The command for executing the IDTPM framework as a standalone job is [runIDTPM.py](https://gitlab.cern.ch/atlas/athena/-/blob/main/InnerDetector/InDetValidation/InDetTrackPerfMon/scripts/runIDTPM.py).
The minimal running command is the following:
```
runIDTPM.py \
    --filesInput <your input ESD/AOD/DAOD file> \
    --trkAnaCfgFile <your config JSON file>
```
An example of TrackAnalysis JSON configuration file has the following content:
```
{
    "TrkAnaTrig2" : {   # ---> 1st trigger-like TrackAnalysis name
        "enabled" : true,
        "TestType"  : "Trigger",
        "RefType"   : "Offline", 
        "TrigTrkKey"    : "HLT_IDTrack_Electron_IDTrig",
        "ChainNames"    : [ "HLT_e.*_idperf_.*" ],
        "MatchingType"    : "DeltaRMatch",
        "dRmax"           : 0.05
    },
    "TrkAnaOffl1" : {   # ---> 2nd offline-like TrackAnalysis name
        "enabled" : true,
        "TestType"  : "Offline",
        "RefType"   : "Truth", 
        "MatchingType"  : "TruthMatch" 
    }
}
```

Pre-tested examples of config JSON files can be found [here](https://gitlab.cern.ch/atlas/athena/-/tree/main/InnerDetector/InDetValidation/InDetTrackPerfMon/data).
