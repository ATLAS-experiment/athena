#
#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

writeOutput = True
doTruth = True
saveTracccEventsToCSV = False
nEventsToSave = 10


def TracccTritonToolCfg(flags, name="TracccTritonTool", **kwargs):
    """Set up a TracccTritonTool tool and return"""

    from AthTritonComps.TritonToolConfig import TritonToolCfg

    acc = ComponentAccumulator()

    kwargs.setdefault("TritonTool", acc.popToolsAndMerge(
                      TritonToolCfg(flags, flags.Tracking.Traccc.Triton.model,
                                    url=flags.Tracking.Traccc.Triton.url,
                                    port=flags.Tracking.Traccc.Triton.port,
                                    ssl=(flags.Tracking.Traccc.Triton.port == 443))))

    acc.setPrivateTools(CompFactory.TracccTritonTool(name, **kwargs))
    return acc


def addTruthClusterAssociations(acc, flags, prefix="", pixelKey=None, stripKey=None):
    from ActsConfig.ActsTruthConfig import (
        ActsPixelClusterToTruthAssociationAlgCfg,
        ActsStripClusterToTruthAssociationAlgCfg,
        ActsTruthParticleHitCountAlgCfg,
    )
    acc.merge(ActsPixelClusterToTruthAssociationAlgCfg(
        flags,
        name=f"{prefix}PixelClusterToTruthAssociationAlg",
        InputTruthParticleLinks="xAODTruthLinks",
        AssociationMapOut=f"{prefix}PixelClustersToTruthParticles",
        Measurements=pixelKey,
    ))
    acc.merge(ActsStripClusterToTruthAssociationAlgCfg(
        flags,
        name=f"{prefix}StripClusterToTruthAssociationAlg",
        InputTruthParticleLinks="xAODTruthLinks",
        AssociationMapOut=f"{prefix}StripClustersToTruthParticles",
        Measurements=stripKey,
    ))
    acc.merge(ActsTruthParticleHitCountAlgCfg(
        flags,
        name=f"{prefix}TruthParticleHitCountAlg",
        PixelClustersToTruthAssociationMap=f"{prefix}PixelClustersToTruthParticles",
        StripClustersToTruthAssociationMap=f"{prefix}StripClustersToTruthParticles",
        TruthParticleHitCountsOut=f"{prefix}TruthParticleHitCounts",
    ))
    return acc


def addTrackTruthDecorations(acc, flags, prefix, tracks_key):

    from ActsConfig.ActsTruthConfig import (
        ActsTrackToTruthAssociationAlgCfg,
        ActsTrackFindingValidationAlgCfg,
        ActsTrackParticleTruthDecorationAlgCfg,
    )

    # Track to truth association
    acc.merge(ActsTrackToTruthAssociationAlgCfg(
        flags,
        name=f"{prefix}TrackToTruthAssociationAlg",
        PixelClustersToTruthAssociationMap=f"{prefix}PixelClustersToTruthParticles",
        StripClustersToTruthAssociationMap=f"{prefix}StripClustersToTruthParticles",
        ACTSTracksLocation=tracks_key,
        AssociationMapOut=f"{tracks_key}ToTruthParticleAssociation",
    ))

    # Validation
    acc.merge(ActsTrackFindingValidationAlgCfg(
        flags,
        name=f"{prefix}TrackFindingValidationAlg",
        TrackToTruthAssociationMap=f"{tracks_key}ToTruthParticleAssociation",
        TruthParticleHitCounts=f"{prefix}TruthParticleHitCounts",
    ))

    # Decoration
    acc.merge(ActsTrackParticleTruthDecorationAlgCfg(
        flags,
        name=f"{prefix}TrackParticleTruthDecorationAlg",
        TrackToTruthAssociationMaps=[
            f"{tracks_key}ToTruthParticleAssociation"],
        TrackParticleContainerName=f"{prefix}TrackParticles",
        TruthParticleHitCounts=f"{prefix}TruthParticleHitCounts",
        ComputeTrackRecoEfficiency=True,
    ))

    return acc


def TritonTracccTrackMakerCfg(flags, name="TritonTracccTrackMaker", **kwargs):
    """Set up a TrackMaker algorithm and return it"""
    acc = ComponentAccumulator()

    prefix = "Traccc"
    track_container_name = f'{prefix}Tracks'
    track_particles_name = f"{prefix}TrackParticles"

    # Configure the TracccTritonTool
    kwargs.setdefault("TracccTritonTool", acc.popToolsAndMerge(TracccTritonToolCfg(flags,
                      SaveEventsToCSV=saveTracccEventsToCSV,
                      MaxEventsToSave=nEventsToSave)))

    from ActsConfig.ActsGeometryConfig import ActsTrackingGeometrySvcCfg
    kwargs.setdefault("TrackingGeometrySvc", acc.getPrimaryAndMerge(ActsTrackingGeometrySvcCfg(flags)))

    # Pixel and strip geometry
    from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
    acc.merge(ITkPixelReadoutGeometryCfg(flags))
    from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
    acc.merge(ITkStripReadoutGeometryCfg(flags))

    try:
        if flags.Tracking.ActiveConfig.extension != "Acts":
            raise RuntimeError(f"wrong tracking pass: {flags.Tracking.ActiveConfig.extension}")
    except AttributeError:
        flags = flags.cloneAndReplace("Tracking.ActiveConfig",
                                      "Tracking.ITkActsPass")

    # Main tracking alg
    acc.addEventAlgo(CompFactory.TritonTracccTrackMaker(name, doTruth=doTruth, **kwargs))

    ################################################################################
    # Convert ActsTrk::TrackContainer to xAOD::TrackParticleContainer

    from ActsConfig.ActsEventCnvConfig import ActsTrackToTrackParticleCnvAlgCfg
    acc.merge(ActsTrackToTrackParticleCnvAlgCfg(flags,
        name=f"{prefix}TrackToTrackParticleCnvAlg",
        ACTSTracksLocation=[track_container_name],
        TrackParticlesOutKey=track_particles_name))


    pixel_key = "xAODPixelClustersFromInDetCluster"
    strip_key = "xAODStripClustersFromInDetCluster"

    # if doTruth:
    if doTruth:
        addTruthClusterAssociations(
            acc, flags, prefix=prefix, pixelKey=pixel_key, stripKey=strip_key)
        addTrackTruthDecorations(acc, flags, prefix, track_container_name)


    if writeOutput:

        # Adding the output to the AOD file
        inputList = []

        inputList.append("xAOD::TruthParticleContainer#*")
        inputList.append("xAOD::TruthParticleAuxContainer#*")
        inputList.append("xAOD::TrackJacobianContainer#*")
        inputList.append("xAOD::TrackJacobianAuxContainer#*")
        inputList.append("xAOD::TrackMeasurementContainer#*")
        inputList.append("xAOD::TrackMeasurementAuxContainer#*")
        inputList.append("xAOD::TrackSurfaceContainer#*")
        inputList.append("xAOD::TrackSurfaceAuxContainer#*")
        inputList.append("xAOD::TrackParticleContainer#*")
        inputList.append("xAOD::TrackParticleAuxContainer#*")

        if doTruth:
            inputList.append("xAOD::PixelClusterContainer#xAODPixelClustersFromInDetCluster")
            inputList.append("xAOD::StripClusterContainer#xAODStripClustersFromInDetCluster")
            inputList.append("xAOD::PixelClusterAuxContainer#xAODPixelClustersFromInDetClusterAux.")
            inputList.append("xAOD::StripClusterAuxContainer#xAODStripClustersFromInDetClusterAux.")
        else:
            inputList.append("xAOD::PixelClusterContainer#*")
            inputList.append("xAOD::StripClusterContainer#*")
            inputList.append("xAOD::PixelClusterAuxContainer#*")
            inputList.append("xAOD::StripClusterAuxContainer#*")

        from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
        acc.merge(OutputStreamCfg(flags, 'AOD', ItemList=inputList))

    return acc