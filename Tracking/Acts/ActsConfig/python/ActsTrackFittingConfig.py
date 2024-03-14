#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from ActsConfig.ActsConfigFlags import TrackFitterType
from ActsInterop import UnitConstants

def ActsFitterCfg(flags, name: str = "ActsKalmanFitter", **kwargs):
    result = ComponentAccumulator()

    if flags.Acts.fitFromPRD:
        #RotCreatorTool and BroadRotCreatorTool for calibration purposes
        if 'RotCreatorTool' not in kwargs:
            from TrkConfig.TrkRIO_OnTrackCreatorConfig import ITkRotCreatorCfg
            ITkRotCreator = result.popToolsAndMerge(ITkRotCreatorCfg(flags))
            kwargs.setdefault('RotCreatorTool', ITkRotCreator)

        if 'BroadRotCreatorTool' not in kwargs:
            from TrkConfig.TrkRIO_OnTrackCreatorConfig import ITkBroadRotCreatorCfg
            ITkBroadRotCreator = result.popToolsAndMerge(ITkBroadRotCreatorCfg(flags))
            kwargs.setdefault('BroadRotCreatorTool', ITkBroadRotCreator)


    # Make sure this is set correctly!
    #  /eos/project-a/acts/public/MaterialMaps/ATLAS/material-maps-Pixel-SCT.json

    if "TrackingGeometryTool" not in kwargs:
        from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
        kwargs["TrackingGeometryTool"] = result.popToolsAndMerge(ActsTrackingGeometryToolCfg(flags)) # PrivateToolHandle

    if "ExtrapolationTool" not in kwargs:
        from ActsConfig.ActsGeometryConfig import ActsExtrapolationToolCfg
        kwargs["ExtrapolationTool"] = result.popToolsAndMerge(
            ActsExtrapolationToolCfg(flags, MaxSteps=10000)
        ) # PrivateToolHandle

    if flags.Acts.trackFitterType is TrackFitterType.KalmanFitter:
        kwargs.setdefault("ReverseFilteringPt", 1.0 * UnitConstants.GeV)
        kwargs.setdefault("OverstepLimit", 300 * UnitConstants.um)

    from ActsConfig.ActsEventCnvConfig import ActsToTrkConverterToolCfg
    actsConverter = result.popToolsAndMerge(ActsToTrkConverterToolCfg(flags))
    kwargs["ATLASConverterTool"] = actsConverter

    if "SummaryTool" not in kwargs:
        from TrkConfig.TrkTrackSummaryToolConfig import InDetTrackSummaryToolCfg
        summary = result.getPrimaryAndMerge(
            InDetTrackSummaryToolCfg(flags, 
                                     #  doHolesInDet=False
                                     )
        )
        kwargs["SummaryTool"] = summary

    if flags.Detector.GeometryITk:
        from InDetConfig.InDetBoundaryCheckToolConfig import ITkBoundaryCheckToolCfg

        kwargs.setdefault(
            "BoundaryCheckTool", result.popToolsAndMerge(ITkBoundaryCheckToolCfg(flags))
        )
    else:
        from InDetConfig.InDetBoundaryCheckToolConfig import InDetBoundaryCheckToolCfg

        kwargs.setdefault(
            "BoundaryCheckTool",
            result.popToolsAndMerge(InDetBoundaryCheckToolCfg(flags)),
        )

    if flags.Acts.trackFitterType is TrackFitterType.KalmanFitter:    # This flag is by default set to KalmanFitter
        result.setPrivateTools(CompFactory.ActsTrk.KalmanFitter(name, **kwargs))
    elif flags.Acts.trackFitterType is TrackFitterType.GaussianSumFitter:
        name = name.replace("KalmanFitter", "GaussianSumFitter")
        result.setPrivateTools(CompFactory.ActsTrk.GaussianSumFitter(name, **kwargs))

    return result



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

def forceITkActsReFitterAlgCfg(flags): #Use this flag in the --postInclude of ActsKfRefiting.sh to fit from the PRD (uncalibrated); Else to fit from the ROT (calibrated), use `(...).ActsReFitterAlgCfg` flag directly
   flags = flags.cloneAndReplace("Tracking.ActiveConfig",
                                 flags.Tracking.ITkPrimaryPassConfig.value)
   return ActsReFitterAlgCfg(flags)


def writeAdditionalTracks(flags, trackName='ResolvedTracks', newTrackName='ReFitted_Tracks'):
    result = ComponentAccumulator()
    
    from xAODTrackingCnv.xAODTrackingCnvConfig import ITkTrackParticleCnvAlgCfg

    if flags.Tracking.doTruth:
        from InDetConfig.ITkTrackTruthConfig import ITkTrackTruthCfg
        result.merge(ITkTrackTruthCfg(flags,
              Tracks = trackName,
              DetailedTruth = f"{trackName}DetailedTruth",
              TracksTruth = f"{trackName}TruthCollection"))
        result.merge(ITkTrackTruthCfg(flags,
              Tracks = newTrackName,
              DetailedTruth = f"{newTrackName}DetailedTruth",
              TracksTruth = f"{newTrackName}TruthCollection"))

    result.merge(ITkTrackParticleCnvAlgCfg(flags,
           name = f"{trackName}TrackParticleCnvAlg",
           TrackContainerName = trackName,
           xAODTrackParticlesFromTracksContainerName = f"{trackName}TrackParticles",
           TrackTruthContainerName = f"{trackName}TruthCollection")) 
    result.merge(ITkTrackParticleCnvAlgCfg(flags,
           name = f"{newTrackName}TrackParticleCnvAlg",
           TrackContainerName = newTrackName,
           xAODTrackParticlesFromTracksContainerName = f"{newTrackName}TrackParticles",
           TrackTruthContainerName = f"{newTrackName}TruthCollection")) 

    from OutputStreamAthenaPool.OutputStreamConfig import addToESD, addToAOD
    itemList = [f"xAOD::TrackParticleContainer#{trackName}TrackParticles",
                f"xAOD::TrackParticleAuxContainer#{trackName}TrackParticlesAux.",
                f"xAOD::TrackParticleContainer#{newTrackName}TrackParticles",
                f"xAOD::TrackParticleAuxContainer#{newTrackName}TrackParticlesAux."]

    result.merge(addToESD(flags, itemList))
    result.merge(addToAOD(flags, itemList))
    
    return result

