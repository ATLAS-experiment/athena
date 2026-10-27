#
#  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

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


def TritonTracccTrackMakerCfg(flags, name="TritonTracccTrackMaker", **kwargs):
    """Set up traccc-as-a-service tracking: the cells of each event are sent
    to the server, and the traccc collections it returns are converted with
    the same algorithms as the local device chain.

    Scheduled by ITkActsDeviceTrackRecoCfg for the primary pass with
    flags.Acts.Device.tritonMode = DeviceTritonMode.Client, which also adds
    the PRD association and truth"""
    acc = ComponentAccumulator()

    # Keys of the traccc collections returned by the server
    cells_key = "TracccCells"
    measurements_key = "TracccTritonMeasurements"
    clusters_key = "TracccTritonClusters"
    traccc_tracks_key = "TracccTritonTracks"

    # Create RoI for secondary passes (e.g. LargeD0) to reuse
    from ActsConfig.ActsRegionsOfInterestConfig import ActsRegionsOfInterestCreatorAlgCfg
    acc.merge(ActsRegionsOfInterestCreatorAlgCfg(flags,
        name=f"{flags.Tracking.ActiveConfig.extension}RegionsOfInterestCreatorAlg"))

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
        HostMR=acc.popToolsAndMerge(HostMemoryResourceToolCfg(flags)),
        CopyProviderTool=acc.popToolsAndMerge(HostCopyToolCfg(flags)),
        InputMeasurements=measurements_key,
        InputClusters=clusters_key,
        InputCells=cells_key,
        ConvertClustersWithCells=flags.Tracking.doTruth,
        OutputPixelClusters="ITkPixelClusters",
        OutputPixelSpacePoints="ITkPixelSpacePoints",
        OutputMeasToPixelSP="ITkTracccMeasToPixelSP",
        OutputMeasToStripCl="ITkTracccMeasToStripCl",
        OutputStripClusters="ITkStripClusters",
    ))

    if flags.Tracking.ActiveConfig.useITkStripSeeding or (flags.Acts.SpacePoints.doStrip and not flags.Tracking.ActiveConfig.isSecondaryPass):
        from ActsConfig.ActsSpacePointFormationConfig import ActsStripSpacePointFormationAlgCfg
        acc.merge(ActsStripSpacePointFormationAlgCfg(flags,
            name=f"{flags.Tracking.ActiveConfig.extension}StripSpacePointFormationAlg",
            StripClusters="ITkStripClusters",
            StripSpacePoints="ITkStripSpacePoints",
            StripOverlapSpacePoints="ITkStripOverlapSpacePoints"))

    acc.merge(TracccTrackConverterAlgCfg(flags,
        HostMR=acc.popToolsAndMerge(HostMemoryResourceToolCfg(flags)),
        CopyProviderTool=acc.popToolsAndMerge(HostCopyToolCfg(flags)),
        InputPixelClusters="ITkPixelClusters",
        InputStripClusters="ITkStripClusters",
        InputMeasToPixelSP="ITkTracccMeasToPixelSP",
        InputMeasToStripCl="ITkTracccMeasToStripCl",
        InputTracks=traccc_tracks_key,
        OutputTracks=f"{flags.Tracking.ActiveConfig.extension}Tracks",
    ))

    return acc
