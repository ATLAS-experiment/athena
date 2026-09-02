# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# ==============================================================================
# Provides configs for the tools used for e-gamma decorations used in DAOD
# building
# ==============================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


# PhotonsDirectionAlg
def PhotonsDirectionAlgCfg(flags, name, **kwargs):
    """Configure the PhotonsDirectionAlg"""
    acc = ComponentAccumulator()
    acc.addEventAlgo(CompFactory.DerivationFramework.PhotonsDirectionAlg(name, **kwargs))
    return acc


# E-gamma selection tool wrapper
def EGSelectionToolWrapperCfg(flags, name, **kwargs):
    """Configure the E-gamma selection tool wrapper"""
    acc = ComponentAccumulator()
    sgName = kwargs.pop("StoreGateEntryName", "")
    if not sgName:
        raise AttributeError("StoreGateEntryName not set")
    kwargs.setdefault("decoratorPass", sgName)
    kwargs.setdefault("decoratorIsEM", sgName + "IsEMValue")
    acc.setPrivateTools(CompFactory.DerivationFramework.EGSelectionToolWrapper(name, **kwargs))
    return acc


# Electron likelihood tool wrapper
def EGElectronLikelihoodToolWrapperCfg(flags, name, **kwargs):
    """Configure the electron likelihood tool wrapper"""
    acc = ComponentAccumulator()
    sgName = kwargs.pop("StoreGateEntryName", "")
    containerName = kwargs.setdefault("ContainerName", "")
    if not sgName:
        raise AttributeError("StoreGateEntryName not set")
    if not containerName:
        raise AttributeError("ContainerName empty string")
    storeTResult = kwargs.setdefault("StoreTResult", False)
    storeMultipleOutputs = kwargs.setdefault("StoreMultipleOutputs", False)
    sgMultipleNames = kwargs.pop("StoreGateEntryMultipleNames", [])
    # Write decoration handle keys
    kwargs.setdefault("decoratorPass", sgName)
    kwargs.setdefault("decoratorIsEM", sgName + "IsEMValue")
    kwargs.setdefault("decoratorResult", sgName + "Result" if storeTResult else "")
    kwargs.setdefault("decoratorMultipleOutputs", sgMultipleNames if storeMultipleOutputs else [])
    acc.setPrivateTools(CompFactory.DerivationFramework.EGElectronLikelihoodToolWrapper(name, **kwargs))
    return acc

# Photon BDT decrotation tool
def EGPhotonBDTToolDecoratorCfg(flags, name, **kwargs):
    """Configure the E-gamma selection tool decorator"""
    acc = ComponentAccumulator()
    sgName = kwargs.pop("StoreGateEntryName", "")
    if not sgName:
        raise AttributeError("StoreGateEntryName not set")
    kwargs.setdefault("decoratorScore", sgName + "Score")
    acc.setPrivateTools(CompFactory.DerivationFramework.EGPhotonBDTToolDecorator(name, **kwargs))
    return acc

# Photon BDT selection tool wrapper
def EGPhotonBDTToolWrapperCfg(flags, name, **kwargs):
    """Configure the E-gamma selection tool wrapper"""
    acc = ComponentAccumulator()
    sgName = kwargs.pop("StoreGateEntryName", "")
    if not sgName:
        raise AttributeError("StoreGateEntryName not set")
    wpName = kwargs.pop("WorkingPointName", "")
    if not wpName:
        raise AttributeError("WorkingPointName not set")
    kwargs.setdefault("decoratorPass", sgName+wpName)
    kwargs.setdefault("decoratorIsEM", sgName+wpName + "IsEMValue")
    acc.setPrivateTools(CompFactory.DerivationFramework.EGPhotonBDTToolWrapper(name, **kwargs))
    return acc

# Photon cleaning tool wrapper
def EGPhotonCleaningWrapperCfg(flags, name, **kwargs):
    """Configure the photon cleaning tool wrapper"""
    acc = ComponentAccumulator()
    sgName = kwargs.pop("StoreGateEntryName", "DFCommonPhotonsCleaning")
    # Write decoration handle keys
    kwargs.setdefault("decoratorPass", sgName)
    kwargs.setdefault("decoratorPassDelayed", sgName + "NoTime")
    acc.setPrivateTools(CompFactory.DerivationFramework.EGPhotonCleaningWrapper(name, **kwargs))
    return acc


# Electron ambiguity tool
def EGElectronAmbiguityToolCfg(flags, name, **kwargs):
    """Configure the electron ambiguity tool"""
    acc = ComponentAccumulator()
    EGElectronAmbiguityTool = CompFactory.DerivationFramework.EGElectronAmbiguityTool
    acc.setPrivateTools(EGElectronAmbiguityTool(name, **kwargs))
    return acc


# Background electron classification tool
def BkgElectronClassificationCfg(flags, name, **kwargs):
    """Configure the background electron classification tool"""
    acc = ComponentAccumulator()
    from MCTruthClassifier.MCTruthClassifierConfig import DFCommonMCTruthClassifierCfg
    kwargs.setdefault("MCTruthClassifierTool", acc.popToolsAndMerge(
        DFCommonMCTruthClassifierCfg(flags)))
    acc.addEventAlgo(CompFactory.DerivationFramework.BkgElectronClassification(name, **kwargs))
    return acc


# Standard + LRT electron collection merger
def ElectronMergerCfg(flags, name, **kwargs):  # TODO Remove as, no clients??
    """Configure the track particle merger tool"""
    acc = ComponentAccumulator()
    ElectronMerger = CompFactory.DerivationFramework.ElectronMergerTool
    acc.setPrivateTools(ElectronMerger(name, **kwargs))
    return acc


def PhotonVertexSelectionWrapperCfg(
        flags, name="PhotonVertexSelectionWrapper", **kwargs):
    acc = ComponentAccumulator()
    prefix = kwargs.pop("DecorationPrefix", "")
    if prefix: prefix += "_"
    kwargs.setdefault("pt", prefix + "pt")
    kwargs.setdefault("eta", prefix + "eta")
    kwargs.setdefault("phi", prefix + "phi")
    kwargs.setdefault("sumPt", prefix + "sumPt")
    kwargs.setdefault("sumPt2", prefix + "sumPt2")

    if "PhotonPointingTool" not in kwargs:
        from PhotonVertexSelection.PhotonVertexSelectionConfig import (
            PhotonPointingToolCfg)
        kwargs.setdefault("PhotonPointingTool", acc.popToolsAndMerge(
            PhotonPointingToolCfg(flags)))

    acc.setPrivateTools(
        CompFactory.DerivationFramework.PhotonVertexSelectionWrapper(
            name, **kwargs))
    return acc


def PhotonVertexSelectionWrapperKernelCfg(
        flags, name="PhotonVertexSelectionWrapperKernel", **kwargs):
    acc = ComponentAccumulator()

    augmentationTools = [
        acc.addPublicTool(acc.popToolsAndMerge(PhotonVertexSelectionWrapperCfg(flags)))
    ]
    for i, tool in enumerate(augmentationTools):
        acc.addEventAlgo(CompFactory.DerivationFramework.CommonAugmentation(f"{name}Aug{i}", AugmentationTools = [tool]))
    return acc


def EGammaCookieCutClusterToolCfg(flags, name = 'EGCookieCutTool', **kwargs):
    acc = ComponentAccumulator()
    # needed for reading cells, do not rely on other config to do that
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    acc.merge(LArGMCfg(flags))
    from TileGeoModel.TileGMConfig import TileGMCfg
    acc.merge(TileGMCfg(flags))

    clusterKey = kwargs.setdefault("ClusterContainerName", "ForwardElectronCookieCutClusters")
    kwargs.setdefault("ClusterContainerLinksName", clusterKey + "_links")
    #These two properties need to be in sync
    if kwargs["ClusterContainerLinksName"] != (clusterKey + "_links"):
        raise AttributeError("ClusterContainerLinksName ({}) is not syncrhonised with ClusterContainerName ({})".format(kwargs["ClusterContainerLinksName"], clusterKey))
    kwargs.setdefault('StoreCookedMoments', False)
    kwargs.setdefault('StoreInputMoments', False)
    kwargs.setdefault("SGKey_electrons", flags.Egamma.Keys.Output.ForwardElectrons)
    momentNames = [
        "CENTER_X", "CENTER_Y", "CENTER_Z",
        "SECOND_LAMBDA", "LATERAL", "LONGITUDINAL", "ENG_FRAC_MAX",
        "SECOND_R", "CENTER_LAMBDA", "SECOND_ENG_DENS", "SIGNIFICANCE" ]
    import ROOT
    moments = [
        ROOT.xAOD.CaloCluster.CENTER_X, ROOT.xAOD.CaloCluster.CENTER_Y, ROOT.xAOD.CaloCluster.CENTER_Z,
        ROOT.xAOD.CaloCluster.SECOND_LAMBDA, ROOT.xAOD.CaloCluster.LATERAL, ROOT.xAOD.CaloCluster.LONGITUDINAL, ROOT.xAOD.CaloCluster.ENG_FRAC_MAX,
        ROOT.xAOD.CaloCluster.SECOND_R, ROOT.xAOD.CaloCluster.CENTER_LAMBDA, ROOT.xAOD.CaloCluster.SECOND_ENG_DENS, ROOT.xAOD.CaloCluster.SIGNIFICANCE ]
    kwargs.setdefault("MomentNames", momentNames)
    kwargs.setdefault("Moments", moments)
    cookedMoments = [ "cookiecut" + moment for moment in momentNames] if kwargs['StoreCookedMoments'] else []
    originalMoments = [ "original" + moment for moment in momentNames] if kwargs['StoreInputMoments'] else []
    electronDecorations = [i for sublist in zip(cookedMoments, originalMoments) for i in sublist]
    electronDecorations += ["cookiecutClusterLink"]
    kwargs.setdefault("SGKey_electrons_decorations", electronDecorations)

    from CaloTools.CaloNoiseCondAlgConfig import CaloNoiseCondAlgCfg
    acc.merge(CaloNoiseCondAlgCfg(flags,"totalNoise"))
    from CaloRec.CaloTopoClusterConfig import getTopoMoments
    momentsMaker = acc.popToolsAndMerge(getTopoMoments(flags))
    kwargs.setdefault("ClusterMomentMaker",[momentsMaker])

    acc.setPrivateTools(
        CompFactory.DerivationFramework.EGammaCookieCutClusterTool(
            name, **kwargs))
    return acc

def EGammaEnergyCalibrationWrapperCfg(
        flags,
        name="TransformerEnergyCalibration",
        **kwargs):
    acc = ComponentAccumulator()

    from egammaMVACalib.egammaMVACalibConfig import egammaTransformerSvcCfg

    kwargs.setdefault("decoratorTransformerEnergy", "TransformerEnergy")
    kwargs.setdefault("decoratorTransformerEnergyPhoton", "TransformerEnergy")
    kwargs.setdefault("TransformerCalibSvc", acc.getPrimaryAndMerge(egammaTransformerSvcCfg(flags)))

    acc.addEventAlgo(CompFactory.DerivationFramework.EGammaEnergyCalibrationWrapper(name, **kwargs))

    return acc

# ====================================================================
# SHOWER SHAPE CORRECTIONS IN MC
# The default tunes are set in
# PhysicsAnalysis/ElectronPhotonID/EGammaVariableCorrection/python/EGammaVariableCorrectionConfig.py
# AF3 is tuned to FullSim, so same FFs can be used for AF3 and FS
# ====================================================================

def ElectronFudgeAlgorithmCfg(flags, name="ElectronFudgeAlgorithm", **kwargs):
    acc = ComponentAccumulator()

    if "EGammaFudgeTool" not in kwargs:
        from EGammaVariableCorrection.EGammaVariableCorrectionConfig import (
            ElectronVariableCorrectionToolCfg)
        ElectronVariableCorrectionTool = acc.popToolsAndMerge(
            ElectronVariableCorrectionToolCfg(flags)
        )
        acc.addPublicTool(ElectronVariableCorrectionTool)
        kwargs.setdefault("EGammaFudgeTool", ElectronVariableCorrectionTool)

    kwargs.setdefault("Input", "Electrons")
    kwargs.setdefault("Output", "FudgedElectrons")

    acc.addEventAlgo(CompFactory.DerivationFramework.EGammaFudgeAlgorithm(name, **kwargs))
    return acc

def PhotonFudgeAlgorithmCfg(flags, name="PhotonFudgeAlgorithm", **kwargs):
    acc = ComponentAccumulator()

    if "EGammaFudgeTool" not in kwargs:
        from EGammaVariableCorrection.EGammaVariableCorrectionConfig import (
            PhotonVariableCorrectionToolCfg)
        PhotonVariableCorrectionTool = acc.popToolsAndMerge(
            PhotonVariableCorrectionToolCfg(flags)
        )
        acc.addPublicTool(PhotonVariableCorrectionTool)
        kwargs.setdefault("EGammaFudgeTool", PhotonVariableCorrectionTool)

    kwargs.setdefault("Input", "Photons")
    kwargs.setdefault("Output", "FudgedPhotons")

    acc.addEventAlgo(CompFactory.DerivationFramework.EGammaFudgeAlgorithm(name, **kwargs))
    return acc

def PhotonNFFudgeAlgorithmCfg(flags, name="PhotonNFFudgeAlgorithm", **kwargs):
    acc = ComponentAccumulator()

    if "EGammaFudgeTool" not in kwargs:
        nFoldsNF = flags.Egamma.NFoldsNF if flags.hasFlag('Egamma.NFoldsNF') else None
        from EGammaVariableCorrection.EGammaVariableCorrectionConfig import (
            ElectronPhotonVariableNFCorrectionToolCfg)
        PhotonVariableNFCorrectionTool = acc.popToolsAndMerge(
                ElectronPhotonVariableNFCorrectionToolCfg(flags, nFolds=nFoldsNF)
            )
        acc.addPublicTool(PhotonVariableNFCorrectionTool)
        kwargs.setdefault("EGammaFudgeTool", PhotonVariableNFCorrectionTool)

    kwargs.setdefault("Input", "Photons")
    kwargs.setdefault("Output", "NFFudgedPhotons")

    acc.addEventAlgo(CompFactory.DerivationFramework.EGammaFudgeAlgorithm(name, **kwargs))
    return acc
