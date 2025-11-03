# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# ==============================================================================
# Provides configs for the tools used for building/thinning tracking related
# object containers and decorations in the DAODs
# ==============================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType
from AthenaCommon.Constants import INFO

# Track collection merger
def InDetLRTMergeCfg(flags, name="InDetLRTMerge", **kwargs):
    acc = ComponentAccumulator()
    alg = CompFactory.CP.TrackParticleMergerAlg(name, **kwargs)
    kwargs.setdefault("InputTrackParticleLocations",
                      ["InDetTrackParticles", "InDetLargeD0TrackParticles"])
    kwargs.setdefault("OutputTrackParticleLocation",
                      "InDetWithLRTTrackParticles")
    kwargs.setdefault("CreateViewCollection", True)
    acc.addEventAlgo(alg, primary=True)
    return acc

# Used in vertex fit track decorator


def UsedInVertexFitTrackDecoratorCfg(
        flags, name="UsedInVertexFitTrackDecorator", **kwargs):
    """Configure the UsedInVertexFitTrackDecorator"""
    acc = ComponentAccumulator()

    if "UsedInFitDecoratorTool" not in kwargs:
        from InDetConfig.InDetUsedInFitTrackDecoratorToolConfig import (
            InDetUsedInFitTrackDecoratorToolCfg)
        kwargs.setdefault("UsedInFitDecoratorTool", acc.popToolsAndMerge(
            InDetUsedInFitTrackDecoratorToolCfg(flags)))

    acc.addPublicTool(
        CompFactory.DerivationFramework.UsedInVertexFitTrackDecorator(
            name, **kwargs), primary=True)
    return acc



def HardScatterVertexDecoratorCfg(flags, name = "DFCommonHSDecorator", **kwargs):
    """Configure the hard process vertex decorator"""
    acc = ComponentAccumulator()
    from InDetConfig.InDetHardScatterSelectionToolConfig import InDetHardScatterSelectionToolCfg
    kwargs.setdefault("HardScatterSelectionTool", acc.getPrimaryAndMerge(InDetHardScatterSelectionToolCfg(flags, name = "HSSelectionTool",
                                                                                                                ReturnDeco = False)))
    kwargs.setdefault("VertexContainerName", "PrimaryVertices")
    kwargs.setdefault("HardScatterDecoName", "hardScatterVertexLink")
    the_tool = CompFactory.DerivationFramework.HardScatterVertexDecorator(name = "HardScatterDecorTool", **kwargs)
    acc.addPublicTool(the_tool, primary=True)
    the_alg = CompFactory.DerivationFramework.CommonAugmentation(name, AugmentationTools=[the_tool])
    acc.addEventAlgo(the_alg)
    return acc

# TrackStateOnSurface decorator

def TrackStateOnSurfaceDecoratorCfg(flags, name, **kwargs):
    """Configure the TSOS decorator"""
    acc = ComponentAccumulator()

    # To produce SCT_DetectorElementCollection
    from SCT_GeoModel.SCT_GeoModelConfig import SCT_ReadoutGeometryCfg
    acc.merge(SCT_ReadoutGeometryCfg(flags))

    kwargs.setdefault("DecorationPrefix", "notSet")

    if "Updator" not in kwargs:
        from TrkConfig.TrkMeasurementUpdatorConfig import KalmanUpdatorCfg
        kwargs.setdefault("Updator", acc.popToolsAndMerge(
            KalmanUpdatorCfg(flags)))
    
    if "TrackExtrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
        kwargs.setdefault("TrackExtrapolator", acc.popToolsAndMerge(
            AtlasExtrapolatorCfg(flags)))

    if "HoleSearch" not in kwargs:
        from InDetConfig.InDetTrackHoleSearchConfig import (
            InDetTrackHoleSearchToolCfg)
        kwargs.setdefault("HoleSearch", acc.popToolsAndMerge(
            InDetTrackHoleSearchToolCfg(flags)))

    if "ResidualPullCalculator" not in kwargs:
        from TrkConfig.TrkResidualPullCalculatorConfig import (
            ResidualPullCalculatorCfg)
        kwargs.setdefault("ResidualPullCalculator", acc.popToolsAndMerge(
            ResidualPullCalculatorCfg(flags)))

    kwargs.setdefault("StorePixel", flags.Tracking.writeExtendedSi_PRDInfo)
    kwargs.setdefault("StoreSCT", flags.Tracking.writeExtendedSi_PRDInfo)
    kwargs.setdefault("StoreTRT", flags.Tracking.writeExtendedTRT_PRDInfo)
    kwargs.setdefault("AddExtraEventInfo", flags.Beam.Type is BeamType.Cosmics)

    if kwargs["StoreTRT"]:
        if "TRT_ToT_dEdx" not in kwargs:
            from InDetConfig.TRT_ElectronPidToolsConfig import TRT_dEdxToolCfg
            kwargs.setdefault("TRT_ToT_dEdx", acc.popToolsAndMerge(
                TRT_dEdxToolCfg(flags)))

        if "TRT_CalDbTool" not in kwargs:
            from TRT_ConditionsServices.TRT_ConditionsServicesConfig import (
                TRT_CalDbToolCfg)
            kwargs.setdefault("TRT_CalDbTool", acc.popToolsAndMerge(
                TRT_CalDbToolCfg(flags)))

    kwargs.setdefault("PRDtoTrackMap", "PRDtoTrackMapMerge_CombinedInDetTracks")

    acc.addPublicTool(
        CompFactory.DerivationFramework.TrackStateOnSurfaceDecorator(
            name, **kwargs), primary=True)
    return acc

def TSOS_CommonKernelCfg(flags, name="TSOS_CommonKernel",
                         listOfExtensions=[]):
    acc = ComponentAccumulator()

    listOfAugmTools = []
    for extension in listOfExtensions:
        # Fix inconsistency in naming convention for R3LargeD0
        if extension == "R3LargeD0":
            extension = "LargeD0"
        TrackStateOnSurfaceDecorator = acc.getPrimaryAndMerge(
            TrackStateOnSurfaceDecoratorCfg(
                flags, name = f"{extension}TrackStateOnSurfaceDecorator",
                ContainerName = f"InDet{extension}TrackParticles",
                PixelMsosName = f"{extension}PixelMSOSs",
                SctMsosName = f"{extension}SCT_MSOSs",
                TrtMsosName = f"{extension}TRT_MSOSs"))
        TrackStateOnSurfaceDecorator.DecorationPrefix = "Reco_"
        listOfAugmTools.append(TrackStateOnSurfaceDecorator)

    acc.addEventAlgo(CompFactory.DerivationFramework.CommonAugmentation(
        name, AugmentationTools=listOfAugmTools))
    return acc

def DFTrackStateOnSurfaceDecoratorCfg(
        flags, name="DFTrackStateOnSurfaceDecorator", **kwargs):
    kwargs.setdefault("StorePixel", flags.InDet.DAODStorePixel)
    kwargs.setdefault("StoreSCT", flags.InDet.DAODStoreSCT)
    kwargs.setdefault("StoreTRT", flags.InDet.DAODStoreTRT)
    # never decorate EventInfo with TRTPhase, doubt this is useful for IDTIDE
    kwargs.setdefault("AddExtraEventInfo", flags.InDet.DAODStoreExtra and flags.Beam.Type is BeamType.Cosmics)
    kwargs.setdefault("DecorationPrefix", "")
    kwargs.setdefault("PRDtoTrackMap", "")
    kwargs.setdefault("OutputLevel", INFO)
    return TrackStateOnSurfaceDecoratorCfg(flags, name, **kwargs)

def DFInDetTSOSKernelCfg(flags, name='DFInDetTSOSKernel'):
    acc = ComponentAccumulator()

    # ====================================================================
    # AUGMENTATION TOOLS
    # ====================================================================
    tsos_augmentationTools = []

    DFTSOS = acc.getPrimaryAndMerge(DFTrackStateOnSurfaceDecoratorCfg(flags))
    tsos_augmentationTools.append(DFTSOS)

    # shared between IDTIDE and IDTRKVALID
    acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel(
        name,
        AugmentationTools=tsos_augmentationTools,
        ThinningTools=[],
        OutputLevel=INFO))
    return acc

def ObserverTrackStateOnSurfaceDecoratorCfg(
        flags, name="ObserverTrackStateOnSurfaceDecorator", **kwargs):
    kwargs.setdefault("ContainerName", "InDetObservedTrackParticles")
    kwargs.setdefault("DecorationPrefix", "ObservedTrack_")
    kwargs.setdefault("PixelMsosName", "ObservedTrack_Pixel_MSOSs")
    kwargs.setdefault("SctMsosName", "ObservedTrack_SCT_MSOSs")
    kwargs.setdefault("TrtMsosName", "ObservedTrack_TRT_MSOSs")
    kwargs.setdefault("AddPRD", True)
    kwargs.setdefault("StoreHoles", False)
    return TrackStateOnSurfaceDecoratorCfg(flags, name, **kwargs)

def ObserverTSOS_CommonKernelCfg(flags, name="ObserverTSOS_CommonKernel"):
    acc = ComponentAccumulator()
    ObserverTrackStateOnSurfaceDecorator = acc.getPrimaryAndMerge(
        ObserverTrackStateOnSurfaceDecoratorCfg(flags))
    acc.addEventAlgo(CompFactory.DerivationFramework.CommonAugmentation(
        name, AugmentationTools=[ObserverTrackStateOnSurfaceDecorator]))
    return acc

def PseudoTrackStateOnSurfaceDecoratorCfg(
        flags, name="PseudoTrackStateOnSurfaceDecorator", **kwargs):
    kwargs.setdefault("ContainerName", "InDetPseudoTrackParticles")
    kwargs.setdefault("DecorationPrefix", "Pseudo_")
    kwargs.setdefault("PixelMsosName", "Pseudo_Pixel_MSOSs")
    kwargs.setdefault("SctMsosName", "Pseudo_SCT_MSOSs")
    kwargs.setdefault("TrtMsosName", "Pseudo_TRT_MSOSs")
    kwargs.setdefault("AddPRD", True)
    kwargs.setdefault("StoreHoles", False)
    return TrackStateOnSurfaceDecoratorCfg(flags, name, **kwargs)

def PseudoTSOS_CommonKernelCfg(flags, name="PseudoTSOS_CommonKernel"):
    acc = ComponentAccumulator()
    PseudoTrackStateOnSurfaceDecorator = acc.getPrimaryAndMerge(
        PseudoTrackStateOnSurfaceDecoratorCfg(flags))
    acc.addEventAlgo(CompFactory.DerivationFramework.CommonAugmentation(
        name, AugmentationTools=[PseudoTrackStateOnSurfaceDecorator]))
    return acc

def SiSPSeedsTrackStateOnSurfaceDecoratorCfg(
        flags, name="SiSPSeedsTrackStateOnSurfaceDecorator", **kwargs):
    kwargs.setdefault("ContainerName", "SiSPSeedSegmentsTrackParticles")
    kwargs.setdefault("DecorationPrefix", "Reco_")
    kwargs.setdefault("PixelMsosName", "SiSPSeedSegments_PixelMSOSs")
    kwargs.setdefault("SctMsosName", "SiSPSeedSegments_SCT_MSOSs")
    kwargs.setdefault("TrtMsosName", "SiSPSeedSegments_TRT_MSOSs")
    kwargs.setdefault("AddPRD", True)
    kwargs.setdefault("StoreHoles", False)
    return TrackStateOnSurfaceDecoratorCfg(flags, name, **kwargs)

def SiSPSeedsTSOS_CommonKernelCfg(flags, name="SiSPSeedsTSOS_CommonKernel",
                             listOfExtensions=[]):
    acc = ComponentAccumulator()

    listOfAugmTools = []
    for extension in listOfExtensions:
        SiSPSeedsTrackStateOnSurfaceDecorator = acc.getPrimaryAndMerge(
            SiSPSeedsTrackStateOnSurfaceDecoratorCfg(
                flags, name = f"SiSPSeeds{extension}TrackStateOnSurfaceDecorator",
                ContainerName = f"SiSPSeedSegments{extension}TrackParticles",
                PixelMsosName = f"SiSPSeedSegments{extension}_PixelMSOSs",
                SctMsosName = f"SiSPSeedSegments{extension}_SCT_MSOSs",
                TrtMsosName = f"SiSPSeedSegments{extension}_TRT_MSOSs"))
        listOfAugmTools.append(SiSPSeedsTrackStateOnSurfaceDecorator)

    acc.addEventAlgo(CompFactory.DerivationFramework.CommonAugmentation(
        name, AugmentationTools=listOfAugmTools))
    return acc

def SiSPTrackStateOnSurfaceDecoratorCfg(
        flags, name="SiSPTrackStateOnSurfaceDecorator", **kwargs):
    kwargs.setdefault("ContainerName", "SiSPSeededTracksTrackParticles")
    kwargs.setdefault("DecorationPrefix", "Reco_")
    kwargs.setdefault("PixelMsosName", "SiSP_PixelMSOSs")
    kwargs.setdefault("SctMsosName", "SiSP_SCT_MSOSs")
    kwargs.setdefault("TrtMsosName", "SiSP_TRT_MSOSs")
    kwargs.setdefault("AddPRD", True)
    kwargs.setdefault("StoreHoles", False)
    return TrackStateOnSurfaceDecoratorCfg(flags, name, **kwargs)

def SiSPTSOS_CommonKernelCfg(flags, name="SiSPTSOS_CommonKernel",
                             listOfExtensions=[]):
    acc = ComponentAccumulator()

    listOfAugmTools = []
    for extension in listOfExtensions:
        SiSPTrackStateOnSurfaceDecorator = acc.getPrimaryAndMerge(
            SiSPTrackStateOnSurfaceDecoratorCfg(
                flags, name = f"SiSP{extension}TrackStateOnSurfaceDecorator",
                ContainerName = f"SiSPSeededTracks{extension}TrackParticles",
                PixelMsosName = f"SiSP{extension}_PixelMSOSs",
                SctMsosName = f"SiSP{extension}_SCT_MSOSs",
                TrtMsosName = f"SiSP{extension}_TRT_MSOSs"))
        listOfAugmTools.append(SiSPTrackStateOnSurfaceDecorator)

    acc.addEventAlgo(CompFactory.DerivationFramework.CommonAugmentation(
        name, AugmentationTools=listOfAugmTools))
    return acc

def GSFTrackStateOnSurfaceDecoratorCfg(
        flags, name="GSFTrackStateOnSurfaceDecorator", **kwargs):
    kwargs.setdefault("ContainerName", "GSFTrackParticles")
    kwargs.setdefault("DecorationPrefix", "GSF_")
    kwargs.setdefault("PixelMsosName", "GSF_Pixel_MSOSs")
    kwargs.setdefault("SctMsosName", "GSF_SCT_MSOSs")
    kwargs.setdefault("TrtMsosName", "GSF_TRT_MSOSs")
    kwargs.setdefault("PRDtoTrackMap", "")
    return TrackStateOnSurfaceDecoratorCfg(flags, name, **kwargs)

def GSFTSOS_CommonKernelCfg(flags, name="GSFTSOS_CommonKernel"):
    if flags.Detector.GeometryITk:
        return ITkGSFTSOS_CommonKernelCfg(flags)
    acc = ComponentAccumulator()
    GSFTrackStateOnSurfaceDecorator = acc.getPrimaryAndMerge(
        GSFTrackStateOnSurfaceDecoratorCfg(flags))
    acc.addEventAlgo(CompFactory.DerivationFramework.CommonAugmentation(
        name, AugmentationTools=[GSFTrackStateOnSurfaceDecorator]))
    return acc

def ITkTrackStateOnSurfaceDecoratorCfg(
        flags, name="TrackStateOnSurfaceDecorator", **kwargs):
    """Configure the TSOS decorator"""
    # To produce ITkStripDetectorElementCollection
    from StripGeoModelXml.ITkStripGeoModelConfig import (
        ITkStripReadoutGeometryCfg)
    acc = ITkStripReadoutGeometryCfg(flags)

    kwargs.setdefault("DecorationPrefix", "notSet")

    if "Updator" not in kwargs:
        from TrkConfig.TrkMeasurementUpdatorConfig import KalmanUpdatorCfg
        kwargs.setdefault("Updator", acc.popToolsAndMerge(
            KalmanUpdatorCfg(flags)))
    
    if "TrackExtrapolator" not in kwargs:
        from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
        kwargs.setdefault("TrackExtrapolator", acc.popToolsAndMerge(
            AtlasExtrapolatorCfg(flags)))

    if "HoleSearch" not in kwargs:
        from InDetConfig.InDetTrackHoleSearchConfig import (
            ITkTrackHoleSearchToolCfg)
        kwargs.setdefault("HoleSearch", acc.popToolsAndMerge(
            ITkTrackHoleSearchToolCfg(flags)))

    if "ResidualPullCalculator" not in kwargs:
        from TrkConfig.TrkResidualPullCalculatorConfig import (
            ResidualPullCalculatorCfg)
        kwargs.setdefault("ResidualPullCalculator", acc.popToolsAndMerge(
            ResidualPullCalculatorCfg(flags)))

    kwargs.setdefault("AddExtraEventInfo", False) # Controls decoration of TrtPhaseTime
    kwargs.setdefault("PixelMapName", "ITkPixelClustersOffsets")
    kwargs.setdefault("SctMapName", "ITkStripClustersOffsets")
    kwargs.setdefault("PixelClustersName", "ITkPixelMeasurements")
    kwargs.setdefault("SctClustersName", "ITkStripMeasurements")
    kwargs.setdefault("PRDtoTrackMap", "ITkPRDtoTrackMapMerge_CombinedITkTracks")
    kwargs.setdefault("PixelMsosName", "ITkPixelMSOSs")
    kwargs.setdefault("SctMsosName", "ITkStripMSOSs")
    kwargs.setdefault("SCTDetEleCollKey", "ITkStripDetectorElementCollection")

    acc.addPublicTool(
        CompFactory.DerivationFramework.TrackStateOnSurfaceDecorator(
            name, **kwargs), primary=True)
    return acc

def ITkTSOS_CommonKernelCfg(flags, name="ITkTSOS_CommonKernel",
                            listOfExtensions=[]):
    acc = ComponentAccumulator()
    listOfAugmTools = []
    for extension in listOfExtensions:
        TrackStateOnSurfaceDecorator = acc.getPrimaryAndMerge(
            ITkTrackStateOnSurfaceDecoratorCfg(flags,
                                               DecorationPrefix = "Reco_"))
        listOfAugmTools.append(TrackStateOnSurfaceDecorator)

    acc.addEventAlgo(CompFactory.DerivationFramework.CommonAugmentation(
        name, AugmentationTools=listOfAugmTools))
    return acc

def DFITkTrackStateOnSurfaceDecoratorCfg(
        flags, name="DFITkTrackStateOnSurfaceDecorator", **kwargs):
    kwargs.setdefault("StorePixel", flags.ITk.DAODStorePixel)
    kwargs.setdefault("StoreSCT", flags.ITk.DAODStoreStrip)
    kwargs.setdefault("DecorationPrefix", "")
    kwargs.setdefault("PRDtoTrackMap", "")
    kwargs.setdefault("OutputLevel", INFO)
    return ITkTrackStateOnSurfaceDecoratorCfg(flags, name, **kwargs)

def DFITkTSOSKernelCfg(flags, name='DFITkTSOSKernel'):
    acc = ComponentAccumulator()

    # ====================================================================
    # AUGMENTATION TOOLS
    # ====================================================================
    tsos_augmentationTools = []

    DFTSOS = acc.getPrimaryAndMerge(DFITkTrackStateOnSurfaceDecoratorCfg(flags))
    tsos_augmentationTools.append(DFTSOS)

    # shared between IDTIDE and IDTRKVALID
    acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel(
        name,
        AugmentationTools=tsos_augmentationTools,
        ThinningTools=[],
        OutputLevel=INFO))
    return acc

def ITkSiSPSeedsTrackStateOnSurfaceDecoratorCfg(
        flags, name="ITkSiSPSeedsTrackStateOnSurfaceDecorator", **kwargs):
    kwargs.setdefault("ContainerName", "SiSPSeedSegmentsTrackParticles")
    kwargs.setdefault("DecorationPrefix", "Reco_")
    kwargs.setdefault("PixelMsosName", "SiSPSeedSegments_ITkPixelMSOSs")
    kwargs.setdefault("SctMsosName", "SiSPSeedSegments_ITkStripMSOSs")
    kwargs.setdefault("TrtMsosName", "")
    kwargs.setdefault("AddPRD", True)
    kwargs.setdefault("StoreHoles", False)
    return ITkTrackStateOnSurfaceDecoratorCfg(flags, name, **kwargs)

def ITkSiSPSeedsTSOS_CommonKernelCfg(flags, name="ITkSiSPSeedsTSOS_CommonKernel",
                                     listOfExtensions=[]):
    acc = ComponentAccumulator()

    listOfAugmTools = []
    for extension in listOfExtensions:
        SiSPSeedsTrackStateOnSurfaceDecorator = acc.getPrimaryAndMerge(
            ITkSiSPSeedsTrackStateOnSurfaceDecoratorCfg(
                flags, name = f"SiSPSeeds{extension}TrackStateOnSurfaceDecorator",
                ContainerName = f"SiSPSeedSegments{extension}TrackParticles",
                PixelMsosName = f"SiSPSeedSegments{extension}_ITkPixelMSOSs",
                SctMsosName = f"SiSPSeedSegments{extension}_ITkStripMSOSs"))
        listOfAugmTools.append(SiSPSeedsTrackStateOnSurfaceDecorator)

    acc.addEventAlgo(CompFactory.DerivationFramework.CommonAugmentation(
        name, AugmentationTools=listOfAugmTools))
    return acc

def ITkSiSPTrackStateOnSurfaceDecoratorCfg(
        flags, name="SiSPTrackStateOnSurfaceDecorator", **kwargs):
    kwargs.setdefault("ContainerName", "SiSPSeededTracksTrackParticles")
    kwargs.setdefault("DecorationPrefix", "SiSP_")
    kwargs.setdefault("PixelMsosName", "SiSP_ITkPixel_MSOSs")
    kwargs.setdefault("SctMsosName", "SiSP_ITkStrip_MSOSs")
    kwargs.setdefault("AddPRD", True)
    kwargs.setdefault("StoreHoles", False)
    return ITkTrackStateOnSurfaceDecoratorCfg(flags, name, **kwargs)

def ITkSiSPTSOS_CommonKernelCfg(flags, name="ITkSiSPTSOS_CommonKernel",
                                listOfExtensions=[]):
    acc = ComponentAccumulator()
    listOfAugmTools = []
    for extension in listOfExtensions:
        ITkSiSPTrackStateOnSurfaceDecorator = acc.getPrimaryAndMerge(
            ITkSiSPTrackStateOnSurfaceDecoratorCfg(
                flags, name = f"SiSP{extension}TrackStateOnSurfaceDecorator",
                ContainerName = f"SiSPSeededTracks{extension}TrackParticles",
                PixelMsosName = f"SiSP{extension}_ITkPixel_MSOSs",
                SctMsosName = f"SiSP{extension}_ITkStrip_MSOSs"))
        listOfAugmTools.append(ITkSiSPTrackStateOnSurfaceDecorator)

    acc.addEventAlgo(CompFactory.DerivationFramework.CommonAugmentation(
        name, AugmentationTools=listOfAugmTools))
    return acc

def ITkGSFTrackStateOnSurfaceDecoratorCfg(
        flags, name="ITkGSFTrackStateOnSurfaceDecorator", **kwargs):
    kwargs.setdefault("ContainerName", "GSFTrackParticles")
    kwargs.setdefault("DecorationPrefix", "GSF_")
    kwargs.setdefault("PixelMsosName", "GSF_ITkPixel_MSOSs")
    kwargs.setdefault("SctMsosName", "GSF_ITkStrip_MSOSs")
    kwargs.setdefault("TrtMsosName", "")
    kwargs.setdefault("PRDtoTrackMap", "")
    kwargs.setdefault("AddExtraEventInfo", flags.Beam.Type is BeamType.Cosmics)
    return ITkTrackStateOnSurfaceDecoratorCfg(flags, name, **kwargs)

def ITkGSFTSOS_CommonKernelCfg(flags, name="ITkGSFTSOS_CommonKernel"):
    acc = ComponentAccumulator()
    GSFTrackStateOnSurfaceDecorator = acc.getPrimaryAndMerge(
        ITkGSFTrackStateOnSurfaceDecoratorCfg(flags))
    acc.addEventAlgo(CompFactory.DerivationFramework.CommonAugmentation(
        name, AugmentationTools=[GSFTrackStateOnSurfaceDecorator]))
    return acc

# Expression of Z0 at the primary vertex


def TrackParametersAtPVCfg(flags, name, **kwargs):
    """Configure the TrackParametersAtPV tool"""
    acc = ComponentAccumulator()
    acc.addPublicTool(CompFactory.DerivationFramework.TrackParametersAtPV(
        name, **kwargs), primary=True)
    return acc

# Pseudotrack selector


def PseudoTrackSelectorCfg(flags, name, **kwargs):
    """Configure the pseudotrack selector"""
    acc = ComponentAccumulator()

    if "trackTruthOriginTool" not in kwargs:
        from InDetTrackSystematicsTools.InDetTrackSystematicsToolsConfig import InDetTrackTruthOriginToolCfg
        kwargs.setdefault("trackTruthOriginTool", acc.popToolsAndMerge(
            InDetTrackTruthOriginToolCfg(flags)))

    acc.addPublicTool(
        CompFactory.DerivationFramework.PseudoTrackSelector(
            name, **kwargs), primary=True)
    return acc

# Tool for decorating tracks with the outcome of the track selector tool


def InDetTrackSelectionToolWrapperCfg(
        flags, name, CutLevel="TightPrimary", **kwargs):
    """Configure the InDetTrackSelectionToolWrapper"""
    acc = ComponentAccumulator()

    if "TrackSelectionTool" not in kwargs:
        from InDetConfig.InDetTrackSelectionToolConfig import (
            InDetTrackSelectionToolCfg)
        kwargs.setdefault("TrackSelectionTool", acc.popToolsAndMerge(
            InDetTrackSelectionToolCfg(
                flags, name="InDetTrackSelectionTool_"+CutLevel,
                CutLevel=CutLevel)))

    acc.addPublicTool(
        CompFactory.DerivationFramework.InDetTrackSelectionToolWrapper(
            name, **kwargs), primary=True)
    return acc

# Tool for decorating the track and calo isolation


def IsolationTrackDecoratorCfg(flags, name, **kwargs):
    """Confiure the isolation track decorator tool"""
    acc = ComponentAccumulator()
    IsolationTrackDecorator = CompFactory.DerivationFramework.IsolationTrackDecorator
    acc.addPublicTool(IsolationTrackDecorator(name, **kwargs),
                      primary = True)
    return acc

# Tool for thinning TrackParticle containers via string selection


def TrackParticleThinningCfg(flags, name, **kwargs):
    """Configure the TrackParticleThining tool"""
    if flags.Detector.GeometryITk:
        return ITkTrackParticleThinningCfg(flags, name, **kwargs)

    # To produce SCT_DetectorElementCollection
    from SCT_GeoModel.SCT_GeoModelConfig import SCT_ReadoutGeometryCfg
    acc = SCT_ReadoutGeometryCfg(flags)

    acc.addPublicTool(CompFactory.DerivationFramework.TrackParticleThinning(
        name, **kwargs), primary=True)
    return acc

# Tool for thinning PixelClusters via string selection


def PixelClusterThinningCfg(
        flags, name="PixelClusterThinningTool", **kwargs):
    pix_lumi_sel = "PixelClusters.bec == 0 && PixelClusters.layer >= 0 && PixelClusters.sizeZ > 1 && ( PixelClusters.nRDO < PixelClusters.sizeZ + 5 ) && PixelClusters.charge > 30000"
    kwargs.setdefault("SelectionString", pix_lumi_sel)
    acc = ComponentAccumulator()
    acc.addPublicTool(
        CompFactory.DerivationFramework.TrackMeasurementThinning(
            name, **kwargs), primary=True)
    return acc


def IDTIDEThinningToolCfg(flags, name="IDTIDEThinningTool", **kwargs):
    if not flags.Detector.EnablePixel:
        kwargs.setdefault("InDetTrackStatesPixKey", "")
        kwargs.setdefault("InDetTrackMeasurementsPixKey", "")
    if not flags.Detector.EnableSCT:
        kwargs.setdefault("InDetTrackStatesSctKey", "")
        kwargs.setdefault("InDetTrackMeasurementsSctKey", "")
    if not flags.Detector.EnableTRT:
        kwargs.setdefault("InDetTrackStatesTrtKey", "")
        kwargs.setdefault("InDetTrackMeasurementsTrtKey", "")

    kwargs.setdefault("SelectionString", "abs(IDTIDEInDetTrackZ0AtPV) < 5.0")
    # If true, Complains about missing PixelMSOSs
    kwargs.setdefault("ThinHitsOnTrack", False)

    from DerivationFrameworkInDet.InDetToolsConfig import (
        TrackParticleThinningCfg)
    return TrackParticleThinningCfg(flags, name, **kwargs)

def ITkTrackParticleThinningCfg(flags, name, **kwargs):
    """Configure the TrackParticleThining tool"""
    # To produce ITkStripDetectorElementCollection
    from StripGeoModelXml.ITkStripGeoModelConfig import (
        ITkStripReadoutGeometryCfg)
    acc = ITkStripReadoutGeometryCfg(flags)

    kwargs.setdefault("InDetTrackStatesPixKey", "ITkPixelMSOSs")
    kwargs.setdefault("InDetTrackMeasurementsPixKey", "ITkPixelMeasurements")
    kwargs.setdefault("InDetTrackStatesSctKey", "ITkStripMSOSs")
    kwargs.setdefault("InDetTrackMeasurementsSctKey", "ITkStripMeasurements")
    kwargs.setdefault("InDetTrackStatesTrtKey", "")
    kwargs.setdefault("InDetTrackMeasurementsTrtKey", "")
    kwargs.setdefault("SCTDetEleCollKey", "ITkStripDetectorElementCollection")

    acc.addPublicTool(CompFactory.DerivationFramework.TrackParticleThinning(
        name, **kwargs), primary=True)
    return acc

def ITkTIDEThinningToolCfg(flags, name="ITkTIDEThinningTool", **kwargs):
    if not flags.Detector.EnableITkPixel:
        kwargs.setdefault("InDetTrackStatesPixKey", "")
        kwargs.setdefault("InDetTrackMeasurementsPixKey", "")
    if not flags.Detector.EnableITkStrip:
        kwargs.setdefault("InDetTrackStatesSctKey", "")
        kwargs.setdefault("InDetTrackMeasurementsSctKey", "")

    kwargs.setdefault("SelectionString", "abs(IDTIDEInDetTrackZ0AtPV) < 5.0")
    # If true, Complains about missing PixelMSOSs
    kwargs.setdefault("ThinHitsOnTrack", False)

    from DerivationFrameworkInDet.InDetToolsConfig import (
        ITkTrackParticleThinningCfg)
    return ITkTrackParticleThinningCfg(flags, name, **kwargs)

# Tool for thinning TrackParticles that aren't associated with muons


def MuonTrackParticleThinningCfg(flags, name, **kwargs):
    """Configure the MuonTrackParticleThinning tool"""
    acc = ComponentAccumulator()
    acc.addPublicTool(
        CompFactory.DerivationFramework.MuonTrackParticleThinning(
            name, **kwargs), primary=True)
    return acc

# Tool for thinning TrackParticles that aren't associated with taus


def TauTrackParticleThinningCfg(flags, name, **kwargs):
    """Configure the TauTrackParticleThinning tool"""
    acc = ComponentAccumulator()
    acc.addPublicTool(CompFactory.DerivationFramework.TauTrackParticleThinning(
        name, **kwargs), primary=True)
    return acc

# Tool for thinning TrackParticles that aren't associated high-pt di-taus


def DiTauTrackParticleThinningCfg(flags, name, **kwargs):
    """Configure the DiTauTrackParticleThinning tool"""
    acc = ComponentAccumulator()
    acc.addPublicTool(
        CompFactory.DerivationFramework.DiTauTrackParticleThinning(
            name, **kwargs), primary=True)
    return acc

# Tool for thinning TrackParticles that are associated with jets


def JetTrackParticleThinningCfg(flags, name, **kwargs):
    """Configure the JetTrackParticleThinning tool"""
    acc = ComponentAccumulator()
    acc.addPublicTool(CompFactory.DerivationFramework.JetTrackParticleThinning(
        name, **kwargs), primary=True)
    return acc


def UFOTrackParticleThinningCfg(flags, name, **kwargs):
    """Configure the UFOTrackParticleThinning tool"""
    acc = ComponentAccumulator()
    acc.addPublicTool(CompFactory.DerivationFramework.UFOTrackParticleThinning(
        name, **kwargs), primary=True)
    return acc


def JetConstituentThinningCfg(flags, name, **kwargs):
    """Configure the JetConstituentThinning tool"""
    acc = ComponentAccumulator()
    acc.addPublicTool(CompFactory.DerivationFramework.JetConstituentThinning(
        name, **kwargs), primary=True)
    return acc


def JetGhostThinningCfg(flags, name, **kwargs):
    """Configure the JetGhostThinning tool"""
    acc = ComponentAccumulator()
    acc.addPublicTool(CompFactory.DerivationFramework.JetGhostThinning(
        name, **kwargs), primary=True)
    return acc


def TauJetLepRMParticleThinningCfg(flags, name, **kwargs):
    """Configure the DiTauTrackParticleThinning tool"""
    acc = ComponentAccumulator()
    acc.addPublicTool(
        CompFactory.DerivationFramework.TauJets_LepRMParticleThinning(
            name, **kwargs), primary=True)
    return acc

# Tool for thinning TrackParticles that aren't associated with egamma objects


def EgammaTrackParticleThinningCfg(flags, name, **kwargs):
    """Configure the EgammaTrackParticleThinning tool"""
    acc = ComponentAccumulator()
    acc.addPublicTool(
        CompFactory.DerivationFramework.EgammaTrackParticleThinning(
            name, **kwargs), primary=True)
    return acc

# Track to vertex wrapper


def TrackToVertexWrapperCfg(flags, name, **kwargs):
    """Configure the TrackToVertexWrapper tool"""
    acc = ComponentAccumulator()

    if "TrackToVertexIPEstimator" not in kwargs:
        from TrkConfig.TrkVertexFitterUtilsConfig import (
            TrackToVertexIPEstimatorCfg)
        kwargs.setdefault("TrackToVertexIPEstimator", acc.popToolsAndMerge(
            TrackToVertexIPEstimatorCfg(flags)))

    kwargs.setdefault("ContainerName", "InDetTrackParticles")

    acc.addPublicTool(CompFactory.DerivationFramework.TrackToVertexWrapper(
        name, **kwargs), primary=True)
    return acc


def IDTIDETruthThinningToolCfg(flags, name="IDTIDETruthThinningTool", **kwargs):
    kwargs.setdefault("WritePartons", True)
    kwargs.setdefault("WriteHadrons", True)
    kwargs.setdefault("WriteBHadrons", True)
    kwargs.setdefault("WriteGeant", True)
    kwargs.setdefault("GeantPhotonPtThresh", 20000)
    kwargs.setdefault("WriteTauHad", True)
    kwargs.setdefault("PartonPtThresh", -1.0)
    kwargs.setdefault("WriteBSM", True)
    kwargs.setdefault("WriteBosons", True)
    kwargs.setdefault("WriteBosonProducts", True)
    kwargs.setdefault("WriteBSMProducts", True)
    kwargs.setdefault("WriteTopAndDecays", True)
    kwargs.setdefault("WriteEverything", True)
    kwargs.setdefault("WriteAllLeptons", True)
    kwargs.setdefault("WriteLeptonsNotFromHadrons", True)
    kwargs.setdefault("WriteNotPhysical", True)
    kwargs.setdefault("WriteFirstN", -1)
    kwargs.setdefault("PreserveAncestors", True)
    kwargs.setdefault("PreserveGeneratorDescendants", True)

    from DerivationFrameworkMCTruth.TruthDerivationToolsConfig import (
        MenuTruthThinningCfg)
    return MenuTruthThinningCfg(flags, name, **kwargs)

def IDTRKVALIDTruthThinningToolCfg(
        flags, name="IDTRKVALIDTruthThinningTool", **kwargs):

    kwargs.setdefault("WriteEverything", True)
    kwargs.setdefault("WriteFirstN", -1)
    kwargs.setdefault("PreserveAncestors", True)
    kwargs.setdefault("PreserveGeneratorDescendants", True)

    from DerivationFrameworkMCTruth.TruthDerivationToolsConfig import (
        MenuTruthThinningCfg)
    return MenuTruthThinningCfg(flags, name, **kwargs)

def EventInfoBSErrDecoratorCfg(
        flags, name="EventInfoBSErrDecorator", **kwargs):
    acc = ComponentAccumulator()
    acc.addPublicTool(CompFactory.DerivationFramework.EventInfoBSErrDecorator(
        name, **kwargs), primary=True)
    return acc

def UnassociatedHitsGetterToolCfg(
        flags, name="UnassociatedHitsGetterTool", **kwargs):
    acc = ComponentAccumulator()

    if "AssociationTool" not in kwargs:
        from InDetConfig.InDetAssociationToolsConfig import (
            InDetPRDtoTrackMapToolGangedPixelsCfg)
        kwargs.setdefault("AssociationTool", acc.popToolsAndMerge(
            InDetPRDtoTrackMapToolGangedPixelsCfg(flags)))

    acc.setPrivateTools(
        CompFactory.DerivationFramework.UnassociatedHitsGetterTool(name, **kwargs))
    return acc

def UnassociatedHitsDecoratorCfg(
        flags, name="UnassociatedHitsDecorator", **kwargs):
    acc = ComponentAccumulator()

    if "UnassociatedHitsGetter" not in kwargs:
        from DerivationFrameworkInDet.InDetToolsConfig import (
            UnassociatedHitsGetterToolCfg)
        kwargs.setdefault("UnassociatedHitsGetter", acc.popToolsAndMerge(
            UnassociatedHitsGetterToolCfg(flags)))

    acc.addPublicTool(CompFactory.DerivationFramework.UnassociatedHitsDecorator(
        name, **kwargs), primary=True)
    return acc
