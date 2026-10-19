# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.Enums import BeamType

def TRTSegmentFindingCfg(flags, InputCollections = None,
                         ExtraAssociationTracks = None):
    acc = ComponentAccumulator()

    #
    # --- get list of already associated hits (always do this, even if no other tracking ran before)
    #
    if InputCollections is not None:
        from InDetConfig.InDetTrackPRD_AssociationConfig import (
            InDetTrackPRD_AssociationCfg)
        acc.merge(InDetTrackPRD_AssociationCfg(
            flags,
            name = 'InDetSegmentTrackPRD_Association',
            AssociationMapName = 'InDetSegmentPRDtoTrackMap',
            TracksName = list(InputCollections) + list(ExtraAssociationTracks or [])))

    #
    # --- TRT track reconstruction
    #
    if flags.Beam.Type is BeamType.Cosmics:
        from InDetConfig.TRT_TrackSegmentsFinderConfig import (
            TRT_TrackSegmentsFinder_Cosmics_Cfg)
        acc.merge(TRT_TrackSegmentsFinder_Cosmics_Cfg(flags))
    else:
        from InDetConfig.TRT_TrackSegmentsFinderConfig import (
            TRT_TrackSegmentsFinderCfg)
        acc.merge(TRT_TrackSegmentsFinderCfg(flags,
                                             InputCollections = InputCollections))

        #
        # --- load TRT validation alg
        #
        if flags.Tracking.doTruth:
            from InDetConfig.InDetSegmentDriftCircleAssValidationConfig import (
                SegmentDriftCircleAssValidationCfg)
            acc.merge(SegmentDriftCircleAssValidationCfg(flags))

    return acc

def TRTSegmentFinding_Phase_Cfg(flags):
    acc = ComponentAccumulator()

    #
    # --- TRT track reconstruction
    #
    if flags.Beam.Type is BeamType.Cosmics:
        from InDetConfig.TRT_TrackSegmentsFinderConfig import (
            TRT_TrackSegmentsFinder_Cosmics_Cfg)
        acc.merge(TRT_TrackSegmentsFinder_Cosmics_Cfg(
            flags,
            name='InDetTRT_TrackSegmentsFinder_Phase_Cosmics',
            SegmentsLocation = 'TRTSegments_Phase'))
    else:
        from InDetConfig.TRT_TrackSegmentsFinderConfig import (
            TRT_TrackSegmentsFinder_Phase_Cfg)
        acc.merge(TRT_TrackSegmentsFinder_Phase_Cfg(flags))

        #
        # --- load TRT validation alg
        #
        if flags.Tracking.doTruth:
            from InDetConfig.InDetSegmentDriftCircleAssValidationConfig import (
                SegmentDriftCircleAssValidationCfg)
            acc.merge(SegmentDriftCircleAssValidationCfg(
                flags,
                name = "InDetSegmentDriftCircleAssValidation_Phase"))

    return acc

def TRTSegmentFinding_TrackSegments_Cfg(flags):
    acc = ComponentAccumulator()

    #
    # --- TRT track reconstruction
    #
    if flags.Beam.Type is BeamType.Cosmics:
        from InDetConfig.TRT_TrackSegmentsFinderConfig import (
            TRT_TrackSegmentsFinder_Cosmics_Cfg)
        acc.merge(TRT_TrackSegmentsFinder_Cosmics_Cfg(
            flags,
            name='InDetTRT_TrackSegmentsFinder_TrackSegments_Cosmics',
            SegmentsLocation = 'TRTSegmentsTRT'))
    else:
        from InDetConfig.TRT_TrackSegmentsFinderConfig import (
            TRT_TrackSegmentsFinder_TrackSegments_Cfg)
        acc.merge(TRT_TrackSegmentsFinder_TrackSegments_Cfg(flags))

        #
        # --- load TRT validation alg
        #
    
        if flags.Tracking.doTruth:
            from InDetConfig.InDetSegmentDriftCircleAssValidationConfig import (
                SegmentDriftCircleAssValidation_TrackSegments_Cfg)
            acc.merge(SegmentDriftCircleAssValidation_TrackSegments_Cfg(flags))

    return acc


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags.Input.Files=defaultTestFiles.RDO_RUN2

    # disable calo for this test
    flags.Detector.EnableCalo = False

    numThreads=1
    flags.Concurrency.NumThreads=numThreads
    flags.Concurrency.NumConcurrentEvents=numThreads # Might change this later, but good enough for the moment.

    flags = flags.cloneAndReplace("Tracking.ActiveConfig","Tracking.MainPass")

    flags.lock()
    flags.dump()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    top_acc = MainServicesCfg(flags)
    
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    top_acc.merge(PoolReadCfg(flags))

    from InDetConfig.TRTPreProcessing import TRTPreProcessingCfg
    top_acc.merge(TRTPreProcessingCfg(flags))
    top_acc.merge(TRTSegmentFindingCfg(flags))

    iovsvc = top_acc.getService('IOVDbSvc')
    iovsvc.OutputLevel=5

    top_acc.store(open("test_TRTSegmentFinding.pkl", "wb"))

    import sys
    if "--norun" not in sys.argv:
        sc = top_acc.run(5)
        sys.exit(not sc.isSuccess())
