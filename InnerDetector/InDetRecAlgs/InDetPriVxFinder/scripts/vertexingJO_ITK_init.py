# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

import sys
import argparse

from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentAccumulator import printProperties
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from TrkConfig.VertexFindingFlags import VertexSetup

from AthenaCommon.Constants import VERBOSE
from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg

if __name__ == "__main__":

    parser = argparse.ArgumentParser(
        description="Primary vertex finding configuration for ITk"
    )
    parser.add_argument(
        "--seeder",
        type=str,
        default="Gaussian",
        choices=["Grid", "Gaussian"],
        help="Choose between Grid or Gaussian seeder."
    )
    parser.add_argument(
        "--output",
        type=str,
        default="AOD",
        help="Name of the output file"
    )
    parser.add_argument(
        "--maxEvents",
        type=int,
        default=-1,
        help="Number of events to process. Default: -1 (all)."
    )
    args = parser.parse_args()

    mlog = logging.getLogger("vertexingJO_ITK_init")
    mlog.setLevel(logging.INFO)

    flags = initConfigFlags()

    flags.Exec.MaxEvents = args.maxEvents
    flags.Input.Files = args.input

    flags.Output.AODFileName = args.output
    flags.Exec.OutputLevel = VERBOSE

    if args.seeder == "Gaussian":
        flags.Tracking.PriVertex.setup = VertexSetup.ActsGaussAMVF
        mlog.info("Using Gaussian seeder for primary vertex finding.")
        vxContainerName = "PrimaryVertices_Gauss"
    elif args.seeder == "Grid":
        flags.Tracking.PriVertex.setup = VertexSetup.ActsGridDensity
        mlog.info("Using Grid seeder for primary vertex finding.")
        vxContainerName = "PrimaryVertices_Grid"

        mlog.info("Grid parameters:")
        mlog.info("  gridMainGridSize = %s", flags.Tracking.PriVertex.gridMainGridSize)
        mlog.info("  gridTrkGridSize = %s", flags.Tracking.PriVertex.gridTrkGridSize)
        mlog.info("  gridUseHighestSumZPosition = %s", flags.Tracking.PriVertex.gridUseHighestSumZPosition)
        mlog.info("  gridMaxD0Significance = %s", flags.Tracking.PriVertex.gridMaxD0Significance)
        mlog.info("  gridMaxZ0Significance = %s", flags.Tracking.PriVertex.gridMaxZ0Significance)
    else:
        raise ValueError(f"Invalid seeder '{args.seeder}'; choose 'Grid' or 'Gaussian'.")

    flags.dump()
    flags.lock()

    cfg = MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))

    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    cfg.merge(SetupMetaDataForStreamCfg(flags, "AOD"))

    from InDetConfig.InDetPriVxFinderConfig import primaryVertexFindingCfg
    cfg.merge(primaryVertexFindingCfg(flags, vxCandidatesOutputName=vxContainerName))

    mlog.info("Configuring primary vertex finding...")
    mlog.setLevel(VERBOSE)

    printProperties(
        mlog,
        cfg.getEventAlgo("InDetPriVxFinder"),
        nestLevel=2,
        printDefaults=True
    )
    cfg.printConfig(withDetails=True, summariseProps=True)

    itemList = [
        "xAOD::ElectronContainer#Electrons",
        "xAOD::ElectronAuxContainer#*",
        "xAOD::PhotonContainer#Photons",
        "xAOD::PhotonAuxContainer#*",
        "xAOD::MuonContainer#Muons",
        "xAOD::MuonAuxContainer#*",
        "xAOD::TauJetContainer#*",
        "xAOD::TauJetAuxContainer#*",
        "xAOD::TruthVertexContainer#*",
        "xAOD::TruthVertexAuxContainer#*",
        "xAOD::VertexContainer#*",
        "xAOD::VertexAuxContainer#*",
        "xAOD::TrackParticleContainer#*",
        "xAOD::TrackParticleAuxContainer#*",
        "xAOD::TruthParticleContainer#*",
        "xAOD::TruthParticleAuxContainer#*",
        "xAOD::TruthEventContainer#*",
        "xAOD::TruthEventAuxContainer#*",
        "xAOD::EventInfo#EventInfo",
        "xAOD::EventAuxInfo#EventInfoAux",
        "xAOD::TruthPileupEventContainer#*",
        "xAOD::TruthPileupEventAuxContainer#*"
    ]

    cfg.merge(OutputStreamCfg(flags, "AOD", ItemList=itemList))

    status = cfg.run()
    sys.exit(not status.isSuccess())
