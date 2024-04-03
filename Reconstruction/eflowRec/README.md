# Overview

This package contains the main body of code used for Global Particle Flow in ATLAS.

The main algorithms are configured in [PFRun3Config](https://gitlab.cern.ch/atlas/athena/-/blob/main/Reconstruction/eflowRec/python/PFRun3Config.py) in the current setup. An example of how to run eflowRec from ESD, in Athena bia python, can be found in [PFRunESDtoAOD_WithJetsTausMET_mc21.py](https://gitlab.cern.ch/atlas/athena/-/blob/main/Reconstruction/eflowRec/python/PFRunESDtoAOD_WithJetsTausMET_mc21.py) - this also rebuilds topoclusters, jets, taus and MET.

Each algorithm inherits from an [AthAlgorithm](https://gitlab.cern.ch/atlas/athena/-/blob/main/Control/AthenaBaseComps/AthenaBaseComps/AthAlgorithm.h) or [AthReentrantAlgorithm](https://gitlab.cern.ch/atlas/athena/-/blob/main/Control/AthenaBaseComps/AthenaBaseComps/AthReentrantAlgorithm.h) (only the latter can be considered thread safe). Either may or may not also make use of one or several [AthAlgTool](https://gitlab.cern.ch/atlas/athena/-/blob/main/Control/AthenaBaseComps/AthenaBaseComps/AthAlgTool.h) which can be considered a smaller unit of algorithm.

The EDM for xAOD particle flow objects in release 21 was the [PFO](https://gitlab.cern.ch/atlas/athena/-/blob/main/Event/xAOD/xAODPFlow/xAODPFlow/versions/PFO_v1.h) and from release 22 onwards is the [FlowElement](https://gitlab.cern.ch/atlas/athena/-/blob/main/Event/xAOD/xAODPFlow/xAODPFlow/versions/FlowElement_v1.h).

Additional detailed documentation on each c++ file can be found in [doxygen](https://atlas-sw-doxygen.web.cern.ch/atlas-sw-doxygen/atlas_22.0.X-DOX/docs/html/dir_16e5ef933818970a3aca1a7c61395a43.html).

The physics logic that was used in the algorithm in 2017 is in the 2017 ATLAS [paper](https://arxiv.org/abs/1703.10485) on particle flow. Furthermore modifications to the physics of the algorithm logic were subsequently made which are documented [here](https://twiki.cern.ch/twiki/bin/view/AtlasProtected/ParticleFlowChargeSubtraction).

# Further Details

To further understand what eflowRec does one could start with the python configuration code, [PFCfg in PFRun3Config.py](https://gitlab.cern.ch/atlas/athena/-/blob/main/Reconstruction/eflowRec/python/PFRun3Config.py#L59), which specifies which algorithms are run

Here is a quick overview of the algorithms, though as noted in the overview a lot more details can be found in the [doxygen](https://atlas-sw-doxygen.web.cern.ch/atlas-sw-doxygen/atlas_22.0.X-DOX/docs/html/dir_16e5ef933818970a3aca1a7c61395a43.html):

- [PFLeptonSelector](https://gitlab.cern.ch/atlas/athena/-/blob/main/Reconstruction/eflowRec/eflowRec/PFLeptonSelector.h) - selects electrons and muons for later usage.
- [PFTrackSelector](https://gitlab.cern.ch/atlas/athena/-/blob/main/Reconstruction/eflowRec/eflowRec/PFTrackSelector.h) - selects which tracks would be used as input to particle flow (makes use of electrons/muons above to veto their tracks as inputs). Selected tracks are used to create [eflowRecTrack](https://gitlab.cern.ch/atlas/athena/-/blob/main/Reconstruction/eflowRec/eflowRec/eflowRecTrack.h) objects
- [PFAlgorithm](https://gitlab.cern.ch/atlas/athena/-/blob/main/Reconstruction/eflowRec/eflowRec/PFAlgorithm.h) - executes some [AthAlgTools](https://gitlab.cern.ch/atlas/athena/-/blob/main/Control/AthenaBaseComps/AthenaBaseComps/AthAlgTool.h) as follows:
    - [PFClusterSelectorTool](https://gitlab.cern.ch/atlas/athena/-/blob/main/Reconstruction/eflowRec/eflowRec/PFClusterSelectorTool.h) - selects calorimeter topocluster and creates [eflowRecCluster](https://gitlab.cern.ch/atlas/athena/-/blob/main/Reconstruction/eflowRec/eflowRec/eflowRecCluster.h) objects.
    - [PFSubtractionTool](https://gitlab.cern.ch/atlas/athena/-/blob/main/Reconstruction/eflowRec/eflowRec/PFSubtractionTool.h) (configured as [PFCellLevelSubtractionTool](https://gitlab.cern.ch/atlas/athena/-/blob/main/Reconstruction/eflowRec/python/PFCfg.py#L62) on the python side) - takes tracks and looks for 1 matching calorimeter cluster, subtracts charged showers from matched calorimeter clusters if certain criteria are met.
    - [PFSubtractionTool](https://gitlab.cern.ch/atlas/athena/-/blob/main/Reconstruction/eflowRec/eflowRec/PFSubtractionTool.h) (configured as [PFRecoverSplitShowersTool](https://gitlab.cern.ch/atlas/athena/-/blob/main/Reconstruction/eflowRec/python/PFCfg.py#L94) on the python side) - as above, but only uses tracks which did not have a good track-cluster match above. This iteration allows > 1 calorimeter cluster to match to a track.
    - [PFMomentCalculatorTool](https://gitlab.cern.ch/atlas/athena/-/blob/main/Reconstruction/eflowRec/eflowRec/PFMomentCalculatorTool.h) - calculate cluster moments for remaining calorimeter clusters
    - [PFLCCalibTool](https://gitlab.cern.ch/atlas/athena/-/blob/main/Reconstruction/eflowRec/eflowRec/PFLCCalibTool.h) - calculate Local Hadron Calibration (LC) energy scale for remaining calorimeter cluster

Finally it creates [FlowElement](https://gitlab.cern.ch/atlas/athena/-/blob/main/Event/xAOD/xAODPFlow/xAODPFlow/versions/FlowElement_v1.h) (FE) objects, which represent 4-vectors of both charged and neutral objects. These are then used in downstream algorithms (e.g jet finding) or analysis code etc:
- [PFChargedFlowElementCreatorAlgorithm](https://gitlab.cern.ch/atlas/athena/-/blob/main/Reconstruction/eflowRec/eflowRec/PFChargedFlowElementCreatorAlgorithm.h) - create charged FE (represent tracks)  
- [PFNeutralFlowElementCreatorAlgorithm](https://gitlab.cern.ch/atlas/athena/-/blob/main/Reconstruction/eflowRec/eflowRec/PFNeutralFlowElementCreatorAlgorithm.h) - create neutral FE (represent clusters)  
- [PFLCNeutralFlowElementCreatorAlgorithm](https://gitlab.cern.ch/atlas/athena/-/blob/main/Reconstruction/eflowRec/eflowRec/PFLCNeutralFlowElementCreatorAlgorithm.h) - create representation of neutral FE at LC scale
- [PFEGamFlowElementAssoc](https://gitlab.cern.ch/atlas/athena/-/blob/main/Reconstruction/eflowRec/eflowRec/PFEGamFlowElementAssoc.h) - adds links between FE and egamma (electron/photon) objects, the links being based on whether the same track(s) or calorimeter cluster(s) were used to create the egamma/FE objects
 - [PFMuonFlowElementAssoc](https://gitlab.cern.ch/atlas/athena/-/blob/main/Reconstruction/eflowRec/eflowRec/PFMuonFlowElementAssoc.h) - same thing for muons
 - [PFTauFlowElementAssoc](https://gitlab.cern.ch/atlas/athena/-/blob/main/Reconstruction/eflowRec/eflowRec/PFTauFlowElementAssoc.h) - same thing for taus





