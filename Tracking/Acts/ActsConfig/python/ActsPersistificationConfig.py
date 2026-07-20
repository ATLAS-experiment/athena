# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def PersistifyClusters(flags,
                       *,
                       pixelClusterCollections: list[str] = None,
                       stripClusterCollections: list[str] = None,
                       hgtdClusterCollections: list[str] = None) -> ComponentAccumulator:
    toAOD = []
    if pixelClusterCollections is not None:
        pixel_cluster_shortlist = ['-pixelClusterLink',
                                   '-validationMeasurementLink']
        pixel_cluster_variables = '.'.join(pixel_cluster_shortlist)
        for pixelClusterCollection in pixelClusterCollections:
            toAOD += [f'xAOD::PixelClusterContainer#{pixelClusterCollection}',
                      f'xAOD::PixelClusterAuxContainer#{pixelClusterCollection}Aux.{pixel_cluster_variables}']

    if stripClusterCollections is not None:
        strip_cluster_shortlist = ['-sctClusterLink',
                                   '-validationMeasurementLink']
        strip_cluster_variables = '.'.join(strip_cluster_shortlist)
        for stripClusterCollection in stripClusterCollections:
            toAOD += [f"xAOD::StripClusterContainer#{stripClusterCollection}",
                      f"xAOD::StripClusterAuxContainer#{stripClusterCollection}Aux.{strip_cluster_variables}"]

    if hgtdClusterCollections is not None:
        hgtd_cluster_shortlist = ['-hgtdClusterLink']
        hgtd_cluster_variables = '.'.join(hgtd_cluster_shortlist)
        for hgtdClusterCollection in hgtdClusterCollections:
            toAOD += [f"xAOD::HGTDClusterContainer#{hgtdClusterCollection}",
                      f"xAOD::HGTDClusterAuxContainer#{hgtdClusterCollection}Aux.{hgtd_cluster_variables}"]

    acc = ComponentAccumulator()
    if len(toAOD) == 0:
        return acc

    from OutputStreamAthenaPool.OutputStreamConfig import addToAOD    
    acc.merge(addToAOD(flags, toAOD))
    return acc


def PersistifySpacePoints(flags,
                          *,
                          pixelSpacePointCollections: list[str] = None,
                          stripSpacePointCollections: list[str] = None) -> ComponentAccumulator:
    toAOD = []
    aux_container_type = "xAOD::SpacePointAuxContainer"
    if flags.Acts.EDM.SlimContent:
        aux_container_type = "xAOD::AuxContainerBase!"

    if pixelSpacePointCollections is not None:
        pixel_spacepoint_shortlist = ['-measurements',
                                      '-pixelSpacePointLink']
        if flags.Acts.EDM.SlimContent:
            pixel_spacepoint_shortlist = ['globalPosition']
        pixel_spacepoint_variables = '.'.join(pixel_spacepoint_shortlist)
        for pixelSpacePointCollection in pixelSpacePointCollections:
            toAOD += [f'xAOD::SpacePointContainer#{pixelSpacePointCollection}',
                      f"{aux_container_type}#{pixelSpacePointCollection}Aux.{pixel_spacepoint_variables}"]

    if stripSpacePointCollections is not None:
        strip_spacepoint_shortlist = ['topHalfStripLength', 
                                      'bottomHalfStripLength', 
                                      'topStripDirection',
                                      'bottomStripDirection',
                                      'stripCenterDistance',
                                      'topStripCenter',
                                      'measurementLink']
        if flags.Acts.EDM.SlimContent:
            strip_spacepoint_shortlist = ['globalPosition']
        strip_spacepoint_variables = '.'.join(strip_spacepoint_shortlist)
        for stripSpacePointCollection in stripSpacePointCollections:
            toAOD += [f'xAOD::SpacePointContainer#{stripSpacePointCollection}',
                      f"{aux_container_type}#{stripSpacePointCollection}Aux.{strip_spacepoint_variables}"]
            
    acc = ComponentAccumulator()
    if len(toAOD) == 0:
        return acc

    from OutputStreamAthenaPool.OutputStreamConfig import addToAOD
    acc.merge(addToAOD(flags, toAOD))
    return acc


def PersistifyTracks(flags,
                     *,
                     extensions: list[str] = None) -> ComponentAccumulator:
    toAOD = []
    if extensions is not None:
        for prefix in extensions:
            toAOD += [f"xAOD::TrackSummaryContainer#{prefix}TrackSummary",
                      f"xAOD::TrackSummaryAuxContainer#{prefix}TrackSummaryAux.",
                      f"xAOD::TrackStateContainer#{prefix}TrackStates",
                      f"xAOD::TrackStateAuxContainer#{prefix}TrackStatesAux.-uncalibratedMeasurement",
                      f"xAOD::TrackParametersContainer#{prefix}TrackParameters",
                      f"xAOD::TrackParametersAuxContainer#{prefix}TrackParametersAux.",
                      f"xAOD::TrackJacobianContainer#{prefix}TrackJacobians",
                      f"xAOD::TrackJacobianAuxContainer#{prefix}TrackJacobiansAux.",
                      f"xAOD::TrackMeasurementContainer#{prefix}TrackMeasurements",
                      f"xAOD::TrackMeasurementAuxContainer#{prefix}TrackMeasurementsAux.",
                      f"xAOD::TrackSurfaceContainer#{prefix}TrackStateSurfaces",
                      f"xAOD::TrackSurfaceAuxContainer#{prefix}TrackStateSurfacesAux.",
                      f"xAOD::TrackSurfaceContainer#{prefix}TrackSurfaces",
                      f"xAOD::TrackSurfaceAuxContainer#{prefix}TrackSurfacesAux."]
    
    acc = ComponentAccumulator()
    if len(toAOD) == 0:
        return acc

    from OutputStreamAthenaPool.OutputStreamConfig import addToAOD
    acc.merge(addToAOD(flags, toAOD))
    return acc


def PersistifyTrackParticles(flags,
                             *,
                             trackParticleCollections: list[str] = None) -> ComponentAccumulator:
    toAOD = []
    if trackParticleCollections is not None:
        trackparticles_shortlist = ['-clusterAssociation',
                                    '-TTVA_AMVFVertices_forReco',
                                    '-AssoClustersUFO',
                                    '-TTVA_AMVFWeights_forReco',
                                    '-trackParameterCovarianceMatrices',
                                     '-parameterX', '-parameterY', '-parameterZ',
                                     '-parameterPX', '-parameterPY', '-parameterPZ',
                                     '-parameterPosition']

        # exclude TTVA decorations
        trackparticles_shortlist += ['-TTVA_AMVFVertices',
                                     '-TTVA_AMVFWeights']
        # acts track link
        if not flags.Acts.EDM.PersistifyTracks:
            trackparticles_shortlist.append('-actsTrack')

        trackparticles_variables = ".".join(trackparticles_shortlist)        
        # remove track decorations used internally by FTAG software
        from InDetConfig.InDetTrackOutputConfig import FTAG_AUXDATA
        trackparticles_variables += '.-'.join([''] + FTAG_AUXDATA)
        # exclude IDTIDE decorations
        from DerivationFrameworkInDet.IDTIDE import IDTIDE_AOD_EXCLUDED_AUXDATA
        trackparticles_variables += '.-'.join([''] + IDTIDE_AOD_EXCLUDED_AUXDATA)
        from DerivationFrameworkInDet.IDTRKVALID import IDTRKVALID_AOD_EXCLUDED_AUXDATA
        trackparticles_variables += '.-'.join([''] + IDTRKVALID_AOD_EXCLUDED_AUXDATA)

        for trackParticleCollection in trackParticleCollections:
            toAOD += [f"xAOD::TrackParticleContainer#{trackParticleCollection}",
                      f"xAOD::TrackParticleAuxContainer#{trackParticleCollection}Aux." + trackparticles_variables]
    
    acc = ComponentAccumulator()
    if len(toAOD) == 0:
        return acc

    from OutputStreamAthenaPool.OutputStreamConfig import addToAOD
    acc.merge(addToAOD(flags, toAOD))
    return acc
