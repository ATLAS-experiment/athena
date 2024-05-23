# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.Enums import BeamType

def TRTStandaloneCfg(flags, InputCollections = None):
    acc = ComponentAccumulator()

    #
    # --- get list of already associated hits (always do this, even if no other tracking ran before)
    #
    prd_to_track_map = ''
    if flags.Tracking.ActiveConfig.usePrdAssociationTool:
        prd_to_track_map = 'InDetTRTonly_PRDtoTrackMap'
        from InDetConfig.InDetTrackPRD_AssociationConfig import (
            InDetTrackPRD_AssociationCfg)
        acc.merge(InDetTrackPRD_AssociationCfg(flags,
                                               name = 'InDetTRTonly_TrackPRD_Association',
                                               AssociationMapName = prd_to_track_map,
                                               TracksName = list(InputCollections)))

    if flags.Beam.Type is BeamType.Cosmics:
        #
        # --- cosmics segment to track conversion for Barrel
        #
        from InDetConfig.TRT_SegmentsToTrackConfig import TRT_Cosmics_SegmentsToTrackCfg
        acc.merge(TRT_Cosmics_SegmentsToTrackCfg(flags, name = 'InDetTRT_SegmentsToTrack_Barrel',
                                                 OutputTrackCollection = 'TRTStandaloneTracks',
                                                 InputSegmentsCollection = 'TRTSegments',
                                                 InputAssociationMapName = prd_to_track_map))

    else:
        #
        # --- TRT standalone tracks algorithm
        #
        from InDetConfig.TRT_StandaloneTrackFinderConfig import (
            TRT_StandaloneTrackFinderCfg)
        acc.merge(TRT_StandaloneTrackFinderCfg(flags,
                                               InputSegmentsLocation = 'TRTSegments',
                                               PRDtoTrackMap = prd_to_track_map))
        if flags.Tracking.doTruth:
            from InDetConfig.TrackTruthConfig import InDetTrackTruthCfg
            acc.merge(InDetTrackTruthCfg(
                flags,
                Tracks = "TRTStandaloneTracks",
                DetailedTruth = "TRTStandaloneTracksDetailedTruth",
                TracksTruth = "TRTStandaloneTracksTruthCollection"))

    return acc

def TRT_TrackSegment_Cfg(flags):
    acc = ComponentAccumulator()

    if flags.Beam.Type is BeamType.Cosmics:
        #
        # --- cosmics segment to track conversion for Barrel
        #
        from InDetConfig.TRT_SegmentsToTrackConfig import TRT_Cosmics_SegmentsToTrackCfg
        acc.merge(TRT_Cosmics_SegmentsToTrackCfg(flags, name = 'InDetTRT_Cosmics_SegmentsToTrack',
                                                 OutputTrackCollection = 'StandaloneTRTTracks',
                                                 InputSegmentsCollection = 'TRTSegmentsTRT'))

    else:
        #
        # --- TRT standalone tracks algorithm
        #
        from InDetConfig.TRT_StandaloneTrackFinderConfig import (
            TRT_TrackSegment_TrackFinderCfg)
        acc.merge(TRT_TrackSegment_TrackFinderCfg(flags,
                                                  InputSegmentsLocation = 'TRTSegmentsTRT'))


    return acc


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags.Input.Files=defaultTestFiles.RDO_RUN2

    # disable calo for this test
    flags.Detector.EnableCalo = False

    # TODO: TRT only?

    numThreads=1
    flags.Concurrency.NumThreads=numThreads
    flags.Concurrency.NumConcurrentEvents=numThreads


    flags.lock()
    flags.dump()
    
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    top_acc = MainServicesCfg(flags)

    ################################ Aditional configurations ###############################
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    top_acc.merge(PoolReadCfg(flags))

    from TRT_GeoModel.TRT_GeoModelConfig import TRT_ReadoutGeometryCfg
    top_acc.merge(TRT_ReadoutGeometryCfg( flags ))

    from PixelGeoModel.PixelGeoModelConfig import PixelReadoutGeometryCfg
    top_acc.merge( PixelReadoutGeometryCfg(flags) )

    from SCT_GeoModel.SCT_GeoModelConfig import SCT_ReadoutGeometryCfg
    top_acc.merge(SCT_ReadoutGeometryCfg(flags))

    ############################# TRTPreProcessing configuration ############################
    from InDetConfig.TRTPreProcessing import TRTPreProcessingCfg
    top_acc.merge(TRTPreProcessingCfg(flags))
    ########################### TRTSegmentFindingCfg configuration ##########################
    from InDetConfig.TRTSegmentFindingConfig import TRTSegmentFindingCfg
    top_acc.merge(TRTSegmentFindingCfg(flags))
    #########################################################################################
    ############################### TRTStandalone configuration #############################
    top_acc.merge(TRTStandaloneCfg(flags))

    iovsvc = top_acc.getService('IOVDbSvc')
    iovsvc.OutputLevel=5

    top_acc.run(25)
    top_acc.store(open("test_TRTStandaloneConfig.pkl", "wb"))
