# tauRec package

This package is the python steering package for tau reconstruction and the c++ algorithms. This package has two algorithms: the TauProcessorAlg (also referred to as TauBuilder in some places) and  the TauRunnerAlg that call tools defined in tauRecTools. This page contains a quick summary of the structure with links to the relevant code. For more details please see the [tauRec r22 tWiki page](https://twiki.cern.ch/twiki/bin/view/AtlasProtected/TauRecR22).

### Tau Reconstruction chain:

Two main algorithms are run, the TauBuilder and the TauRunner, each of which runs the tools implemented in [tauRecTools](https://gitlab.cern.ch/atlas/athena/blob/main/Reconstruction/tauRecTools/)

The following is a summary of the configuration and the python steering files: 

0. The entry point called by general reconstruction is the [TauReconstructionCfg](https://gitlab.cern.ch/atlas/athena/-/blob/main/Reconstruction/tauRec/python/TauConfig.py) function. This in turn calls three main configuration blocks, one for the tau building ( from seed jet to tau object ), one to run the topoclustering for the pi0 reconstruction/tau particle flow, and the last block to schedule other tools acting on the tau object. 

1. The first config block is the [TauBuildAlgCfg](https://gitlab.cern.ch/atlas/athena/-/blob/main/Reconstruction/tauRec/python/TauConfig.py), where the TauBuilder algorithm is created and where all the steps starting from the seedjet to tau track classification and cluster association. All the tools configuration can be found in [TauToolHolder](https://gitlab.cern.ch/atlas/athena/-/blob/main/Reconstruction/tauRec/python/TauToolHolder.py) while the tool implementation can be found in [tauRecTools](https://gitlab.cern.ch/atlas/athena/blob/main/Reconstruction/tauRecTools/)

2. The second config block deals with the tools scheduled to perform the topoclustering for the pi0 reconstruction/tau particle flow. A separated block is needed because this runs per event, unlike the builder and the runner which are running per object. 

3. The third config block is the [TauRunnerAlgCfg](https://gitlab.cern.ch/atlas/athena/-/blob/main/Reconstruction/tauRec/python/TauConfig.py), where the TauRunner algorithm is created and where all steps from tau particle flow to the TauEnergyScale, Tau Identification/Classification are steered. All the tools configuration can be found in [TauToolHolder](https://gitlab.cern.ch/atlas/athena/-/blob/main/Reconstruction/tauRec/python/TauToolHolder.py), while the tool implementation can be found in [tauRecTools](https://gitlab.cern.ch/atlas/athena/blob/main/Reconstruction/tauRecTools/)

Note that the tools can be created with input arguments, so that they can be changed. This is required in some cases where a tool is used multiple times but with different configurations. In this case the tool configuration in [TauToolHolder](https://gitlab.cern.ch/atlas/athena/-/blob/main/Reconstruction/tauRec/python/TauToolHolder.py) can be changed using the flags from [TauConfigFlags](https://gitlab.cern.ch/atlas/athena/-/blob/main/Reconstruction/tauRec/python/TauConfigFlags.py). This allows options to be set for multiple tools at once, and for them to be set via the command line


### TauBuilder

The TauBuilder is used to:
  * Create tau candidates from jet seeds
  * Build a tau candidate if jet seed passes minimal kinematic criteria
  * Associate a vertex
  * Associate tracks
  * Classify tracks
  * Associate vertex corrected clusters 
  * Create Pi0 candidates

The builder sets up the required tools. 

### TauRunner

This algorithm runs calculations that require input from the tools scheduled in the previous algorithms and:

  * Construct pi0 clusters (part of substructure)
  * Run substructure tools
  * Run Calo based EnergyScale
  * Run PanTau
  * MVA TauEnergyScale
  * Apply pt cut on calibrated taus
  * Build identification variables used tauID or subsequent algorithms
  * Tau Identification score and flattening
  * Evaluate and decorate scores

The structure is similar to the TauBuilder

### TauToolHolder

This module contains the functions to configure each tauRecTool. 

### TauConfigFlags

Each tool has steerable properties which can be configured using the [TauConfigFlags](https://gitlab.cern.ch/atlas/athena/-/blob/main/Reconstruction/tauRec/python/TauConfigFlags.py). Please consult this file to understand for example:

1. define a variable in one place to be used in numerous tools (e.g. cvmfs CALIBPATH folder)
2. allow for the setting of variables on the command line:  --preExec ' all:flags.Tau.MinPt=0.; flags.Tau.MinPt0p=0 ' 

Being able to set variables on the command line is extremely important.  In this way we can request an r-tag without making any code changes.

