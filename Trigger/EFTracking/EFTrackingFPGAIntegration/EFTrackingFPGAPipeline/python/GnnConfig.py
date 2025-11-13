# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration 

if __name__ == "__main__":
    from argparse import ArgumentParser
    argumentParser = ArgumentParser()
    argumentParser.add_argument(
        "--xclbinPath", 
        required = True,
    )

    argumentParser.add_argument(
        "--inputTestVectorPath",
        default = "/eos/project-a/atlas-eftracking/TestVectors/FPGATrackSim_TVs/Test_Vectors_v0-6-3d/F150_Region34_SingleMuon/slicing_PixelFirst_output.txt",
    )

    argumentParser.add_argument(
        "--graphOutputPath", 
        default = "graph_output.txt",
    )

    argumentParser.add_argument(
        "--eventOutputPath", 
        default = "event_output.txt",
    )

    argumentParser.add_argument(
        "--inferenceOutputPath", 
        default = "inference_output.txt",
    )

    argumentParser.add_argument(
        "--bufferSize", 
        type = int, 
        default = 8192,
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
        name = "inputDataStreamLoader",
        bufferSize = arguments.bufferSize,
        inputCsvPath = arguments.inputTestVectorPath,
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
            ["dynamicLengthWideUnloader:{dynamicLengthWideUnloader_1}", "graphOutputDataStream", 1],
            ["dynamicLengthWideUnloader:{dynamicLengthWideUnloader_2}", "eventOutputDataStream", 1],
            ["dynamicLengthWideUnloader:{dynamicLengthWideUnloader_3}", "inferenceOutputDataStream", 1],
        ],
        vSizeInterfaces = [
            ["configurableLengthWideLoader:{configurableLengthWideLoader_1}", "inputDataStream", 2],
        ],
        kernelOrder = [
            [
                "configurableLengthWideLoader:{configurableLengthWideLoader_1}",
                #"GNN_GC:{GNN_GC_1}",
                "dynamicLengthWideUnloader:{dynamicLengthWideUnloader_1}",
                "dynamicLengthWideUnloader:{dynamicLengthWideUnloader_2}",
                "dynamicLengthWideUnloader:{dynamicLengthWideUnloader_3}",
            ],
        ]
    ))

    from EFTrackingFPGAUtility.EFTrackingDataStreamUnloaderAlgorithmConfig import EFTrackingDataStreamUnloaderAlgorithmCfg
    acc.merge(EFTrackingDataStreamUnloaderAlgorithmCfg(
        flags,
        name = "graphDataStreamUnloader",
        outputCsvPath = arguments.graphOutputPath,
        outputDataStream = "graphOutputDataStream",
    ))

    acc.merge(EFTrackingDataStreamUnloaderAlgorithmCfg(
        flags,
        name = "eventDataStreamUnloader",
        outputCsvPath = arguments.eventOutputPath,
        outputDataStream = "eventOutputDataStream",
    ))

    acc.merge(EFTrackingDataStreamUnloaderAlgorithmCfg(
        flags,
        name = "inferenceDataStreamUnloader",
        outputCsvPath = arguments.graphOutputPath,
        outputDataStream = "inferenceOutputDataStream",
    ))

    acc.run(2)

