# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
# 
# Heavily Inspired by ThinInDetForwardTrackParticles.py

__doc__ = """
          Instantiate the InDetTrackParticles Cluster Thinning
          """

from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def ThinInDetClustersCfg(flags, name="ThinInDetClustersAlg", **kwargs):
    mlog = logging.getLogger(name)
    mlog.info("Starting InDetTrackParticle Cluster Thinning configuration")
    acc = ComponentAccumulator()

    kwargs.setdefault("StreamName", "StreamAOD")

    if not flags.Detector.EnablePixel:
        kwargs.setdefault("InDetTrackStatesPixKey", "")
        kwargs.setdefault("InDetTrackMeasurementsPixKey", "")
    if not flags.Detector.EnableSCT:
        kwargs.setdefault("InDetTrackStatesSctKey", "")
        kwargs.setdefault("InDetTrackMeasurementsSctKey", "")
    if not flags.Detector.EnableTRT:
        kwargs.setdefault("InDetTrackStatesTrtKey", "")
        kwargs.setdefault("InDetTrackMeasurementsTrtKey", "")

    if not flags.Tracking.thinPixelClustersOnTrack:
        kwargs.setdefault("ThinPixelHitsOnTrack", False)
    if not flags.Tracking.thinSCTClustersOnTrack:
        kwargs.setdefault("ThinSCTHitsOnTrack", False)
    if not flags.Tracking.thinTRTClustersOnTrack:
        kwargs.setdefault("ThinTRTHitsOnTrack", False)

    kwargs.setdefault("InDetTrackParticlesKey", "InDetTrackParticles")
    kwargs.setdefault("SelectionString", "InDetTrackParticles.pt>10*GeV")

    ################
    # Cross-checks #
    ################
    
    if flags.Tracking.thinPixelClustersOnTrack and not flags.Tracking.writeExtendedSi_PRDInfo:
        msg = "Requested to thin pixel hits on track, but flags.Tracking.writeExtendedSi_PRDInfo is set to false, so the relevant containers will not be written to the AOD.  Cannot add ThinInDetClustersAlg."
        mlog.error( msg )
        raise RuntimeError( msg )

    if flags.Tracking.thinSCTClustersOnTrack and not flags.Tracking.writeExtendedSi_PRDInfo:
        msg = "Requested to thin SCT hits on track, but flags.Tracking.writeExtendedSi_PRDInfo is set to false, so the relevant containers will not be written to the AOD.  Cannot add ThinInDetClustersAlg."
        mlog.error( msg )
        raise RuntimeError( msg )

    if flags.Tracking.thinTRTClustersOnTrack and not flags.Tracking.writeExtendedTRT_PRDInfo:
        msg = "Requested to thin TRT hits on track, but flags.Tracking.writeExtendedTRT_PRDInfo is set to false, so the relevant containers will not be written to the AOD.  Cannot add ThinInDetClustersAlg."
        mlog.error( msg )
        raise RuntimeError( msg )

    nTrackContainers = len(flags.Tracking.thinInDetClustersTrackContainers)
    nSelectionStrings = len(flags.Tracking.thinInDetClustersSelectionStrings)
    nPixelMSOSContainers = len(flags.Tracking.thinInDetClustersPixelMSOSContainers)
    nSCTMSOSContainers = len(flags.Tracking.thinInDetClustersSCTMSOSContainers)
    nTRTMSOSContainers = len(flags.Tracking.thinInDetClustersTRTMSOSContainers)

    if not nTrackContainers == nSelectionStrings:
        msg = f"Number of selection strings ({nSelectionStrings}) does not match number of track containers ({nTrackContainers}).  Cannot add ThinInDetClustersAlg."
        mlog.error( msg )
        raise RuntimeError( msg )

    if flags.Tracking.thinPixelClustersOnTrack:
        if not nTrackContainers == nPixelMSOSContainers:
            msg = f"Number of pixel MSOS containers ({nPixelMSOSContainers}) does not match number of track containers ({nTrackContainers}).  Cannot add ThinInDetClustersAlg."
            mlog.error( msg )
            raise RuntimeError( msg )

    if flags.Tracking.thinSCTClustersOnTrack:
        if not nTrackContainers == nSCTMSOSContainers:
            msg = f"Number of SCT MSOS containers ({nSCTMSOSContainers}) does not match number of track containers ({nTrackContainers}).  Cannot add ThinInDetClustersAlg."
            mlog.error( msg )
            raise RuntimeError( msg )
    
    if flags.Tracking.thinTRTClustersOnTrack:
        if not nTrackContainers == nTRTMSOSContainers:
            msg = f"Number of TRT MSOS containers ({nTRTMSOSContainers}) does not match number of track containers ({nTrackContainers}).  Cannot add ThinInDetClustersAlg."
            mlog.error( msg )
            raise RuntimeError( msg )
        
    for idx,trackContainer in enumerate(flags.Tracking.thinInDetClustersTrackContainers):
        selectionString = flags.Tracking.thinInDetClustersSelectionStrings[idx]
        if trackContainer not in selectionString:
            mlog.warning(f"Track container {trackContainer} is not used in the associated selection string: {selectionString}.") 

    ##############################
    # Loop over track containers # 
    ##############################

    for idx,trackContainer in enumerate(flags.Tracking.thinInDetClustersTrackContainers):
        kwargs["InDetTrackParticlesKey"]= trackContainer
        kwargs["SelectionString"] = flags.Tracking.thinInDetClustersSelectionStrings[idx]

        # TODO: Test selection strings for seeds and candidates 
        # Change selections without keyword-based argument without changing code (flags)
        
        # Thin hits on track from requested detector technologies     
        if flags.Tracking.thinPixelClustersOnTrack:
            kwargs["ThinPixelHitsOnTrack"] = True
            kwargs["InDetTrackStatesPixKey"] = flags.Tracking.thinInDetClustersPixelMSOSContainers[idx]
            kwargs["InDetTrackMeasurementsPixKey"] = (
                "ITkPixelMeasurements" if flags.Detector.GeometryITk
                else "PixelClusters")
        if flags.Tracking.thinSCTClustersOnTrack:
            kwargs["ThinSCTHitsOnTrack"] = True
            kwargs["InDetTrackStatesSctKey"] = flags.Tracking.thinInDetClustersSCTMSOSContainers[idx]
            kwargs["InDetTrackMeasurementsSctKey"] = (
                "ITkStripMeasurements" if flags.Detector.GeometryITk
                else "SCT_Clusters")
        if flags.Tracking.thinTRTClustersOnTrack:
            kwargs["ThinTRTHitsOnTrack"] = True
            kwargs["InDetTrackStatesTrtKey"] = flags.Tracking.thinInDetClustersTRTMSOSContainers[idx]
            kwargs["InDetTrackMeasurementsTrtKey"] = "TRT_DriftCircles"
     
        acc.addEventAlgo(CompFactory.ThinInDetClustersAlg(name+"_"+trackContainer, **kwargs))

    return acc


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from AthenaConfiguration.ComponentAccumulator import printProperties
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RDO_RUN2
    flags.Tracking.writeExtendedSi_PRDInfo=True
    flags.Tracking.writeExtendedTRT_PRDInfo=True
    flags.Tracking.thinPixelClustersOnTrack=True
    flags.Tracking.thinSCTClustersOnTrack=True
    flags.Tracking.thinTRTClustersOnTrack=True
    flags.Output.doWriteAOD = True  # To test the AOD parts
    flags.lock()
    acc = MainServicesCfg(flags)
    acc.merge(ThinInDetClustersCfg(flags))
    mlog = logging.getLogger("ThinInDetClustersConfigTest")
    printProperties(
        mlog,
        acc.getEventAlgo("ThinInDetClustersAlg"),
        nestLevel=1,
        printDefaults=True,
    )

    with open("thinindetclusterscfg.pkl", "wb") as f:
        acc.store(f)
