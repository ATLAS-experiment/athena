# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

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


def InDetTrackRecoOutputCfg(flags, extensions_list=None):
    if extensions_list is None:
        extensions_list = []

    from OutputStreamAthenaPool.OutputStreamConfig import addToESD, addToAOD
    toAOD = []
    toESD = []

    # excluded track aux data
    excludedAuxData = ('-clusterAssociation.-TTVA_AMVFVertices_forReco'
                       '.-TTVA_AMVFWeights_forReco')
    # remove track decorations used internally by FTAG software
    excludedAuxData += '.-'.join([''] + FTAG_AUXDATA)
    excludedAuxData += ('.-trackParameterCovarianceMatrices'
                        '.-parameterX.-parameterY.-parameterZ'
                        '.-parameterPX.-parameterPY.-parameterPZ'
                        '.-parameterPosition')

    # exclude TTVA decorations
    excludedAuxData += '.-TTVA_AMVFVertices.-TTVA_AMVFWeights'

    # exclude IDTIDE decorations
    from DerivationFrameworkInDet.IDTIDE import IDTIDE_AOD_EXCLUDED_AUXDATA
    excludedAuxData += '.-'.join([''] + IDTIDE_AOD_EXCLUDED_AUXDATA)

    if not (flags.Tracking.writeExtendedSi_PRDInfo or
            flags.Tracking.writeExtendedTRT_PRDInfo):
        excludedAuxData += '.-msosLink'

    ##### ESD #####
    # Save full and zero-suppressed BCM rdos
    # (the latter is needed to allow writting to AOD and the former will hopefully be removed in future):
    toESD += [
        "BCM_RDO_Container#BCM_RDOs",
        "BCM_RDO_Container#BCM_CompactDOs",
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
        toESD += [
            "InDet::PixelGangedClusterAmbiguities#SplitClusterAmbiguityMap"]
    toESD += ["IDCInDetBSErrContainer#SCT_FlaggedCondData"]

    from InDetConfig.TrackRecoConfig import ClusterSplitProbabilityContainerName
    toESD += ["Trk::ClusterSplitProbabilityContainer#" +
              ClusterSplitProbabilityContainerName(flags)]

    # add tracks
    if flags.Tracking.doStoreTrackSeeds:
        listOfExtensionsRequesting = [
            e for e in extensions_list
            if (e == '' or flags.Tracking.__getattr__(e+'Pass').storeTrackSeeds) ]
        for extension in listOfExtensionsRequesting:
            toESD += ["TrackCollection#SiSPSeedSegments"+extension]

    if flags.Tracking.doTrackSegmentsPixel:
        toESD += ["TrackCollection#ResolvedPixelTracks"]
        if flags.Tracking.doTruth:
            toESD += [
                "TrackTruthCollection#ResolvedPixelTracksTruthCollection",
                "DetailedTrackTruthCollection#ResolvedPixelTracksDetailedTruth"]

    if flags.Tracking.doTrackSegmentsSCT:
        toESD += ["TrackCollection#ResolvedSCTTracks"]
        if flags.Tracking.doTruth:
            toESD += [
                "TrackTruthCollection#ResolvedSCTTracksTruthCollection",
                "DetailedTrackTruthCollection#ResolvedSCTTracksDetailedTruth"]

    if flags.Tracking.doTrackSegmentsTRT:
        toESD += ["TrackCollection#StandaloneTRTTracks"]
        if flags.Tracking.doTruth:
            toESD += [
                "TrackTruthCollection#StandaloneTRTTracksTruthCollection",
                "DetailedTrackTruthCollection#StandaloneTRTTracksDetailedTruth"]

    if flags.Tracking.doPseudoTracking:
        toESD += ["TrackCollection#InDetPseudoTracks"]
        if flags.Tracking.doTruth:
            toESD += [
                "TrackTruthCollection#InDetPseudoTracksTruthCollection",
                "DetailedTrackTruthCollection#InDetPseudoTracksDetailedTruth"]

    if flags.Tracking.doTIDE_AmbiTrackMonitoring:
        toESD += ["TrackCollection#ObservedTracksCollection"]

    # add the forward tracks for combined muon reconstruction
    if flags.Tracking.doForwardTracks:
        toESD += ["TrackCollection#ResolvedForwardTracks"]
        if flags.Tracking.doTruth:
            toESD += [
                "TrackTruthCollection#ResolvedForwardTracksTruthCollection",
                "DetailedTrackTruthCollection#ResolvedForwardTracksDetailedTruth"]

    if flags.Tracking.doTrackSegmentsDisappearing:
        toESD += ["TrackCollection#DisappearingTracks"]
        if flags.Tracking.doTruth:
            toESD += [
                "TrackTruthCollection#DisappearingTracksTruthCollection",
                "DetailedTrackTruthCollection#DisappearingTracksDetailedTruth"]

    if flags.Tracking.doLowPtRoI:
        toESD += ["xAOD::VertexContainer#RoIVerticesLowPtRoI",
                  "xAOD::VertexAuxContainer#RoIVerticesLowPtRoIAux."]
        if flags.Tracking.LowPtRoIPass.storeSeparateContainer:
            toESD += ["TrackCollection#ExtendedLowPtRoITracks"]
            if flags.Tracking.doTruth:
                toESD += [
                    "TrackTruthCollection#ExtendedLowPtRoITracksTruthCollection",
                    "DetailedTrackTruthCollection#ExtendedLowPtRoITracksDetailedTruth"]

    # Save (Detailed) Track Truth
    if flags.Tracking.doTruth:
        toESD += [
            "TrackTruthCollection#CombinedInDetTracksTruthCollection",
            "DetailedTrackTruthCollection#CombinedInDetTracksDetailedTruth"]

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
    toAOD += [
        "xAOD::TrackParticleContainer#InDetTrackParticles",
        f"xAOD::TrackParticleAuxContainer#InDetTrackParticlesAux.{excludedAuxData}",
        "xAOD::TrackParticleContainer#InDetForwardTrackParticles",
        f"xAOD::TrackParticleAuxContainer#InDetForwardTrackParticlesAux.{excludedAuxData}",
        "xAOD::TrackParticleContainer#InDetLargeD0TrackParticles",
        f"xAOD::TrackParticleAuxContainer#InDetLargeD0TrackParticlesAux.{excludedAuxData}"
    ]

    if flags.Tracking.doTrackSegmentsDisappearing:
        toAOD += [
            "xAOD::TrackParticleContainer#InDetDisappearingTrackParticles",
            f"xAOD::TrackParticleAuxContainer#InDetDisappearingTrackParticlesAux.{excludedAuxData}"]
    if flags.Tracking.doLowPtRoI:
        toAOD += ["xAOD::VertexContainer#RoIVerticesLowPtRoI",
                  "xAOD::VertexAuxContainer#RoIVerticesLowPtRoIAux."]
        if flags.Tracking.LowPtRoIPass.storeSeparateContainer:
            toAOD += [
                "xAOD::TrackParticleContainer#InDetLowPtRoITrackParticles",
                f"xAOD::TrackParticleAuxContainer#InDetLowPtRoITrackParticlesAux.{excludedAuxData}"]

    if flags.Tracking.doTrackSegmentsPixel:
        toAOD += [
            "xAOD::TrackParticleContainer#InDetPixelTrackParticles",
            f"xAOD::TrackParticleAuxContainer#InDetPixelTrackParticlesAux.{excludedAuxData}"]
    if flags.Tracking.doTrackSegmentsSCT:
        toAOD += [
            "xAOD::TrackParticleContainer#InDetSCTTrackParticles",
            f"xAOD::TrackParticleAuxContainer#InDetSCTTrackParticlesAux.{excludedAuxData}"]
    if flags.Tracking.doTrackSegmentsTRT:
        toAOD += [
            "xAOD::TrackParticleContainer#InDetTRTTrackParticles",
            f"xAOD::TrackParticleAuxContainer#InDetTRTTrackParticlesAux.{excludedAuxData}"]

    if flags.Tracking.doPseudoTracking:
        toAOD += [
            "xAOD::TrackParticleContainer#InDetPseudoTrackParticles",
            f"xAOD::TrackParticleAuxContainer#InDetPseudoTrackParticlesAux.{excludedAuxData}"]
        if flags.Tracking.doTruth:
            toAOD += [
                "TrackTruthCollection#InDetPseudoTrackTruthCollection",
                "DetailedTrackTruthCollection#InDetPseudoTrackDetailedTruth"]

    if flags.Tracking.doTIDE_AmbiTrackMonitoring:
        toAOD += [
            "xAOD::TrackParticleContainer#InDetObservedTrackParticles",
            f"xAOD::TrackParticleAuxContainer#InDetObservedTrackParticlesAux.{excludedAuxData}"]
        if flags.Tracking.doTruth:
            toAOD += [
                "TrackTruthCollection#InDetObservedTrackTruthCollection",
                "DetailedTrackTruthCollection#ObservedDetailedTracksTruth"]

    if flags.Tracking.doStoreSiSPSeededTracks:
        # get list of extensions requesting track candidates.
        # Add always the Primary Pass.
        listOfExtensionsRequesting = [
            e for e in extensions_list
            if (e == '' or
                flags.Tracking.__getattr__(e+'Pass').storeSiSPSeededTracks) ]
        for extension in listOfExtensionsRequesting:
            toAOD += [
                f"xAOD::TrackParticleContainer#SiSPSeededTracks{extension}TrackParticles",
                f"xAOD::TrackParticleAuxContainer#SiSPSeededTracks{extension}TrackParticlesAux.{excludedAuxData}"
            ]

    if flags.Tracking.doStoreTrackSeeds:
        # get list of extensions requesting track seeds. Add always the Primary Pass.
        listOfExtensionsRequesting = [
            e for e in extensions_list
            if (e == '' or flags.Tracking.__getattr__(e+'Pass').storeTrackSeeds) ]
        for extension in listOfExtensionsRequesting:
            toAOD += [
                f"xAOD::TrackParticleContainer#SiSPSeedSegments{extension}TrackParticles",
                f"xAOD::TrackParticleAuxContainer#SiSPSeedSegments{extension}TrackParticlesAux."
            ]

    if (flags.Tracking.writeExtendedSi_PRDInfo or
        flags.Tracking.writeExtendedTRT_PRDInfo):
        toAOD += [
            "xAOD::TrackMeasurementValidationContainer#PixelClusters",
            "xAOD::TrackMeasurementValidationAuxContainer#PixelClustersAux.",
            "xAOD::TrackMeasurementValidationContainer#SCT_Clusters",
            "xAOD::TrackMeasurementValidationAuxContainer#SCT_ClustersAux.",
            "xAOD::TrackMeasurementValidationContainer#TRT_DriftCircles",
            "xAOD::TrackMeasurementValidationAuxContainer#TRT_DriftCirclesAux."
        ]
        # get list of extensions requesting track seeds. Add always the Primary Pass.
        listOfExtensionsRequesting = [
            e for e in extensions_list
            if (e == '' or flags.Tracking.__getattr__(e+'Pass').storeTrackSeeds) ]
        for extension in listOfExtensionsRequesting:
            toAOD += [
                f"xAOD::TrackStateValidationContainer#{extension}Pixel_MSOSs",
                f"xAOD::TrackStateValidationAuxContainer#{extension}Pixel_MSOSsAux.",
                f"xAOD::TrackStateValidationContainer#{extension}SCT_MSOSs",
                f"xAOD::TrackStateValidationAuxContainer#{extension}SCT_MSOSsAux.",
                f"xAOD::TrackStateValidationContainer#{extension}TRT_MSOSs",
                f"xAOD::TrackStateValidationAuxContainer#{extension}TRT_MSOSsAux."
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
        if flags.Tracking.doStoreSiSPSeededTracks:
            # get list of extensions requesting track seeds. Add always the Primary Pass.
            listOfExtensionsRequesting = [
                e for e in extensions_list
                if (e == '' or flags.Tracking.__getattr__(e+'Pass').storeTrackSeeds) ]
            for extension in listOfExtensionsRequesting:
                toAOD += [
                    f"xAOD::TrackStateValidationContainer#SiSP{extension}_Pixel_MSOSs",
                    f"xAOD::TrackStateValidationAuxContainer#SiSP{extension}_Pixel_MSOSsAux.",
                    f"xAOD::TrackStateValidationContainer#SiSP{extension}_SCT_MSOSs",
                    f"xAOD::TrackStateValidationAuxContainer#SiSP{extension}_SCT_MSOSsAux.",
                    f"xAOD::TrackStateValidationContainer#SiSP{extension}_TRT_MSOSs",
                    f"xAOD::TrackStateValidationAuxContainer#SiSP{extension}_TRT_MSOSsAux."
                ]

    if flags.Tracking.doV0Finder:
        excludedVtxAuxData = "-vxTrackAtVertex.-MvfFitInfo.-isInitialized.-VTAV"
        V0Vertices = ["V0UnconstrVertices", "V0KshortVertices",
                      "V0LambdaVertices", "V0LambdabarVertices"]
        for v0 in V0Vertices:
            toAOD += [
                f"xAOD::VertexContainer#{v0}",
                f"xAOD::VertexAuxContainer#{v0}Aux.{excludedVtxAuxData}",
            ]

    result = ComponentAccumulator()
    result.merge(addToESD(flags, toESD + toAOD))
    result.merge(addToAOD(flags, toAOD))

    return result

