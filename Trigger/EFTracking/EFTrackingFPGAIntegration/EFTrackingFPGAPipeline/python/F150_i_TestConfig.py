# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration 

if __name__ == "__main__":
    from argparse import ArgumentParser
    argumentParser = ArgumentParser()
    argumentParser.add_argument(
        "--xclbinPath", 
        default = "/eos/project/a/atlas-eftracking/FPGA_compilation/FPGA_compilation_hw/22_pathfinder_HLS/Pathfinder_hw.xclbin",
    )

    argumentParser.add_argument(
        "--inputPath",
        default = "/eos/project-a/atlas-eftracking/TestVectors/FPGATrackSim_TVs/Test_Vectors_v0-6-3c/F150_Region34_SingleMuon/stripL2G_output.txt",
    )

    argumentParser.add_argument(
        "--slicingEngineOutputPath", 
        default = "slicing_engine_output.txt",
    )

    argumentParser.add_argument(
        "--insideOutOutputPath", 
        default = "inside_out_output.txt",
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
        inputCsvPath = arguments.inputPath,
        inputDataStream = "inputDataStream",
    ))

    from EFTrackingFPGAPipeline.EFTrackingXrtAlgorithmConfig import EFTrackingXrtAlgorithmCfg
    acc.merge(EFTrackingXrtAlgorithmCfg(
        flags, 
        bufferSize = arguments.bufferSize,
        inputInterfaces = [
            ["configurableLengthWideLoader:{configurableLengthWideLoader_1}", "inputDataStream", 0],
        ],
        outputInterfaces = [
            ["dynamicLengthWideUnloader:{dynamicLengthWideUnloader_1}", "slicingEngineOutputDataStream", 1],
            ["mem_write", "insideOutOutputDataStream", 0]
        ],
        vSizeInterfaces = [
            ["configurableLengthWideLoader:{configurableLengthWideLoader_1}", "inputDataStream", 2],
        ],
        sharedInterfaces = [
            ["mem_read", 0, "dynamicLengthWideUnloader:{dynamicLengthWideUnloader_1}", 1],
        ],
        kernelOrder = [
            [
                "configurableLengthWideLoader:{configurableLengthWideLoader_1}",
                "dynamicLengthWideUnloader:{dynamicLengthWideUnloader_1}",
            ],
            [
                "mem_read",
                "mem_write",
            ],
        ],
    ))

    from EFTrackingFPGAUtility.EFTrackingDataStreamUnloaderAlgorithmConfig import EFTrackingDataStreamUnloaderAlgorithmCfg
    acc.merge(EFTrackingDataStreamUnloaderAlgorithmCfg(
        flags,
        name = "slicingEngineDataStreamUnloaderAlgorithm",
        outputCsvPath = arguments.slicingEngineOutputPath,
        outputDataStream = "slicingEngineOutputDataStream",
    ))

    acc.merge(EFTrackingDataStreamUnloaderAlgorithmCfg(
        flags,
        name = "insideOutDataStreamUnloaderAlgorithm",
        outputCsvPath = arguments.insideOutOutputPath,
        outputDataStream = "insideOutOutputDataStream",
    ))

    acc.run(arguments.events)

