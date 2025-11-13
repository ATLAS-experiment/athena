# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# ==============================================================================
# Provides configs for the tools used for e-gamma decorations used in DAOD
# building
# ==============================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


# PhotonsDirectionTool
def PhotonsDirectionToolCfg(flags, name, **kwargs):
    """Configure the PhotonsDirectionTool"""
    acc = ComponentAccumulator()
    PhotonsDirectionTool = CompFactory.DerivationFramework.PhotonsDirectionTool
    acc.addPublicTool(PhotonsDirectionTool(name, **kwargs), primary=True)
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
    EGSelectionToolWrapper = CompFactory.DerivationFramework.EGSelectionToolWrapper
    acc.addPublicTool(EGSelectionToolWrapper(name, **kwargs), primary=True)
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
    kwargs.setdefault("decoratorMultipleOutputs", [containerName + "." + n for n in sgMultipleNames] if storeMultipleOutputs else [])
    # FIXME Would ideally do this, but currently this syntax overwrites the parent container key
    #kwargs.setdefault("decoratorMultipleOutputs", sgMultipleNames if storeMultipleOutputs else [])
    acc.setPrivateTools(CompFactory.DerivationFramework.EGElectronLikelihoodToolWrapper(name, **kwargs))
    return acc


# Photon cleaning tool wrapper
def EGPhotonCleaningWrapperCfg(flags, name, **kwargs):
    """Configure the photon cleaning tool wrapper"""
    acc = ComponentAccumulator()
    sgName = kwargs.pop("StoreGateEntryName", "DFCommonPhotonsCleaning")
    # Write decoration handle keys
    kwargs.setdefault("decoratorPass", sgName)
    kwargs.setdefault("decoratorPassDelayed", sgName + "NoTime")
    EGPhotonCleaningWrapper = CompFactory.DerivationFramework.EGPhotonCleaningWrapper
    acc.addPublicTool(EGPhotonCleaningWrapper(name, **kwargs), primary=True)
    return acc


# Electron ambiguity tool
def EGElectronAmbiguityToolCfg(flags, name, **kwargs):
    """Configure the electron ambiguity tool"""
    acc = ComponentAccumulator()
    EGElectronAmbiguityTool = CompFactory.DerivationFramework.EGElectronAmbiguityTool
    acc.addPublicTool(EGElectronAmbiguityTool(name, **kwargs), primary=True)
    return acc


# Background electron classification tool
def BkgElectronClassificationCfg(flags, name, **kwargs):
    """Configure the background electron classification tool"""
    acc = ComponentAccumulator()
    BkgElectronClassification = (
        CompFactory.DerivationFramework.BkgElectronClassification
    )
    acc.addPublicTool(BkgElectronClassification(name, **kwargs), primary=True)
    return acc


# Standard + LRT electron collection merger
def ElectronMergerCfg(flags, name, **kwargs):
    """Configure the track particle merger tool"""
    acc = ComponentAccumulator()
    ElectronMerger = CompFactory.DerivationFramework.ElectronMergerTool
    acc.addPublicTool(ElectronMerger(name, **kwargs), primary=True)
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
    kwargs.setdefault("AugmentationTools", augmentationTools)

    acc.addEventAlgo(
        CompFactory.DerivationFramework.DerivationKernel(name, **kwargs))
    return acc


def EGammaCookieCutClusterToolCfg(flags, name = 'EGCookieCutTool', **kwargs):
    acc = ComponentAccumulator()
    # needed for reading cells, do not rely on other config to do that
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    acc.merge(LArGMCfg(flags))
    from TileGeoModel.TileGMConfig import TileGMCfg
    acc.merge(TileGMCfg(flags))

    kwargs.setdefault("ClusterContainerName", "ForwardElectronCookieCutClusters")
    kwargs.setdefault("ClusterContainerLinksName", kwargs["ClusterContainerName"] + "_links")
    #These two properties need to be in sync
    if kwargs["ClusterContainerLinksName"] is not kwargs["ClusterContainerName"] + "_links":
        raise AttributeError("ClusterContainerLinksName is not syncrhonised with ClusterContainerName")
    kwargs.setdefault('StoreCookedMoments', False)
    kwargs.setdefault('StoreInputMoments', False)
    kwargs.setdefault("SGKey_electrons", flags.Egamma.Keys.Output.ForwardElectrons)
    # TODO Need to keep this in sync with EGammaCookieCutClusterTool::m_vecMName (make into a property?)
    momentNames = ["SECOND_LAMBDA", "LATERAL", "LONGITUDINAL", "ENG_FRAC_MAX",
                   "SECOND_R", "CENTER_LAMBDA", "SECOND_ENG_DENS", "SIGNIFICANCE"]
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
