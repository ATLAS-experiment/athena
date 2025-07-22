# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

################################################################################
#
#@file TauToolHolder.py
#
#@brief All tau tools needed for tau reconstruction are configured here.
#
################################################################################

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from AthenaConfiguration.Enums import ProductionStep
from AthenaCommon.SystemOfUnits import GeV, deg

########################################################################
# JetSeedBuilder
@AccumulatorCache
def JetSeedBuilderCfg(flags):
    result = ComponentAccumulator()
    _name = flags.Tau.ActiveConfig.prefix + 'JetSeedBuilder'

    JetSeedBuilder = CompFactory.getComp("JetSeedBuilder")
    JetSeedBuilder = JetSeedBuilder(name = _name)
    
    result.setPrivateTools(JetSeedBuilder)
    return result

#########################################################################
def TauVertexFinderCfg(flags):
    result = ComponentAccumulator()
    _name = flags.Tau.ActiveConfig.prefix + 'TauVertexFinder'

    from InDetConfig.InDetTrackSelectionToolConfig import Tau_InDetTrackSelectionToolForTJVACfg
    from TrackVertexAssociationTool.TrackVertexAssociationToolConfig import TauTTVAToolCfg

    # Algorithm that overwrites numTrack() and charge() of tauJets in container
    # from tauRecTools.tauRecToolsConf import TauVertexFinder
    TauVertexFinder = CompFactory.getComp("TauVertexFinder")
    TauVertexFinder = TauVertexFinder(name = _name,
                                      UseTJVA                 = flags.Tau.doTJVA,
                                      AssociatedTracks="GhostTrack", # OK??
                                      InDetTrackSelectionToolForTJVA = result.popToolsAndMerge(Tau_InDetTrackSelectionToolForTJVACfg(flags)),
                                      Key_trackPartInputContainer= flags.Tau.ActiveConfig.TrackCollection,
                                      Key_vertexInputContainer = flags.Tau.ActiveConfig.VertexCollection,
                                      TVATool = result.popToolsAndMerge(TauTTVAToolCfg(flags)),
                                      inEleRM = flags.Tau.ActiveConfig.inTauEleRM,
                                      )

    result.setPrivateTools(TauVertexFinder)
    return result

########################################################################
# Tau energy calibration and tau axis direction
def TauAxisCfg(flags):
    result = ComponentAccumulator()
    _name = flags.Tau.ActiveConfig.prefix + 'TauAxis'

    TauAxisSetter = CompFactory.getComp("TauAxisSetter")
    TauAxisSetter = TauAxisSetter(  name = _name,
                                    ClusterCone = 0.2,
                                    VertexCorrection = flags.Tau.doVertexCorrection )

    result.setPrivateTools(TauAxisSetter)
    return result

#########################################################################

def getParticleCache(flags):
    #If reading from ESD we not create a cache of extrapolations to the calorimeter, so we should signify this by setting the cache key to a null string
    if flags.Common.ProductionStep is ProductionStep.Reconstruction:
        ParticleCache = "ParticleCaloExtension"
    else:
        ParticleCache = ""

    return ParticleCache

########################################################################
# Tau-Track Association
def TauTrackFinderCfg(flags):
    result = ComponentAccumulator()
    _name = flags.Tau.ActiveConfig.prefix + 'TauTrackFinder'

    # BeamSpot Conditions
    from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
    result.merge(BeamSpotCondAlgCfg(flags))

    from TrackToVertex.TrackToVertexConfig import TrackToVertexCfg
    from TrackToCalo.TrackToCaloConfig import ParticleCaloExtensionToolCfg
    from TrkConfig.TrkVertexFitterUtilsConfig import AtlasTrackToVertexIPEstimatorCfg
    from InDetConfig.InDetTrackSelectorToolConfig import TauRecInDetTrackSelectorToolCfg

    TauTrackFinder = CompFactory.getComp("TauTrackFinder")
    TauTrackFinder = TauTrackFinder(name = _name,
                                    MaxJetDrTau = 0.2,
                                    MaxJetDrWide = 0.4,
                                    TrackSelectorToolTau      = result.popToolsAndMerge(TauRecInDetTrackSelectorToolCfg(flags)),
                                    TrackToVertexTool         = result.popToolsAndMerge(TrackToVertexCfg(flags)),
                                    ParticleCaloExtensionTool = result.popToolsAndMerge(ParticleCaloExtensionToolCfg(flags)),
                                    tauParticleCache = getParticleCache(flags), # only returns a string
                                    removeDuplicateCoreTracks = flags.Tau.RemoveDupeCoreTracks,
                                    useGhostTracks = flags.Tau.useGhostTracks,
                                    ghostTrackDR = flags.Tau.ghostTrackDR,
                                    Key_jetContainer = (flags.Tau.ActiveConfig.SeedJetCollection if flags.Tau.useGhostTracks else ""),
                                    Key_trackPartInputContainer = flags.Tau.ActiveConfig.TrackCollection,
                                    Key_LargeD0TrackInputContainer = (flags.Tau.ActiveConfig.LargeD0TrackCollection if flags.Tau.associateLRT else ""),
                                    TrackToVertexIPEstimator = result.popToolsAndMerge(AtlasTrackToVertexIPEstimatorCfg(flags)),
                                    inEleRM = flags.Tau.ActiveConfig.inTauEleRM,
    )
    result.setPrivateTools(TauTrackFinder)
    return result

########################################################################
# Associate the cluster in jet constituents to the tau candidate
def TauClusterFinderCfg(flags):
    result = ComponentAccumulator()
    _name = flags.Tau.ActiveConfig.prefix + 'TauClusterFinder'


    TauClusterFinder = CompFactory.getComp("TauClusterFinder")
    TauClusterFinder = TauClusterFinder(
        name = _name, 
        inEleRM = flags.Tau.ActiveConfig.inTauEleRM,
    )


    result.setPrivateTools(TauClusterFinder)
    return result

########################################################################
def TauVertexedClusterDecoratorCfg(flags):
    result = ComponentAccumulator()
    _name = flags.Tau.ActiveConfig.prefix + 'TauVertexedClusterDecorator'

    TauVertexedClusterDecorator = CompFactory.getComp("TauVertexedClusterDecorator")
    myTauVertexedClusterDecorator = TauVertexedClusterDecorator(name = _name,
                                                                SeedJet = flags.Tau.ActiveConfig.SeedJetCollection)

    result.setPrivateTools(myTauVertexedClusterDecorator)
    return result

########################################################################
def TauTrackRNNCfg(flags):
    result = ComponentAccumulator()
    _name = flags.Tau.ActiveConfig.prefix + 'TauTrackRNNClassifier'

    TrackRNN = CompFactory.tauRecTools.TrackRNN
    _RNN = TrackRNN(name = _name + "_TrackRNN",
                    InputWeightsPath = flags.Tau.tauRNNTrackClassConfig,
                    calibFolder = flags.Tau.tauRecToolsCVMFSPath )

    result.setPrivateTools(_RNN)
    return result

########################################################################
def TauTrackRNNClassifierCfg(flags):
    result = ComponentAccumulator()
    _name = flags.Tau.ActiveConfig.prefix + 'TauTrackRNNClassifier'

    # create tool alg
    TauTrackRNNClassifier = CompFactory.tauRecTools.TauTrackRNNClassifier

    import cppyy
    cppyy.load_library('libxAODTau_cDict')

    _classifyLRT = True
    if flags.Tau.associateLRT and not flags.Tau.classifyLRT:
        _classifyLRT = False

    myTauTrackClassifier = TauTrackRNNClassifier( name = _name,
                                                  Classifiers = [ result.popToolsAndMerge(TauTrackRNNCfg(flags)) ],
                                                  classifyLRT = _classifyLRT )

    result.setPrivateTools(myTauTrackClassifier)
    return result

########################################################################
# Tau energy calibration
def EnergyCalibrationLCCfg(flags):
    result = ComponentAccumulator()
    _name = flags.Tau.ActiveConfig.prefix +'EnergyCalibrationLC'

    TauCalibrateLC = CompFactory.getComp("TauCalibrateLC")
    TauCalibrateLC = TauCalibrateLC(name = _name,
                                    calibrationFile = flags.Tau.CalibrateLCConfig,
                                    Key_vertexInputContainer = flags.Tau.ActiveConfig.VertexCollection)
            
    result.setPrivateTools(TauCalibrateLC)
    return result

########################################################################
# Tau cell variables calculation
def CellVariablesCfg(flags):
    result = ComponentAccumulator()
    _name = flags.Tau.ActiveConfig.prefix + 'CellVariables'

    TauCellVariables = CompFactory.getComp("TauCellVariables")
    TauCellVariables = TauCellVariables(name = _name,
                                        VertexCorrection = flags.Tau.doVertexCorrection)

    result.setPrivateTools(TauCellVariables)
    return result

#########################################################################
# ele veto variables
def ElectronVetoVarsCfg(flags):
    result = ComponentAccumulator()
    _name = flags.Tau.ActiveConfig.prefix + 'TauElectronVetoVars'
    
    from TrackToCalo.TrackToCaloConfig import ParticleCaloExtensionToolCfg

    TauElectronVetoVariables = CompFactory.getComp("TauElectronVetoVariables")
    TauElectronVetoVariables = TauElectronVetoVariables(name = _name,
                                                        VertexCorrection = flags.Tau.doVertexCorrection,
                                                        ParticleCaloExtensionTool = result.popToolsAndMerge(ParticleCaloExtensionToolCfg(flags)),
                                                        tauEVParticleCache = getParticleCache(flags))

    result.setPrivateTools(TauElectronVetoVariables)
    return result

#########################################################################
# Photon Shot Finder
def TauShotFinderCfg(flags):
    result = ComponentAccumulator()
    _name = flags.Tau.ActiveConfig.prefix + 'TauShotFinder'

    shotPtCut_1Photon = flags.Tau.shotPtCut_1Photon
    shotPtCut_2Photons = flags.Tau.shotPtCut_2Photons

    
    from CaloClusterCorrection.StandardCellWeightCalib import getCellWeightTool
    CaloWeightTool = getCellWeightTool(flags=flags)


    TauShotFinder = CompFactory.getComp("TauShotFinder")
    TauShotFinder = TauShotFinder(
        name                                = _name,
        CaloWeightTool                      = result.popToolsAndMerge(CaloWeightTool),
        NCellsInEta                         = 5,
        MinPtCut                            = shotPtCut_1Photon,
        AutoDoubleShotCut                   = shotPtCut_2Photons,
        Key_RemovedClusterInputContainer    = flags.Tau.ActiveConfig.RemovedElectronClusters,
        RemoveElectronCells                 = flags.Tau.ActiveConfig.RemoveElectronCells,
        inEleRM                             = flags.Tau.ActiveConfig.inTauEleRM,
    )

    result.setPrivateTools(TauShotFinder)
    return result

#########################################################################
# Pi0 algo
# Cluster finder for Pi0 algo
def Pi0ClusterFinderCfg(flags):
    result = ComponentAccumulator()
    _name = flags.Tau.ActiveConfig.prefix + 'Pi0ClusterFinder'

    TauPi0CreateROI = CompFactory.getComp("TauPi0CreateROI")
    
    myTauPi0CreateROI = TauPi0CreateROI(
        name                             = _name,
        Key_caloCellInputContainer       = "AllCalo",
        Key_RemovedClusterInputContainer = flags.Tau.ActiveConfig.RemovedElectronClusters,
        inEleRM                          = flags.Tau.ActiveConfig.inTauEleRM,
        RemoveElectronCells              = flags.Tau.ActiveConfig.RemoveElectronCells,
    )

    result.setPrivateTools(myTauPi0CreateROI)
    return result

#########################################################################
# Cell finalizer tool for BuildAlg
def TauCellFinalizerCfg(flags):
    result = ComponentAccumulator()
    _name = flags.Tau.ActiveConfig.prefix + 'tauPi0CellContainerFinalizer'

    CaloCellContainerFinalizerTool = CompFactory.getComp("CaloCellContainerFinalizerTool")
    TauCellContainerFinalizer = CaloCellContainerFinalizerTool(name=_name)

    result.setPrivateTools(TauCellContainerFinalizer)
    return result

#########################################################################
# Calo Tools
#########################################################################

def TauCaloLCClassificationCfg(flags):
    result = ComponentAccumulator()
    _name = flags.Tau.ActiveConfig.prefix + 'LCClassify'

    CaloLCClassificationTool = CompFactory.getComp("CaloLCClassificationTool")
    LCClassify = CaloLCClassificationTool(_name)
    LCClassify.ClassificationKey   = "EMFracClassify"
    LCClassify.UseSpread = False
    LCClassify.MaxProbability = 0.5
    # add the moments EM_PROBABILITY, HAD_WEIGHT, OOC_WEIGHT, DM_WEIGHT to the AOD:
    LCClassify.StoreClassificationProbabilityInAOD = True
    LCClassify.WeightingOfNegClusters = flags.Calo.TopoCluster.doTreatEnergyCutAsAbsolute
    LCClassify.UseNormalizedEnergyDensity = True

    result.setPrivateTools(LCClassify)
    return result

def TauCaloLCWeightCfg(flags):
    result = ComponentAccumulator()
    _name = flags.Tau.ActiveConfig.prefix + 'CaloLCWeightTool'

    # Schedule total noise cond alg
    from CaloTools.CaloNoiseCondAlgConfig import CaloNoiseCondAlgCfg
    result.merge(CaloNoiseCondAlgCfg(flags,"totalNoise"))
    # Schedule electronic noise cond alg (needed for LC weights)
    result.merge(CaloNoiseCondAlgCfg(flags,"electronicNoise"))

    CaloLCWeightTool = CompFactory.getComp("CaloLCWeightTool")
    LCWeight = CaloLCWeightTool(_name)
    LCWeight.CorrectionKey       = "H1ClusterCellWeights"
    LCWeight.SignalOverNoiseCut  = 2.0
    LCWeight.UseHadProbability   = True

    result.setPrivateTools(LCWeight)
    return result

def TauCaloLCOutOfClusterCfg(flags):
    result = ComponentAccumulator()
    _name = flags.Tau.ActiveConfig.prefix + 'CaloLCOutOfClusterTool'

    CaloLCOutOfClusterTool = CompFactory.getComp("CaloLCOutOfClusterTool")
    LCOut = CaloLCOutOfClusterTool(_name)
    LCOut.CorrectionKey       = "OOCCorrection"
    LCOut.UseEmProbability    = False
    LCOut.UseHadProbability   = True

    result.setPrivateTools(LCOut)
    return result

def TauCaloLCOutPi0Cfg(flags):
    result = ComponentAccumulator()
    _name = flags.Tau.ActiveConfig.prefix + 'LCOutPi0'

    CaloLCOutOfClusterTool = CompFactory.getComp("CaloLCOutOfClusterTool")
    LCOutPi0 = CaloLCOutOfClusterTool(_name)
    LCOutPi0.CorrectionKey    = "OOCPi0Correction"
    LCOutPi0.UseEmProbability  = True
    LCOutPi0.UseHadProbability = False

    result.setPrivateTools(LCOutPi0)
    return result

def TauCaloLCDeadMaterialCfg(flags):
    result = ComponentAccumulator()
    _name = flags.Tau.ActiveConfig.prefix + 'CaloLCDeadMaterialTool'

    CaloLCDeadMaterialTool = CompFactory.getComp("CaloLCDeadMaterialTool")
    LCDeadMaterial = CaloLCDeadMaterialTool(_name)
    LCDeadMaterial.HadDMCoeffKey       = "HadDMCoeff2"
    LCDeadMaterial.ClusterRecoStatus   = 0
    LCDeadMaterial.WeightModeDM        = 2
    LCDeadMaterial.UseHadProbability   = True
    LCDeadMaterial.WeightingOfNegClusters = flags.Calo.TopoCluster.doTreatEnergyCutAsAbsolute

    result.setPrivateTools(LCDeadMaterial)
    return result

def TauCaloClusterLocalCalibCfg(flags):
    result = ComponentAccumulator()
    _name = flags.Tau.ActiveConfig.prefix + 'CaloLocalCalib'

    CaloClusterLocalCalib = CompFactory.getComp("CaloClusterLocalCalib")
    LocalCalib =  CaloClusterLocalCalib(_name)
    LocalCalib.ClusterClassificationTool     = [result.popToolsAndMerge(TauCaloLCClassificationCfg(flags))]
    LocalCalib.ClusterRecoStatus             = [1,2]
    LocalCalib.LocalCalibTools               = [result.popToolsAndMerge(TauCaloLCWeightCfg(flags))]
    LocalCalib.WeightingOfNegClusters = flags.Calo.TopoCluster.doTreatEnergyCutAsAbsolute

    result.setPrivateTools(LocalCalib)
    return result

def TauCaloOOCCalibCfg(flags):
    result = ComponentAccumulator()
    _name = flags.Tau.ActiveConfig.prefix + 'OOCCalibTool'

    CaloClusterLocalCalib = CompFactory.getComp("CaloClusterLocalCalib")
    OOCCalib   = CaloClusterLocalCalib (_name)
    OOCCalib.ClusterRecoStatus   = [1,2]
    OOCCalib.LocalCalibTools     = [result.popToolsAndMerge(TauCaloLCOutOfClusterCfg(flags))]
    OOCCalib.WeightingOfNegClusters = flags.Calo.TopoCluster.doTreatEnergyCutAsAbsolute

    result.setPrivateTools(OOCCalib)
    return result

def TauCaloDMCalibCfg(flags):
    result = ComponentAccumulator()
    _name = flags.Tau.ActiveConfig.prefix + 'DMCalib'

    CaloClusterLocalCalib = CompFactory.getComp("CaloClusterLocalCalib")
    DMCalib =  CaloClusterLocalCalib(_name)
    DMCalib.ClusterRecoStatus             = [1,2]
    DMCalib.LocalCalibTools = [result.popToolsAndMerge(TauCaloLCDeadMaterialCfg(flags))]
    DMCalib.WeightingOfNegClusters = flags.Calo.TopoCluster.doTreatEnergyCutAsAbsolute

    result.setPrivateTools(DMCalib)
    return result

def TauCaloOOCPi0CalibCfg(flags):
    result = ComponentAccumulator()
    _name = flags.Tau.ActiveConfig.prefix + 'OOCPi0CalibTool'

    CaloClusterLocalCalib = CompFactory.getComp("CaloClusterLocalCalib")
    OOCPi0Calib   = CaloClusterLocalCalib (_name)
    OOCPi0Calib.ClusterRecoStatus   = [1,2]
    OOCPi0Calib.LocalCalibTools     = [result.popToolsAndMerge(TauCaloLCOutPi0Cfg(flags))]
    OOCPi0Calib.WeightingOfNegClusters = flags.Calo.TopoCluster.doTreatEnergyCutAsAbsolute

    result.setPrivateTools(OOCPi0Calib)
    return result

def TauCaloTopoClusterMakerCfg(flags):
    result = ComponentAccumulator()
    _name = flags.Tau.ActiveConfig.prefix + 'Pi0TopoClusterMaker'

    # Schedule total noise cond alg
    from CaloTools.CaloNoiseCondAlgConfig import CaloNoiseCondAlgCfg
    result.merge(CaloNoiseCondAlgCfg(flags,"totalNoise"))
    # Schedule electronic noise cond alg (needed for LC weights)
    result.merge(CaloNoiseCondAlgCfg(flags,"electronicNoise"))

    CaloTopoClusterMaker = CompFactory.getComp("CaloTopoClusterMaker")
    TopoClusterForTaus = CaloTopoClusterMaker(_name)

    TopoClusterForTaus.CellsName = flags.Tau.ActiveConfig.TauCommonPi0Cells
    TopoClusterForTaus.CalorimeterNames=["LAREM"]
    TopoClusterForTaus.SeedSamplingNames = ["PreSamplerB", "EMB1", "EMB2", "PreSamplerE", "EME1", "EME2"]
    TopoClusterForTaus.NeighborOption                    = "super3D"
    TopoClusterForTaus.RestrictHECIWandFCalNeighbors     = False
    TopoClusterForTaus.RestrictPSNeighbors               = True
    TopoClusterForTaus.CellThresholdOnEorAbsEinSigma     = 0.0
    TopoClusterForTaus.NeighborThresholdOnEorAbsEinSigma = 2.0
    TopoClusterForTaus.SeedThresholdOnEorAbsEinSigma     = 4.0
    TopoClusterForTaus.SeedCutsInAbsE                    = True
    TopoClusterForTaus.ClusterCutsInAbsEt                = True
    TopoClusterForTaus.ClusterEtorAbsEtCut               = 0.5*GeV # 0.0*MeV in standard CaloCalTopoCluster JobOptions!
    TopoClusterForTaus.TwoGaussianNoise                  = flags.Calo.TopoCluster.doTwoGaussianNoise
    # timing cut on seed cell to suppress out-of-time pileup
    TopoClusterForTaus.SeedCutsInT = flags.Calo.TopoCluster.doTimeCut
    TopoClusterForTaus.CutOOTseed = flags.Calo.TopoCluster.extendTimeCut and flags.Calo.TopoCluster.doTimeCut
    TopoClusterForTaus.UseTimeCutUpperLimit = flags.Calo.TopoCluster.useUpperLimitForTimeCut
    TopoClusterForTaus.TimeCutUpperLimit = flags.Calo.TopoCluster.timeCutUpperLimit
    TopoClusterForTaus.XTalkEM2 = flags.Calo.TopoCluster.xtalkEM2
    TopoClusterForTaus.XTalkEM2D = flags.Calo.TopoCluster.xtalkEM2D
    TopoClusterForTaus.XTalkEM2n = flags.Calo.TopoCluster.xtalkEM2n
    TopoClusterForTaus.XTalkEM3 = flags.Calo.TopoCluster.xtalkEM3
    TopoClusterForTaus.XTalkEMEta = flags.Calo.TopoCluster.xtalkEMEta
    TopoClusterForTaus.XTalkDeltaT = flags.Calo.TopoCluster.xtalkDeltaT
    TopoClusterForTaus.XTalk2Eratio1 = flags.Calo.TopoCluster.xtalk2Eratio1
    TopoClusterForTaus.XTalk2Eratio2 = flags.Calo.TopoCluster.xtalk2Eratio2
    TopoClusterForTaus.XTalk3Eratio = flags.Calo.TopoCluster.xtalk3Eratio
    TopoClusterForTaus.XTalkEtaEratio = flags.Calo.TopoCluster.xtalkEtaEratio
    TopoClusterForTaus.XTalk2DEratio = flags.Calo.TopoCluster.xtalk2DEratio

    result.setPrivateTools(TopoClusterForTaus)
    return result

def TauCaloTopoClusterSplitterCfg(flags):

    result = ComponentAccumulator()
    _name = flags.Tau.ActiveConfig.prefix + 'Pi0TopoClusterSplitter'

    CaloTopoClusterSplitter = CompFactory.getComp("CaloTopoClusterSplitter")
    TopoSplitterForTaus = CaloTopoClusterSplitter(_name)

    # cells from the following samplings will be able to form local
    # maxima. The excluded samplings are PreSamplerB, EMB1,
    # PreSamplerE, EME1, all Tile samplings, all HEC samplings and the
    # two rear FCal samplings.
    TopoSplitterForTaus.SamplingNames = ["EMB2","EME2"] # Do we want to use EMB3 and EME3?
    # cells from the following samplings will also be able to form
    # local maxima but only if they are not overlapping in eta and phi
    # with local maxima in previous samplings from the primary list.
    TopoSplitterForTaus.SecondarySamplingNames = ["EMB1","EME1"]
    TopoSplitterForTaus.ShareBorderCells = True
    TopoSplitterForTaus.RestrictHECIWandFCalNeighbors  = False
    TopoSplitterForTaus.WeightingOfNegClusters = flags.Calo.TopoCluster.doTreatEnergyCutAsAbsolute

    result.setPrivateTools(TopoSplitterForTaus)
    return result

def TauCaloClusterMomentsMakerCfg(flags):
    result = ComponentAccumulator()
    _name = flags.Tau.ActiveConfig.prefix + 'Pi0TopoMoments'

    # Schedule total noise cond alg
    from CaloTools.CaloNoiseCondAlgConfig import CaloNoiseCondAlgCfg
    result.merge(CaloNoiseCondAlgCfg(flags,"totalNoise"))
    # Schedule electronic noise cond alg (needed for LC weights)
    result.merge(CaloNoiseCondAlgCfg(flags,"electronicNoise"))

    CaloClusterMomentsMaker = CompFactory.getComp("CaloClusterMomentsMaker")
    TopoMomentsForTaus = CaloClusterMomentsMaker(_name)
    TopoMomentsForTaus.WeightingOfNegClusters = flags.Calo.TopoCluster.doTreatEnergyCutAsAbsolute
    TopoMomentsForTaus.MaxAxisAngle = 20*deg
    TopoMomentsForTaus.TwoGaussianNoise = flags.Calo.TopoCluster.doTwoGaussianNoise
    TopoMomentsForTaus.MinBadLArQuality = 4000
    TopoMomentsForTaus.MomentsNames = ["FIRST_PHI"
                                       ,"FIRST_ETA"
                                       ,"SECOND_R"
                                       ,"SECOND_LAMBDA"
                                       ,"DELTA_PHI"
                                       ,"DELTA_THETA"
                                       ,"DELTA_ALPHA"
                                       ,"CENTER_X"
                                       ,"CENTER_Y"
                                       ,"CENTER_Z"
                                       ,"CENTER_MAG"
                                       ,"CENTER_LAMBDA"
                                       ,"LATERAL"
                                       ,"LONGITUDINAL"
                                       ,"ENG_FRAC_EM"
                                       ,"ENG_FRAC_MAX"
                                       ,"ENG_FRAC_CORE"
                                       ,"FIRST_ENG_DENS"
                                       ,"SECOND_ENG_DENS"
                                       ,"ISOLATION"]

    result.setPrivateTools(TopoMomentsForTaus)
    return result

def TauCaloClusterBadChannelCfg(flags):
    result = ComponentAccumulator()
    _name = flags.Tau.ActiveConfig.prefix + 'CaloClusterBadChannelList'

    CaloClusterBadChannelListCorr = CompFactory.getComp("CaloClusterBadChannelList")
    BadChannelListCorrForTaus = CaloClusterBadChannelListCorr(_name)

    result.setPrivateTools(BadChannelListCorrForTaus)
    return result

#####################
# create Pi0 clusters
def Pi0ClusterCreatorCfg(flags):
    result = ComponentAccumulator()
    _name = flags.Tau.ActiveConfig.prefix + 'Pi0ClusterCreator'

    TauPi0ClusterCreator = CompFactory.getComp("TauPi0ClusterCreator")
    TauPi0ClusterCreator = TauPi0ClusterCreator(name = _name)

    result.setPrivateTools(TauPi0ClusterCreator)
    return result

#####################
# Set energy of cluster to take care of charged pion energy deposited in the ECAL
def Pi0ClusterScalerCfg(flags): 
    result = ComponentAccumulator()
    _name = flags.Tau.ActiveConfig.prefix + 'Pi0ClusterScaler'

    TauPi0ClusterScaler = CompFactory.getComp("TauPi0ClusterScaler")
    TauPi0ClusterScaler = TauPi0ClusterScaler(name = _name)

    result.setPrivateTools(TauPi0ClusterScaler)
    return result

#####################
# calculate MVA scores of pi0 clusters
def Pi0ScoreCalculatorCfg(flags):
    result = ComponentAccumulator()
    _name = flags.Tau.ActiveConfig.prefix + 'Pi0ScoreCalculator'

    TauPi0ScoreCalculator = CompFactory.getComp("TauPi0ScoreCalculator")
    TauPi0ScoreCalculator = TauPi0ScoreCalculator(name = _name,
                                                  BDTWeightFile = flags.Tau.Pi0ScoreConfig)

    result.setPrivateTools(TauPi0ScoreCalculator)
    return result

#####################
# select pi0 clusters
def Pi0SelectorCfg(flags):
    result = ComponentAccumulator()
    _name = flags.Tau.ActiveConfig.prefix + 'Pi0Selector'

    TauPi0Selector = CompFactory.getComp("TauPi0Selector")
    TauPi0Selector = TauPi0Selector(name = _name,
                                    ClusterEtCut         = flags.Tau.pi0EtCuts,
                                    ClusterBDTCut_1prong = flags.Tau.pi0MVACuts_1prong,
                                    ClusterBDTCut_mprong = flags.Tau.pi0MVACuts_mprong)

    result.setPrivateTools(TauPi0Selector)
    return result

#########################################################################
# Tau Vertex Variables
def TauVertexVariablesCfg(flags):
    result = ComponentAccumulator()
    _name = flags.Tau.ActiveConfig.prefix + 'TauVertexVariables'

    from TrkConfig.TrkVertexFittersConfig import TauAdaptiveVertexFitterCfg
    from TrkConfig.TrkVertexSeedFinderToolsConfig import (
        CrossDistancesSeedFinderCfg)

    TauVertexVariables = CompFactory.getComp("TauVertexVariables")
    TauVertexVariables = TauVertexVariables(  name = _name,
                                              VertexFitter = result.popToolsAndMerge(TauAdaptiveVertexFitterCfg(flags)),
                                              SeedFinder = result.popToolsAndMerge(CrossDistancesSeedFinderCfg(flags)) )

    result.setPrivateTools(TauVertexVariables)
    return result

#########################################################################
# Tau Variables
def TauCommonCalcVarsCfg(flags):
    result = ComponentAccumulator()
    _name = flags.Tau.ActiveConfig.prefix + 'TauCommonCalcVars'

    TauCommonCalcVars = CompFactory.getComp("TauCommonCalcVars")
    TauCommonCalcVars = TauCommonCalcVars(name = _name)
    
    result.setPrivateTools(TauCommonCalcVars)
    return result

#########################################################################
# Tau Variables
def TauSubstructureCfg(flags):
    result = ComponentAccumulator()
    _name = flags.Tau.ActiveConfig.prefix + 'TauSubstructure'

    TauSubstructureVariables = CompFactory.getComp("TauSubstructureVariables")
    TauSubstructureVariables = TauSubstructureVariables(  name = _name )

    result.setPrivateTools(TauSubstructureVariables)
    return result

########################################################################
# MvaTESVariableDecorator
def MvaTESVariableDecoratorCfg(flags):
    result = ComponentAccumulator()
    _name = flags.Tau.ActiveConfig.prefix + 'MvaTESVariableDecorator'

    eventShapeCollection = flags.Tau.ActiveConfig.EventShapeCollection
    if 'EMPFlow' in flags.Tau.ActiveConfig.SeedJetCollection:
        eventShapeCollection = "Kt4EMPFlowEventShape"
    elif 'EMTopo' in flags.Tau.ActiveConfig.SeedJetCollection:
        eventShapeCollection ="Kt4EMTopoOriginEventShape"

    MvaTESVariableDecorator = CompFactory.getComp("MvaTESVariableDecorator")
    MvaTESVariableDecorator = MvaTESVariableDecorator(name = _name,
                                                      Key_vertexInputContainer = flags.Tau.ActiveConfig.VertexCollection,
                                                      EventShapeKey = eventShapeCollection,
                                                      VertexCorrection = flags.Tau.doVertexCorrection)
    result.setPrivateTools(MvaTESVariableDecorator)
    return result

########################################################################
# MvaTESEvaluator
def MvaTESEvaluatorCfg(flags):
    result = ComponentAccumulator()
    _name = flags.Tau.ActiveConfig.prefix + 'MvaTESEvaluator'

    MvaTESEvaluator = CompFactory.getComp("MvaTESEvaluator")
    MvaTESEvaluator = MvaTESEvaluator(name = _name,
                                      WeightFileName = flags.Tau.MvaTESConfig,
                                      WeightFileName0p = flags.Tau.MvaTESConfig0p )

    result.setPrivateTools(MvaTESEvaluator)
    return result

def TauIDVarCalculatorCfg(flags):
    result = ComponentAccumulator()
    _name = flags.Tau.ActiveConfig.prefix + 'TauIDVarCalculator'

    TauIDVarCalculator = CompFactory.getComp("TauIDVarCalculator")    
    myTauIDVarCalculator = TauIDVarCalculator(name=_name,
                                              VertexCorrection = flags.Tau.doVertexCorrection)

    result.setPrivateTools(myTauIDVarCalculator)
    return result

def TauJetRNNEvaluatorCfg(flags):
    result = ComponentAccumulator()
    _name = flags.Tau.ActiveConfig.prefix + 'TauJetRNN'

    TauJetRNNEvaluator = CompFactory.getComp("TauJetRNNEvaluator")
    RNNConf = flags.Tau.TauJetRNNConfig
    myTauJetRNNEvaluator = TauJetRNNEvaluator(name = _name,
                                              NetworkFile0P = "",
                                              NetworkFile1P = RNNConf[0],
                                              NetworkFile2P = RNNConf[1],
                                              NetworkFile3P = RNNConf[2],
                                              OutputVarname = "RNNJetScore",
                                              MaxTracks = 10,
                                              MaxClusters = 6,
                                              MaxClusterDR = 1.0,
                                              VertexCorrection = flags.Tau.doVertexCorrection,
                                              InputLayerScalar = "scalar",
                                              InputLayerTracks = "tracks",
                                              InputLayerClusters = "clusters",
                                              OutputLayer = "rnnid_output",
                                              OutputNode = "sig_prob")

    result.setPrivateTools(myTauJetRNNEvaluator)
    return result

def TauWPDecoratorJetRNNCfg(flags):
    import PyUtils.RootUtils as ru
    ROOT = ru.import_root()
    import cppyy
    cppyy.load_library('libxAODTau_cDict')

    result = ComponentAccumulator()
    _name = flags.Tau.ActiveConfig.prefix + 'TauWPDecoratorJetRNN'

    TauWPDecorator = CompFactory.getComp("TauWPDecorator")
    WPConf = flags.Tau.TauJetRNNWPConfig
    myTauWPDecorator = TauWPDecorator( name=_name,
                                       flatteningFile1Prong = WPConf[0],
                                       flatteningFile2Prong = WPConf[1],
                                       flatteningFile3Prong = WPConf[2],
                                       CutEnumVals =
                                       [ ROOT.xAOD.TauJetParameters.IsTauFlag.JetRNNSigVeryLoose,
                                         ROOT.xAOD.TauJetParameters.IsTauFlag.JetRNNSigLoose,
                                         ROOT.xAOD.TauJetParameters.IsTauFlag.JetRNNSigMedium,
                                         ROOT.xAOD.TauJetParameters.IsTauFlag.JetRNNSigTight ],
                                       SigEff1P = [0.95, 0.85, 0.75, 0.60],
                                       SigEff2P = [0.95, 0.75, 0.60, 0.45],
                                       SigEff3P = [0.95, 0.75, 0.60, 0.45],
                                       ScoreName = "RNNJetScore",
                                       NewScoreName = "RNNJetScoreSigTrans",
                                       DefineWPs = True )

    result.setPrivateTools(myTauWPDecorator)
    return result

def TauGNNEvaluatorCfg(flags, version=0, applyTrackSel=False):
    result = ComponentAccumulator()
    _name = flags.Tau.ActiveConfig.prefix + 'TauGNN_v' + str(version)

    TauGNNEvaluator = CompFactory.getComp("TauGNNEvaluator")
    GNNConf = flags.Tau.TauGNNConfig[version]
    myTauGNNEvaluator = TauGNNEvaluator(name = _name,
                                              NetworkFileInclusive = GNNConf,
                                              OutputVarname = flags.Tau.GNTauScoreName[version],
                                              OutputPTau = "GNTauProbTau",
                                              OutputPJet = "GNTauProbJet",
                                              MaxTracks = flags.Tau.GNTauMaxTracks[version], 
                                              MaxClusters = flags.Tau.GNTauMaxClusters[version],
                                              MaxClusterDR = 15.0,
                                              MinTauPt = flags.Tau.MinPtDAOD,
                                              ApplyTrackSel = applyTrackSel,
                                              VertexCorrection = flags.Tau.doVertexCorrection,
                                              InputLayerScalar = "tau_vars",
                                              InputLayerTracks = "track_vars",
                                              InputLayerClusters = "cluster_vars",
                                              NodeNameTau=flags.Tau.GNTauNodeNameTau,
                                              NodeNameJet=flags.Tau.GNTauNodeNameJet)

    result.setPrivateTools(myTauGNNEvaluator)
    return result

def TauWPDecoratorGNNCfg(flags, version, tauContainerName=""):
    result = ComponentAccumulator()
    _name = flags.Tau.ActiveConfig.prefix + 'TauWPDecoratorGNN_v' + str(version)

    TauWPDecorator = CompFactory.getComp("TauWPDecorator")
    WPConf = flags.Tau.TauGNNWP[version]
    myTauWPDecorator = TauWPDecorator(name=_name,
                                      flatteningFile1Prong = WPConf[0],
                                      flatteningFile2Prong = WPConf[1],
                                      flatteningFile3Prong = WPConf[2],
                                      TauContainerName = tauContainerName,
                                      DecorWPNames = flags.Tau.GNTauDecorWPNames[version],
                                      DecorWPCutEffs1P = [0.95, 0.85, 0.75, 0.60],
                                      DecorWPCutEffs2P = [0.95, 0.75, 0.60, 0.45],
                                      DecorWPCutEffs3P = [0.95, 0.75, 0.60, 0.45],
                                      ScoreName = flags.Tau.GNTauScoreName[version],
                                      NewScoreName = flags.Tau.GNTauTransScoreName[version],
                                      DefineWPs = True)
    result.setPrivateTools(myTauWPDecorator)
    return result

def TauEleRNNEvaluatorCfg(flags):
    result = ComponentAccumulator()
    _name = flags.Tau.ActiveConfig.prefix + 'TauEleRNN'

    TauJetRNNEvaluator = CompFactory.getComp("TauJetRNNEvaluator")
    RNNConf = flags.Tau.TauEleRNNConfig    
    myTauEleRNNEvaluator = TauJetRNNEvaluator(name = _name,
                                              NetworkFile1P = RNNConf[0],
                                              NetworkFile3P = RNNConf[1],
                                              OutputVarname = "RNNEleScore",
                                              MaxTracks = 10,
                                              MaxClusters = 6,
                                              MaxClusterDR = 1.0,
                                              VertexCorrection = flags.Tau.doVertexCorrection,
                                              InputLayerScalar = "scalar",
                                              InputLayerTracks = "tracks",
                                              InputLayerClusters = "clusters",
                                              OutputLayer = "rnneveto_output",
                                              OutputNode = "sig_prob")

    result.setPrivateTools(myTauEleRNNEvaluator)
    return result

def TauWPDecoratorEleRNNCfg(flags):
    import PyUtils.RootUtils as ru
    ROOT = ru.import_root()
    import cppyy
    cppyy.load_library('libxAODTau_cDict')

    result = ComponentAccumulator()
    _name = flags.Tau.ActiveConfig.prefix + 'TauWPDecoratorEleRNN'

    TauWPDecorator = CompFactory.getComp("TauWPDecorator")
    WPConf = flags.Tau.TauEleRNNWPConfig
    myTauEleWPDecorator = TauWPDecorator( name=_name,
                                       flatteningFile1Prong = WPConf[0],
                                       flatteningFile3Prong = WPConf[1],
                                       CutEnumVals =
                                       [ ROOT.xAOD.TauJetParameters.IsTauFlag.EleRNNLoose,
                                         ROOT.xAOD.TauJetParameters.IsTauFlag.EleRNNMedium,
                                         ROOT.xAOD.TauJetParameters.IsTauFlag.EleRNNTight ],
                                       SigEff1P = [0.95, 0.90, 0.85],
                                       SigEff3P = [0.98, 0.95, 0.90],
                                       UseAbsEta = True ,
                                       ScoreName = "RNNEleScore",
                                       NewScoreName = "RNNEleScoreSigTrans",
                                       DefineWPs = True)

    result.setPrivateTools(myTauEleWPDecorator)
    return result

def TauWPDecoratorEleRNNFixCfg(flags):
    result = ComponentAccumulator()
    _name = flags.Tau.ActiveConfig.prefix + 'TauWPDecoratorEleRNNFix_v1'

    TauWPDecorator = CompFactory.getComp("TauWPDecorator")
    WPConf = flags.Tau.TauEleRNNWPfix
    myTauEleWPDecorator = TauWPDecorator(name = _name,
                                         flatteningFile1Prong = WPConf[0],
                                         flatteningFile3Prong = WPConf[1],
                                         DecorWPNames = [ "EleRNNLoose_v1", "EleRNNMedium_v1", "EleRNNTight_v1" ],
                                         DecorWPCutEffs1P = [0.95, 0.90, 0.85],
                                         DecorWPCutEffs3P = [0.98, 0.95, 0.90],
                                         UseAbsEta = True,
                                         ScoreName = "RNNEleScore",
                                         NewScoreName = "RNNEleScoreSigTrans_v1",
                                         DefineWPs = True)

    result.setPrivateTools(myTauEleWPDecorator)
    return result

def TauDecayModeNNClassifierCfg(flags):
    result = ComponentAccumulator()
    _name = flags.Tau.ActiveConfig.prefix + 'TauDecayModeNNClassifier'

    TauDecayModeNNClassifier = CompFactory.getComp("TauDecayModeNNClassifier")
    myTauDecayModeNNClassifier = TauDecayModeNNClassifier(name=_name,
                                                          WeightFile=flags.Tau.DecayModeNNClassifierConfig)

    result.setPrivateTools(myTauDecayModeNNClassifier)
    return result

def TauAODSelectorCfg(flags):
    result = ComponentAccumulator()
    _name = flags.Tau.ActiveConfig.prefix + 'TauAODSelector'

    TauAODSelector = CompFactory.getComp("TauAODSelector")
    myTauAODSelector = TauAODSelector(name=_name,
                                      Min0pTauPt = flags.Tau.MinPt0p,
                                      MinTauPt = flags.Tau.MinPt,
                                      doEarlyStopping = flags.Tau.doEarlyStopping and not flags.Output.doWriteESD)

    result.setPrivateTools(myTauAODSelector)
    return result

########################################################################
# TauCombinedTES
def TauCombinedTESCfg(flags):
    result = ComponentAccumulator()    
    _name = flags.Tau.ActiveConfig.prefix + 'TauCombinedTES'

    TauCombinedTES = CompFactory.getComp("TauCombinedTES")
    myTauCombinedTES = TauCombinedTES(name = _name,
                                      WeightFileName = flags.Tau.CombinedTESConfig)

    result.setPrivateTools(myTauCombinedTES)
    return result

########################################################################
# muon removal tool
def TauAODMuonRemovalCfg(flags):
    result = ComponentAccumulator()   
    _name = flags.Tau.ActiveConfig.prefix + 'MuonRemoval'
    TauAODMuonRemovalTool = CompFactory.getComp("TauAODMuonRemovalTool")
    myMuonRemoval = TauAODMuonRemovalTool(      name                   = _name,
                                                Key_MuonInputContainer = 'Muons',
                                                doMuonTrkRm            = True,
                                                doMuonClsRm            = True,
                                                muonIDWP               = 'Medium'
    )
    result.setPrivateTools(myMuonRemoval)
    return result

# electron excluder tool
def TauEleOverlapChecker(flags):
    result = ComponentAccumulator()
    _name = flags.Tau.ActiveConfig.prefix + 'TauEleOverlapChecker'
    TauEleOverlapChecker = CompFactory.getComp("TauEleOverlapChecker")
    myTauEleOverlapChecker = TauEleOverlapChecker(
        name                         = _name,
        Key_RemovedClustersContainer = flags.Tau.ActiveConfig.RemovedElectronClusters,
        Key_RemovedTracksContainer   = flags.Tau.ActiveConfig.RemovedElectronTracks,
        CheckingCone                 = flags.Tau.ActiveConfig.EleRM_CheckingConeSize,
    )
    result.setPrivateTools(myTauEleOverlapChecker)
    return result
