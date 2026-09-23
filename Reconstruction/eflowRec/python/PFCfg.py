# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.Enums import LHCPeriod

def PFTrackSelectorAlgCfg(inputFlags,algName,useCaching=True):
    PFTrackSelectorFactory=CompFactory.PFTrackSelector
    PFTrackSelector=PFTrackSelectorFactory(algName)

    result = ComponentAccumulator()

    if inputFlags.PF.useActsExtrapolation:
        #do something
        from ActsConfig.ActsGeometryConfig import ActsExtrapolationToolCfg
        ActsExtrapolationTool = result.popToolsAndMerge(ActsExtrapolationToolCfg(inputFlags,MaxSteps=10000))
        PFTrackCaloExtensionTool=CompFactory.PFTrackCaloExtensionTool
        TrackCaloExtensionTool=PFTrackCaloExtensionTool("PFTrackCaloExtensionTool_ACTS",ExtrapolationTool=ActsExtrapolationTool)
        PFTrackSelector.trackExtrapolatorTool = TrackCaloExtensionTool
    else:

        from TrackToCalo.TrackToCaloConfig import ParticleCaloExtensionToolCfg
        pcExtensionTool = result.popToolsAndMerge(ParticleCaloExtensionToolCfg(inputFlags))

        eflowTrackCaloExtensionTool=CompFactory.eflowTrackCaloExtensionTool
        TrackCaloExtensionTool=eflowTrackCaloExtensionTool(TrackCaloExtensionTool=pcExtensionTool)
        if (not useCaching):
          TrackCaloExtensionTool.PFParticleCache = ""

        PFTrackSelector.trackExtrapolatorTool = TrackCaloExtensionTool

    from InDetTrackSelectionTool.InDetTrackSelectionToolConfig import (
        PFTrackSelectionToolCfg)
    PFTrackSelector.trackSelectionTool = result.popToolsAndMerge(PFTrackSelectionToolCfg(inputFlags))

    # P->T conversion extra dependencies
    if inputFlags.Detector.GeometryITk:
        PFTrackSelector.ExtraInputs = {
            ("InDetDD::SiDetectorElementCollection", "ConditionStore+ITkPixelDetectorElementCollection"),
            ("InDetDD::SiDetectorElementCollection", "ConditionStore+ITkStripDetectorElementCollection"),
        }
    else:
        PFTrackSelector.ExtraInputs = {
            ("InDetDD::SiDetectorElementCollection", "ConditionStore+PixelDetectorElementCollection"),
            ("InDetDD::SiDetectorElementCollection", "ConditionStore+SCT_DetectorElementCollection"),
            ("InDetDD::TRT_DetElementContainer", "ConditionStore+TRT_DetElementContainer"),
        }

    result.addEventAlgo (PFTrackSelector, primary=True)

    return result

def getPFClusterSelectorTool(inputFlags,clustersin,calclustersin,algName):

    PFClusterSelectorToolFactory = CompFactory.PFClusterSelectorTool
    PFClusterSelectorTool = PFClusterSelectorToolFactory(algName)
    if clustersin is not None:
        PFClusterSelectorTool.clustersName = clustersin
    if calclustersin is not None:
        PFClusterSelectorTool.calClustersName = calclustersin

    if inputFlags.PF.useTruthCheating:
        if inputFlags.PF.useTrackClusterTruthMatching:
            PFClusterSelectorTool.CaloClusterReadDecorHandleKey_NLeadingTruthParticles = "CaloTopoClusters." + inputFlags.Calo.TopoCluster.CalibrationHitDecorationName

    if inputFlags.PF.useTopoTowers:
        PFClusterSelectorTool.clustersName="CaloTopoSignal"
        PFClusterSelectorTool.calClustersName="CaloCalTopoSignal"

    return PFClusterSelectorTool

def getPFTrackClusterMatchingTool(inputFlags,matchCut,distanceType,clusterPositionType,name):
    PFTrackClusterMatchingTool = CompFactory.PFTrackClusterMatchingTool
    MatchingTool = PFTrackClusterMatchingTool(name)
    MatchingTool.ClusterPositionType = clusterPositionType
    MatchingTool.DistanceType = distanceType
    MatchingTool.MatchCut = matchCut*matchCut
    return MatchingTool


def getPFCellLevelSubtractionTool(inputFlags,toolName):
    PFCellLevelSubtractionToolFactory = CompFactory.PFSubtractionTool
    PFCellLevelSubtractionTool = PFCellLevelSubtractionToolFactory(toolName,useNNEnergy = inputFlags.PF.useMLEOverP)

    if inputFlags.GeoModel.Run <= LHCPeriod.Run3:
        eflowCellEOverPTool_Run2_mc20_JetETMiss = CompFactory.eflowCellEOverPTool_Run2_mc20_JetETMiss
        PFCellLevelSubtractionTool.eflowCellEOverPTool = eflowCellEOverPTool_Run2_mc20_JetETMiss()
    else:
        PFCellEOverPTool = CompFactory.PFCellEOverPTool
        PFCellLevelSubtractionTool.eflowCellEOverPTool = PFCellEOverPTool("PFCellEOverPTool", referenceFileLocation = inputFlags.PF.EOverP_CellOrdering_ReferenceLocation)
        #this should always be false for any reference derived, except eflowCellEOverPTool_mc12_HLLHC.h or eflowCellEOverPTool_Run2_mc20_JetETMiss.h
        PFCellLevelSubtractionTool.useLegacyEBinIndex=False

    if(inputFlags.PF.EOverPMode):
        PFCellLevelSubtractionTool.CalcEOverP = True
        PFCellLevelSubtractionTool.nClusterMatchesToUse = -1
    else:
        PFCellLevelSubtractionTool.nClusterMatchesToUse = 1

    if(inputFlags.PF.EOverPMode):
        PFCellLevelSubtractionTool.PFTrackClusterMatchingTool = getPFTrackClusterMatchingTool(inputFlags,0.2,"EtaPhiSquareDistance","PlainEtaPhi","CalObjBldMatchingTool")
    else:
        PFCellLevelSubtractionTool.PFTrackClusterMatchingTool = getPFTrackClusterMatchingTool(inputFlags,1.64,"EtaPhiSquareSignificance","GeomCenterEtaPhi","CalObjBldMatchingTool")

    PFCellLevelSubtractionTool.PFTrackClusterMatchingTool_02 = getPFTrackClusterMatchingTool(inputFlags,0.2,"EtaPhiSquareDistance","PlainEtaPhi","MatchingTool_Pull_02")

    if inputFlags.PF.useMLEOverP:
        PFEnergyPredictorTool = CompFactory.PFEnergyPredictorTool("PFCellLevelEnergyPredcictorTool",ModelPath = inputFlags.PF.EOverP_NN_Model)
        PFCellLevelSubtractionTool.NNEnergyPredictorTool = PFEnergyPredictorTool
    PFCellLevelSubtractionTool.addCPData = inputFlags.PF.addCPData

    if inputFlags.PF.useTruthCheating:
        if inputFlags.PF.useTrackClusterTruthMatching:
            PFCellLevelSubtractionTool.CaloClusterReadDecorHandleKey_NLeadingTruthParticles = "CaloTopoClusters." + inputFlags.Calo.TopoCluster.CalibrationHitDecorationName
            PFCellLevelSubtractionTool.useTrackClusterTruthMatching=True

        if inputFlags.PF.useTruthForChargedShowerSubtraction:
            PFCellLevelSubtractionTool.useTruthForChargedShowerSubtraction = True
            PFCellLevelSubtractionTool.PFSimulateTruthShowerTool = CompFactory.PFSimulateTruthShowerTool("PFSimulateTruthShowerTool")

    return PFCellLevelSubtractionTool



def getPFUnifiedCellLevelMatchingTool(inputFlags,toolName):
    if inputFlags.PF.useTruthCheating and inputFlags.PF.useTrackClusterTruthMatching:
        PFUnifiedCellLevelMatchingToolFactory = CompFactory.PFUnifiedMatchingTruthTool
        PFUnifiedCellLevelMatchingTool = PFUnifiedCellLevelMatchingToolFactory(toolName+"_truthCheating")
        PFUnifiedCellLevelMatchingTool.CaloClusterReadDecorHandleKey_NLeadingTruthParticles = "CaloTopoClusters." + inputFlags.Calo.TopoCluster.CalibrationHitDecorationName
    else:
        PFUnifiedCellLevelMatchingToolFactory = CompFactory.PFUnifiedMatchingTool
        PFUnifiedCellLevelMatchingTool = PFUnifiedCellLevelMatchingToolFactory(toolName)

    if inputFlags.GeoModel.Run <= LHCPeriod.Run3:
        eflowCellEOverPTool_Run2_mc20_JetETMiss = CompFactory.eflowCellEOverPTool_Run2_mc20_JetETMiss
        PFUnifiedCellLevelMatchingTool.eflowCellEOverPTool = eflowCellEOverPTool_Run2_mc20_JetETMiss()
    else:
        PFCellEOverPTool = CompFactory.PFCellEOverPTool
        PFUnifiedCellLevelMatchingTool.eflowCellEOverPTool = PFCellEOverPTool("PFCellEOverPTool", referenceFileLocation = inputFlags.PF.EOverP_CellOrdering_ReferenceLocation)
        #this should always be false for any reference derived, except eflowCellEOverPTool_mc12_HLLHC.h or eflowCellEOverPTool_Run2_mc20_JetETMiss.h
        PFUnifiedCellLevelMatchingTool.useLegacyEBinIndex=False

    if(inputFlags.PF.EOverPMode):
        PFUnifiedCellLevelMatchingTool.nClusterMatchesToUse = -1
    else:
        PFUnifiedCellLevelMatchingTool.nClusterMatchesToUse = 1

    if(inputFlags.PF.EOverPMode):
        PFUnifiedCellLevelMatchingTool.PFTrackClusterMatchingTool = getPFTrackClusterMatchingTool(inputFlags,0.2,"EtaPhiSquareDistance","PlainEtaPhi","CalObjBldMatchingTool")
    else:
        PFUnifiedCellLevelMatchingTool.PFTrackClusterMatchingTool = getPFTrackClusterMatchingTool(inputFlags,1.64,"EtaPhiSquareSignificance","GeomCenterEtaPhi","CalObjBldMatchingTool")

    PFUnifiedCellLevelMatchingTool.PFTrackClusterMatchingTool_02 = getPFTrackClusterMatchingTool(inputFlags,0.2,"EtaPhiSquareDistance","PlainEtaPhi","MatchingTool_Pull_02")

    PFUnifiedCellLevelMatchingTool.addCPData = inputFlags.PF.addCPData

    return PFUnifiedCellLevelMatchingTool


def getPFUnifiedCellLevelSubtractionOnlyTool(inputFlags,toolName):
    if inputFlags.PF.useTruthCheating and inputFlags.PF.useTruthForChargedShowerSubtraction:
        PFUnifiedCellLevelSubtractionOnlyToolFactory = CompFactory.PFUnifiedSubtractionOnlyTruthTool
        PFUnifiedCellLevelSubtractionOnlyTool = PFUnifiedCellLevelSubtractionOnlyToolFactory(toolName+"_truthCheating", useNNEnergy = inputFlags.PF.useMLEOverP)
        PFUnifiedCellLevelSubtractionOnlyTool.PFSimulateTruthShowerTool = CompFactory.PFSimulateTruthShowerTool("PFSimulateTruthShowerTool")
    else:
        PFUnifiedCellLevelSubtractionOnlyToolFactory = CompFactory.PFUnifiedSubtractionOnlyTool
        PFUnifiedCellLevelSubtractionOnlyTool = PFUnifiedCellLevelSubtractionOnlyToolFactory(toolName, useNNEnergy = inputFlags.PF.useMLEOverP)

    if inputFlags.GeoModel.Run <= LHCPeriod.Run3:   
        eflowCellEOverPTool_Run2_mc20_JetETMiss = CompFactory.eflowCellEOverPTool_Run2_mc20_JetETMiss
        PFUnifiedCellLevelSubtractionOnlyTool.eflowCellEOverPTool = eflowCellEOverPTool_Run2_mc20_JetETMiss()
    else:
        PFCellEOverPTool = CompFactory.PFCellEOverPTool
        PFUnifiedCellLevelSubtractionOnlyTool.eflowCellEOverPTool = PFCellEOverPTool("PFCellEOverPTool", referenceFileLocation = inputFlags.PF.EOverP_CellOrdering_ReferenceLocation)
        #this should always be false for any reference derived, except eflowCellEOverPTool_mc12_HLLHC.h or eflowCellEOverPTool_Run2_mc20_JetETMiss.h
        PFUnifiedCellLevelSubtractionOnlyTool.useLegacyEBinIndex=False

    if(inputFlags.PF.EOverPMode):
        PFUnifiedCellLevelSubtractionOnlyTool.CalcEOverP = True

    PFUnifiedCellLevelSubtractionOnlyTool.addCPData = inputFlags.PF.addCPData

    if inputFlags.PF.useMLEOverP:
        PFEnergyPredictorTool = CompFactory.PFEnergyPredictorTool("PFCellLevelEnergyPredcictorTool",ModelPath = inputFlags.PF.EOverP_NN_Model)
        PFUnifiedCellLevelSubtractionOnlyTool.NNEnergyPredictorTool = PFEnergyPredictorTool


    return PFUnifiedCellLevelSubtractionOnlyTool




def getPFRecoverSplitShowersTool(inputFlags,toolName):
    PFRecoverSplitShowersToolFactory = CompFactory.PFSubtractionTool
    PFRecoverSplitShowersTool = PFRecoverSplitShowersToolFactory(toolName,useNNEnergy = inputFlags.PF.useMLEOverP)

    if inputFlags.GeoModel.Run <= LHCPeriod.Run3:
        eflowCellEOverPTool_Run2_mc20_JetETMiss = CompFactory.eflowCellEOverPTool_Run2_mc20_JetETMiss
        PFRecoverSplitShowersTool.eflowCellEOverPTool = eflowCellEOverPTool_Run2_mc20_JetETMiss("eflowCellEOverPTool_Run2_mc20_JetETMiss_Recover")
    else:
        PFCellEOverPTool = CompFactory.PFCellEOverPTool
        PFRecoverSplitShowersTool.eflowCellEOverPTool = PFCellEOverPTool("PFCellEOverPTool_Recover", referenceFileLocation = inputFlags.PF.EOverP_CellOrdering_ReferenceLocation)
        #this should always be false for any reference derived, except eflowCellEOverPTool_mc12_HLLHC.h or eflowCellEOverPTool_Run2_mc20_JetETMiss.h
        PFRecoverSplitShowersTool.useLegacyEBinIndex=False

    PFRecoverSplitShowersTool.RecoverSplitShowers = True

    if inputFlags.PF.useMLEOverP:
        PFEnergyPredictorTool = CompFactory.PFEnergyPredictorTool("PFRecoverSplitShowersEnergyPredcictorTool",ModelPath = inputFlags.PF.EOverP_NN_Model)
        PFRecoverSplitShowersTool.NNEnergyPredictorTool = PFEnergyPredictorTool

    return PFRecoverSplitShowersTool


def getPFUnifiedRecoverSplitShowersMatchingTool(inputFlags,toolName):
    # RecoverSplitShowers is gated off when useTruthCheating is set (see getOfflinePFAlgorithm),
    # so the truth-matching variant of this factory is unreachable by construction.
    PFUnifiedRecoverSplitShowersMatchingToolFactory = CompFactory.PFUnifiedMatchingTool
    PFUnifiedRecoverSplitShowersMatchingTool = PFUnifiedRecoverSplitShowersMatchingToolFactory(toolName)

    if inputFlags.GeoModel.Run <= LHCPeriod.Run3:
        eflowCellEOverPTool_Run2_mc20_JetETMiss = CompFactory.eflowCellEOverPTool_Run2_mc20_JetETMiss
        PFUnifiedRecoverSplitShowersMatchingTool.eflowCellEOverPTool = eflowCellEOverPTool_Run2_mc20_JetETMiss("eflowCellEOverPTool_Run2_mc20_JetETMiss_Recover")
    else:
        PFCellEOverPTool = CompFactory.PFCellEOverPTool
        PFUnifiedRecoverSplitShowersMatchingTool.eflowCellEOverPTool = PFCellEOverPTool("PFCellEOverPTool_Recover", referenceFileLocation = inputFlags.PF.EOverP_CellOrdering_ReferenceLocation)
        #this should always be false for any reference derived, except eflowCellEOverPTool_mc12_HLLHC.h or eflowCellEOverPTool_Run2_mc20_JetETMiss.h
        PFUnifiedRecoverSplitShowersMatchingTool.useLegacyEBinIndex=False

    PFUnifiedRecoverSplitShowersMatchingTool.RecoverSplitShowers = True

    return PFUnifiedRecoverSplitShowersMatchingTool



def getPFUnifiedRecoverSplitShowersSubtractionOnlyTool(inputFlags,toolName):
    # RecoverSplitShowers is gated off when useTruthCheating is set (see getOfflinePFAlgorithm),
    # so the truth-cheating variant of this factory is unreachable by construction.
    PFUnifiedRecoverSplitShowersSubtractionOnlyToolFactory = CompFactory.PFUnifiedSubtractionOnlyTool
    PFUnifiedRecoverSplitShowersSubtractionOnlyTool = PFUnifiedRecoverSplitShowersSubtractionOnlyToolFactory(toolName, useNNEnergy = inputFlags.PF.useMLEOverP)

    if inputFlags.GeoModel.Run <= LHCPeriod.Run3:
        eflowCellEOverPTool_Run2_mc20_JetETMiss = CompFactory.eflowCellEOverPTool_Run2_mc20_JetETMiss
        PFUnifiedRecoverSplitShowersSubtractionOnlyTool.eflowCellEOverPTool = eflowCellEOverPTool_Run2_mc20_JetETMiss("eflowCellEOverPTool_Run2_mc20_JetETMiss_Recover")
    else:
        PFCellEOverPTool = CompFactory.PFCellEOverPTool
        PFUnifiedRecoverSplitShowersSubtractionOnlyTool.eflowCellEOverPTool = PFCellEOverPTool("PFCellEOverPTool_Recover", referenceFileLocation = inputFlags.PF.EOverP_CellOrdering_ReferenceLocation)
        #this should always be false for any reference derived, except eflowCellEOverPTool_mc12_HLLHC.h or eflowCellEOverPTool_Run2_mc20_JetETMiss.h
        PFUnifiedRecoverSplitShowersSubtractionOnlyTool.useLegacyEBinIndex=False

    PFUnifiedRecoverSplitShowersSubtractionOnlyTool.RecoverSplitShowers = True

    if inputFlags.PF.useMLEOverP:
        PFEnergyPredictorTool = CompFactory.PFEnergyPredictorTool("PFRecoverSplitShowersEnergyPredcictorTool",ModelPath = inputFlags.PF.EOverP_NN_Model)
        PFUnifiedRecoverSplitShowersSubtractionOnlyTool.NNEnergyPredictorTool = PFEnergyPredictorTool

    return PFUnifiedRecoverSplitShowersSubtractionOnlyTool



def getPFMomentCalculatorTool(inputFlags, momentsToCalculateList, use_unified = False):
    result=ComponentAccumulator()
    if use_unified:
        PFMomentCalculatorToolFactory = CompFactory.PFUnifiedMomentCalculatorTool
    else:
        PFMomentCalculatorToolFactory = CompFactory.PFMomentCalculatorTool

    PFMomentCalculatorTool = PFMomentCalculatorToolFactory("PFMomentCalculatorTool")

    from CaloRec.CaloTopoClusterConfig import getTopoMoments
    PFClusterMomentsMaker = result.popToolsAndMerge(getTopoMoments(inputFlags))
    if (len(momentsToCalculateList) > 0):
        PFClusterMomentsMaker.MomentsNames = momentsToCalculateList
    PFMomentCalculatorTool.CaloClusterMomentsMaker = PFClusterMomentsMaker

    PFClusterCollectionTool = CompFactory.PFClusterCollectionTool
    PFMomentCalculatorTool.PFClusterCollectionTool = PFClusterCollectionTool("PFClusterCollectionTool")

    if(inputFlags.PF.useCalibHitTruthClusterMoments):
        PFMomentCalculatorTool.UseCalibHitTruth=True
        from CaloRec.CaloTopoClusterConfig import getTopoCalibMoments
        PFMomentCalculatorTool.CaloCalibClusterMomentsMaker2 = getTopoCalibMoments(inputFlags)

    result.setPrivateTools(PFMomentCalculatorTool)
    return result

def getPFLCCalibTool(inputFlags,use_unified = False):

    if use_unified:
        PFLCCalibTool = CompFactory.PFUnifiedLCCalibTool
    else:
        PFLCCalibTool = CompFactory.PFLCCalibTool

    PFLCCalibTool = PFLCCalibTool("PFLCCalibTool")

    PFClusterCollectionTool = CompFactory.PFClusterCollectionTool
    PFLCCalibTool.eflowRecClusterCollectionTool = PFClusterCollectionTool("PFClusterCollectionTool_LCCalib")
    PFLCCalibTool.UseLocalWeight = False

    from CaloRec.CaloTopoClusterConfig import getTopoClusterLocalCalibTools
    lcCalibToolList = getTopoClusterLocalCalibTools(inputFlags)

    PFLCCalibTool.CaloClusterLocalCalib=lcCalibToolList[0]
    PFLCCalibTool.CaloClusterLocalCalibOOCC=lcCalibToolList[1]
    PFLCCalibTool.CaloClusterLocalCalibOOCCPi0=lcCalibToolList[2]
    PFLCCalibTool.CaloClusterLocalCalibDM=lcCalibToolList[3]

    return PFLCCalibTool

def getChargedFlowElementCreatorAlgorithm(inputFlags,chargedFlowElementOutputName,nameSuffix="",eflowCaloObjectContainerName="eflowCaloObjects"):
    FlowElementChargedCreatorAlgorithmFactory = CompFactory.PFChargedFlowElementCreatorAlgorithm
    FlowElementChargedCreatorAlgorithm = FlowElementChargedCreatorAlgorithmFactory("PFChargedFlowElementCreatorAlgorithm"+nameSuffix)
    FlowElementChargedCreatorAlgorithm.eflowCaloObjectContainerName = eflowCaloObjectContainerName
    if chargedFlowElementOutputName:
        FlowElementChargedCreatorAlgorithm.FlowElementOutputName=chargedFlowElementOutputName
    if(inputFlags.PF.EOverPMode):
        FlowElementChargedCreatorAlgorithm.FlowElementOutputName="EOverPChargedParticleFlowObjects"
        FlowElementChargedCreatorAlgorithm.EOverPMode = True
    if inputFlags.PF.addCPData:
        FlowElementChargedCreatorAlgorithm.addCPData = True

    return FlowElementChargedCreatorAlgorithm

def getNeutralFlowElementCreatorAlgorithm(inputFlags,neutralFlowElementOutputName,nameSuffix="",eflowCaloObjectContainerName="eflowCaloObjects"):
    FlowElementNeutralCreatorAlgorithmFactory = CompFactory.PFNeutralFlowElementCreatorAlgorithm
    FlowElementNeutralCreatorAlgorithm = FlowElementNeutralCreatorAlgorithmFactory("PFNeutralFlowElementCreatorAlgorithm"+nameSuffix)
    FlowElementNeutralCreatorAlgorithm.eflowCaloObjectContainerName = eflowCaloObjectContainerName
    if neutralFlowElementOutputName:
        FlowElementNeutralCreatorAlgorithm.FlowElementOutputName=neutralFlowElementOutputName
    if(inputFlags.PF.EOverPMode):
        FlowElementNeutralCreatorAlgorithm.FlowElementOutputName="EOverPNeutralParticleFlowObjects"
    if(inputFlags.PF.useCalibHitTruthClusterMoments and inputFlags.PF.addClusterMoments):
        FlowElementNeutralCreatorAlgorithm.UseCalibHitTruth=True
    if inputFlags.PF.addCPData:
        FlowElementNeutralCreatorAlgorithm.addCPData = True

    return FlowElementNeutralCreatorAlgorithm

def getLCNeutralFlowElementCreatorAlgorithm(inputFlags,neutralFlowElementOutputName):
    LCFlowElementNeutralCreatorAlgorithmFactory = CompFactory.PFLCNeutralFlowElementCreatorAlgorithm
    LCFlowElementNeutralCreatorAlgorithm = LCFlowElementNeutralCreatorAlgorithmFactory("PFLCNeutralFlowElementCreatorAlgorithm")
    if neutralFlowElementOutputName:
      LCFlowElementNeutralCreatorAlgorithm.FELCOutputName=neutralFlowElementOutputName
    if(inputFlags.PF.EOverPMode):
      LCFlowElementNeutralCreatorAlgorithm.FEInputContainerName="EOverPNeutralParticleFlowObjects"
      LCFlowElementNeutralCreatorAlgorithm.FELCOutputName="EOverPLCNeutralParticleFlowObjects"

    return LCFlowElementNeutralCreatorAlgorithm

# Factory function to create the ML-based neutral flow element creator algorithm
def getPFOClusterMLCorrectionAlgorithmCfg(inputFlags, inputNameBase = "JetETMiss", outputNameBase = "JetETMissClusterMLCorrected"):
    alg = CompFactory.PFOClusterMLCorrectionAlgorithm("PFOClusterMLCorrectionAlgorithm")
    suffix = "ParticleFlowObjects"
    alg.NeutralPFlowInputContainer = inputNameBase + "Neutral" + suffix
    alg.ChargedPFlowInputContainer = inputNameBase + "Charged" + suffix
    alg.NeutralPFlowOutputContainer = outputNameBase + "Neutral"  + suffix
    alg.ChargedPFlowOutputContainer = outputNameBase + "Charged" + suffix

    return alg

def getNeutralPFOClusterMLCorrectionToolCfg(inputFlags, toolName="NeutralPFOClusterMLCorrectionTool", **kwargs):
    if toolName == "NeutralPFOClusterMLCorrectionTool":
        tool = CompFactory.NeutralPFOClusterMLCorrectionTool(toolName, **kwargs)
    else:
        raise ValueError(f"Unknown tool name: {toolName}")

    ca = ComponentAccumulator()
    ca.setPrivateTools(tool)
    return ca




def getEGamFlowElementAssocAlgorithm(inputFlags, algName="", **kwargs):

    kwargs.setdefault("neutral_FE_cont_name", "")
    kwargs.setdefault("charged_FE_cont_name", "")
    kwargs.setdefault("doTCC", False)
    kwargs.setdefault("useGlobal", False)

    PFEGamFlowElementLinkerAlgorithmFactory=CompFactory.PFEGamFlowElementAssoc
    if not algName:
        algName = "PFEGamFlowElementAssoc"
    PFEGamFlowElementLinkerAlgorithm=PFEGamFlowElementLinkerAlgorithmFactory(algName)

    #set an an alternate name if needed
    #this uses some gaudi core magic, namely that you can change the name of the handle as it is a callable attribute, despite the attribute not being explicitly listed in the header
    #for a key of type SG::WriteDecorHandle<xAOD::SomeCont>someKey{this,"SpecificContainerName","myContainerName","other-labels"}
    #setting algorithm.SpecificContainerName="myNewContainerName" changes parameter "myContainerName"
    #(also applies to ReadHandles)
    if kwargs['neutral_FE_cont_name']:
        PFEGamFlowElementLinkerAlgorithm.JetEtMissNeutralFlowElementContainer = kwargs['neutral_FE_cont_name']

    if kwargs['charged_FE_cont_name']:
        PFEGamFlowElementLinkerAlgorithm.JetEtMissChargedFlowElementContainer = kwargs['charged_FE_cont_name']

    if kwargs['doTCC']:
        # ReadHandles to change
        PFEGamFlowElementLinkerAlgorithm.JetEtMissNeutralFlowElementContainer="TrackCaloClustersNeutral"
        PFEGamFlowElementLinkerAlgorithm.JetEtMissChargedFlowElementContainer="TrackCaloClustersCharged"

        #Now to change the writeHandles
        # first the Electron -> FE links
        EL_NFE_Link=str(PFEGamFlowElementLinkerAlgorithm.ElectronNeutralFEDecorKey)
        PFEGamFlowElementLinkerAlgorithm.ElectronNeutralFEDecorKey=EL_NFE_Link.replace("FELinks","TCCLinks")
        EL_CFE_Link=str(PFEGamFlowElementLinkerAlgorithm.ElectronChargedFEDecorKey)
        PFEGamFlowElementLinkerAlgorithm.ElectronChargedFEDecorKey=EL_CFE_Link.replace("FELinks","TCCLinks")
        #then the converse case (FE -> Electron)

        PFEGamFlowElementLinkerAlgorithm.ChargedFEElectronDecorKey="TCC_ElectronLinks"
        PFEGamFlowElementLinkerAlgorithm.NeutralFEElectronDecorKey="TCC_ElectronLinks"


        # first the Photon -> FE links
        PH_NFE_Link=str(PFEGamFlowElementLinkerAlgorithm.PhotonNeutralFEDecorKey)
        PFEGamFlowElementLinkerAlgorithm.PhotonNeutralFEDecorKey=PH_NFE_Link.replace("FELinks","TCCLinks")
        PH_CFE_Link=str(PFEGamFlowElementLinkerAlgorithm.PhotonChargedFEDecorKey)
        PFEGamFlowElementLinkerAlgorithm.PhotonChargedFEDecorKey=PH_CFE_Link.replace("FELinks","TCCLinks")
        #then the converse case (FE -> Photons)

        PFEGamFlowElementLinkerAlgorithm.ChargedFEPhotonDecorKey="TCC_PhotonLinks"
        PFEGamFlowElementLinkerAlgorithm.NeutralFEPhotonDecorKey="TCC_PhotonLinks"

    if kwargs['useGlobal']:
        # ReadHandles to change
        PFEGamFlowElementLinkerAlgorithm.JetEtMissNeutralFlowElementContainer="GlobalNeutralParticleFlowObjects"
        PFEGamFlowElementLinkerAlgorithm.JetEtMissChargedFlowElementContainer="GlobalChargedParticleFlowObjects"

        #Now to change the writeHandles
        # first the Electron -> FE links
        EL_NFE_Link=str(PFEGamFlowElementLinkerAlgorithm.ElectronNeutralFEDecorKey)
        PFEGamFlowElementLinkerAlgorithm.ElectronNeutralFEDecorKey=EL_NFE_Link.replace("FELinks","GlobalFELinks")
        EL_CFE_Link=str(PFEGamFlowElementLinkerAlgorithm.ElectronChargedFEDecorKey)
        PFEGamFlowElementLinkerAlgorithm.ElectronChargedFEDecorKey=EL_CFE_Link.replace("FELinks","GlobalFELinks")
        #then the converse case (FE -> Electron)

        PFEGamFlowElementLinkerAlgorithm.ChargedFEElectronDecorKey="GlobalFE_ElectronLinks"
        PFEGamFlowElementLinkerAlgorithm.NeutralFEElectronDecorKey="GlobalFE_ElectronLinks"


        # first the Photon -> FE links
        PH_NFE_Link=str(PFEGamFlowElementLinkerAlgorithm.PhotonNeutralFEDecorKey)
        PFEGamFlowElementLinkerAlgorithm.PhotonNeutralFEDecorKey=PH_NFE_Link.replace("FELinks","GlobalFELinks")
        PH_CFE_Link=str(PFEGamFlowElementLinkerAlgorithm.PhotonChargedFEDecorKey)
        PFEGamFlowElementLinkerAlgorithm.PhotonChargedFEDecorKey=PH_CFE_Link.replace("FELinks","GlobalFELinks")
        #then the converse case (FE -> Photons)

        PFEGamFlowElementLinkerAlgorithm.ChargedFEPhotonDecorKey="TCC_PhotonLinks"
        PFEGamFlowElementLinkerAlgorithm.NeutralFEPhotonDecorKey="TCC_PhotonLinks"



    return PFEGamFlowElementLinkerAlgorithm

def getMuonFlowElementAssocAlgorithm(inputFlags, algName="", **kwargs):

    kwargs.setdefault("neutral_FE_cont_name", "")
    kwargs.setdefault("charged_FE_cont_name", "")
    kwargs.setdefault("LinkNeutralFEClusters", True)
    kwargs.setdefault("doTCC", False)
    kwargs.setdefault("useGlobal", False)

    useMuonTopoClusters = False
    from AthenaConfiguration.Enums import ProductionStep
    # set 'useMuonTopoClusters=True' if running on AOD, as do not have calorimeter cells for CaloCalTopoCluster
    # Assumes that in production workflows this only happens in "Derivation" or if DQ environment is AOD
    if inputFlags.Common.ProductionStep in [ProductionStep.Derivation] or inputFlags.DQ.Environment == "AOD":
        useMuonTopoClusters = True


    PFMuonFlowElementLinkerAlgorithmFactory=CompFactory.PFMuonFlowElementAssoc
    if not algName:
        algName="PFMuonFlowElementAssoc"
    PFMuonFlowElementLinkerAlgorithm=PFMuonFlowElementLinkerAlgorithmFactory(algName)

    #set an an alternate name if needed
    #this uses some gaudi core magic, namely that you can change the name of the handle as it is a callable attribute, despite the attribute not being explicitly listed in the header as such
    #for a key of type SG::WriteDecorHandle<xAOD::SomeCont>someKey{this,"SpecificContainerName","myContainerName","other-labels"}
    #setting algorithm.SpecificContainerName="myNewContainerName" changes parameter "myContainerName" to "myNewContainerName"
    if kwargs['neutral_FE_cont_name']:
        #update the readhandle
        PFMuonFlowElementLinkerAlgorithm.JetEtMissNeutralFlowElementContainer = kwargs['neutral_FE_cont_name']
        #update the write handle for the link

    if kwargs['charged_FE_cont_name']:
        PFMuonFlowElementLinkerAlgorithm.JetEtMissChargedFlowElementContainer = kwargs['charged_FE_cont_name']

    PFMuonFlowElementLinkerAlgorithm.LinkNeutralFEClusters = kwargs['LinkNeutralFEClusters']
    PFMuonFlowElementLinkerAlgorithm.useMuonTopoClusters = useMuonTopoClusters

    #prototype on AOD with the linkers already defined - so need to rename the output links to something besides their default name.

    #Track Calo cluster (TCC) specific configuration. Input is differently named FE container, and in the AOD step specifically
    if kwargs['doTCC']:
        #input containers are TrackCaloClustersCharged and TrackCaloClustersNeutral, so rename them
        #service_key="StoreGateSvc+"
        service_key=""
        PFMuonFlowElementLinkerAlgorithm.MuonContainer=service_key+"Muons"
        PFMuonFlowElementLinkerAlgorithm.JetEtMissChargedFlowElementContainer=service_key+"TrackCaloClustersCharged"
        PFMuonFlowElementLinkerAlgorithm.JetEtMissNeutralFlowElementContainer=service_key+"TrackCaloClustersNeutral"

        #Output
        #rename the FE_MuonLinks as TCC_MuonLinks
        #rename output containers
        PFMuonFlowElementLinkerAlgorithm.MuonContainer_chargedFELinks="chargedTCCLinks"
        PFMuonFlowElementLinkerAlgorithm.MuonContainer_neutralFELinks="neutralTCCLinks"
        PFMuonFlowElementLinkerAlgorithm.JetETMissNeutralFlowElementContainer_FE_MuonLinks="TCC_MuonLinks"
        PFMuonFlowElementLinkerAlgorithm.JetETMissChargedFlowElements_FE_MuonLinks="TCC_MuonLinks"
        PFMuonFlowElementLinkerAlgorithm.FlowElementContainer_ChargedFE_ennergy_matched_muon="TCC_efrac_matched_muon"
        # several variables relating to Neutral Flow Elements/TCCs to Muons for debug. perhaps at some point these should be removed by default
        PFMuonFlowElementLinkerAlgorithm.FlowElementContainer_nMatchedMuons="TCC_nMatchedMuons"
        PFMuonFlowElementLinkerAlgorithm.FlowElementContainer_FE_efrac_matched_muon="TCC_efrac_matched_muon"

        PFMuonFlowElementLinkerAlgorithm.MuonContainer_muon_efrac_matched_FE="muon_efrac_matched_TCC"
        # this is because the algorithm adds this debug container which we don't need
        PFMuonFlowElementLinkerAlgorithm.MuonContainer_ClusterInfo_deltaR="deltaR_muon_clus_TCCalg"

    if kwargs['useGlobal']:
        PFMuonFlowElementLinkerAlgorithm.JetEtMissChargedFlowElementContainer="GlobalChargedParticleFlowObjects"
        PFMuonFlowElementLinkerAlgorithm.JetEtMissNeutralFlowElementContainer="GlobalNeutralParticleFlowObjects"

        PFMuonFlowElementLinkerAlgorithm.MuonContainer_chargedFELinks="chargedGlobalFELinks"
        PFMuonFlowElementLinkerAlgorithm.MuonContainer_neutralFELinks="neutralGlobalFELinks"

        PFMuonFlowElementLinkerAlgorithm.JetETMissNeutralFlowElementContainer_FE_MuonLinks="GlobalFE_MuonLinks"
        PFMuonFlowElementLinkerAlgorithm.JetETMissChargedFlowElements_FE_MuonLinks="GlobalFE_MuonLinks"
        PFMuonFlowElementLinkerAlgorithm.FlowElementContainer_ChargedFE_ennergy_matched_muon="GlobalFE_efrac_matched_muon"

        PFMuonFlowElementLinkerAlgorithm.FlowElementContainer_nMatchedMuons="GlobalFE_nMatchedMuons"
        PFMuonFlowElementLinkerAlgorithm.FlowElementContainer_FE_efrac_matched_muon="GlobalFE_efrac_matched_muon"

        PFMuonFlowElementLinkerAlgorithm.MuonContainer_muon_efrac_matched_FE="muon_efrac_matched_GlobalFE"
        # this is because the algorithm adds this debug container which we don't need
        PFMuonFlowElementLinkerAlgorithm.MuonContainer_ClusterInfo_deltaR="deltaR_muon_clus_GlobalFEalg"

    if kwargs['LinkNeutralFEClusters'] and not useMuonTopoClusters:
       # We dereference links to cells, so make sure we have the
       # dependency.
       PFMuonFlowElementLinkerAlgorithm.ExtraInputs.add(('CaloCellContainer', inputFlags.Egamma.Keys.Input.CaloCells))

    if kwargs['LinkNeutralFEClusters']:
        if kwargs['doTCC']:
            # since the cells are deleted on AOD, if you try to run the link between NFE and Muon on AOD, it will crash. Terminate to catch this.
            # This is a known bug to rectify soon
            from AthenaCommon.Logging import logging
            msg=logging.getLogger("PFCfg.py::getMuonFlowElementAssocAlgorithm")
            msg.error("Neutral FE from AOD configured to be linked with Muon. This link will fail due to missing CaloCells in the AOD")
            msg.info("Terminating job")
            exit(0)


    return PFMuonFlowElementLinkerAlgorithm

def getTauFlowElementAssocAlgorithm(inputFlags, algName="", **kwargs):

    kwargs.setdefault("neutral_FE_cont_name", "")
    kwargs.setdefault("charged_FE_cont_name", "")
    kwargs.setdefault("doTCC", False)
    kwargs.setdefault("useGlobal", False)

    PFTauFlowElementLinkerAlgorithmFactory=CompFactory.PFTauFlowElementAssoc
    if not algName:
        algName = "PFTauFlowElementAssoc"

    PFTauFlowElementLinkerAlgorithm=PFTauFlowElementLinkerAlgorithmFactory(algName)

    #set an an alternate name if needed
    #this uses some gaudi core magic, namely that you can change the name of the handle as it is a callable attribute, despite the attribute not being explicitly listed in the header
    #for a key of type SG::WriteDecorHandle<xAOD::SomeCont>someKey{this,"SpecificContainerName","myContainerName","other-labels"}
    #setting algorithm.SpecificContainerName="myNewContainerName" changes parameter "myContainerName"
    #(also applies to ReadHandles)
    if kwargs['neutral_FE_cont_name']:
        PFTauFlowElementLinkerAlgorithm.JetETMissNeutralFlowElementContainer = kwargs['neutral_FE_cont_name']

    if kwargs['charged_FE_cont_name']:
        PFTauFlowElementLinkerAlgorithm.JetETMissChargedFlowElementContainer = kwargs['charged_FE_cont_name']

    if kwargs['doTCC']:
         PFTauFlowElementLinkerAlgorithm.JetETMissNeutralFlowElementContainer="TrackCaloClustersNeutral"
         PFTauFlowElementLinkerAlgorithm.JetETMissChargedFlowElementContainer="TrackCaloClustersCharged"

         PFTauFlowElementLinkerAlgorithm.TauNeutralFEDecorKey="neutralTCCLinks"
         PFTauFlowElementLinkerAlgorithm.TauChargedFEDecorKey="chargedTCCLinks"

         PFTauFlowElementLinkerAlgorithm.NeutralFETauDecorKey="TCC_TauLinks"
         PFTauFlowElementLinkerAlgorithm.ChargedFETauDecorKey="TCC_TauLinks"

    #This allows to set the links on the global particle flow containers created by JetPFlowSelectionAlg in JetRecTools
    if kwargs['useGlobal']:
        PFTauFlowElementLinkerAlgorithm.JetETMissNeutralFlowElementContainer="GlobalNeutralParticleFlowObjects"
        PFTauFlowElementLinkerAlgorithm.JetETMissChargedFlowElementContainer="GlobalChargedParticleFlowObjects"

        PFTauFlowElementLinkerAlgorithm.TauNeutralFEDecorKey="neutralGlobalFELinks"
        PFTauFlowElementLinkerAlgorithm.TauChargedFEDecorKey="chargedGlobalFELinks"

        PFTauFlowElementLinkerAlgorithm.NeutralFETauDecorKey="GlobalFE_TauLinks"
        PFTauFlowElementLinkerAlgorithm.ChargedFETauDecorKey="GlobalFE_TauLinks"

    return PFTauFlowElementLinkerAlgorithm

def getOfflinePFAlgorithm(inputFlags, **kwargs):
    result=ComponentAccumulator()

    PFAlgorithm=CompFactory.PFAlgorithm
    PFAlgorithm = PFAlgorithm("PFAlgorithm")

    if inputFlags.HeavyIon.Egamma.doSubtractedClusters:
        PFAlgorithm.PFClusterSelectorTool = getPFClusterSelectorTool(inputFlags,inputFlags.HeavyIon.Egamma.UncalibCaloTopoCluster,inputFlags.HeavyIon.Egamma.CaloTopoCluster,"PFClusterSelectorTool")
    else:
        topoClustersName="CaloTopoClusters"
        PFAlgorithm.PFClusterSelectorTool = getPFClusterSelectorTool(inputFlags,topoClustersName,"CaloCalTopoClusters","PFClusterSelectorTool")

    if inputFlags.PF.useUnified:
        PFAlgorithm.useUnified = True
        PFAlgorithm.UnifiedBaseTools  = [getPFUnifiedCellLevelMatchingTool(inputFlags, "PFUnifiedCellLevelMatchingTool")]
        PFAlgorithm.UnifiedBaseTools += [getPFUnifiedCellLevelSubtractionOnlyTool(inputFlags, "PFUnifiedCellLevelSubtractionOnlyTool")]
        if not inputFlags.PF.EOverPMode and not inputFlags.PF.useTruthCheating:
            PFAlgorithm.UnifiedBaseTools += [getPFUnifiedRecoverSplitShowersMatchingTool(inputFlags, "PFUnifiedRecoverSplitShowersMatchingTool")]
            PFAlgorithm.UnifiedBaseTools += [getPFUnifiedRecoverSplitShowersSubtractionOnlyTool(inputFlags, "PFUnifiedRecoverSplitShowersSubtractionOnlyTool")]

        PFUnifiedMomentCalculatorTools=result.popToolsAndMerge(getPFMomentCalculatorTool(inputFlags,[], use_unified = True))
        PFAlgorithm.UnifiedBaseTools += [PFUnifiedMomentCalculatorTools]
        PFAlgorithm.UnifiedBaseTools += [getPFLCCalibTool(inputFlags, use_unified = True)]
        if inputFlags.PF.EOverPMode:
            PFAlgorithm.UnifiedBaseTools += [CompFactory.PFUnifiedRadialEnergyCalculatorTool()]

    else:
        PFAlgorithm.SubtractionToolList = [getPFCellLevelSubtractionTool(inputFlags,"PFCellLevelSubtractionTool")]

        if not inputFlags.PF.EOverPMode and not inputFlags.PF.useTruthCheating:
            PFAlgorithm.SubtractionToolList += [getPFRecoverSplitShowersTool(inputFlags,"PFRecoverSplitShowersTool")]

        PFMomentCalculatorTools=result.popToolsAndMerge(getPFMomentCalculatorTool(inputFlags,[]))
        PFAlgorithm.BaseToolList = [PFMomentCalculatorTools]
        PFAlgorithm.BaseToolList += [getPFLCCalibTool(inputFlags)]
        if inputFlags.PF.EOverPMode:
            PFAlgorithm.BaseToolList += [CompFactory.PFRadialEnergyCalculatorTool()]

    result.addEventAlgo(PFAlgorithm)
    return result



def PFGlobalFlowElementLinkingCfg(inputFlags, **kwargs):
    result=ComponentAccumulator()

    kwargs.setdefault("useGlobal", True)

    result.addEventAlgo(getTauFlowElementAssocAlgorithm(inputFlags, algName="PFTauGlobalFlowElementAssoc", **kwargs))
    result.addEventAlgo(getMuonFlowElementAssocAlgorithm(inputFlags, algName="PFMuonGlobalFlowElementAssoc", **kwargs))
    result.addEventAlgo(getEGamFlowElementAssocAlgorithm(inputFlags, algName="PFEGamGlobalFlowElementAssoc", **kwargs))
    return result
