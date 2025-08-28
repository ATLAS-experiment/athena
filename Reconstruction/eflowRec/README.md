# Overview

This package contains the main body of code used for Global Particle Flow in ATLAS.

The main algorithms are configured in [PFRun3Config](https://gitlab.cern.ch/atlas/athena/-/blob/main/Reconstruction/eflowRec/python/PFRun3Config.py) in the current setup. An example of how to run eflowRec from ESD, in Athena bia python, can be found in [PFRunESDtoAOD_WithJetsTausMET_mc21.py](https://gitlab.cern.ch/atlas/athena/-/blob/main/Reconstruction/eflowRec/python/PFRunESDtoAOD_WithJetsTausMET_mc21.py) - this also rebuilds topoclusters, jets, taus and MET.

Each algorithm inherits from an [AthAlgorithm](https://gitlab.cern.ch/atlas/athena/-/blob/main/Control/AthenaBaseComps/AthenaBaseComps/AthAlgorithm.h) or [AthReentrantAlgorithm](https://gitlab.cern.ch/atlas/athena/-/blob/main/Control/AthenaBaseComps/AthenaBaseComps/AthReentrantAlgorithm.h) (only the latter can be considered thread safe). Either may or may not also make use of one or several [AthAlgTool](https://gitlab.cern.ch/atlas/athena/-/blob/main/Control/AthenaBaseComps/AthenaBaseComps/AthAlgTool.h) which can be considered a smaller unit of algorithm.

The EDM for xAOD particle flow objects in release 21 was the [PFO](https://gitlab.cern.ch/atlas/athena/-/blob/main/Event/xAOD/xAODPFlow/xAODPFlow/versions/PFO_v1.h) and from release 22 onwards is the [FlowElement](https://gitlab.cern.ch/atlas/athena/-/blob/main/Event/xAOD/xAODPFlow/xAODPFlow/versions/FlowElement_v1.h).

Additional detailed documentation on each c++ file can be found in [doxygen](https://atlas-sw-doxygen.web.cern.ch/atlas-sw-doxygen/atlas_22.0.X-DOX/docs/html/dir_16e5ef933818970a3aca1a7c61395a43.html).

The physics logic that was used in the algorithm in 2017 is in the 2017 ATLAS [paper](https://arxiv.org/abs/1703.10485) on particle flow. Furthermore modifications to the physics of the algorithm logic were subsequently made which are documented [here](https://twiki.cern.ch/twiki/bin/view/AtlasProtected/ParticleFlowChargeSubtraction).

Below you can find details of the reconstruction algorithms for particle flow. This is followed by a list of variables, with explanations of their meaning, that the particle flow objects contain.

#  Details of Reconstruction Algorithms

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

# Charged Object Variables (Standard Reconstruction)
In addition to the 4-vector variables (which are taken from the ID track, at perigee, that the object represents) the following variables are available and have dedicated access functions:

- ChargedObjectLinks - a vector of ElementLink to IParticles. In practice this is always of size 1 and contains an ElementLink to the ID track represented by this object. Access via [chargedObjects()](https://gitlab.cern.ch/atlas/athena/-/blob/main/Event/xAOD/xAODPFlow/xAODPFlow/versions/FlowElement_v1.h)
- Charge - the electric charge of the ID track that this object represents. Access via [charge()](https://gitlab.cern.ch/atlas/athena/-/blob/main/Event/xAOD/xAODPFlow/xAODPFlow/versions/FlowElement_v1.h).
- OtherObjectsAndWeights - a vector of pairs, where each pair corresponds to an ElementLink to a CaloCluster and a double. The link represents a topocluster that the track was matched to AND that had energy removed in the charged shower subtraction procedure, whilst the double represents the amount of energy. Access via [otherObjectsAndWeights()](https://gitlab.cern.ch/atlas/athena/-/blob/main/Event/xAOD/xAODPFlow/xAODPFlow/versions/FlowElement_v1.h).
- SignalType - defines the type of particle flow object, which in this case is "ChargedPFlow". Available signal types are defined in the [FlowElement](https://gitlab.cern.ch/atlas/athena/-/blob/main/Event/xAOD/xAODPFlow/xAODPFlow/versions/FlowElement_v1.h) object. Access via [signalType()](https://gitlab.cern.ch/atlas/athena/-/blob/main/Event/xAOD/xAODPFlow/xAODPFlow/versions/FlowElement_v1.h).

The following variables can be accessed using accessors (or ReadDecorHandle):

- IsInDenseEnvironment - if the particle, which this object represents, is defined to pass through a dense environment of the calorimeter this is set to be true. For such particles no charged shower subtraction is performed in the matched topocluster(s).
- TracksExpectedEnergyDeposit - the amount of energy the track was expected to deposit in the calorimeter. This is calculated by a looking up a reference value, e/p (calorimeter energy/track momentum), and then multiplying the measured track energy by that reference value.
- FE_ElectronLinks - a vector of ElementLinks to Electrons. Any Electron which shares the ID track with this object is included in the list.
- FE_PhotonLinks - a vector of ElementLinks to Photons. Any Photon which shares the ID track with this object is included in the list.
- FE_MuonLinks - a vector of ElementLinks to Muons. Any Muon which shares the ID track with this object is included in the list.
- FE_TauLinks - a vector of ElementLinks to Taus. Any Tau which has a track, retrieved using the [tracks()](https://gitlab.cern.ch/atlas/athena/-/blob/main/Event/xAOD/xAODTau/xAODTau/versions/TauJet_v3.h) method, which is the same ID track represented by this object is included in this list.

# Neutral Object Variables (Standard Reconstruction)

In addition to the 4-vector variables (which are taken from the topocluster that the object represents the following variables are available and have dedicated access functions:

- Charge - the electric charge of the topocluster that this object represents (will be 0). Access via [charge()](https://gitlab.cern.ch/atlas/athena/-/blob/main/Event/xAOD/xAODPFlow/xAODPFlow/versions/FlowElement_v1.h).
- OtherObjectLinks - a vector of ElementLink to IParticles. In practice this is always of size 1 and contains an ElementLink to a topocluster. This is the topocluster that was used as input to particle flow and this topocluster is stored at the Local Hadron Calibration scale - in contrast the object represents a copy of that topocluster, at EM scale, that has had charged shower subtraction performed on it.
- SignalType - defines the type of particle flow object, which in this case is "NeutralPFlow". Available signal types are defined in the [FlowElement](https://gitlab.cern.ch/atlas/athena/-/blob/main/Event/xAOD/xAODPFlow/xAODPFlow/versions/FlowElement_v1.h) object. Access via [signalType()](https://gitlab.cern.ch/atlas/athena/-/blob/main/Event/xAOD/xAODPFlow/xAODPFlow/versions/FlowElement_v1.h).

The following variables can be accessed using accessors (or ReadDecorHandle):

- FE_ElectronLinks - a vector of ElementLinks to Electrons. Any Electron which shares the topocluster with this object is included in the list.
- FE_PhotonLinks - a vector of ElementLinks to Photons. Any Photon which shares the topocluster with this object is included in the list.
- FE_MuonLinks - a vector of ElementLinks to Muons. Any Muon which shares a calorimeter cell with this object is included in the list.
- FE_nMatchedMuons - number of muons that are matched according to the above definition.
- FE_efrac_matched_muon - sum of muon matched cell energy to topocluster energy, according to the definition above.
- FE_TauLinks - a vector of ElementLinks to Taus. Any Tau which has a topocluster, retrieved using the [clusters()](https://gitlab.cern.ch/atlas/athena/-/blob/main/Event/xAOD/xAODTau/xAODTau/versions/TauJet_v3.h) (and is within the dR(tauAxis,cluster) < 0.2 after a cluster vertex correction) method, which is the same topocluster represented by this object is included in this list.
- LAYERENERGY_TILE0 - this corresponds to the energy in the caloriemter layers TileBar0 and TileExt0 in the topocluster that this object represents.
- TIMING - this is the time of the topocluster that this object represents.

The following list of cluster moments are stored. They correspond to the moments on the topoclusters that this object represents. All cluster moments are defined [here](https://twiki.cern.ch/twiki/bin/viewauth/AtlasComputing/ClusterMoments).

- CENTER_MAG
- SECOND_R
- CENTER_LAMBDA
- ENG_BAD_CELLS
- N_BAD_CELLS
- BADLARQ_FRAC
- ENG_POS
- AVG_LAR_Q
- AVG_TILE_Q
- ISOLATION 
- SECOND_LAMBDA
- EM_PROBABILITY

The following list of calorimeter sampling energies are stored. They correspond to the energy in that calorimeter layer in the topocluster that this object represents.

- LAYERENERGY_PreSamplerB
- LAYERENERGY_EMB1
- LAYERENERGY_EMB2
- LAYERENERGY_EMB3
- LAYERENERGY_PreSamplerE
- LAYERENERGY_EME1
- LAYERENERGY_EME2
- LAYERENERGY_EME3
- LAYERENERGY_HEC0
- LAYERENERGY_HEC1
- LAYERENERGY_HEC2
- LAYERENERGY_HEC3
- LAYERENERGY_TileBar0
- LAYERENERGY_TileBar1
- LAYERENERGY_TileBar2
- LAYERENERGY_TileGap1
- LAYERENERGY_TileGap2
- LAYERENERGY_TileGap3
- LAYERENERGY_TileExt0
- LAYERENERGY_TileExt1
- LAYERENERGY_TileExt2
- LAYERENERGY_FCAL0
- LAYERENERGY_FCAL1
- LAYERENERGY_FCAL2
- LAYERENERGY_MINIFCAL0
- LAYERENERGY_MINIFCAL1
- LAYERENERGY_MINIFCAL2
- LAYERENERGY_MINIFCAL3

# Charged Object Variables (CP Mode)

This mode can be enabled in reconstruction by setting the flag PF.addCPData to true. The data is to be used by experts to further understand the performance of the algorithm.

The contents are identical to standard reconstruction with the addition of the following variables which can be accessed via an accessor or ReadDecorHandle:


- dRPrimes - list of deltaPrime values corresponding to the deltaRPrime value used in the initial track-cluster matching. Only values for matched topoclusters are stored. 
- EtaEM2 - the ID track that this object represents is extrapolated to the second layer of either the EMB or EME in the calorimeter. This is the eta coordinate of the extrapolated coordinate in that layer.
- isRecovered - if true it means the ID track that which this object represents underwent the split shower recovery process.
- numMatchedClusters - number of topoclusters that were matched to the ID track that this object represents.
- PhiEM2 - the ID track that this object represents is extrapolated to the second layer of either the EMB or EME in the calorimeter. This is the phi coordinate of the extrapolated coordinate in that layer.
- Pull15 - this is a fraction where the numerator is the sum of energy of matched topoclusters (to the ID track represented by this object) in a cone of size 0.15 minus the expected energy deposit. The denominator is the expected energy sigma - this is calculated by multiplying the ID track energy by the width of the reference e/p distribution.
- SubtractedCaloCells (called cellCPData in older AOD) - list of pairs, where each pair is an ElementLink to a CaloCell and a double. The double corresponds to the cell weight. Every cell which was removed from a topocluster by the ID track represented by this object is included in the list.

# Neutral Object Variables (CP Mode)

This mode can be enabled in reconstruction by setting the flag PF.addCPData to true. The data is to be used by experts to further understand the performance of the algorithm.

The contents are identical to standard reconstruction with the addition of the following variables which can be accessed via an accessor or ReadDecorHandle:

- ClusterWidthEta - variance of the cluster eta coordinate.
- ClusterWidthPhi - variance of the cluster phi coordinate.

# Charged Object Variables (EOverP Mode)

This mode can be enabled in reconstruction by setting the flag PF.EOverPMode to true. This mode disables the charged shower subtraction procedure so that the particle flow objects can be used to measure e/p. 

The contents are (mostly - see below) identical to standard reconstruction with the addition of the following variables which can be accessed via an accessor or ReadDecorHandle:

- LayerHED - using the matched topoclusters, to the ID track represented by this object, the longitudinal shower profile is measured.  This variable is the calorimeter layer in which the high density shower core is measured to be  located.

The following 3 vectors (also can be accessed via an accessor or ReadDecorHandle) correspond to lists of properties of the calorimeter cells found in the matched topocluster(s) (to the ID track represented by this object)

- LayerVectorCellOrdering - this is a vector of calorimeter layer values.
- RadiusVectorCellOrdering - this is vector of calorimeter cell ring indices, each multiplied by the ring thickness (which is a fixed value for each calorimeter layer)
- AvgEdensityVectorCellOrdering - this is a vector of cell energy densities. Within a cell ring all cells get given the average cell energy density in that ring.

The following variables are modified compared to the default reconstruction mode:

- eta - the ID track that this object represents is extrapolated to the second layer of either the EMB or EME in the calorimeter. This is the eta coordinate of the extrapolated coordinate in that layer.
- phi - the ID track that this object represents is extrapolated to the second layer of either the EMB or EME in the calorimeter. This is the phi coordinate of the extrapolated coordinate in that layer.







