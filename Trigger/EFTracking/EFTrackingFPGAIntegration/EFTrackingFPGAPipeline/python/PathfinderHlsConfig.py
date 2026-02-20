# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration 

if __name__ == "__main__":
    from argparse import ArgumentParser
    argumentParser = ArgumentParser()
    argumentParser.add_argument(
        "--xclbinPath", 
        default = "/eos/project/a/atlas-eftracking/FPGA_compilation/FPGA_compilation_hw/22_pathfinder_HLS/Pathfinder_hw.xclbin",
    )

    argumentParser.add_argument(
        "--hitTestVectorPath",
        default = "/eos/project/a/atlas-eftracking/TestVectors/FPGATrackSim_TVs/Test_Vectors_v0-6-3d/F150_Region34_SingleMuon/stripL2G_output.txt",
    )

    argumentParser.add_argument(
        "--trackTestVectorPath",
        default = "/eos/project/a/atlas-eftracking/TestVectors/FPGATrackSim_TVs/Test_Vectors_v0-6-3d/F150_Region34_SingleMuon/pattern_reco_output.txt",
    )

    argumentParser.add_argument(
        "--outputPath", 
        default = "pathfinder_output.txt",
    )

    argumentParser.add_argument(
        "--bufferSize", 
        type = int, 
        default = 8192,
    )

    argumentParser.add_argument(
        "--events", 
        type = int, 
        default = 2,
    )

    argumentParser.add_argument(
        "--verbose", 
        action = "store_true",
    )

    arguments = argumentParser.parse_args()

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    if arguments.verbose:
        from AthenaCommon.Constants import DEBUG
        flags.Exec.OutputLevel = DEBUG

    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    from AthenaConfiguration.ComponentFactory import CompFactory 
    acc.addService(CompFactory.ChronoStatSvc(
        PrintUserTime = True,
        PrintSystemTime = True,
        PrintEllapsedTime = True,
    ))

    acc.addService(CompFactory.AthXRT.DeviceMgmtSvc(XclbinPathsList = [arguments.xclbinPath]))

    from EFTrackingFPGAUtility.EFTrackingDataStreamLoaderAlgorithmConfig import EFTrackingDataStreamLoaderAlgorithmCfg
    acc.merge(EFTrackingDataStreamLoaderAlgorithmCfg(
        flags,
        name = "dataStreamLoader",
        bufferSize = arguments.bufferSize,
        GHITZTxtInputPaths = [
            arguments.trackTestVectorPath,
            arguments.hitTestVectorPath,
        ],
        GHITZTxtInputKeys = [
            "pattern_reco_output",
            "stripL2G_output",
        ],
    ))

    from EFTrackingFPGAPipeline.EFTrackingXrtAlgorithmConfig import EFTrackingXrtAlgorithmCfg
    acc.merge(EFTrackingXrtAlgorithmCfg(
        flags, 
        inputInterfaces = [
            ["configurableLengthWideLoader:{configurableLengthWideLoader_1}", "pattern_reco_output", 0],
            ["configurableLengthWideLoader:{configurableLengthWideLoader_2}", "stripL2G_output", 0],
        ],
        outputInterfaces = [
            ["dynamicLengthWideUnloader:{dynamicLengthWideUnloader_1}", "pathfinder_output", 1],
        ],
        vSizeInterfaces = [
            ["configurableLengthWideLoader:{configurableLengthWideLoader_1}", "pattern_reco_output", 2],
            ["configurableLengthWideLoader:{configurableLengthWideLoader_2}", "stripL2G_output", 2],
        ],
        kernelOrder = [
            [
                "configurableLengthWideLoader:{configurableLengthWideLoader_1}",
                "configurableLengthWideLoader:{configurableLengthWideLoader_2}",
                "dynamicLengthWideUnloader:{dynamicLengthWideUnloader_1}",
            ],
        ]
    ))

    from EFTrackingFPGAUtility.EFTrackingDataStreamUnloaderAlgorithmConfig import EFTrackingDataStreamUnloaderAlgorithmCfg
    acc.merge(EFTrackingDataStreamUnloaderAlgorithmCfg(
        flags,
        name = "dataStreamUnloader",
        GHITZTxtInputPaths = [
            arguments.outputPath,
        ],
        GHITZTxtInputKeys = [
            "pathfinder_output",
        ],
    ))

    acc.run(arguments.events)

