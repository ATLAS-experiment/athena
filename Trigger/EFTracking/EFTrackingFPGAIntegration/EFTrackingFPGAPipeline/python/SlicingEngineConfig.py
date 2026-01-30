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
        default = "/eos/project/a/atlas-eftracking/TestVectors/FPGATrackSim_TVs/Test_Vectors_v0-6-3d/F150_Region34_SingleMuon/stripL2G_output.txt",
    )

    argumentParser.add_argument(
        "--outputPath", 
        default = "slicing_engine_output.txt",
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
        name = "inputDataStreamLoader",
        bufferSize = arguments.bufferSize,
        GHITZTxtInputPaths = [
            arguments.inputTestVectorPath,
        ],
        GHITZTxtInputKeys = [
            "stripL2G_output",
        ],
    ))

    from EFTrackingFPGAPipeline.EFTrackingXrtAlgorithmConfig import EFTrackingXrtAlgorithmCfg
    acc.merge(EFTrackingXrtAlgorithmCfg(
        flags,
        bufferSize = arguments.bufferSize,
        inputInterfaces = [
            ["configurableLengthWideLoader:{configurableLengthWideLoader_1}", "stripL2G_output", 0],
        ],
        outputInterfaces = [
            ["dynamicLengthWideUnloader:{dynamicLengthWideUnloader_1}", "slicing_engine_output", 1],
        ],
        vSizeInterfaces = [
            ["configurableLengthWideLoader:{configurableLengthWideLoader_1}", "stripL2G_output", 2],
        ],
        kernelOrder = [
            [
                "configurableLengthWideLoader:{configurableLengthWideLoader_1}",
                "dynamicLengthWideUnloader:{dynamicLengthWideUnloader_1}",
            ],
        ]
    ))

    acc.merge(EFTrackingDataStreamLoaderAlgorithmCfg(
        flags,
        name = "outputDataStreamLoader",
        bufferSize = arguments.bufferSize,
        GHITZTxtOutputPaths = [
            arguments.outputPath,
        ],
        GHITZTxtOutputKeys = [
            "slicing_engine_output",
        ],
    ))

    acc.run(arguments.events)

