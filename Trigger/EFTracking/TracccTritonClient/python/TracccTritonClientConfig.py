#
#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

writeOutput = True
doTruth = True


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
    """Set up traccc-as-a-service tracking: the cells of each event are sent
    to the server, and the traccc collections it returns are converted with
    the same algorithms as the local device chain"""
    acc = ComponentAccumulator()

    prefix = "Traccc"
    track_container_name = f'{prefix}Tracks'
    track_particles_name = f"{prefix}TrackParticles"

    # Keys of the traccc collections returned by the server, and of their conversions.
    cells_key = "TracccCells"
    measurements_key = "TracccTritonMeasurements"
    clusters_key = "TracccTritonClusters"
    traccc_tracks_key = "TracccTritonTracks"
    pixel_key = f"{prefix}PixelClusters"
    strip_key = f"{prefix}StripClusters"
    meas_to_pixel_sp_key = f"{prefix}MeasToPixelSP"
    meas_to_strip_cl_key = f"{prefix}MeasToStripCl"

    from ActsConfig.ActsGeometryConfig import ActsTrackingGeometrySvcCfg
    acc.merge(ActsTrackingGeometrySvcCfg(flags))

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

    # Traccc detector description needed by the conversions
    geo_id_mapping_name = "TracccGeometryIdMapping"
    host_detector_name = "TracccHostDetectorGeometry"
    from ActsGPUGeometry.ActsGPUGeometryConfig import JSONDeviceDetectorDescriptionProviderSvcCfg
    acc.merge(JSONDeviceDetectorDescriptionProviderSvcCfg(flags,
        HostConditionsObjectName="TracccHostCondConfig",
        HostDigitizationObjectName="TracccHostDigitizationConfig",
        DeviceConditionsObjectName="TracccDeviceCondConfig",
        DeviceDigitizationObjectName="TracccDeviceDigitizationConfig",
        GeoIdMappingObjectName=geo_id_mapping_name,
        HostDetectorName=host_detector_name,
    ))

    # All traccc collections on the client live in host memory
    from AthDeviceComps.AthDeviceCompsConfig import HostCopyToolCfg, HostMemoryResourceToolCfg

    # Convert the Pixel/Strip RDOs into traccc cells
    from ActsGPUEventCnv.ActsGPUEventCnvConfig import RDOtoTracccCellConverterAlgCfg
    copies_tool = CompFactory.AthDevice.CopiesAdaptorTool(
        "TracccCellsHostCopiesTool",
        HostCopyTool=acc.popToolsAndMerge(HostCopyToolCfg(flags)),
        DeviceCopyTool=acc.popToolsAndMerge(HostCopyToolCfg(flags)))
    acc.merge(RDOtoTracccCellConverterAlgCfg(flags,
        HostMR=acc.popToolsAndMerge(HostMemoryResourceToolCfg(flags)),
        DeviceMR=acc.popToolsAndMerge(HostMemoryResourceToolCfg(flags)),
        CopiesTool=copies_tool,
        TracccCells=cells_key))

    # Send the cells to the server, record what it returns
    kwargs.setdefault("TracccTritonTool", acc.popToolsAndMerge(TracccTritonToolCfg(flags)))
    kwargs.setdefault("HostMR", acc.popToolsAndMerge(HostMemoryResourceToolCfg(flags)))
    kwargs.setdefault("TracccCells", cells_key)
    kwargs.setdefault("OutputTracccMeasurements", measurements_key)
    kwargs.setdefault("OutputTracccClusters", clusters_key)
    kwargs.setdefault("OutputTracccTracks", traccc_tracks_key)
    acc.addEventAlgo(CompFactory.TritonTracccTrackMaker(name, **kwargs))

    # From here on, exactly as in the local device chain
    from ActsGPUEventCnv.ActsGPUEventCnvConfig import (
        TracccMeasurementConverterAlgCfg,
        TracccTrackConverterAlgCfg,
    )
    acc.merge(TracccMeasurementConverterAlgCfg(flags,
        name=f"{prefix}TritonMeasurementConverterAlg",
        HostMR=acc.popToolsAndMerge(HostMemoryResourceToolCfg(flags)),
        CopyProviderTool=acc.popToolsAndMerge(HostCopyToolCfg(flags)),
        InputMeasurements=measurements_key,
        InputClusters=clusters_key,
        InputCells=cells_key,
        ConvertClustersWithCells=doTruth,
        GeoIdMapping=geo_id_mapping_name,
        OutputPixelClusters=pixel_key,
        OutputPixelSpacePoints=f"{prefix}PixelSpacePoints",
        OutputMeasToPixelSP=meas_to_pixel_sp_key,
        OutputMeasToStripCl=meas_to_strip_cl_key,
        OutputStripClusters=strip_key,
    ))

    acc.merge(TracccTrackConverterAlgCfg(flags,
        name=f"{prefix}TritonTrackConverterAlg",
        HostMR=acc.popToolsAndMerge(HostMemoryResourceToolCfg(flags)),
        CopyProviderTool=acc.popToolsAndMerge(HostCopyToolCfg(flags)),
        GeoIdMapping=geo_id_mapping_name,
        HostDetectorName=host_detector_name,
        InputPixelClusters=pixel_key,
        InputStripClusters=strip_key,
        InputMeasToPixelSP=meas_to_pixel_sp_key,
        InputMeasToStripCl=meas_to_strip_cl_key,
        InputTracks=traccc_tracks_key,
        OutputTracks=track_container_name,
    ))

    ################################################################################
    # Convert ActsTrk::TrackContainer to xAOD::TrackParticleContainer

    from ActsConfig.ActsEventCnvConfig import ActsTrackToTrackParticleCnvAlgCfg
    acc.merge(ActsTrackToTrackParticleCnvAlgCfg(flags,
        name=f"{prefix}TrackToTrackParticleCnvAlg",
        ACTSTracksLocation=[track_container_name],
        TrackParticlesOutKey=track_particles_name))

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
            inputList.append(f"xAOD::PixelClusterContainer#{pixel_key}")
            inputList.append(f"xAOD::StripClusterContainer#{strip_key}")
            inputList.append(f"xAOD::PixelClusterAuxContainer#{pixel_key}Aux.")
            inputList.append(f"xAOD::StripClusterAuxContainer#{strip_key}Aux.")
        else:
            inputList.append("xAOD::PixelClusterContainer#*")
            inputList.append("xAOD::StripClusterContainer#*")
            inputList.append("xAOD::PixelClusterAuxContainer#*")
            inputList.append("xAOD::StripClusterAuxContainer#*")

        from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
        acc.merge(OutputStreamCfg(flags, 'AOD', ItemList=inputList))

    return acc