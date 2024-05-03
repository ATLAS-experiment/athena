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

        if flags.Reco.EnableHGTDExtension:
            hgtd_cluster_shortlist = ['-hgtdClusterLink']

            hgtd_cluster_variables = '.'.join(hgtd_cluster_shortlist)
            
            toAOD += ['xAOD::HGTDClusterContainer#HGTD_Clusters',
                      'xAOD::HGTDClusterAuxContainer#HGTD_ClustersAux.' + hgtd_cluster_variables]

        if flags.Acts.doITkConversion:
            toAOD += ['xAOD::StripClusterContainer#ITkConversionStripClusters',
                      'xAOD::StripClusterAuxContainer#ITkConversionStripClustersAux.' + strip_cluster_variables]
        
    if flags.Acts.EDM.PersistifySpacePoints:
        pixel_spacepoint_shortlist = ['-measurements',
                                      '-pixelSpacePointLink']
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
        trackPrefixes = ['Acts', 'ActsResolved',
                         'ActsConversion', 'ActsConversionResolved',
                         'ActsHeavyIon', 'ActsHeavyIonResolved',
                         'ActsCombined']
        for prefix in trackPrefixes:
            toAOD +=  [f"xAOD::TrackSummaryContainer#{prefix}TrackSummary",
                       f"xAOD::TrackSummaryAuxContainer#{prefix}TrackSummaryAux.",
                       f"xAOD::TrackStateContainer#{prefix}TrackStates",
                       f"xAOD::TrackStateAuxContainer#{prefix}TrackStatesAux.",                
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
                
    # If there is nothing to persistify, returns an empty CA
    if len(toAOD) == 0:
        return acc

    from OutputStreamAthenaPool.OutputStreamConfig import addToAOD    
    acc.merge(addToAOD(flags, toAOD))
    return acc
