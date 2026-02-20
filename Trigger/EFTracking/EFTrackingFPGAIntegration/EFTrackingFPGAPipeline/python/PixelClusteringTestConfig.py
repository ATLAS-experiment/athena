# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration 

if __name__ == "__main__":
    from argparse import ArgumentParser
    argumentParser = ArgumentParser()
    argumentParser.add_argument(
        "--xclbinPath", 
        required = True,
    )

    argumentParser.add_argument(
        "--testVectorPath",
        required = True,
    )

    argumentParser.add_argument(
        "--outputClusterPath", 
        default = "pixel_clustering_output.txt",
    )

    argumentParser.add_argument(
        "--outputEdmPath", 
        default = "pixel_clustering_edm_output.txt",
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
            arguments.inputTestVectorPath,
        ],
        GHITZTxtInputKeys = [
            "pixel_clustering_input",
        ],
    ))

    from EFTrackingFPGAPipeline.EFTrackingXrtAlgorithmConfig import EFTrackingXrtAlgorithmCfg
    acc.merge(EFTrackingXrtAlgorithmCfg(
        flags, 
        bufferSize = arguments.bufferSize,
        inputInterfaces = [
            ["configurableLengthWideLoader:{configurableLengthWideLoader_1}", "pixel_clustering_input", 0],
        ],
        outputInterfaces = [
            ["dynamicLengthWideUnloader:{dynamicLengthWideUnloader_1}", "pixel_clustering_output", 1],
            ["dynamicLengthWideUnloader:{dynamicLengthWideUnloader_2}", "pixel_clustering_edm_output", 1],
        ],
        vSizeInterfaces = [
            ["configurableLengthWideLoader:{configurableLengthWideLoader_1}", "pixel_clustering_input", 2],
        ],
    ))

    from EFTrackingFPGAUtility.EFTrackingDataStreamUnloaderAlgorithmConfig import EFTrackingDataStreamUnloaderAlgorithmCfg
    acc.merge(EFTrackingDataStreamUnloaderAlgorithmCfg(
        flags,
        name = "dataStreamUnloader",
        GHITZTxtOutputPaths = [
            arguments.outputPath,
            arguments.outputEdmPath,
        ],
        GHITZTxtOutputKeys = [
            "pixel_clustering_output",
            "pixel_clustering_edm_output",
        ],
    ))

    acc.run(arguments.events)

