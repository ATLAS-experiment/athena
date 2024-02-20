# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def PersistifyActsEDMCfg(flags) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    toAOD = []

    if flags.Acts.EDM.PersistifyClusters or flags.Acts.EDM.PersistifySpacePoints:
        pixel_cluster_shortlist = ['-pixelClusterLink']
        strip_cluster_shortlist = ['-sctClusterLink']
        
        pixel_cluster_variables = '.'.join(pixel_cluster_shortlist)
        strip_cluster_variables = '.'.join(strip_cluster_shortlist)

        toAOD += ['xAOD::PixelClusterContainer#ITkPixelClusters',
                  'xAOD::PixelClusterAuxContainer#ITkPixelClustersAux.' + pixel_cluster_variables,
                  'xAOD::StripClusterContainer#ITkStripClusters',
                  'xAOD::StripClusterAuxContainer#ITkStripClustersAux.' + strip_cluster_variables]

        if flags.Acts.doITkConversion:
            toAOD += ['xAOD::StripClusterContainer#ITkConversionStripClusters',
                      'xAOD::StripClusterAuxContainer#ITkConversionStripClustersAux.' + strip_cluster_variables]
        
    if flags.Acts.EDM.PersistifySpacePoints:
        pixel_spacepoint_shortlist = ['-measurements']
        strip_spacepoint_shortlist = ['topHalfStripLength', 
                                      'bottomHalfStripLength', 
                                      'topStripDirection',
                                      'bottomStripDirection',
                                      'stripCenterDistance',
                                      'topStripCenter',
                                      'measurementLink']

        pixel_spacepoint_variables = '.'.join(pixel_spacepoint_shortlist)
        strip_spacepoint_variables = '.'.join(strip_spacepoint_shortlist)
        
        toAOD += ['xAOD::SpacePointContainer#ITkPixelSpacePoints',
                  'xAOD::SpacePointAuxContainer#ITkPixelSpacePointsAux.' + pixel_spacepoint_variables,
                  'xAOD::SpacePointContainer#ITkStripSpacePoints',
                  'xAOD::SpacePointAuxContainer#ITkStripSpacePointsAux.' + strip_spacepoint_variables,
                  'xAOD::SpacePointContainer#ITkStripOverlapSpacePoints',
                  'xAOD::SpacePointAuxContainer#ITkStripOverlapSpacePointsAux.' + strip_spacepoint_variables]

    if flags.Acts.EDM.PersistifyTracks:

        toAOD +=  ["xAOD::TrackSummaryContainer#SiSPSeededActsTrackTrackSummary",
                   "xAOD::TrackSummaryAuxContainer#SiSPSeededActsTrackTrackSummaryAux.",
                   "xAOD::TrackStateContainer#SiSPSeededActsTrackStates",
                   "xAOD::TrackStateAuxContainer#SiSPSeededActsTrackStatesAux.",                
                   "xAOD::TrackParametersContainer#SiSPSeededActsTrackParameters",
                   "xAOD::TrackParametersAuxContainer#SiSPSeededActsTrackParametersAux.",
                   "xAOD::TrackJacobianContainer#SiSPSeededActsTrackJacobians",
                   "xAOD::TrackJacobianAuxContainer#SiSPSeededActsTrackJacobiansAux.",
                   "xAOD::TrackMeasurementContainer#SiSPSeededActsTrackMeasurements",
                   "xAOD::TrackMeasurementAuxContainer#SiSPSeededActsTrackMeasurementsAux.",
                   "xAOD::TrackSurfaceContainer#SiSPSeededActsTrackStateSurfaces",
                   "xAOD::TrackSurfaceAuxContainer#SiSPSeededActsTrackStateSurfacesAux.",
                   "xAOD::TrackSurfaceContainer#SiSPSeededActsTrackSurfaces",
                   "xAOD::TrackSurfaceAuxContainer#SiSPSeededActsTrackSurfacesAux."]
                
    # If there is nothing to persistify, returns an empty CA
    if len(toAOD) == 0:
        return acc

    from OutputStreamAthenaPool.OutputStreamConfig import addToAOD    
    acc.merge(addToAOD(flags, toAOD))
    return acc
