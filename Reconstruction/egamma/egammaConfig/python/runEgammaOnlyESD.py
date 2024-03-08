# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# authors:
#    Asim Mohammed Aslam <asim.mohammed.aslam@cern.ch>
#    Fernando Monticelli <Fernando.Monticelli@cern.ch>
#    Jean-Baptiste De Vivie <devivie@lpsc.in2p3.fr>
#    Christos Anastopoulos <Christos.Anastopoulos@cern.ch>

# Simple script to run a
# Egamma job from ESD
#
# python runEgammaOnlyESD.py
# or
# pythong -m egammaConfig.runEgammaOnlyESD

import sys


def _run(args):
    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    flags = initConfigFlags()
    # input
    from AthenaConfiguration.TestDefaults import defaultTestFiles

    flags.Exec.MaxEvents = args.maxEvents
    if not args.inputFileList:
        flags.Input.Files = defaultTestFiles.ESD
    else:
        flags.Input.Files = args.inputFileList

    from AthenaConfiguration.Enums import ProductionStep

    flags.Common.ProductionStep = ProductionStep.Reconstruction
    # Disable detectors we do not need
    flags.Detector.GeometryMuon = False
    flags.Detector.EnableAFP = False
    flags.Detector.EnableLucid = False
    flags.Detector.EnableZDC = False
    flags.Input.isMC = True

    # output
    flags.Output.AODFileName = args.outputAODFile

    #
    flags.Egamma.Keys.Output.CaloClusters = "new_egammaClusters"
    flags.Egamma.Keys.Output.Electrons = "new_Electrons"
    flags.Egamma.Keys.Output.Photons = "new_Photons"

    # Setup detector flags
    from AthenaConfiguration.DetectorConfigFlags import setupDetectorFlags

    setupDetectorFlags(
        flags, None, use_metadata=True, toggle_geometry=True, keep_beampipe=True
    )

    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg

    acc = MainServicesCfg(flags)

    from AtlasGeoModel.GeoModelConfig import GeoModelCfg

    acc.merge(GeoModelCfg(flags))

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg

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

    # Algorithms to run

    # For being able to read pre Run-3 data w/ Trk objects
    from TrkEventCnvTools.TrkEventCnvToolsConfig import TrkEventCnvSuperToolCfg

    acc.merge(TrkEventCnvSuperToolCfg(flags))

    # Redo topo
    from CaloRec.CaloTopoClusterConfig import CaloTopoClusterCfg

    acc.merge(CaloTopoClusterCfg(flags))

    from egammaAlgs.egammaTopoClusterCopierConfig import egammaTopoClusterCopierCfg

    acc.merge(egammaTopoClusterCopierCfg(flags))

    from egammaAlgs.egammaRecBuilderConfig import egammaRecBuilderCfg

    acc.merge(egammaRecBuilderCfg(flags))

    from egammaAlgs.egammaSuperClusterBuilderConfig import (
        electronSuperClusterBuilderCfg,
        photonSuperClusterBuilderCfg,
    )

    acc.merge(electronSuperClusterBuilderCfg(flags))
    acc.merge(photonSuperClusterBuilderCfg(flags))

    from egammaAlgs.xAODEgammaBuilderConfig import xAODEgammaBuilderCfg

    acc.merge(xAODEgammaBuilderCfg(flags, name="xAODEgammaBuilder", sequenceName=None))

    from egammaConfig.egammaOutputConfig import egammaOutputCfg

    acc.merge(egammaOutputCfg(flags))

    from AthenaConfiguration.Utils import setupLoggingLevels

    setupLoggingLevels(flags, acc)

    if args.doCopyOriginalCollections:
        from OutputStreamAthenaPool.OutputStreamConfig import addToAOD

        toAOD = [
            "xAOD::PhotonContainer#Photons",
            "xAOD::PhotonAuxContainer#Photons"
            f"Aux.{flags.Egamma.Keys.Output.PhotonsSuppAOD}",
            "xAOD::ElectronContainer#Electrons",
            "xAOD::ElectronAuxContainer#Electrons"
            f"Aux.{flags.Egamma.Keys.Output.ElectronsSuppAOD}",
        ]
        acc.merge(addToAOD(flags, toAOD))

    # running
    statusCode = acc.run()
    return statusCode


if __name__ == "__main__":
    statusCode = None

    # Argument parsing
    from argparse import ArgumentParser

    parser = ArgumentParser("egammaFromESD")
    parser.add_argument(
        "-m",
        "--maxEvents",
        default=100,
        type=int,
        help="The number of events to run. -1 runs all events.",
    )
    parser.add_argument(
        "-i", "--inputFileList", nargs="*", help="list of input ESD files"
    )
    parser.add_argument(
        "-o", "--outputAODFile", default="myAOD.pool.root", help="Output file name"
    )
    parser.add_argument(
        "--doCopyOriginalCollections",
        default=False,
        action="store_true",
        help="store original electron and photon collections",
    )
    args = parser.parse_args()

    statusCode = _run(args)
    assert statusCode is not None, "Issue while running"
    sys.exit(not statusCode.isSuccess())
