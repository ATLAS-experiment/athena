#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from ActsConfig.ActsConfigFlags import TrackFitterType
from ActsInterop import UnitConstants

def ActsFitterCfg(flags,
                  name: str = "ActsFitterTool",
                  **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    if flags.Acts.fitFromPRD:
        #RotCreatorTool and BroadRotCreatorTool for calibration purposes
        if 'RotCreatorTool' not in kwargs:
            from TrkConfig.TrkRIO_OnTrackCreatorConfig import ITkRotCreatorCfg
            kwargs.setdefault('RotCreatorTool', acc.popToolsAndMerge(ITkRotCreatorCfg(flags)))

        if 'BroadRotCreatorTool' not in kwargs:
            from TrkConfig.TrkRIO_OnTrackCreatorConfig import ITkBroadRotCreatorCfg
            kwargs.setdefault('BroadRotCreatorTool', acc.popToolsAndMerge(ITkBroadRotCreatorCfg(flags)))

    # Make sure this is set correctly!
    #  /eos/project-a/acts/public/MaterialMaps/ATLAS/material-maps-Pixel-SCT.json

    if "TrackingGeometryTool" not in kwargs:
        from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
        kwargs.setdefault("TrackingGeometryTool", acc.popToolsAndMerge(ActsTrackingGeometryToolCfg(flags)))

    if "ExtrapolationTool" not in kwargs:
        from ActsConfig.ActsGeometryConfig import ActsExtrapolationToolCfg
        kwargs.setdefault("ExtrapolationTool", acc.popToolsAndMerge(ActsExtrapolationToolCfg(flags, MaxSteps=10000)))

    if flags.Acts.trackFitterType is TrackFitterType.KalmanFitter:
        kwargs.setdefault("ReverseFilteringPt", 1.0 * UnitConstants.GeV)

    if 'ATLASConverterTool' not in kwargs:
        from ActsConfig.ActsEventCnvConfig import ActsToTrkConverterToolCfg
        kwargs.setdefault('ATLASConverterTool', acc.popToolsAndMerge(ActsToTrkConverterToolCfg(flags)))

    if "SummaryTool" not in kwargs:
        from TrkConfig.TrkTrackSummaryToolConfig import InDetTrackSummaryToolCfg
        kwargs.setdefault('SummaryTool', acc.getPrimaryAndMerge(InDetTrackSummaryToolCfg(flags)))

    if 'BoundaryCheckTool' not in kwargs:
        if flags.Detector.GeometryITk:
            from InDetConfig.InDetBoundaryCheckToolConfig import ITkBoundaryCheckToolCfg
            kwargs.setdefault("BoundaryCheckTool", acc.popToolsAndMerge(ITkBoundaryCheckToolCfg(flags)))
        else:
            from InDetConfig.InDetBoundaryCheckToolConfig import InDetBoundaryCheckToolCfg
            kwargs.setdefault("BoundaryCheckTool",acc.popToolsAndMerge(InDetBoundaryCheckToolCfg(flags)))

    if flags.Acts.trackFitterType is TrackFitterType.KalmanFitter:    # This flag is by default set to KalmanFitter
        acc.setPrivateTools(CompFactory.ActsTrk.KalmanFitterTool(name, **kwargs))
    elif flags.Acts.trackFitterType is TrackFitterType.GaussianSumFitter:
        name = name.replace("KalmanFitter", "GaussianSumFitter")
        acc.setPrivateTools(CompFactory.ActsTrk.GaussianSumFitterTool(name, **kwargs))
    elif flags.Acts.trackFitterType is TrackFitterType.GlobalChiSquareFitter:
        name = name.replace("KalmanFitter", "GlobalChiSquareFitter")
        acc.setPrivateTools(CompFactory.ActsTrk.GlobalChiSquareFitterTool(name, OutputLevel=1, **kwargs))

    return acc



def ActsReFitterAlgCfg(flags,
                       name : str = "ActsReFitterAlg",
                       **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault("ActsFitter", acc.popToolsAndMerge(ActsFitterCfg(flags)))
    kwargs.setdefault("TrackName", "ResolvedTracks")
    kwargs.setdefault("NewTrackName", "Refitted_Tracks")
    kwargs.setdefault("DoReFitFromPRD", flags.Acts.fitFromPRD)
    acc.addEventAlgo(CompFactory.ActsTrk.ReFitterAlg(name, **kwargs))

    if flags.Acts.writeTrackCollection:
        acc.merge(writeAdditionalTracks(flags))

    return acc

def forceITkActsReFitterAlgCfg(flags) -> ComponentAccumulator:
    #Use this flag in the --postInclude of ActsKfRefiting.sh to fit from the PRD (uncalibrated); Else to fit from the ROT (calibrated), use `(...).ActsReFitterAlgCfg` flag directly
   flags = flags.cloneAndReplace("Tracking.ActiveConfig",
                                 flags.Tracking.ITkPrimaryPassConfig.value)
   return ActsReFitterAlgCfg(flags)


def writeAdditionalTracks(flags,
                          trackName: str = 'ResolvedTracks',
                          newTrackName: str ='ReFitted_Tracks') -> ComponentAccumulator:
    acc = ComponentAccumulator()
    
    from xAODTrackingCnv.xAODTrackingCnvConfig import ITkTrackParticleCnvAlgCfg

    if flags.Tracking.doTruth:
        from InDetConfig.ITkTrackTruthConfig import ITkTrackTruthCfg
        acc.merge(ITkTrackTruthCfg(flags,
                                   Tracks = trackName,
                                   DetailedTruth = f"{trackName}DetailedTruth",
                                   TracksTruth = f"{trackName}TruthCollection"))
        acc.merge(ITkTrackTruthCfg(flags,
                                   Tracks = newTrackName,
                                   DetailedTruth = f"{newTrackName}DetailedTruth",
                                   TracksTruth = f"{newTrackName}TruthCollection"))

    acc.merge(ITkTrackParticleCnvAlgCfg(flags,
                                        name = f"{trackName}TrackParticleCnvAlg",
                                        TrackContainerName = trackName,
                                        xAODTrackParticlesFromTracksContainerName = f"{trackName}TrackParticles",
                                        TrackTruthContainerName = f"{trackName}TruthCollection")) 
    acc.merge(ITkTrackParticleCnvAlgCfg(flags,
                                        name = f"{newTrackName}TrackParticleCnvAlg",
                                        TrackContainerName = newTrackName,
                                        xAODTrackParticlesFromTracksContainerName = f"{newTrackName}TrackParticles",
                                        TrackTruthContainerName = f"{newTrackName}TruthCollection")) 
    
    from OutputStreamAthenaPool.OutputStreamConfig import addToESD, addToAOD
    itemList = [f"xAOD::TrackParticleContainer#{trackName}TrackParticles",
                f"xAOD::TrackParticleAuxContainer#{trackName}TrackParticlesAux.",
                f"xAOD::TrackParticleContainer#{newTrackName}TrackParticles",
                f"xAOD::TrackParticleAuxContainer#{newTrackName}TrackParticlesAux."]

    acc.merge(addToESD(flags, itemList))
    acc.merge(addToAOD(flags, itemList))
    return acc    

