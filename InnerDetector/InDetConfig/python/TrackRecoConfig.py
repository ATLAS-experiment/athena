# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import BeamType, Format

_flags_set = []  # For caching
_extensions_list = [] # For caching

def CombinedTrackingPassFlagSets(flags):

    global _flags_set
    if _flags_set:
        return _flags_set

    flags_set = []

    # Primary Pass
    if flags.Tracking.doVtxLumi:
        flags = flags.cloneAndReplace("Tracking.ActiveConfig",
                                      "Tracking.VtxLumiPass")
    elif flags.Tracking.doVtxBeamSpot:
        flags = flags.cloneAndReplace("Tracking.ActiveConfig",
                                      "Tracking.VtxBeamSpotPass")
    elif flags.Beam.Type is BeamType.Cosmics:
        flags = flags.cloneAndReplace("Tracking.ActiveConfig",
                                      "Tracking.CosmicsPass")
    elif flags.Reco.EnableHI:
        flags = flags.cloneAndReplace("Tracking.ActiveConfig",
                                      "Tracking.HeavyIonPass")
    elif flags.Tracking.doHighPileup:
        flags = flags.cloneAndReplace("Tracking.ActiveConfig",
                                      "Tracking.HighPileupPass")
    elif flags.Tracking.doMinBias:
        flags = flags.cloneAndReplace("Tracking.ActiveConfig",
                                      "Tracking.MinBiasPass")
    elif flags.Tracking.doRobustReco:
        flags = flags.cloneAndReplace("Tracking.ActiveConfig",
                                      "Tracking.RobustRecoPass")
    else:
        flags = flags.cloneAndReplace("Tracking.ActiveConfig",
                                      "Tracking.MainPass")

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

    # Detector ByteStream pre-processing
    if flags.Input.Format is Format.BS:
        from PixelRawDataByteStreamCnv.PixelRawDataByteStreamCnvConfig import (
            PixelRawDataProviderAlgCfg)
        result.merge(PixelRawDataProviderAlgCfg(flags))

        from SCT_RawDataByteStreamCnv.SCT_RawDataByteStreamCnvConfig import (
            SCTRawDataProviderCfg, SCTEventFlagWriterCfg)
        result.merge(SCTRawDataProviderCfg(flags))
        result.merge(SCTEventFlagWriterCfg(flags))

        from TRT_RawDataByteStreamCnv.TRT_RawDataByteStreamCnvConfig import (
            TRTRawDataProviderCfg)
        result.merge(TRTRawDataProviderCfg(flags))

    from InDetConfig.SiliconPreProcessing import InDetRecPreProcessingSiliconCfg
    result.merge(InDetRecPreProcessingSiliconCfg(flags))
    from InDetConfig.TRTPreProcessing import TRTPreProcessingCfg
    result.merge(TRTPreProcessingCfg(flags))

    ClusterSplitProbContainer = ''

    from InDetConfig.TrackingSiPatternConfig import TrackingSiPatternCfg
    from InDetConfig.TrackTruthConfig import InDetTrackTruthCfg
    from InDetConfig.TRTStandaloneConfig import TRTStandaloneCfg
    from InDetConfig.TRTExtensionConfig import NewTrackingTRTExtensionCfg
    from xAODTrackingCnv.xAODTrackingCnvConfig import (
        TrackParticleCnvAlgPIDCheckCfg)
    from TrkConfig.TrkTrackCollectionMergerConfig import (
        TrackCollectionMergerAlgCfg)

    # ------------------------------------------------------------
    #
    # ----------- Subdetector pattern from New Tracking
    #
    # ------------------------------------------------------------

    # Pixel track segment finding
    if flags.Tracking.doTrackSegmentsPixel:
        flagsPixel = flags.cloneAndReplace("Tracking.ActiveConfig",
                                           "Tracking.PixelPass")
        PixelTrackContainer = "ResolvedPixelTracks"

        result.merge(TrackingSiPatternCfg(
            flagsPixel,
            InputCollections=[],
            ResolvedTrackCollectionKey=PixelTrackContainer,
            SiSPSeededTrackCollectionKey="SiSPSeededPixelTracks",
            ClusterSplitProbContainer=ClusterSplitProbContainer))

        ClusterSplitProbContainer = (
            "InDetAmbiguityProcessorSplitProb" +
            flagsPixel.Tracking.ActiveConfig.extension)

        if flags.Tracking.doTruth:
            result.merge(InDetTrackTruthCfg(
                flagsPixel,
                Tracks=PixelTrackContainer,
                DetailedTruth=PixelTrackContainer+"DetailedTruth",
                TracksTruth=PixelTrackContainer+"TruthCollection"))

        from xAODTrackingCnv.xAODTrackingCnvConfig import (
            TrackParticleCnvAlgNoPIDCfg)
        result.merge(TrackParticleCnvAlgNoPIDCfg(
            flags,
            name=PixelTrackContainer+"CnvAlg",
            TrackContainerName=PixelTrackContainer,
            xAODTrackParticlesFromTracksContainerName=(
                "InDetPixelTrackParticles")))

    # SCT track segment finding
    if flags.Tracking.doTrackSegmentsSCT:
        flagsSCT = flags.cloneAndReplace("Tracking.ActiveConfig",
                                         "Tracking.SCTPass")
        SCTTrackContainer = "ResolvedSCTTracks"

        result.merge(TrackingSiPatternCfg(
            flagsSCT,
            InputCollections=[],
            ResolvedTrackCollectionKey=SCTTrackContainer,
            SiSPSeededTrackCollectionKey="SiSPSeededSCTTracks",
            ClusterSplitProbContainer=ClusterSplitProbContainer))

        ClusterSplitProbContainer = (
            "InDetAmbiguityProcessorSplitProb" +
            flagsSCT.Tracking.ActiveConfig.extension)

        from xAODTrackingCnv.xAODTrackingCnvConfig import (
            TrackParticleCnvAlgNoPIDCfg)
        result.merge(TrackParticleCnvAlgNoPIDCfg(
            flags,
            name=SCTTrackContainer+"CnvAlg",
            TrackContainerName=SCTTrackContainer,
            xAODTrackParticlesFromTracksContainerName=(
                "InDetSCTTrackParticles")))

    # TRT track segment finding
    if flags.Tracking.doTrackSegmentsTRT:
        flagsTRT = flags.cloneAndReplace("Tracking.ActiveConfig",
                                         "Tracking.TRTPass")
        from InDetConfig.TRTSegmentFindingConfig import (
            TRTSegmentFinding_TrackSegments_Cfg)
        result.merge(TRTSegmentFinding_TrackSegments_Cfg(flagsTRT))

        from InDetConfig.TRTStandaloneConfig import TRT_TrackSegment_Cfg
        result.merge(TRT_TrackSegment_Cfg(flagsTRT))

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

    for current_flags in flags_set:

        extension = (
            "" if isPrimaryPass else
            current_flags.Tracking.ActiveConfig.extension)
        _extensions_list.append(extension)

        # ---------------------------------------
        # ----   TRTStandalone pass
        # ---------------------------------------

        if flags.Tracking.doTRTStandalone and extension == "TRTStandalone":
            result.merge(TRTStandaloneCfg(
                current_flags,
                InputCollections=InputCombinedInDetTracks))

            TRTTrackContainer = "TRTStandaloneTracks"
            InputCombinedInDetTracks += [TRTTrackContainer]
            InputExtendedInDetTracks += [TRTTrackContainer]
            StatTrackCollections += [TRTTrackContainer]
            StatTrackTruthCollections += [TRTTrackContainer+"TruthCollection"]

            if flags.Tracking.doTrackSegmentsTRT:
                result.merge(TrackParticleCnvAlgNoPIDCfg(
                    flags,
                    name=TRTTrackContainer+"CnvAlg",
                    TrackContainerName=TRTTrackContainer,
                    xAODTrackParticlesFromTracksContainerName=(
                        "InDetTRTTrackParticles")))

            continue  # Skip rest of config for the TRTStandalone pass

        # ---------------------------------------
        # ----   All the passes but TRTStandalone
        # ---------------------------------------

        ResolvedTracks = "Resolved" + extension + "Tracks"

        # for track overlay, save resolved track name
        # for final merged track collection
        if (flags.Overlay.doTrackOverlay and
            current_flags.Tracking.ActiveConfig.storeSeparateContainer and
                not current_flags.Tracking.ActiveConfig.useTRTExtension):
            ResolvedTracks = flags.Overlay.SigPrefix + ResolvedTracks

        # Tweak to match old config key
        if "LargeD0" in extension:
            ResolvedTracks = "ResolvedLargeD0Tracks"

        # Old config had inconsistent "SiSPSeeded" vs "SiSpSeeded" keys
        # Updated in new config
        SiSPSeededTracks = "SiSPSeeded" + extension + "Tracks"

        # ---------------------------------------
        # --- Si pattern, if not done in the cosmic preprocessing
        # ---------------------------------------

        if not (isPrimaryPass and flags.Beam.Type is BeamType.Cosmics):
            # Old config had inconsistent "SiSPSeeded" vs "SiSpSeeded" keys
            SiSPSeededTracks = "SiSPSeeded" + extension + "Tracks"
            result.merge(TrackingSiPatternCfg(
                current_flags,
                InputCollections=InputExtendedInDetTracks,
                ResolvedTrackCollectionKey=ResolvedTracks,
                SiSPSeededTrackCollectionKey=SiSPSeededTracks,
                ClusterSplitProbContainer=ClusterSplitProbContainer))
            StatTrackCollections += [SiSPSeededTracks, ResolvedTracks]
            StatTrackTruthCollections += [SiSPSeededTracks+"TruthCollection",
                                          ResolvedTracks+"TruthCollection"]

        TrackContainer = ResolvedTracks
        if (flags.Overlay.doTrackOverlay and
                current_flags.Tracking.ActiveConfig.storeSeparateContainer):
            TrackContainer = "Resolved" + extension + "Tracks"

        # ---------------------------------------
        # --- TRT extension
        # ---------------------------------------

        if current_flags.Tracking.ActiveConfig.useTRTExtension:
            ExtendedTracks = "Extended" + extension + "Tracks"
            # Tweaks to match old config key
            if extension == "Disappearing":
                ExtendedTracks = "ExtendedTracksDisappearing"
            elif "LargeD0" in extension:
                ExtendedTracks = "ExtendedLargeD0Tracks"
                if flags.Overlay.doTrackOverlay:
                    ExtendedTracks = flags.Overlay.SigPrefix+"ExtendedLargeD0Tracks"
            ExtendedTracksMap = "ExtendedTracksMap" + extension

            result.merge(NewTrackingTRTExtensionCfg(
                current_flags,
                SiTrackCollection=ResolvedTracks,
                ExtendedTrackCollection=ExtendedTracks,
                ExtendedTracksMap=ExtendedTracksMap))

            TrackContainer = ExtendedTracks
            if flags.Overlay.doTrackOverlay and "LargeD0" in extension:
                TrackContainer = "ExtendedLargeD0Tracks"
            StatTrackCollections += [ExtendedTracks]
            StatTrackTruthCollections += [ExtendedTracks+"TruthCollection"]

        # ---------------------------------------
        # --- Store separate container if needed
        # ---------------------------------------

        if current_flags.Tracking.ActiveConfig.storeSeparateContainer:
            # Dummy Merger to fill additional info
            # for PRD-associated pixel tracklets
            # Can also run on all separate collections like R3LargeD0
            # but kept consistent with legacy config

            AssociationMapName = ""

            if extension == "Disappearing" or flags.Overlay.doTrackOverlay:

                if extension == "Disappearing":
                    InputTracks = [TrackContainer]
                    if flags.Overlay.doTrackOverlay:
                        InputTracks += [flags.Overlay.BkgPrefix +
                                        extension + "Tracks"]
                    TrackContainer = extension+"Tracks"
                    AssociationMapName = "PRDtoTrackMap" + TrackContainer
                    MergerOutputTracks = TrackContainer
                elif flags.Overlay.doTrackOverlay:
                    # schedule merger to combine signal and background tracks
                    InputTracks = [flags.Overlay.SigPrefix+TrackContainer,
                                   flags.Overlay.BkgPrefix+TrackContainer]
                    AssociationMapName = ("PRDtoTrackMapResolved" +
                                          extension + "Tracks")
                    MergerOutputTracks = TrackContainer

                result.merge(TrackCollectionMergerAlgCfg(
                    current_flags,
                    name="InDetTrackCollectionMerger"+extension,
                    InputCombinedTracks=InputTracks,
                    OutputCombinedTracks=MergerOutputTracks,
                    AssociationMapName=AssociationMapName))

            if flags.Tracking.doTruth:
                result.merge(InDetTrackTruthCfg(
                    current_flags,
                    Tracks=TrackContainer,
                    DetailedTruth=TrackContainer+"DetailedTruth",
                    TracksTruth=TrackContainer+"TruthCollection"))

            # Need specific handling with TrackParticles for R3LargeD0
            # not to break downstream configs
            xAODTrackParticlesName = (
                "InDetLargeD0TrackParticles" if "LargeD0" in extension else
                "InDet" + extension + "TrackParticles")

            result.merge(TrackParticleCnvAlgPIDCheckCfg(
                current_flags,
                name=extension + "TrackParticleCnvAlg",
                TrackContainerName=TrackContainer,
                xAODTrackParticlesFromTracksContainerName=(
                    xAODTrackParticlesName),
                ClusterSplitProbabilityName=ClusterSplitProbContainer,
                AssociationMapName=AssociationMapName))

        else:  # Do not store separate track container
            ClusterSplitProbContainer = (
                "InDetAmbiguityProcessorSplitProb" +
                current_flags.Tracking.ActiveConfig.extension)
            InputCombinedInDetTracks += [TrackContainer]

        InputExtendedInDetTracks += [TrackContainer]

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
                from InDetConfig.BackTrackingConfig import BackTrackingCfg
                result.merge(BackTrackingCfg(
                    current_flags,
                    InputCollections=InputCombinedInDetTracks,
                    ClusterSplitProbContainer=ClusterSplitProbContainer))

                ClusterSplitProbContainer = (
                    "InDetTRT_SeededAmbiguityProcessorSplitProb" +
                    current_flags.Tracking.ActiveConfig.extension)
                TRTSeededTracks = "TRTSeededTracks"
                ResolvedTRTSeededTracks = "ResolvedTRTSeededTracks"
                InputCombinedInDetTracks += [ResolvedTRTSeededTracks]
                InputExtendedInDetTracks += [ResolvedTRTSeededTracks]
                StatTrackCollections += [TRTSeededTracks,
                                         ResolvedTRTSeededTracks]
                StatTrackTruthCollections += [TRTSeededTracks+"TruthCollection",
                                              ResolvedTRTSeededTracks+"TruthCollection"]

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

    if flags.Overlay.doTrackOverlay:
        InputCombinedInDetTracks += [flags.Overlay.BkgPrefix +
                                     "CombinedInDetTracks"]

    result.merge(TrackCollectionMergerAlgCfg(
        flags,
        InputCombinedTracks=InputCombinedInDetTracks,
        OutputCombinedTracks="CombinedInDetTracks",
        AssociationMapName="PRDtoTrackMapCombinedInDetTracks"))

    if flags.Tracking.doTruth:
        result.merge(InDetTrackTruthCfg(
            flags,
            Tracks="CombinedInDetTracks",
            DetailedTruth="CombinedInDetTracksDetailedTruth",
            TracksTruth="CombinedInDetTracksTruthCollection"))

    StatTrackCollections += ["CombinedInDetTracks"]
    StatTrackTruthCollections += ["CombinedInDetTracksTruthCollection"]

    if flags.Tracking.doSlimming:
        from TrkConfig.TrkTrackSlimmerConfig import TrackSlimmerCfg
        result.merge(TrackSlimmerCfg(
            flags,
            TrackLocation=["CombinedInDetTracks"]))

    if flags.Tracking.doTruth:
        result.merge(InDetTrackTruthCfg(flags))

        if (flags.Tracking.doPseudoTracking or
            flags.Tracking.doIdealPseudoTracking):
            result.merge(InDetTrackTruthCfg(
                flags,
                Tracks="InDetPseudoTracks",
                DetailedTruth="InDetPseudoTracksDetailedTruth",
                TracksTruth="InDetPseudoTracksTruthCollection"))
        if flags.Tracking.doTIDE_AmbiTrackMonitoring:
            result.merge(InDetTrackTruthCfg(
                flags,
                Tracks="ObservedTracksCollection",
                DetailedTruth="ObservedTracksCollectionDetailedTruth",
                TracksTruth="ObservedTracksCollectionTruthCollection"))

    # by default, the main TrackParticleCnvAlg will run before
    # "primaryVertexFindingCfg"; in case perigeeExpression is set to "Vertex",
    # this will run after "primaryVertexFindingCfg";
    # the scheduler will always take care of the precedency

    from xAODTrackingCnv.xAODTrackingCnvConfig import TrackParticleCnvAlgCfg
    result.merge(TrackParticleCnvAlgCfg(
        flags,
        ClusterSplitProbabilityName=ClusterSplitProbabilityContainerName(
            flags),
        AssociationMapName="PRDtoTrackMapCombinedInDetTracks"))

    if (flags.Tracking.doPseudoTracking or
        flags.Tracking.doIdealPseudoTracking):
        result.merge(TrackParticleCnvAlgCfg(
            flags,
            name="PseudoTrackParticleCnvAlg",
            TrackContainerName="InDetPseudoTracks",
            xAODTrackParticlesFromTracksContainerName=(
                "InDetPseudoTrackParticles"),
            AssociationMapName=(
                "PRDtoTrackMapCombinedInDetTracks")))

    if flags.Tracking.doTIDE_AmbiTrackMonitoring:
        from xAODTrackingCnv.xAODTrackingCnvConfig import (
            ObserverTrackParticleCnvAlgCfg)
        result.merge(ObserverTrackParticleCnvAlgCfg(
            flags,
            AssociationMapName = (
                "PRDtoTrackMapCombinedInDetTracks")))

    if flags.Tracking.doStoreTrackSeeds:
        from xAODTrackingCnv.xAODTrackingCnvConfig import (
            TrackParticleCnvAlgNoPIDCfg)
        # get list of extensions requesting track seeds. Add always the Primary Pass.
        listOfExtensionsRequesting = [ e for e in _extensions_list if (e == '') or (flags.Tracking.__getattr__(e+'Pass').storeTrackSeeds and flags.Tracking.__getattr__(e+'Pass').storeSeparateContainer) ]
        for extension in listOfExtensionsRequesting:
            TrackContainer = "SiSPSeedSegments"+extension

            if flags.Tracking.doTruth:
                result.merge(InDetTrackTruthCfg(
                    flags,
                    Tracks=TrackContainer,
                    DetailedTruth=f"{TrackContainer}DetailedTruth",
                    TracksTruth=f"{TrackContainer}TruthCollection"))

            result.merge(TrackParticleCnvAlgNoPIDCfg(
                flags,
                name=f"SiSPSeedSegments{extension}CnvAlg",
                TrackContainerName=TrackContainer,
                xAODTrackParticlesFromTracksContainerName=(
                    f"{TrackContainer}TrackParticles")))

    if flags.Tracking.doStoreSiSPSeededTracks:
        from xAODTrackingCnv.xAODTrackingCnvConfig import (
            TrackParticleCnvAlgNoPIDCfg)
        # get list of extensions requesting track candidates. Add always the Primary Pass.
        listOfExtensionsRequesting = [ e for e in _extensions_list if (e == '') or (flags.Tracking.__getattr__(e+'Pass').storeSiSPSeededTracks and flags.Tracking.__getattr__(e+'Pass').storeSeparateContainer) ]
        for extension in listOfExtensionsRequesting:
            AssociationMapNameKey="PRDtoTrackMapCombinedInDetTracks"
            if extension=='Disappearing': AssociationMapNameKey = "PRDtoTrackMapDisappearingTracks"
            elif not (extension == ''): AssociationMapNameKey = f"InDetPRDtoTrackMap{extension}"
            result.merge(TrackParticleCnvAlgNoPIDCfg(
                flags,
                name = f"SiSPSeededTracks{extension}CnvAlg",
                TrackContainerName = f"SiSPSeeded{extension}Tracks",
                xAODTrackParticlesFromTracksContainerName=(
                    f"SiSPSeededTracks{extension}TrackParticles"),
                AssociationMapName=AssociationMapNameKey))

    # ---------------------------------------
    # --- Primary vertexing
    # ---------------------------------------

    if flags.Tracking.doVertexFinding:
        from InDetConfig.InDetPriVxFinderConfig import primaryVertexFindingCfg
        result.merge(primaryVertexFindingCfg(flags))

    if flags.Tracking.doStats:
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
                flags_set[0],  # Use cuts from primary pass
                TracksLocation=StatTrackCollections))

    # ---------------------------------------
    # --- Extra optional decorations
    # ---------------------------------------

    if flags.Tracking.writeExtendedPRDInfo:

        if (flags.Tracking.doTIDE_AmbiTrackMonitoring or
                flags.Tracking.doPseudoTracking):

            from InDetConfig.InDetPrepRawDataToxAODConfig import (
                InDetPixelPrepDataToxAOD_ExtraTruthCfg,
                InDetSCT_PrepDataToxAOD_ExtraTruthCfg,
                InDetTRT_PrepDataToxAOD_ExtraTruthCfg)
            result.merge(InDetPixelPrepDataToxAOD_ExtraTruthCfg(
                flags,
                ClusterSplitProbabilityName=(
                    ClusterSplitProbabilityContainerName(flags))))
            result.merge(InDetSCT_PrepDataToxAOD_ExtraTruthCfg(flags))
            result.merge(InDetTRT_PrepDataToxAOD_ExtraTruthCfg(flags))

        else:

            from InDetConfig.InDetPrepRawDataToxAODConfig import (
                InDetPixelPrepDataToxAODCfg,
                InDetSCT_PrepDataToxAODCfg,
                InDetTRT_PrepDataToxAODCfg)
            result.merge(InDetPixelPrepDataToxAODCfg(
                flags,
                ClusterSplitProbabilityName=(
                    ClusterSplitProbabilityContainerName(flags))))
            result.merge(InDetSCT_PrepDataToxAODCfg(flags))
            result.merge(InDetTRT_PrepDataToxAODCfg(flags))

        from DerivationFrameworkInDet.InDetToolsConfig import (
            TrackStateOnSurfaceDecoratorCfg)
        TrackStateOnSurfaceDecorator = result.getPrimaryAndMerge(
            TrackStateOnSurfaceDecoratorCfg(
                flags, name="TrackStateOnSurfaceDecorator"))
        TrackStateOnSurfaceDecorator.DecorationPrefix = "Reco_"
        result.addEventAlgo(
            CompFactory.DerivationFramework.CommonAugmentation(
                "InDetCommonKernel",
                AugmentationTools=[TrackStateOnSurfaceDecorator]))

        if flags.Tracking.doTIDE_AmbiTrackMonitoring:
            from DerivationFrameworkInDet.InDetToolsConfig import (
                ObserverTrackStateOnSurfaceDecoratorCfg)
            ObserverTrackStateOnSurfaceDecorator = result.getPrimaryAndMerge(
                ObserverTrackStateOnSurfaceDecoratorCfg(flags))
            result.addEventAlgo(
                CompFactory.DerivationFramework.CommonAugmentation(
                    "ObserverInDetCommonKernel",
                    AugmentationTools=[ObserverTrackStateOnSurfaceDecorator]))

        if flags.Tracking.doPseudoTracking:
            from DerivationFrameworkInDet.InDetToolsConfig import (
                PseudoTrackStateOnSurfaceDecoratorCfg)
            PseudoTrackStateOnSurfaceDecorator = result.getPrimaryAndMerge(
                PseudoTrackStateOnSurfaceDecoratorCfg(flags))
            result.addEventAlgo(
                CompFactory.DerivationFramework.CommonAugmentation(
                    "PseudoInDetCommonKernel",
                    AugmentationTools=[PseudoTrackStateOnSurfaceDecorator]))

        if flags.Tracking.doStoreSiSPSeededTracks:
            from DerivationFrameworkInDet.InDetToolsConfig import (
                SiSPTrackStateOnSurfaceDecoratorCfg)
            SiSPTrackStateOnSurfaceDecorator = result.getPrimaryAndMerge(
                SiSPTrackStateOnSurfaceDecoratorCfg(flags))
            result.addEventAlgo(
                CompFactory.DerivationFramework.CommonAugmentation(
                    "SiSPInDetCommonKernel",
                    AugmentationTools=[SiSPTrackStateOnSurfaceDecorator]))

        if flags.Input.isMC:
            from InDetPhysValMonitoring.InDetPhysValDecorationConfig import (
                InDetPhysHitDecoratorAlgCfg)
            result.merge(InDetPhysHitDecoratorAlgCfg(flags))

    # output
    result.merge(InDetTrackRecoOutputCfg(flags))

    return result


# these are used internally by the FTAG software. We generally don't
# want to save them since they can be reconstructed by rerunning
# flavor tagging.
FTAG_AUXDATA = [
    'VxTrackAtVertex',
    'TrackCompatibility',
    'JetFitter_TrackCompatibility_antikt4emtopo',
    'JetFitter_TrackCompatibility_antikt4empflow',
    'btagIp_d0Uncertainty',
    'btagIp_z0SinThetaUncertainty',
    'btagIp_z0SinTheta',
    'btagIp_d0',
    'btagIp_trackMomentum',
    'btagIp_trackDisplacement',
    'btagIp_invalidIp',
]


def InDetTrackRecoOutputCfg(flags):
    from OutputStreamAthenaPool.OutputStreamConfig import addToESD, addToAOD
    toAOD = []
    toESD = []

    # FIXME: special branches without flags
    special = False

    # excluded track aux data
    excludedAuxData = "-clusterAssociation.-TTVA_AMVFVertices_forReco.-TTVA_AMVFWeights_forReco"
    # remove track decorations used internally by FTAG software
    excludedAuxData += '.-'.join([''] + FTAG_AUXDATA)

    if not special:  # not flags.InDet.keepFirstParameters or flags.InDet.keepAdditionalHitsOnTrackParticle
        excludedAuxData += '.-trackParameterCovarianceMatrices.-parameterX.-parameterY.-parameterZ.-parameterPX.-parameterPY.-parameterPZ.-parameterPosition'

    # exclude TTVA decorations
    excludedAuxData += '.-TTVA_AMVFVertices.-TTVA_AMVFWeights'

    # exclude IDTIDE/IDTRKVALID decorations
    excludedAuxData += '.-TrkBLX.-TrkBLY.-TrkBLZ.-TrkIBLX.-TrkIBLY.-TrkIBLZ.-TrkL1X.-TrkL1Y.-TrkL1Z.-TrkL2X.-TrkL2Y.-TrkL2Z'
    if not flags.Tracking.writeExtendedPRDInfo:
        excludedAuxData += '.-msosLink'

    # exclude IDTIDE decorations
    excludedAuxData += ('.-IDTIDE1_biased_PVd0Sigma.-IDTIDE1_biased_PVz0Sigma.-IDTIDE1_biased_PVz0SigmaSinTheta.-IDTIDE1_biased_d0.-IDTIDE1_biased_d0Sigma'
                        '.-IDTIDE1_biased_z0.-IDTIDE1_biased_z0Sigma.-IDTIDE1_biased_z0SigmaSinTheta.-IDTIDE1_biased_z0SinTheta.-IDTIDE1_unbiased_PVd0Sigma.-IDTIDE1_unbiased_PVz0Sigma'
                        '.-IDTIDE1_unbiased_PVz0SigmaSinTheta.-IDTIDE1_unbiased_d0.-IDTIDE1_unbiased_d0Sigma.-IDTIDE1_unbiased_z0.-IDTIDE1_unbiased_z0Sigma.-IDTIDE1_unbiased_z0SigmaSinTheta'
                        '.-IDTIDE1_unbiased_z0SinTheta')

    ##### ESD #####
    # Save full and zero-suppressed BCM rdos
    # (the latter is needed to allow writting to AOD and the former will hopefully be removed in future):
    toESD += [
        "BCM_RDO_Container#BCM_RDOs",
        "BCM_RDO_Container#BCM_CompactDOs",
    ]

    # In case of cosmics we save the RDOs as well
    if special:  # flags.InDet.writeRDOs:
        toESD += [
            "PixelRDO_Container#PixelRDOs",
            "SCT_RDO_Container#SCT_RDOs",
            # "TRT_RDO_Container#TRT_RDOs",
        ]

    # write phase calculation into ESD
    if flags.InDet.doTRTPhase:
        toESD += ["ComTime#TRT_Phase"]

    # Save PRD
    toESD += [
        "InDet::SCT_ClusterContainer#SCT_Clusters",
        "InDet::PixelClusterContainer#PixelClusters",
        "InDet::TRT_DriftCircleContainer#TRT_DriftCircles",
        "InDet::PixelGangedClusterAmbiguities#PixelClusterAmbiguitiesMap",
    ]
    if flags.Tracking.doPixelClusterSplitting:
        toESD += ["InDet::PixelGangedClusterAmbiguities#SplitClusterAmbiguityMap"]
    toESD += ["IDCInDetBSErrContainer#SCT_FlaggedCondData"]
    toESD += ["Trk::ClusterSplitProbabilityContainer#" +
              ClusterSplitProbabilityContainerName(flags)]

    # add tracks
    if flags.Tracking.doStoreTrackSeeds:
        listOfExtensionsRequesting = [ e for e in _extensions_list if (e == '') or (flags.Tracking.__getattr__(e+'Pass').storeTrackSeeds and flags.Tracking.__getattr__(e+'Pass').storeSeparateContainer) ]
        for extension in listOfExtensionsRequesting:
            toESD += ["TrackCollection#SiSPSeedSegments"+extension]

    if flags.Tracking.doTrackSegmentsPixel:
        toESD += ["TrackCollection#ResolvedPixelTracks"]
        if flags.Tracking.doTruth:
            toESD += ["TrackTruthCollection#ResolvedPixelTracksTruthCollection"]
            toESD += ["DetailedTrackTruthCollection#ResolvedPixelTracksDetailedTruth"]

    if flags.Tracking.doTrackSegmentsSCT:
        toESD += ["TrackCollection#ResolvedSCTTracks"]
        if flags.Tracking.doTruth:
            toESD += ["TrackTruthCollection#ResolvedSCTTracksTruthCollection"]
            toESD += ["DetailedTrackTruthCollection#ResolvedSCTTracksDetailedTruth"]

    if flags.Tracking.doTrackSegmentsTRT:
        toESD += ["TrackCollection#StandaloneTRTTracks"]
        if flags.Tracking.doTruth:
            toESD += ["TrackTruthCollection#StandaloneTRTTracksTruthCollection"]
            toESD += ["DetailedTrackTruthCollection#StandaloneTRTTracksDetailedTruth"]

    if flags.Tracking.doPseudoTracking:
        toESD += ["TrackCollection#InDetPseudoTracks"]
        if flags.Tracking.doTruth:
            toESD += ["TrackTruthCollection#InDetPseudoTracksTruthCollection"]
            toESD += ["DetailedTrackTruthCollection#InDetPseudoTracksDetailedTruth"]

    if flags.Tracking.doTIDE_AmbiTrackMonitoring:
        toESD += ["TrackCollection#ObservedTracksCollection"]

    # add the forward tracks for combined muon reconstruction
    if flags.Tracking.doForwardTracks:
        toESD += ["TrackCollection#ResolvedForwardTracks"]
        if flags.Tracking.doTruth:
            toESD += ["TrackTruthCollection#ResolvedForwardTracksTruthCollection"]
            toESD += ["DetailedTrackTruthCollection#ResolvedForwardTracksDetailedTruth"]

    if flags.Tracking.doTrackSegmentsDisappearing:
        toESD += ["TrackCollection#DisappearingTracks"]
        if flags.Tracking.doTruth:
            toESD += ["TrackTruthCollection#DisappearingTracksTruthCollection"]
            toESD += ["DetailedTrackTruthCollection#DisappearingTracksDetailedTruth"]

    if flags.Tracking.doLowPtRoI:
        toESD += ["xAOD::VertexContainer#RoIVerticesLowPtRoI", "xAOD::VertexAuxContainer#RoIVerticesLowPtRoIAux."]
        if flags.Tracking.LowPtRoIPass.storeSeparateContainer:
            toESD += ["TrackCollection#ExtendedLowPtRoITracks"]
            if flags.Tracking.doTruth:
                toESD += ["TrackTruthCollection#ExtendedLowPtRoITracksTruthCollection"]
                toESD += ["DetailedTrackTruthCollection#ExtendedLowPtRoITracksDetailedTruth"] 

    # Add TRT Segments (only if standalone is off).
    # TODO: no TP converter?
    # if not flags.doTRTStandalone:
    #    toESD += ["Trk::SegmentCollection#TRTSegments"]

    # Save (Detailed) Track Truth
    if flags.Tracking.doTruth:
        toESD += ["TrackTruthCollection#TrackTruthCollection"]
        toESD += ["DetailedTrackTruthCollection#DetailedTrackTruth"]

        # save PRD MultiTruth
        toESD += [
            "PRD_MultiTruthCollection#PRD_MultiTruthPixel",
            "PRD_MultiTruthCollection#PRD_MultiTruthSCT",
            "PRD_MultiTruthCollection#PRD_MultiTruthTRT",
        ]

    if not flags.Input.isMC:
        toESD += [
            "InDetBSErrContainer#PixelByteStreamErrs",
            "TRT_BSErrContainer#TRT_ByteStreamErrs",
            "TRT_BSIdErrContainer#TRT_ByteStreamIdErrs",
            "IDCInDetBSErrContainer#SCT_ByteStreamErrs",
        ]

    toESD += ["TrackCollection#CombinedInDetTracks"]

    ##### AOD #####
    toAOD += ["xAOD::TrackParticleContainer#InDetTrackParticles"]
    toAOD += [
        f"xAOD::TrackParticleAuxContainer#InDetTrackParticlesAux.{excludedAuxData}"]
    toAOD += ["xAOD::TrackParticleContainer#InDetForwardTrackParticles"]
    toAOD += [
        f"xAOD::TrackParticleAuxContainer#InDetForwardTrackParticlesAux.{excludedAuxData}"]
    toAOD += ["xAOD::TrackParticleContainer#InDetLargeD0TrackParticles"]
    toAOD += [
        f"xAOD::TrackParticleAuxContainer#InDetLargeD0TrackParticlesAux.{excludedAuxData}"]
    if flags.Tracking.doTrackSegmentsPixel:
        toAOD += ["xAOD::TrackParticleContainer#InDetPixelTrackParticles"]
        toAOD += [
            f"xAOD::TrackParticleAuxContainer#InDetPixelTrackParticlesAux.{excludedAuxData}"]
    if flags.Tracking.doTrackSegmentsDisappearing:
        toAOD += ["xAOD::TrackParticleContainer#InDetDisappearingTrackParticles"]
        toAOD += [
            f"xAOD::TrackParticleAuxContainer#InDetDisappearingTrackParticlesAux.{excludedAuxData}"]
    if flags.Tracking.doLowPtRoI:
        toAOD += ["xAOD::VertexContainer#RoIVerticesLowPtRoI", "xAOD::VertexAuxContainer#RoIVerticesLowPtRoIAux."]
        if flags.Tracking.LowPtRoIPass.storeSeparateContainer:
            toAOD += ["xAOD::TrackParticleContainer#InDetLowPtRoITrackParticles"]
            toAOD += [
                f"xAOD::TrackParticleAuxContainer#InDetLowPtRoITrackParticlesAux.{excludedAuxData}"]    
    if flags.Tracking.doTrackSegmentsSCT:
        toAOD += ["xAOD::TrackParticleContainer#InDetSCTTrackParticles"]
        toAOD += [
            f"xAOD::TrackParticleAuxContainer#InDetSCTTrackParticlesAux.{excludedAuxData}"]
    if flags.Tracking.doTrackSegmentsTRT:
        toAOD += ["xAOD::TrackParticleContainer#InDetTRTTrackParticles"]
        toAOD += [
            f"xAOD::TrackParticleAuxContainer#InDetTRTTrackParticlesAux.{excludedAuxData}"]
    if flags.Tracking.doPseudoTracking:
        toAOD += ["xAOD::TrackParticleContainer#InDetPseudoTrackParticles"]
        toAOD += [
            f"xAOD::TrackParticleAuxContainer#InDetPseudoTrackParticlesAux.{excludedAuxData}"]
        if flags.Tracking.doTruth:
            toAOD += ["TrackTruthCollection#InDetPseudoTrackTruthCollection"]
            toAOD += ["DetailedTrackTruthCollection#InDetPseudoTrackDetailedTruth"]
    if flags.Tracking.doTIDE_AmbiTrackMonitoring:
        toAOD += ["xAOD::TrackParticleContainer#InDetObservedTrackParticles"]
        toAOD += [
            f"xAOD::TrackParticleAuxContainer#InDetObservedTrackParticlesAux.{excludedAuxData}"]
        if flags.Tracking.doTruth:
            toAOD += ["TrackTruthCollection#InDetObservedTrackTruthCollection"]
            toAOD += ["DetailedTrackTruthCollection#ObservedDetailedTracksTruth"]
    if flags.Tracking.doStoreSiSPSeededTracks:
        # get list of extensions requesting track candidates. Add always the Primary Pass.
        listOfExtensionsRequesting = [ e for e in _extensions_list if (e == '') or (flags.Tracking.__getattr__(e+'Pass').storeSiSPSeededTracks and flags.Tracking.__getattr__(e+'Pass').storeSeparateContainer) ]
        for extension in listOfExtensionsRequesting:
            toAOD += [f"xAOD::TrackParticleContainer#SiSPSeededTracks{extension}TrackParticles"]
            toAOD += [
                f"xAOD::TrackParticleAuxContainer#SiSPSeededTracks{extension}TrackParticlesAux.{excludedAuxData}"]

    if flags.Tracking.doStoreTrackSeeds:
        # get list of extensions requesting track seeds. Add always the Primary Pass.
        listOfExtensionsRequesting = [ e for e in _extensions_list if (e == '') or (flags.Tracking.__getattr__(e+'Pass').storeTrackSeeds and flags.Tracking.__getattr__(e+'Pass').storeSeparateContainer) ]
        for extension in listOfExtensionsRequesting:
            toAOD += [
                f"xAOD::TrackParticleContainer#SiSPSeedSegments{extension}TrackParticles",
                f"xAOD::TrackParticleAuxContainer#SiSPSeedSegments{extension}TrackParticlesAux."
            ]
    if flags.Tracking.writeExtendedPRDInfo:
        toAOD += [
            "xAOD::TrackMeasurementValidationContainer#PixelClusters",
            "xAOD::TrackMeasurementValidationAuxContainer#PixelClustersAux.",
            "xAOD::TrackMeasurementValidationContainer#SCT_Clusters",
            "xAOD::TrackMeasurementValidationAuxContainer#SCT_ClustersAux.",
            "xAOD::TrackMeasurementValidationContainer#TRT_DriftCircles",
            "xAOD::TrackMeasurementValidationAuxContainer#TRT_DriftCirclesAux.",
            "xAOD::TrackStateValidationContainer#PixelMSOSs",
            "xAOD::TrackStateValidationAuxContainer#PixelMSOSsAux.",
            "xAOD::TrackStateValidationContainer#SCT_MSOSs",
            "xAOD::TrackStateValidationAuxContainer#SCT_MSOSsAux.",
            "xAOD::TrackStateValidationContainer#TRT_MSOSs",
            "xAOD::TrackStateValidationAuxContainer#TRT_MSOSsAux."
        ]
        if flags.Tracking.doTIDE_AmbiTrackMonitoring:
            toAOD += [
                "xAOD::TrackStateValidationContainer#ObservedTrack_Pixel_MSOSs",
                "xAOD::TrackStateValidationAuxContainer#ObservedTrack_Pixel_MSOSsAux.",
                "xAOD::TrackStateValidationContainer#ObservedTrack_SCT_MSOSs",
                "xAOD::TrackStateValidationAuxContainer#ObservedTrack_SCT_MSOSsAux.",
                "xAOD::TrackStateValidationContainer#ObservedTrack_TRT_MSOSs",
                "xAOD::TrackStateValidationAuxContainer#ObservedTrack_TRT_MSOSsAux."
            ]
        if flags.Tracking.doPseudoTracking:
            toAOD += [
                "xAOD::TrackStateValidationContainer#Pseudo_Pixel_MSOSs",
                "xAOD::TrackStateValidationAuxContainer#Pseudo_Pixel_MSOSsAux.",
                "xAOD::TrackStateValidationContainer#Pseudo_SCT_MSOSs",
                "xAOD::TrackStateValidationAuxContainer#Pseudo_SCT_MSOSsAux.",
                "xAOD::TrackStateValidationContainer#Pseudo_TRT_MSOSs",
                "xAOD::TrackStateValidationAuxContainer#Pseudo_TRT_MSOSsAux."
            ]
        if flags.InDet.Tracking.doStoreSiSPSeededTracks:
            toAOD += [
                "xAOD::TrackStateValidationContainer#SiSP_Pixel_MSOSs",
                "xAOD::TrackStateValidationAuxContainer#SiSP_Pixel_MSOSsAux.",
                "xAOD::TrackStateValidationContainer#SiSP_SCT_MSOSs",
                "xAOD::TrackStateValidationAuxContainer#SiSP_SCT_MSOSsAux.",
                "xAOD::TrackStateValidationContainer#SiSP_TRT_MSOSs",
                "xAOD::TrackStateValidationAuxContainer#SiSP_TRT_MSOSsAux."
            ]

    result = ComponentAccumulator()
    result.merge(addToESD(flags, toESD + toAOD))
    result.merge(addToAOD(flags, toAOD))

    return result


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    # Disable calo for this test
    flags.Detector.EnableCalo = False

    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags.Input.Files = defaultTestFiles.RDO_RUN2
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    top_acc = MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    top_acc.merge(PoolReadCfg(flags))

    #######################################################################
    #################### Additional Configuration  ########################
    if "EventInfo" not in flags.Input.Collections:
        from xAODEventInfoCnv.xAODEventInfoCnvConfig import EventInfoCnvAlgCfg
        top_acc.merge(EventInfoCnvAlgCfg(flags))

    if flags.Input.isMC:
        from xAODTruthCnv.xAODTruthCnvConfig import GEN_AOD2xAODCfg
        top_acc.merge(GEN_AOD2xAODCfg(flags))

    top_acc.merge(InDetTrackRecoCfg(flags))
    from AthenaCommon.Constants import DEBUG
    top_acc.foreach_component("AthEventSeq/*").OutputLevel = DEBUG
    top_acc.printConfig(withDetails=True, summariseProps=True)
    top_acc.store(open("TrackRecoConfig.pkl", "wb"))

    import sys
    if "--norun" not in sys.argv:
        sc = top_acc.run(1)
        if sc.isFailure():
            sys.exit(-1)
