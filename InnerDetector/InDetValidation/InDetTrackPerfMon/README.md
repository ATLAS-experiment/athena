# Introduction
This *InDetTrackPerfMon* (or *IDTPM*) package is design to produce a user-defined set of tracking performance validation histograms.
It can perform the tracking validation for both offline tracking and Inner Detector trigger tracking.
It is used in Physics Validation, for automatised monitoring, but also for user-level performance checks during tracking development work.

More information about the design structure and the various use-cases can be found in the following [documentation page](https://codimd.web.cern.ch/gl-GVAEcTPCgGYQCes7pPg) (currently in R&D).

# Running the code 
Currently, it is possible to run this package "out of the box" without needing to be checked out or built by hand.
Building by hand is needed only when modifying the code.
The tracking performance validation runs on already existing ESD or AOD (or custom DAOD) files, without a reconstruction stes, by calling a set of standalone job options, which are explained in more details below, along with all the possible flags/configurations.

## Setting up the release 
The InDetTrackPerfMon package requires the full athena release. 
In `main`, you can set this up using for example 
``` 
asetup Athena,main,latest 
```  
to get the most up-to date nightly or 
```
asetup Athena,25.0.2
``` 
to get a stable release. Recent releases can be found [like this](https://gitlab.cern.ch/atlas/athena/-/tags?sort=updated_desc&search=release) and the search will filter by name.
Release numbering is explained [here](https://atlassoftwaredocs.web.cern.ch/athena/athena-releases/).

## Standalone job runnning
The standalone script for exacuting the IDTPM framework is [runIDTPM.py](scripts/runIDTPM.py).
For example:
```
runIDTPM.py --filesInput <your input file, ESD/AOD/DAOD> --outputFile MyIDTPM_output.root --trkAnaCfgFile <your config file, JSON>
```
The available flags are:
* `--filesInput FILE` Select the input file in AOD.root format to run on (**required**);
* `--outputFile NAME` changes the name of the output histogram file, i.e. "MyIDTPM_output.root" in the example above;
* `--maxEvents NUM` defines the maximum number of events to run over;
* `--debug` enable DEBUG logging stream;
* `--dirName NAME` sets the name of the main TDirectory in the histogram output file (default is `InDetTrackPerfMonPlots/`;
* `--trkAnaCfgFile CONFIG.json` configure the various instances of the IDTPM tool by setting the relative flags read from a JSON file (see corresponding [section below](#configuration));
* `--unpackTrigChains` for the Trigger analysis, runs a separate instance of the IDTPM tool in parallel for each configured trigger chain (default is `False`).

# Configuration
The InDetTrackPerfMon framework is designed to be rather flexibly configurable, in order to allow the user to steer between the various possible use-cases.
Each IDTPM job shedules the parallelized execution of different instances of the main IDTPM tool, which correspond to the different tracking perfomance analyses being run, here referred to as *TrackAnalyses*.
Each *TrackAnalysis* can be configured completelly independently from the other by setting its corresponding flags and configurable paramenters, which are read from a configuration file in JSON format.
Each entry in this json file shedules a separate *TrackAnalysis* (if the `enabled` flag is set to `true`)
An emxaple of a TrackAnalysis configuration is:
```
    "TrkAnaTrig" : {
        "enabled" : true,
        "TestType"  : "Trigger",
        "RefType"   : "Truth", 
        "TrigTrkKey"    : "HLT_IDTrack_Electron_IDTrig",
        "ChainNames"    : [ "HLT_e26_idperf_loose_L1EM22VHI", 
                            "HLT_e5_idperf_tight_L1EM3" ],
        "MatchingType"  : "TruthMatch", 
        "SubFolder"  : "TrkAnaTrig1/"
}
```
which schedules a TrackAnalysis named "TrkAnaTrig".
Other examples of possible configurations can be found in [trkAnaConfigs_example.json](data/trkAnaConfigs_example.json).

Each of these flags/configurable parameters in the JSON file modify the behaviour of each "sub-tool" called in the main IDTPM tool of that job instance.
The full set of configurable flags can be found in [InDetTrackPerfMonFlags.py](python/InDetTrackPerfMonFlags.py).
