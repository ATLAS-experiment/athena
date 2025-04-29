# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from InDetConfig.InDetTrackOutputConfig import FTAG_AUXDATA

def ITkTrackRecoOutputCfg(flags, extensions_list=None):
    if extensions_list is None:
        extensions_list = []

    from OutputStreamAthenaPool.OutputStreamConfig import addToESD, addToAOD
    toAOD = []
    toESD = []

    # excluded track aux data
    excludedAuxData = ('-clusterAssociation.-TTVA_AMVFVertices_forReco.-AssoClustersUFO'
                       '.-TTVA_AMVFWeights_forReco')
    # remove track decorations used internally by FTAG software
    excludedAuxData += '.-'.join([''] + FTAG_AUXDATA)

    # exclude TTVA decorations
    excludedAuxData += '.-TTVA_AMVFVertices.-TTVA_AMVFWeights'

    # exclude IDTIDE decorations
    from DerivationFrameworkInDet.IDTIDE import IDTIDE_AOD_EXCLUDED_AUXDATA
    excludedAuxData += '.-'.join([''] + IDTIDE_AOD_EXCLUDED_AUXDATA)
    from DerivationFrameworkInDet.IDTRKVALID import IDTRKVALID_AOD_EXCLUDED_AUXDATA
    excludedAuxData += '.-'.join([''] + IDTRKVALID_AOD_EXCLUDED_AUXDATA)

    if not flags.Tracking.writeExtendedSi_PRDInfo:
        excludedAuxData += '.-msosLink'

    # Save PRD
    toESD += [
        "InDet::SCT_ClusterContainer#ITkStripClusters",
        "InDet::PixelClusterContainer#ITkPixelClusters",
        "InDet::PixelGangedClusterAmbiguities#ITkPixelClusterAmbiguitiesMap",
    ]
    if flags.Tracking.doPixelClusterSplitting:
        toESD += [
            "InDet::PixelGangedClusterAmbiguities#ITkSplitClusterAmbiguityMap"]

    from InDetConfig.ITkTrackRecoConfig import ITkClusterSplitProbabilityContainerName
    toESD += ["Trk::ClusterSplitProbabilityContainer#" +
              ITkClusterSplitProbabilityContainerName(flags)]

    # Save (Detailed) Track Truth
    if flags.Tracking.doTruth:
        toESD += [
            "TrackTruthCollection#CombinedITkTracksTrackTruthCollection",
            "DetailedTrackTruthCollection#CombinedITkTracksDetailedTrackTruth"]

    if flags.Tracking.doStoreTrackSeeds:
        listOfExtensionsRequesting = [
            e for e in extensions_list
            if (e == '' or flags.Tracking[f"ITk{e}Pass"].storeTrackSeeds) ]

        for extension in listOfExtensionsRequesting:
            toESD += ["TrackCollection#SiSPSeedSegments"+extension]

    toESD += ["TrackCollection#CombinedITkTracks"]

    ##### AOD #####
    toAOD += [
        "xAOD::TrackParticleContainer#InDetTrackParticles",
        f"xAOD::TrackParticleAuxContainer#InDetTrackParticlesAux.{excludedAuxData}"
    ]

    # This should be activated only if both Legacy and Acts-based tracking
    # are executed at the same time during reconstruction
    toAOD += [
        "xAOD::TrackParticleContainer#ActsInDetTrackParticles",
        f"xAOD::TrackParticleAuxContainer#ActsInDetTrackParticlesAux.{excludedAuxData}"]

    if flags.Tracking.writeExtendedSi_PRDInfo:
        # Different convention wrt Run 3 for TrackMeasurementValidationContainer
        # from ITkPixelClusters to ITkPixelMeasurements
        # This is to avoid clashes with the xAOD::ClusterContainer names, which was not
        # an issue for Run 3 since they were not xAOD back then
        toAOD += [
            "xAOD::TrackMeasurementValidationContainer#ITkPixelMeasurements",
            "xAOD::TrackMeasurementValidationAuxContainer#ITkPixelMeasurementsAux.",
            "xAOD::TrackMeasurementValidationContainer#ITkStripMeasurements",
            "xAOD::TrackMeasurementValidationAuxContainer#ITkStripMeasurementsAux.",
            "xAOD::TrackStateValidationContainer#ITkPixelMSOSs",
            "xAOD::TrackStateValidationAuxContainer#ITkPixelMSOSsAux.",
            "xAOD::TrackStateValidationContainer#ITkStripMSOSs",
            "xAOD::TrackStateValidationAuxContainer#ITkStripMSOSsAux."
        ]

        if flags.Tracking.doStoreSiSPSeededTracks:
            toAOD += [
                "xAOD::TrackStateValidationContainer#SiSP_ITkPixel_MSOSs",
                "xAOD::TrackStateValidationAuxContainer#SiSP_ITkPixel_MSOSsAux.",
                "xAOD::TrackStateValidationContainer#SiSP_ITkStrip_MSOSs",
                "xAOD::TrackStateValidationAuxContainer#SiSP_ITkStrip_MSOSsAux."
            ]

    if (flags.Tracking.doLargeD0 and
            flags.Tracking.storeSeparateLargeD0Container):
        toAOD += [
            "xAOD::TrackParticleContainer#InDetLargeD0TrackParticles",
            f"xAOD::TrackParticleAuxContainer#InDetLargeD0TrackParticlesAux.{excludedAuxData}"
        ]

    if flags.Tracking.doStoreSiSPSeededTracks:
        # get list of extensions requesting track candidates. Add always the Primary Pass.
        listOfExtensionsRequesting = [
            e for e in extensions_list
            if (e == '' or flags.Tracking[f"ITk{e}Pass"].storeSiSPSeededTracks) ]

        for extension in listOfExtensionsRequesting:
            toAOD += [
                f"xAOD::TrackParticleContainer#SiSPSeededTracks{extension}TrackParticles",
                f"xAOD::TrackParticleAuxContainer#SiSPSeededTracks{extension}TrackParticlesAux.{excludedAuxData}"]

    if flags.Tracking.doStoreTrackSeeds:
        # get list of extensions requesting track seeds. Add always the Primary Pass.
        listOfExtensionsRequesting = [
            e for e in extensions_list
            if (e == '' or flags.Tracking[f"ITk{e}Pass"].storeTrackSeeds) ]
        for extension in listOfExtensionsRequesting:
            toAOD += [
                f"xAOD::TrackParticleContainer#SiSPSeedSegments{extension}PixelTrackParticles",
                f"xAOD::TrackParticleAuxContainer#SiSPSeedSegments{extension}PixelTrackParticlesAux.",
                f"xAOD::TrackParticleContainer#SiSPSeedSegments{extension}StripTrackParticles",
                f"xAOD::TrackParticleAuxContainer#SiSPSeedSegments{extension}StripTrackParticlesAux."
            ]

    result = ComponentAccumulator()
    result.merge(addToESD(flags, toAOD+toESD))
    result.merge(addToAOD(flags, toAOD))
    return result
