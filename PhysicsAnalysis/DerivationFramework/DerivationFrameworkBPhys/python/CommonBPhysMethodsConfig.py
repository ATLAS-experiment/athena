# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def BPHY_TrkVKalVrtFitterCfg(flags, BPHYDerivationName, **kwargs):
    from TrkConfig.TrkVKalVrtFitterConfig import BPHY_TrkVKalVrtFitterCfg \
        as TrkVKalVrtFitterCfg
    return TrkVKalVrtFitterCfg(flags, name=BPHYDerivationName+"_VKalVrtFitter", **kwargs)

def BPHY_V0ToolCfg(flags, BPHYDerivationName):
    from TrkConfig.TrkVertexAnalysisUtilsConfig import V0ToolsCfg
    return V0ToolsCfg(flags, BPHYDerivationName+"_V0Tools")

def BPHY_VertexPointEstimatorCfg(flags, BPHYDerivationName):
    from InDetConfig.InDetConversionFinderToolsConfig import \
        BPHY_VertexPointEstimatorCfg as VertexPointEstimatorCfg
    return VertexPointEstimatorCfg(flags, BPHYDerivationName+"_VtxPointEstimator")

def BPHY_InDetDetailedTrackSelectorToolCfg(flags, BPHYDerivationName):
    from InDetConfig.InDetTrackSelectorToolConfig import \
        BPHY_InDetDetailedTrackSelectorToolCfg as \
        InDetDetailedTrackSelectorToolCfg
    return InDetDetailedTrackSelectorToolCfg(flags, BPHYDerivationName+"_InDetDetailedTrackSelectorTool")

def Thin_vtxTrkCfg(flags, name, **kwargs):
    acc = ComponentAccumulator()
    acc.addPublicTool(CompFactory.DerivationFramework.Thin_vtxTrk(name, **kwargs),
                      primary = True)
    return acc

def AugOriginalCountsCfg(flags, name="AugOriginalCounts", **kwargs):
    """ Configures an IAugmentationTool, name and kwargs are passed to the tool configurable constructor """
    acc = ComponentAccumulator()
    kwargs.setdefault("TrackLRTContainer",
                      "InDetLargeD0TrackParticles"
                      if flags.Tracking.doLargeD0 else "")
    acc.setPrivateTools(
        CompFactory.DerivationFramework.AugOriginalCounts(name, **kwargs)) # TODO Migrate to AthReentrantAlgorithm (ATLASG-3211)
    return acc

def getDefaultAllVariables():
    return ["EventInfo"]

def BPhysBGammaFinderCfg(flags, name="BPhysBGammaFinder", **kwargs):
    """ Configures an IAugmentationTool, flags is currently unused, name and kwargs are passed to the tool configurable constructor """
    #Perhaps flesh out in future see ATLASG-2977
    acc = ComponentAccumulator()
    acc.setPrivateTools(
    CompFactory.DerivationFramework.BPhysBGammaFinder(name, **kwargs)) # TODO Migrate to AthReentrantAlgorithm (ATLASG-3211)
    return acc

def BPhysMetadataBaseCfg(flags, name="BPhysMetadataBase", **kwargs):
    """ Configures an IAugmentationTool, flags is currently unused, name and kwargs are passed to the tool configurable constructor """
    #Perhaps flesh out in future see ATLASG-2977
    acc = ComponentAccumulator()
    acc.setPrivateTools(
    CompFactory.DerivationFramework.BPhysMetadataBase(name, **kwargs)) # TODO Migrate to AthReentrantAlgorithm (ATLASG-3211)
    return acc

def Bmumu_metadataCfg(flags, BPHYDerivationName, **kwargs):
    """ Configures an IAugmentationTool, flags is currently unused, BPHYDerivationName is used to build the tool name, kwargs is passed to the tool configuration """
    #Perhaps flesh out in future see ATLASG-2977
    acc = ComponentAccumulator()
    name = BPHYDerivationName + "_Bmumu_metadata"
    kwargs.setdefault("DerivationName",BPHYDerivationName)
    acc.setPrivateTools(
    CompFactory.DerivationFramework.Bmumu_metadata(name, **kwargs)) # TODO Migrate to AthReentrantAlgorithm (ATLASG-3211)
    return acc


def Reco_VertexCfg(flags, name = "Reco_Vertex", **kwargs):
    """ Configures an IAugmentationTool, flags is currently unused, name and kwargs are passed to the tool configurable constructor """
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.DerivationFramework.Reco_Vertex(name, **kwargs)) # TODO Migrate to AthReentrantAlgorithm (ATLASG-3211)
    return acc


def VertexTrackIsolationCfg(flags, name = "VertexTrackIsolation", **kwargs):
    """ Configures an IAugmentationTool, flags is currently unused, name and kwargs are passed to the tool configurable constructor """
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.DerivationFramework.VertexTrackIsolation(name, **kwargs)) # TODO Migrate to AthReentrantAlgorithm (ATLASG-3211)
    return acc


def Select_onia2mumuCfg(flags, name = "Select_onia2mumu", **kwargs):
    """ Configures an IAugmentationTool, flags is currently unused, name and kwargs are passed to the tool configurable constructor """
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.DerivationFramework.Select_onia2mumu(name, **kwargs)) # TODO Migrate to AthReentrantAlgorithm (ATLASG-3211)
    return acc


def JpsiPlusDpstCascadeCfg(flags, name = "JpsiPlusDpstCascade", **kwargs):
    """ Configures an IAugmentationTool, flags is currently unused, name and kwargs are passed to the tool configurable constructor """
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.DerivationFramework.JpsiPlusDpstCascade(name, **kwargs)) # TODO Migrate to AthReentrantAlgorithm (ATLASG-3211)
    return acc


def JpsiPlusDs1CascadeCfg(flags, name = "JpsiPlusDs1Cascade", **kwargs):
    """ Configures an IAugmentationTool, flags is currently unused, name and kwargs are passed to the tool configurable constructor """
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.DerivationFramework.JpsiPlusDs1Cascade(name, **kwargs)) # TODO Migrate to AthReentrantAlgorithm (ATLASG-3211)
    return acc


def JpsiPlusDsCascadeCfg(flags, name = "JpsiPlusDsCascade", **kwargs):
    """ Configures an IAugmentationTool, flags is currently unused, name and kwargs are passed to the tool configurable constructor """
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.DerivationFramework.JpsiPlusDsCascade(name, **kwargs)) # TODO Migrate to AthReentrantAlgorithm (ATLASG-3211)
    return acc


def JpsiPlusV0CascadeCfg(flags, name = "JpsiPlusV0Cascade", **kwargs):
    """ Configures an IAugmentationTool, flags is currently unused, name and kwargs are passed to the tool configurable constructor """
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.DerivationFramework.JpsiPlusV0Cascade(name, **kwargs)) # TODO Migrate to AthReentrantAlgorithm (ATLASG-3211)
    return acc


def JpsiXPlus2V0Cfg(flags, name = "JpsiXPlus2V0", **kwargs):
    """ Configures an IAugmentationTool, flags is currently unused, name and kwargs are passed to the tool configurable constructor """
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.DerivationFramework.JpsiXPlus2V0(name, **kwargs)) # TODO Migrate to AthReentrantAlgorithm (ATLASG-3211)
    return acc


def JpsiXPlusDisplacedCfg(flags, name = "JpsiXPlusDisplaced", **kwargs):
    """ Configures an IAugmentationTool, flags is currently unused, name and kwargs are passed to the tool configurable constructor """
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.DerivationFramework.JpsiXPlusDisplaced(name, **kwargs)) # TODO Migrate to AthReentrantAlgorithm (ATLASG-3211)
    return acc


def MuPlusDpstCascadeCfg(flags, name = "MuPlusDpstCascade", **kwargs):
    """ Configures an IAugmentationTool, flags is currently unused, name and kwargs are passed to the tool configurable constructor """
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.DerivationFramework.MuPlusDpstCascade(name, **kwargs)) # TODO Migrate to AthReentrantAlgorithm (ATLASG-3211)
    return acc


def MuPlusDsCascadeCfg(flags, name = "MuPlusDsCascade", **kwargs):
    """ Configures an IAugmentationTool, flags is currently unused, name and kwargs are passed to the tool configurable constructor """
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.DerivationFramework.MuPlusDsCascade(name, **kwargs)) # TODO Migrate to AthReentrantAlgorithm (ATLASG-3211)
    return acc


def MuonExtrapolationCfg(flags, name = "MuonExtrapolation", **kwargs):
    """ Configures an IAugmentationTool, flags is currently unused, name and kwargs are passed to the tool configurable constructor """
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.DerivationFramework.MuonExtrapolationTool(name, **kwargs)) # TODO Migrate to AthReentrantAlgorithm (ATLASG-3211)
    return acc


def ReVertexCfg(flags, name = "ReVertex", **kwargs):
    """ Configures an IAugmentationTool, flags is currently unused, name and kwargs are passed to the tool configurable constructor """
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.DerivationFramework.ReVertex(name, **kwargs)) # TODO Migrate to AthReentrantAlgorithm (ATLASG-3211)
    return acc


def Reco_4muCfg(flags, name = "Reco_4mu", **kwargs):
    """ Configures an IAugmentationTool, flags is currently unused, name and kwargs are passed to the tool configurable constructor """
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.DerivationFramework.Reco_4mu(name, **kwargs)) # TODO Migrate to AthReentrantAlgorithm (ATLASG-3211)
    return acc


def Reco_mumuCfg(flags, name = "Reco_mumu", **kwargs):
    """ Configures an IAugmentationTool, flags is currently unused, name and kwargs are passed to the tool configurable constructor """
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.DerivationFramework.Reco_mumu(name, **kwargs)) # TODO Migrate to AthReentrantAlgorithm (ATLASG-3211)
    return acc


def PsiPlusPsiSingleVertexCfg(flags, name = "PsiPlusPsiSingleVertex", **kwargs):
    """ Configures an IAugmentationTool, flags is currently unused, name and kwargs are passed to the tool configurable constructor """
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.DerivationFramework.PsiPlusPsiSingleVertex(name, **kwargs)) # TODO Migrate to AthReentrantAlgorithm (ATLASG-3211)
    return acc


def JpsiPlusPsiCascadeCfg(flags, name = "JpsiPlusPsiCascade", **kwargs):
    """ Configures an IAugmentationTool, flags is currently unused, name and kwargs are passed to the tool configurable constructor """
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.DerivationFramework.JpsiPlusPsiCascade(name, **kwargs)) # TODO Migrate to AthReentrantAlgorithm (ATLASG-3211)
    return acc


def PsiPlusPsiCascadeCfg(flags, name = "PsiPlusPsiCascade", **kwargs):
    """ Configures an IAugmentationTool, flags is currently unused, name and kwargs are passed to the tool configurable constructor """
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.DerivationFramework.PsiPlusPsiCascade(name, **kwargs)) # TODO Migrate to AthReentrantAlgorithm (ATLASG-3211)
    return acc
