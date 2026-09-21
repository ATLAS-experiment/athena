# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# authors:
#    Asim Mohammed Aslam <asim.mohammed.aslam@cern.ch>
#    Fernando Monticelli <Fernando.Monticelli@cern.ch>
#    Jean-Baptiste De Vivie <devivie@lpsc.in2p3.fr>
#    Christos Anastopoulos <Christos.Anastopoulos@cern.ch>
#    Raphael Julien Haberle <raphael.julien.haberle@cern.ch>

# Simple script to run the
# GSF + EMCal refit tool from ESD
#
# python runGSFCaloFromESD.py
# or
# python -m egammaConfig.runGSFCaloFromESD

import sys


def _run(args):
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from AthenaConfiguration.Enums import ProductionStep

    flags = initConfigFlags()

    # Input
    flags.Exec.MaxEvents = args.maxEvents
    flags.Input.Files = args.inputFileList or defaultTestFiles.ESD_RUN3_MC
    flags.Input.isMC = True

    # Reconstruction setup
    flags.Common.ProductionStep = ProductionStep.Reconstruction

    # Disable detectors we do not need
    flags.Detector.GeometryMuon = False
    flags.Detector.EnableAFP = False
    flags.Detector.EnableLucid = False
    flags.Detector.EnableZDC = False

    # Output
    flags.Output.AODFileName = args.outputAODFile

    # Setup detector flags
    from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags
    setupDetectorFlags(
        flags,
        None,
        use_metadata=True,
        toggle_geometry=True,
        keep_beampipe=True,
    )

    flags.lock()

    # Main services
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg

    acc = MainServicesCfg(flags)

    # Geometry and input reading
    from AtlasGeoModel.GeoModelConfig import GeoModelCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg

    acc.merge(GeoModelCfg(flags))
    acc.merge(PoolReadCfg(flags))

    if flags.Detector.EnablePixel:
        from PixelGeoModel.PixelGeoModelConfig import PixelReadoutGeometryCfg

        acc.merge(PixelReadoutGeometryCfg(flags))

    if flags.Detector.EnableSCT:
        from SCT_GeoModel.SCT_GeoModelConfig import SCT_ReadoutGeometryCfg

        acc.merge(SCT_ReadoutGeometryCfg(flags))

    if flags.Detector.EnableTRT:
        from TRT_GeoModel.TRT_GeoModelConfig import TRT_ReadoutGeometryCfg

        acc.merge(TRT_ReadoutGeometryCfg(flags))

    if flags.Detector.EnableLAr:
        from LArBadChannelTool.LArBadChannelConfig import LArBadFebCfg

        acc.merge(LArBadFebCfg(flags))

    # Special message service configuration
    from DigitizationConfig.DigitizationSteering import DigitizationMessageSvcCfg

    acc.merge(DigitizationMessageSvcCfg(flags))

    # Needed to read pre-Run-3 data with Trk objects
    from TrkEventCnvTools.TrkEventCnvToolsConfig import TrkEventCnvSuperToolCfg

    acc.merge(TrkEventCnvSuperToolCfg(flags))

    # GSF + EMCal augmentation

    from DerivationFrameworkEGamma.EGammaGSFCalo import EGammaGSFCaloToolsCfg

    GSFCaloTool = acc.popToolsAndMerge(
        EGammaGSFCaloToolsCfg(flags, "GSFCaloImprovement")
    )
    acc.addPublicTool(GSFCaloTool)

    from AthenaConfiguration.ComponentFactory import CompFactory

    acc.addEventAlgo(CompFactory.DerivationFramework.CommonAugmentation("GSFRefitAugAlgo", AugmentationTools=[GSFCaloTool]))
    acc.addEventAlgo(
        CompFactory.DerivationFramework.DerivationKernel(
            "GSFRefitAlgo",
            SkimmingTools=None,
            ThinningTools=None,
        )
    )

    # Standard egamma output
    from AthenaConfiguration.Utils import setupLoggingLevels

    setupLoggingLevels(flags, acc)

    # Output collections + keep original collections

    ItemList = [
        "xAOD::ElectronContainer#Electrons",
        "xAOD::ElectronAuxContainer#Electrons"
        f"Aux.{flags.Egamma.Keys.Output.ElectronsSuppAOD}",
        "xAOD::TrackParticleContainer#GSFTrackParticles",
        "xAOD::TrackParticleAuxContainer#GSFTrackParticles"
        f"Aux.{flags.Egamma.Keys.Output.GSFTrackParticlesSuppAOD}",
        "xAOD::TrackParticleContainer#GSFCaloContainer",
        "xAOD::TrackParticleAuxContainer#GSFCaloContainer"
        f"Aux.{flags.Egamma.Keys.Output.GSFTrackParticlesSuppAOD}",
        "TrackCollection#Tracks",
        "TrackCollection#GSFTracks"
    ]

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from AthenaConfiguration.Enums import MetadataCategory
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg

    stream_name = "AOD_GSFRefit"

    acc.merge(
        OutputStreamCfg(
            flags,
            stream_name,
            ItemList=ItemList,
            AcceptAlgs=["GSFRefitAlgo"],
        )
    )

    acc.merge(
        SetupMetaDataForStreamCfg(
            flags,
            stream_name,
            AcceptAlgs=["GSFRefitAlgo"],
            createMetadata=[
                MetadataCategory.CutFlowMetaData,
                MetadataCategory.FileMetaData,
                MetadataCategory.EventStreamInfo,
            ],
        )
    )

    acc.printConfig(
        withDetails=True,
        summariseProps=True,
        onlyComponents=[],
        printDefaults=True,
    )

    # Run it
    statusCode = acc.run()
    return statusCode


if __name__ == "__main__":
    statusCode = None

    from argparse import ArgumentParser

    parser = ArgumentParser("egammaFromESD")

    parser.add_argument(
        "-m",
        "--maxEvents",
        default=20,
        type=int,
        help="The number of events to run. -1 runs all events.",
    )

    parser.add_argument(
        "-i",
        "--inputFileList",
        nargs="*",
        help="List of input ESD files.",
    )

    parser.add_argument(
        "-o",
        "--outputAODFile",
        default="myAOD.pool.root",
        help="Output AOD file name.",
    )

    args = parser.parse_args()

    status_code = _run(args)
    assert status_code is not None, "Issue while running"
    sys.exit(not status_code.isSuccess())
