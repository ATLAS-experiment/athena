# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.Enums import BeamType, Format
from TrkConfig.TrackingPassFlags import printActiveConfig, printPrimaryConfig

_flags_set = []  # For caching
_extensions_list = [] # For caching

def CombinedTrackingPassFlagSets(flags):

    global _flags_set
    if _flags_set:
        return _flags_set

    flags_set = []

    # Primary Pass
    flags = flags.cloneAndReplace(
        "Tracking.ActiveConfig",
        f"Tracking.{flags.Tracking.PrimaryPassConfig.value}Pass",
        # Keep original flags as some of the subsequent passes use
        # lambda functions relying on them
        keepOriginal=True)
    flags_set += [flags]

    # LRT pass
    if flags.Tracking.doLargeD0 or flags.Tracking.doLowPtLargeD0:

        if flags.Tracking.doLowPtLargeD0:
            flagsLRT = flags.cloneAndReplace("Tracking.ActiveConfig",
                                             "Tracking.LowPtLargeD0Pass")
        elif flags.Tracking.doLargeD0:
            flagsLRT = flags.cloneAndReplace("Tracking.ActiveConfig",
                                             "Tracking.R3LargeD0Pass")

        flags_set += [flagsLRT]

    # LowPtRoI pass (low-pt tracks in high pile-up enviroment)
    if flags.Tracking.doLowPtRoI:
        flagsLowPtRoI = flags.cloneAndReplace("Tracking.ActiveConfig",
                                              "Tracking.LowPtRoIPass")
        flags_set += [flagsLowPtRoI]

    # LowPt pass
    if flags.Tracking.doLowPt:
        flagsLowPt = flags.cloneAndReplace("Tracking.ActiveConfig",
                                           "Tracking.LowPtPass")
        flags_set += [flagsLowPt]

    # VeryLowPt pass
    if flags.Tracking.doVeryLowPt:
        flagsVeryLowPt = flags.cloneAndReplace("Tracking.ActiveConfig",
                                               "Tracking.VeryLowPtPass")
        flags_set += [flagsVeryLowPt]

    # TRT standalone pass
    if flags.Tracking.doTRTStandalone:
        flagsTRTStandalone = flags.cloneAndReplace("Tracking.ActiveConfig",
                                                   "Tracking.TRTStandalonePass")
        flags_set += [flagsTRTStandalone]

    # Forward tracklets
    if flags.Tracking.doForwardTracks:
        flagsForward = flags.cloneAndReplace("Tracking.ActiveConfig",
                                             "Tracking.ForwardPass")
        flags_set += [flagsForward]

    # Disappearing pixel tracklets
    if flags.Tracking.doTrackSegmentsDisappearing:
        flagsDisappearing = flags.cloneAndReplace("Tracking.ActiveConfig",
                                                  "Tracking.DisappearingPass")
        flags_set += [flagsDisappearing]

    # Beam gas
    if flags.Tracking.doBeamGas:
        flagsBeamGas = flags.cloneAndReplace("Tracking.ActiveConfig",
                                             "Tracking.BeamGasPass")
        flags_set += [flagsBeamGas]

    _flags_set = flags_set  # Put into cache

    return flags_set


def ClusterSplitProbabilityContainerName(flags):
    if flags.Detector.GeometryITk:
        from InDetConfig.ITkTrackRecoConfig import (
            ITkClusterSplitProbabilityContainerName)
        return ITkClusterSplitProbabilityContainerName(flags)

    flags_set = CombinedTrackingPassFlagSets(flags)
    extension = flags_set[-1].Tracking.ActiveConfig.extension

    # No ambi processing for TRT standalone, so pick the previous pass
    if extension == "TRTStandalone":
        extension = flags_set[-2].Tracking.ActiveConfig.extension

    ClusterSplitProbContainer = "InDetAmbiguityProcessorSplitProb" + extension

    # Only primary pass + back-tracking
    if len(flags_set) == 1 and flags.Tracking.doBackTracking:
        ClusterSplitProbContainer = (
            "InDetTRT_SeededAmbiguityProcessorSplitProb" + extension)
    return ClusterSplitProbContainer

# TODO find config to validate this


def InDetCosmicsTrackRecoPreProcessingCfg(flags):
    result = ComponentAccumulator()

    from InDetConfig.TrackingSiPatternConfig import TrackingSiPatternCfg
    result.merge(TrackingSiPatternCfg(
        flags,
        InputCollections=[],
        ResolvedTrackCollectionKey="ResolvedTracks",
        SiSPSeededTrackCollectionKey="SiSPSeededTracks"))

    if flags.Detector.EnableTRT:
        from InDetConfig.TRTExtensionConfig import NewTrackingTRTExtensionPhaseCfg
        result.merge(NewTrackingTRTExtensionPhaseCfg(
            flags,
            SiTrackCollection="ResolvedTracks",
            ExtendedTrackCollection="ExtendedTracksPhase",
            ExtendedTracksMap="ExtendedTracksMapPhase"))

        from InDetConfig.TRTSegmentFindingConfig import TRTSegmentFinding_Phase_Cfg
        result.merge(TRTSegmentFinding_Phase_Cfg(flags))

        from InDetConfig.InDetTrackPRD_AssociationConfig import (
            InDetTrackPRD_AssociationCfg)
        result.merge(InDetTrackPRD_AssociationCfg(
            flags, name='InDetTRTonly_TrackPRD_AssociationPhase',
            AssociationMapName='InDetTRTonly_PRDtoTrackMapPhase',
            TracksName=[]))

        from InDetConfig.TRT_SegmentsToTrackConfig import (
            TRT_Cosmics_SegmentsToTrackCfg)
        result.merge(TRT_Cosmics_SegmentsToTrackCfg(
            flags, name='InDetTRT_Cosmics_SegmentsToTrack_Phase',
            InputSegmentsCollection="TRTSegments_Phase",
            OutputTrackCollection="TRT_Tracks_Phase"))

        from InDetConfig.InDetCosmicsEventPhaseConfig import (
            InDetCosmicsEventPhaseCfg)
        result.merge(InDetCosmicsEventPhaseCfg(
            flags,
            InputTracksNames=["TRT_Tracks_Phase"]))

        from InDetConfig.InDetPrepRawDataFormationConfig import (
            InDetTRT_Phase_RIO_MakerCfg)
        result.merge(InDetTRT_Phase_RIO_MakerCfg(flags))

    return result


def InDetPreProcessingCfg(flags):
    result = ComponentAccumulator()

    # Detector ByteStream pre-processing
    if flags.Input.Format is Format.BS:
        if flags.Detector.EnablePixel:
            from PixelRawDataByteStreamCnv.PixelRawDataByteStreamCnvConfig import (
                PixelRawDataProviderAlgCfg)
            result.merge(PixelRawDataProviderAlgCfg(flags))

        if flags.Detector.EnableSCT:
            from SCT_RawDataByteStreamCnv.SCT_RawDataByteStreamCnvConfig import (
                SCTRawDataProviderCfg, SCTEventFlagWriterCfg)
            result.merge(SCTRawDataProviderCfg(flags))
            result.merge(SCTEventFlagWriterCfg(flags))

        if flags.Detector.EnableTRT:
            from TRT_RawDataByteStreamCnv.TRT_RawDataByteStreamCnvConfig import (
                TRTRawDataProviderCfg)
            result.merge(TRTRawDataProviderCfg(flags))

    if flags.Detector.EnablePixel or flags.Detector.EnableSCT:
        from InDetConfig.SiliconPreProcessing import InDetRecPreProcessingSiliconCfg
        result.merge(InDetRecPreProcessingSiliconCfg(flags))
    if flags.Detector.EnableTRT:
        from InDetConfig.TRTPreProcessing import TRTPreProcessingCfg
        result.merge(TRTPreProcessingCfg(flags))

    return result


# Returns CA + ClusterSplitProbContainer
def SiSubDetTrackRecoCfg(flags, detector="",
                         extensions_list=None,
                         ClusterSplitProbContainer=""):
    result = ComponentAccumulator()

    if extensions_list is None:
        extensions_list = []

    flagsDet = flags.cloneAndReplace("Tracking.ActiveConfig",
                                     f"Tracking.{detector}Pass")
    TrackContainer = f"Resolved{detector}Tracks"
    extensions_list.append(flagsDet.Tracking.ActiveConfig.extension)
    printActiveConfig(flagsDet)

    from InDetConfig.TrackingSiPatternConfig import TrackingSiPatternCfg
    result.merge(TrackingSiPatternCfg(
        flagsDet,
        InputCollections=[],
        ResolvedTrackCollectionKey=TrackContainer,
        SiSPSeededTrackCollectionKey=f"SiSPSeeded{detector}Tracks",
        ClusterSplitProbContainer=ClusterSplitProbContainer))

    ClusterSplitProbContainer = (
        "InDetAmbiguityProcessorSplitProb" +
        flagsDet.Tracking.ActiveConfig.extension)

    if flags.Tracking.doTruth:
        from InDetConfig.TrackTruthConfig import InDetTrackTruthCfg
        result.merge(InDetTrackTruthCfg(
            flagsDet,
            Tracks=TrackContainer,
            DetailedTruth=TrackContainer+"DetailedTruth",
            TracksTruth=TrackContainer+"TruthCollection"))

    from xAODTrackingCnv.xAODTrackingCnvConfig import (
        TrackParticleCnvAlgPIDCheckCfg)
    result.merge(TrackParticleCnvAlgPIDCheckCfg(
        flagsDet,
        name=TrackContainer+"CnvAlg",
        TrackContainerName=TrackContainer,
        xAODTrackParticlesFromTracksContainerName=(
            f"InDet{detector}TrackParticles")))

    return result, ClusterSplitProbContainer


def TRTTrackRecoCfg(flags, extensions_list=None):
    result = ComponentAccumulator()

    if extensions_list is None:
        extensions_list = []

    flagsTRT = flags.cloneAndReplace("Tracking.ActiveConfig",
                                     "Tracking.TRTPass")
    extensions_list.append(flagsTRT.Tracking.ActiveConfig.extension)
    printActiveConfig(flagsTRT)

    from InDetConfig.TRTSegmentFindingConfig import (
        TRTSegmentFinding_TrackSegments_Cfg)
    result.merge(TRTSegmentFinding_TrackSegments_Cfg(flagsTRT))

    from InDetConfig.TRTStandaloneConfig import TRT_TrackSegment_Cfg
    result.merge(TRT_TrackSegment_Cfg(flagsTRT))

    return result


def TRTStandalonePassRecoCfg(flags,
                             InputCombinedInDetTracks=None,
                             InputExtendedInDetTracks=None,
                             StatTrackCollections=None,
                             StatTrackTruthCollections=None):
    result = ComponentAccumulator()

    if InputCombinedInDetTracks is None:
        InputCombinedInDetTracks = []
    if InputExtendedInDetTracks is None:
        InputExtendedInDetTracks = []
    if StatTrackCollections is None:
        StatTrackCollections = []
    if StatTrackTruthCollections is None:
        StatTrackTruthCollections = []

    from InDetConfig.TRTStandaloneConfig import TRTStandaloneCfg
    result.merge(TRTStandaloneCfg(flags,
                                  InputCollections=InputCombinedInDetTracks))

    TRTTrackContainer = "TRTStandaloneTracks"
    InputCombinedInDetTracks += [TRTTrackContainer]
    InputExtendedInDetTracks += [TRTTrackContainer]
    StatTrackCollections += [TRTTrackContainer]
    StatTrackTruthCollections += [TRTTrackContainer+"TruthCollection"]

    if flags.Tracking.doTrackSegmentsTRT:
        from xAODTrackingCnv.xAODTrackingCnvConfig import (
            TrackParticleCnvAlgNoPIDCfg)
        result.merge(TrackParticleCnvAlgNoPIDCfg(
            flags, TRTTrackContainer+"CnvAlg",
            TrackContainerName=TRTTrackContainer,
            xAODTrackParticlesFromTracksContainerName=(
                "InDetTRTTrackParticles")))

    return result


# Return CA + TrackContainer
def StoreTrackSeparateContainerCfg(flags, TrackContainer="",
                                   ClusterSplitProbContainer=""):
    result = ComponentAccumulator()
    doTrackOverlay = getattr(flags.TrackOverlay, "ActiveConfig.doTrackOverlay", None) or flags.Overlay.doTrackOverlay

    # Dummy Merger to fill additional info
    # for PRD-associated pixel tracklets
    # Can also run on all separate collections like R3LargeD0
    # but kept consistent with legacy config

    AssociationMapName = ""
    extension = flags.Tracking.ActiveConfig.extension

    if extension == "Disappearing" or doTrackOverlay:
        if extension == "Disappearing":
            InputTracks = [TrackContainer]
            if doTrackOverlay:
                InputTracks += [flags.Overlay.BkgPrefix +
                                extension + "Tracks"]
            TrackContainer = extension+"Tracks"
            AssociationMapName = "PRDtoTrackMap" + TrackContainer
            MergerOutputTracks = TrackContainer
        elif doTrackOverlay:
            # schedule merger to combine signal and background tracks
            InputTracks = [flags.Overlay.SigPrefix+TrackContainer,
                           flags.Overlay.BkgPrefix+TrackContainer]
            AssociationMapName = ("PRDtoTrackMapResolved" +
                                  extension + "Tracks")
            MergerOutputTracks = TrackContainer

        from TrkConfig.TrkTrackCollectionMergerConfig import (
            TrackCollectionMergerAlgCfg)
        result.merge(TrackCollectionMergerAlgCfg(
            flags,
            name = "TrackCollectionMergerAlgCfg"+extension,
            InputCombinedTracks=InputTracks,
            OutputCombinedTracks=MergerOutputTracks,
            AssociationMapName=AssociationMapName))

    if flags.Tracking.doTruth:
        from InDetConfig.TrackTruthConfig import InDetTrackTruthCfg
        result.merge(InDetTrackTruthCfg(
            flags,
            Tracks=TrackContainer,
            DetailedTruth=TrackContainer+"DetailedTruth",
            TracksTruth=TrackContainer+"TruthCollection"))

    # Need specific handling with TrackParticles for R3LargeD0
    # not to break downstream configs
    xAODTrackParticlesName = (
        "InDetLargeD0TrackParticles" if "LargeD0" in extension else
        "InDet" + extension + "TrackParticles")

    from xAODTrackingCnv.xAODTrackingCnvConfig import (
        TrackParticleCnvAlgPIDCheckCfg)
    result.merge(TrackParticleCnvAlgPIDCheckCfg(
        flags,
        name = extension + "TrackParticleCnvAlg",
        TrackContainerName=TrackContainer,
        xAODTrackParticlesFromTracksContainerName=xAODTrackParticlesName,
        ClusterSplitProbabilityName=ClusterSplitProbContainer,
        AssociationMapName=AssociationMapName))

    return result, TrackContainer


# Returns CA + ClusterSplitProbContainer
def BackTrackingRecoCfg(flags,
                        InputCombinedInDetTracks=None,
                        InputExtendedInDetTracks=None,
                        StatTrackCollections=None,
                        StatTrackTruthCollections=None,
                        ClusterSplitProbContainer=""):
    result = ComponentAccumulator()

    if InputCombinedInDetTracks is None:
        InputCombinedInDetTracks = []
    if InputExtendedInDetTracks is None:
        InputExtendedInDetTracks = []
    if StatTrackCollections is None:
        StatTrackCollections = []
    if StatTrackTruthCollections is None:
        StatTrackTruthCollections = []

    from InDetConfig.BackTrackingConfig import BackTrackingCfg
    result.merge(BackTrackingCfg(
        flags,
        InputCollections=InputCombinedInDetTracks,
        ClusterSplitProbContainer=ClusterSplitProbContainer))

    ClusterSplitProbContainer = (
        "InDetTRT_SeededAmbiguityProcessorSplitProb" +
        flags.Tracking.ActiveConfig.extension)
    TRTSeededTracks = "TRTSeededTracks"
    ResolvedTRTSeededTracks = "ResolvedTRTSeededTracks"
    InputCombinedInDetTracks += [ResolvedTRTSeededTracks]
    InputExtendedInDetTracks += [ResolvedTRTSeededTracks]
    StatTrackCollections += [TRTSeededTracks,
                             ResolvedTRTSeededTracks]
    StatTrackTruthCollections += [TRTSeededTracks+"TruthCollection",
                                  ResolvedTRTSeededTracks+"TruthCollection"]

    return result, ClusterSplitProbContainer


# Returns CA + ClusterSplitProbContainer
def TrackRecoPassCfg(flags, extension="",
                     doTrackingSiPattern=True,
                     InputCombinedInDetTracks=None,
                     InputExtendedInDetTracks=None,
                     StatTrackCollections=None,
                     StatTrackTruthCollections=None,
                     ClusterSplitProbContainer=""):
    result = ComponentAccumulator()
    doTrackOverlay = getattr(flags.TrackOverlay, "ActiveConfig.doTrackOverlay", None) or flags.Overlay.doTrackOverlay
    if InputCombinedInDetTracks is None:
        InputCombinedInDetTracks = []
    if InputExtendedInDetTracks is None:
        InputExtendedInDetTracks = []
    if StatTrackCollections is None:
        StatTrackCollections = []
    if StatTrackTruthCollections is None:
        StatTrackTruthCollections = []

    ResolvedTracks = "Resolved" + extension + "Tracks"

    # for track overlay, save resolved track name
    # for final merged track collection
    if (doTrackOverlay and
        flags.Tracking.ActiveConfig.storeSeparateContainer and
        not flags.Tracking.ActiveConfig.useTRTExtension):
        ResolvedTracks = flags.Overlay.SigPrefix + ResolvedTracks

    # Tweak to match old config key
    if "LargeD0" in extension:
        ResolvedTracks = "ResolvedLargeD0Tracks"

    # ---------------------------------------
    # --- Si pattern, if not done in the cosmic preprocessing
    # ---------------------------------------

    if doTrackingSiPattern:
        SiSPSeededTracks = "SiSPSeeded" + extension + "Tracks"
        from InDetConfig.TrackingSiPatternConfig import TrackingSiPatternCfg
        result.merge(TrackingSiPatternCfg(
            flags,
            InputCollections=InputExtendedInDetTracks,
            ResolvedTrackCollectionKey=ResolvedTracks,
            SiSPSeededTrackCollectionKey=SiSPSeededTracks,
            ClusterSplitProbContainer=ClusterSplitProbContainer))
        StatTrackCollections += [SiSPSeededTracks, ResolvedTracks]
        StatTrackTruthCollections += [SiSPSeededTracks+"TruthCollection",
                                      ResolvedTracks+"TruthCollection"]

    TrackContainer = ResolvedTracks
    if (doTrackOverlay and
        flags.Tracking.ActiveConfig.storeSeparateContainer):
        TrackContainer = "Resolved" + extension + "Tracks"

    # ---------------------------------------
    # --- TRT extension
    # ---------------------------------------

    if flags.Tracking.ActiveConfig.useTRTExtension:
        ExtendedTracks = "Extended" + extension + "Tracks"
        # Tweaks to match old config key
        if extension == "Disappearing":
            ExtendedTracks = "ExtendedTracksDisappearing"
        elif "LargeD0" in extension:
            ExtendedTracks = "ExtendedLargeD0Tracks"
            if doTrackOverlay:
                ExtendedTracks = flags.Overlay.SigPrefix+"ExtendedLargeD0Tracks"
        ExtendedTracksMap = "ExtendedTracksMap" + extension

        from InDetConfig.TRTExtensionConfig import NewTrackingTRTExtensionCfg
        result.merge(NewTrackingTRTExtensionCfg(
            flags,
            SiTrackCollection=ResolvedTracks,
            ExtendedTrackCollection=ExtendedTracks,
            ExtendedTracksMap=ExtendedTracksMap))

        TrackContainer = ExtendedTracks
        if doTrackOverlay and "LargeD0" in extension:
            TrackContainer = "ExtendedLargeD0Tracks"
        StatTrackCollections += [ExtendedTracks]
        StatTrackTruthCollections += [ExtendedTracks+"TruthCollection"]

    # ---------------------------------------
    # --- Store separate container if needed
    # ---------------------------------------

    if flags.Tracking.ActiveConfig.storeSeparateContainer:
        acc, TrackContainer = StoreTrackSeparateContainerCfg(
            flags,
            TrackContainer=TrackContainer,
            ClusterSplitProbContainer=ClusterSplitProbContainer)
        result.merge(acc)

    else:  # Do not store separate track container
        ClusterSplitProbContainer = (
            "InDetAmbiguityProcessorSplitProb" +
            flags.Tracking.ActiveConfig.extension)
        InputCombinedInDetTracks += [TrackContainer]

    InputExtendedInDetTracks += [TrackContainer]

    return result, ClusterSplitProbContainer


def TrackFinalCfg(flags,
                  InputCombinedInDetTracks=None,
                  StatTrackCollections=None,
                  StatTrackTruthCollections=None):
    result = ComponentAccumulator()
    doTrackOverlay = getattr(flags.TrackOverlay, "ActiveConfig.doTrackOverlay", None) or flags.Overlay.doTrackOverlay

    if InputCombinedInDetTracks is None:
        InputCombinedInDetTracks = []
    if StatTrackCollections is None:
        StatTrackCollections = []
    if StatTrackTruthCollections is None:
        StatTrackTruthCollections = []

    if doTrackOverlay:
        InputCombinedInDetTracks += [flags.Overlay.BkgPrefix +
                                     "CombinedInDetTracks"]

    TrackContainer = "CombinedInDetTracks"

    from TrkConfig.TrkTrackCollectionMergerConfig import (
        TrackCollectionMergerAlgCfg)
    result.merge(TrackCollectionMergerAlgCfg(
        flags,
        InputCombinedTracks=InputCombinedInDetTracks,
        OutputCombinedTracks=TrackContainer,
        AssociationMapName=f"PRDtoTrackMap{TrackContainer}"))

    if flags.Tracking.doTruth:
        from InDetConfig.TrackTruthConfig import InDetTrackTruthCfg
        result.merge(InDetTrackTruthCfg(
            flags,
            Tracks=TrackContainer,
            DetailedTruth=f"{TrackContainer}DetailedTruth",
            TracksTruth=f"{TrackContainer}TruthCollection"))

    StatTrackCollections += [TrackContainer]
    StatTrackTruthCollections += [f"{TrackContainer}TruthCollection"]

    if flags.Tracking.doSlimming:
        from TrkConfig.TrkTrackSlimmerConfig import TrackSlimmerCfg
        result.merge(TrackSlimmerCfg(
            flags,
            TrackLocation=[TrackContainer]))

    if flags.Tracking.perigeeExpression == "BeamLine":
        from xAODTrackingCnv.xAODTrackingCnvConfig import TrackParticleCnvAlgCfg
        result.merge(TrackParticleCnvAlgCfg(
            flags,
            ClusterSplitProbabilityName=ClusterSplitProbabilityContainerName(
                flags),
            AssociationMapName=f"PRDtoTrackMap{TrackContainer}"))

    return result


def PseudoTrackFinalCfg(flags):
    result = ComponentAccumulator()

    TrackContainer = "InDetPseudoTracks"
    if flags.Tracking.doTruth:
        from InDetConfig.TrackTruthConfig import InDetTrackTruthCfg
        result.merge(InDetTrackTruthCfg(
            flags,
            Tracks=TrackContainer,
            DetailedTruth=TrackContainer + "DetailedTruth",
            TracksTruth=TrackContainer + "TruthCollection"))

    from xAODTrackingCnv.xAODTrackingCnvConfig import TrackParticleCnvAlgCfg
    result.merge(TrackParticleCnvAlgCfg(
        flags,
        name="PseudoTrackParticleCnvAlg",
        TrackContainerName=TrackContainer,
        xAODTrackParticlesFromTracksContainerName=(
            "InDetPseudoTrackParticles"),
        AssociationMapName=(
            "PRDtoTrackMapCombinedInDetTracks")))

    return result


def ObserverTrackFinalCfg(flags):
    result = ComponentAccumulator()

    TrackContainer = "ObservedTracksCollection"
    if flags.Tracking.doTruth:
        from InDetConfig.TrackTruthConfig import InDetTrackTruthCfg
        result.merge(InDetTrackTruthCfg(
            flags,
            Tracks=TrackContainer,
            DetailedTruth=TrackContainer + "DetailedTruth",
            TracksTruth=TrackContainer + "TruthCollection"))

    from xAODTrackingCnv.xAODTrackingCnvConfig import (
        ObserverTrackParticleCnvAlgCfg)
    result.merge(ObserverTrackParticleCnvAlgCfg(
        flags,
        AssociationMapName = (
            "PRDtoTrackMapCombinedInDetTracks")))

    return result


def TrackSeedsFinalCfg(flags):
    result = ComponentAccumulator()

    # get list of extensions requesting track seeds.
    # Add always the Primary Pass.
    listOfExtensionsRequesting = [
        e for e in _extensions_list
        if (e == '' or flags.Tracking[f"{e}Pass"].storeTrackSeeds) ]

    for extension in listOfExtensionsRequesting:
        TrackContainer = "SiSPSeedSegments"+extension

        if flags.Tracking.doTruth:
            from InDetConfig.TrackTruthConfig import InDetTrackTruthCfg
            result.merge(InDetTrackTruthCfg(
                flags,
                Tracks=TrackContainer,
                DetailedTruth=f"{TrackContainer}DetailedTruth",
                TracksTruth=f"{TrackContainer}TruthCollection"))

        from xAODTrackingCnv.xAODTrackingCnvConfig import (
            TrackParticleCnvAlgNoPIDCfg)
        result.merge(TrackParticleCnvAlgNoPIDCfg(
            flags,
            name=f"{TrackContainer}CnvAlg",
            TrackContainerName=TrackContainer,
            xAODTrackParticlesFromTracksContainerName=(
                f"{TrackContainer}TrackParticles")))

    return result


def SiSPSeededTracksFinalCfg(flags):
    result = ComponentAccumulator()

    primaryPassExtension = flags.Tracking[f"{flags.Tracking.PrimaryPassConfig.value}Pass"].extension

    # get list of extensions requesting track candidates.
    # Add always the Primary Pass.
    listOfExtensionsRequesting = [
        e for e in _extensions_list
        if (e=='' or flags.Tracking[f"{e}Pass"].storeSiSPSeededTracks) ]

    for extension in listOfExtensionsRequesting:
        AssociationMapNameKey="PRDtoTrackMapCombinedInDetTracks"
        if extension=='Disappearing':
            AssociationMapNameKey = "PRDtoTrackMapDisappearingTracks"
        elif not (extension == primaryPassExtension):
            AssociationMapNameKey = f"InDetPRDtoTrackMap{extension}"

        from xAODTrackingCnv.xAODTrackingCnvConfig import (
            TrackParticleCnvAlgNoPIDCfg)
        result.merge(TrackParticleCnvAlgNoPIDCfg(
            flags,
            name = f"SiSPSeededTracks{extension}CnvAlg",
            TrackContainerName = f"SiSPSeeded{extension}Tracks",
            xAODTrackParticlesFromTracksContainerName=(
                f"SiSPSeededTracks{extension}TrackParticles"),
            AssociationMapName=AssociationMapNameKey))

    return result


def InDetStatsCfg(flags, StatTrackCollections=None,
                  StatTrackTruthCollections=None):
    result = ComponentAccumulator()

    from InDetConfig.InDetRecStatisticsConfig import (
        InDetRecStatisticsAlgCfg)
    result.merge(InDetRecStatisticsAlgCfg(
        flags,
        TrackCollectionKeys=StatTrackCollections,
        TrackTruthCollectionKeys=(
            StatTrackTruthCollections if flags.Tracking.doTruth else [])))

    if flags.Tracking.doTruth:
        from InDetConfig.InDetTrackClusterAssValidationConfig import (
            InDetTrackClusterAssValidationCfg)
        result.merge(InDetTrackClusterAssValidationCfg(
            flags,
            TracksLocation=StatTrackCollections))

    return result


def ExtendedPRDInfoCfg(flags):
    result = ComponentAccumulator()

    if (flags.Tracking.doTIDE_AmbiTrackMonitoring or
        flags.Tracking.doPseudoTracking):
        from InDetConfig.InDetPrepRawDataToxAODConfig import (
            InDetPixelPrepDataToxAOD_ExtraTruthCfg as PixelPrepDataToxAODCfg,
            InDetSCT_PrepDataToxAOD_ExtraTruthCfg as SCT_PrepDataToxAODCfg,
            InDetTRT_PrepDataToxAOD_ExtraTruthCfg as TRT_PrepDataToxAODCfg)
    else:
        from InDetConfig.InDetPrepRawDataToxAODConfig import (
            InDetPixelPrepDataToxAODCfg as PixelPrepDataToxAODCfg,
            InDetSCT_PrepDataToxAODCfg as SCT_PrepDataToxAODCfg,
            InDetTRT_PrepDataToxAODCfg as TRT_PrepDataToxAODCfg)

    if flags.Tracking.writeExtendedSi_PRDInfo:
        result.merge(PixelPrepDataToxAODCfg(
            flags,
            ClusterSplitProbabilityName=(
                ClusterSplitProbabilityContainerName(flags))))
        result.merge(SCT_PrepDataToxAODCfg(flags))

    if flags.Tracking.writeExtendedTRT_PRDInfo:
        result.merge(TRT_PrepDataToxAODCfg(flags))

    from DerivationFrameworkInDet.InDetToolsConfig import TSOS_CommonKernelCfg
    # Set up one algorithm for each output tracking container    
    # Always done for default pass
    # Done for other passes if pass requests to store track seeds OR track candidates OR requests separate container 
    # Input handling/configuration of algorithm for specific cases is done in TSOS_CommonKernelCfg
    listOfExtensionsRequesting = [
        e for e in _extensions_list if (e == '')
        or (flags.Tracking[f"{e}Pass"].storeTrackSeeds) # Store Si track seeds
        or (flags.Tracking[f"{e}Pass"].storeSiSPSeededTracks) # Store Si candidate tracks
        or (flags.Tracking[f"{e}Pass"].storeSeparateContainer) ] # Particular tracking pass requesting separate container from main pass
    result.merge(TSOS_CommonKernelCfg(
        flags, listOfExtensions = listOfExtensionsRequesting))

    if flags.Tracking.doTIDE_AmbiTrackMonitoring:
        from DerivationFrameworkInDet.InDetToolsConfig import (
            ObserverTSOS_CommonKernelCfg)
        result.merge(ObserverTSOS_CommonKernelCfg(flags))

    if flags.Tracking.doPseudoTracking:
        from DerivationFrameworkInDet.InDetToolsConfig import (
            PseudoTSOS_CommonKernelCfg)
        result.merge(PseudoTSOS_CommonKernelCfg(flags))

    if flags.Tracking.doStoreTrackSeeds:
        from DerivationFrameworkInDet.InDetToolsConfig import (
            SiSPSeedsTSOS_CommonKernelCfg)
        # Setup one algorithm for each output tracking container
        listOfExtensionsRequesting = [
            e for e in _extensions_list if (e == '') or
            flags.Tracking[f"{e}Pass"].storeTrackSeeds ]
        result.merge(SiSPSeedsTSOS_CommonKernelCfg(
            flags, listOfExtensions = listOfExtensionsRequesting))

    if flags.Tracking.doStoreSiSPSeededTracks:
        from DerivationFrameworkInDet.InDetToolsConfig import (
            SiSPTSOS_CommonKernelCfg)
        # Setup one algorithm for each output tracking container
        listOfExtensionsRequesting = [
            e for e in _extensions_list if (e == '') or
            flags.Tracking[f"{e}Pass"].storeSiSPSeededTracks ]
        result.merge(SiSPTSOS_CommonKernelCfg(
            flags, listOfExtensions = listOfExtensionsRequesting))

    if flags.Input.isMC:
        #check if we want to add it for other passes
        listOfExtensionsRequesting = [
            e for e in _extensions_list if (e == '') or
            (flags.Tracking[f"{e}Pass"].storeSiSPSeededTracks and
             flags.Tracking[f"{e}Pass"].storeSeparateContainer) ]
        from InDetPhysValMonitoring.InDetPhysValDecorationConfig import (
            InDetPhysHitDecoratorAlgCfg)
        for extension in listOfExtensionsRequesting:
            result.merge(InDetPhysHitDecoratorAlgCfg(
                flags,
                name=f"InDetPhysHit{extension}DecoratorAlg",
                TrackParticleContainerName=f"InDet{extension}TrackParticles"))

    return result


##############################################################################
#####################     Main ID tracking config       #####################
##############################################################################

def InDetTrackRecoCfg(flags):

    # Bypass to ITk config
    if flags.Detector.GeometryITk:
        from InDetConfig.ITkTrackRecoConfig import ITkTrackRecoCfg
        return ITkTrackRecoCfg(flags)

    """Configures complete ID tracking """
    result = ComponentAccumulator()
    result.merge(InDetPreProcessingCfg(flags))

    ClusterSplitProbContainer = ''

    # ------------------------------------------------------------
    #
    # ----------- Subdetector pattern from New Tracking
    #
    # ------------------------------------------------------------

    # Pixel track segment finding
    if flags.Tracking.doTrackSegmentsPixel:
        acc, ClusterSplitProbContainer = (
            SiSubDetTrackRecoCfg(flags, detector="Pixel",
                                 extensions_list=_extensions_list,
                                 ClusterSplitProbContainer=ClusterSplitProbContainer))
        result.merge(acc)

    # SCT track segment finding
    if flags.Tracking.doTrackSegmentsSCT:
        acc, ClusterSplitProbContainer = (
            SiSubDetTrackRecoCfg(flags, detector="SCT",
                                 extensions_list=_extensions_list,
                                 ClusterSplitProbContainer=ClusterSplitProbContainer))
        result.merge(acc)

    # TRT track segment finding
    if flags.Tracking.doTrackSegmentsTRT:
        result.merge(TRTTrackRecoCfg(flags, extensions_list=_extensions_list))

    flags_set = CombinedTrackingPassFlagSets(flags)

    # Pre-processing for TRT phase in cosmics
    if flags.Beam.Type is BeamType.Cosmics:
        flagsCosmics = flags_set[0]
        result.merge(InDetCosmicsTrackRecoPreProcessingCfg(flagsCosmics))

    # ------------------------------------------------------------
    #
    # ----------- Main passes for standard reconstruction
    #
    # ------------------------------------------------------------

    # Tracks to be ultimately merged in InDetTrackParticle collection
    InputCombinedInDetTracks = []
    # Includes also tracks which end in standalone TrackParticle collections
    InputExtendedInDetTracks = []
    ClusterSplitProbContainer = ""
    StatTrackCollections = []  # To be passed to the InDetRecStatistics alg
    StatTrackTruthCollections = []
    isPrimaryPass = True

    printPrimaryConfig(flags)

    for current_flags in flags_set:
        printActiveConfig(current_flags)

        extension = (
            "" if isPrimaryPass else
            current_flags.Tracking.ActiveConfig.extension)
        _extensions_list.append(extension)

        # ---------------------------------------
        # ----   TRTStandalone pass
        # ---------------------------------------

        if flags.Tracking.doTRTStandalone and extension == "TRTStandalone":
            result.merge(TRTStandalonePassRecoCfg(
                current_flags,
                InputCombinedInDetTracks=InputCombinedInDetTracks,
                InputExtendedInDetTracks=InputExtendedInDetTracks,
                StatTrackCollections=StatTrackCollections,
                StatTrackTruthCollections=StatTrackTruthCollections))

            continue  # Skip rest of config for the TRTStandalone pass

        # ---------------------------------------
        # ----   All the passes but TRTStandalone
        # ---------------------------------------

        acc, ClusterSplitProbContainer = TrackRecoPassCfg(
            current_flags, extension=extension,
            doTrackingSiPattern=not(isPrimaryPass and
                                    flags.Beam.Type is BeamType.Cosmics),
            InputCombinedInDetTracks=InputCombinedInDetTracks,
            InputExtendedInDetTracks=InputExtendedInDetTracks,
            StatTrackCollections=StatTrackCollections,
            StatTrackTruthCollections=StatTrackTruthCollections,
            ClusterSplitProbContainer=ClusterSplitProbContainer)
        result.merge(acc)

        # ---------------------------------------
        # --- A few passes, only after primary pass
        # ---------------------------------------

        if isPrimaryPass:

            # ---------------------------------------
            # --- TRT Segments
            # ---------------------------------------

            if flags.Tracking.doTRTSegments:
                from InDetConfig.TRTSegmentFindingConfig import (
                    TRTSegmentFindingCfg)
                result.merge(TRTSegmentFindingCfg(
                    current_flags,
                    InputCollections=InputCombinedInDetTracks))

            # ---------------------------------------
            # --- BackTracking
            # ---------------------------------------

            if flags.Tracking.doBackTracking:
                acc, ClusterSplitProbContainer = BackTrackingRecoCfg(
                    current_flags,
                    InputCombinedInDetTracks=InputCombinedInDetTracks,
                    InputExtendedInDetTracks=InputExtendedInDetTracks,
                    StatTrackCollections=StatTrackCollections,
                    StatTrackTruthCollections=StatTrackTruthCollections,
                    ClusterSplitProbContainer=ClusterSplitProbContainer)
                result.merge(acc)

            # ---------------------------------------
            # --- PseudoTracking
            # ---------------------------------------

            if (flags.Tracking.doTruth and
                (flags.Tracking.doPseudoTracking or
                 flags.Tracking.doIdealPseudoTracking)):

                from TrkConfig.TrkTruthTrackAlgsConfig import TruthTrackingCfg
                result.merge(TruthTrackingCfg(current_flags))

            isPrimaryPass = False

    # ----------------------------------------------------
    # --- Loop over tracking passes is done, final configs
    # ----------------------------------------------------

    result.merge(
        TrackFinalCfg(flags,
                      InputCombinedInDetTracks=InputCombinedInDetTracks,
                      StatTrackCollections=StatTrackCollections,
                      StatTrackTruthCollections=StatTrackTruthCollections))

    if (flags.Tracking.doPseudoTracking or
        flags.Tracking.doIdealPseudoTracking):
        result.merge(PseudoTrackFinalCfg(flags))

    if flags.Tracking.doTIDE_AmbiTrackMonitoring:
        result.merge(ObserverTrackFinalCfg(flags))

    if flags.Tracking.doStoreTrackSeeds:
        result.merge(TrackSeedsFinalCfg(flags))

    if flags.Tracking.doStoreSiSPSeededTracks:
        result.merge(SiSPSeededTracksFinalCfg(flags))

    # ---------------------------------------
    # --- Primary vertexing
    # ---------------------------------------

    if flags.Tracking.doVertexFinding:
        from InDetConfig.InDetPriVxFinderConfig import primaryVertexFindingCfg
        result.merge(primaryVertexFindingCfg(flags))

    if flags.Tracking.doStats:
        result.merge(InDetStatsCfg(
            flags_set[0], # Use cuts from primary pass
            StatTrackCollections=StatTrackCollections,
            StatTrackTruthCollections=StatTrackTruthCollections))

    # ---------------------------------------
    # --- Extra optional decorations
    # ---------------------------------------

    if flags.Tracking.doV0Finder:
        from InDetConfig.InDetV0FinderConfig import InDetV0FinderCfg
        result.merge(InDetV0FinderCfg(flags))

    if (flags.Tracking.writeExtendedSi_PRDInfo or
        flags.Tracking.writeExtendedTRT_PRDInfo):
        result.merge(ExtendedPRDInfoCfg(flags))

    # output
    from InDetConfig.InDetTrackOutputConfig import InDetTrackRecoOutputCfg
    result.merge(InDetTrackRecoOutputCfg(flags, _extensions_list))

    return result

# Run with python -m InDetConfig.TrackRecoConfig
# Works with Data RAW (Format.BS), MC RDO, and MC ESD
def TrackRecoConfigTest(flags=None):

    store_config = False
    if flags is None:
        store_config = True
        from AthenaConfiguration.AllConfigFlags import initConfigFlags
        flags = initConfigFlags()

        # Disable calo for this test
        from InDetConfig.ConfigurationHelpers import OnlyTrackingPreInclude
        OnlyTrackingPreInclude(flags)

        from AthenaConfiguration.TestDefaults import defaultTestFiles, defaultConditionsTags
        flags.Input.Files = defaultTestFiles.RDO_RUN3
        flags.IOVDb.GlobalTag = defaultConditionsTags.RUN3_MC
        flags.Exec.MaxEvents = 1

        flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    top_acc = MainServicesCfg(flags)

    if flags.Input.Format is not Format.BS:
        from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
        top_acc.merge(PoolReadCfg(flags))

    if flags.Detector.EnableCalo:
        if flags.Input.Format is Format.BS:
            from CaloRec.CaloRecoConfig import CaloRecoCfg
            top_acc.merge(CaloRecoCfg(flags)) 
        else:
            from CaloRec.CaloTopoClusterConfig import CaloTopoClusterCfg
            top_acc.merge(CaloTopoClusterCfg(flags))

    if flags.Input.Format is not Format.BS:
        # TODO find better way to determine RDO vs ESD input?
        if "PixelRDOs" in flags.Input.Collections:
            if "EventInfo" not in flags.Input.Collections:
                from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoCnvAlgCfg
                top_acc.merge(EventInfoCnvAlgCfg(flags))

            if flags.Input.isMC:
                from xAODTruthCnv.xAODTruthCnvConfig import GEN_AOD2xAODCfg
                top_acc.merge(GEN_AOD2xAODCfg(flags))
        else:
            if flags.Tracking.doTruth :
                from xAODTruthCnv.RedoTruthLinksConfig import RedoTruthLinksAlgCfg
                top_acc.merge( RedoTruthLinksAlgCfg(flags) )

    top_acc.merge(InDetTrackRecoCfg(flags))

    if store_config:
        from AthenaCommon.Constants import DEBUG
        top_acc.foreach_component("AthEventSeq/*").OutputLevel = DEBUG
        top_acc.printConfig(withDetails=True, summariseProps=True)
        top_acc.store(open("TrackRecoConfig.pkl", "wb"))

    import sys
    if "--norun" not in sys.argv:
        sc = top_acc.run()
        if sc.isFailure():
            sys.exit(-1)

if __name__ == "__main__":
    TrackRecoConfigTest()
