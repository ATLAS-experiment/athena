# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration 

import ROOT

def PathfinderHlsCfg(flags, **kwargs):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    kwargs.setdefault("bufferSize", 8192)

    from EFTrackingFPGAPipeline.EFTrackingXrtAlgorithmConfig import EFTrackingXrtAlgorithmCfg
    from json import dumps
    acc.merge(EFTrackingXrtAlgorithmCfg(
        flags, 
        bufferSize = kwargs.bufferSize,
        kernelDefinitionsJsonString = dumps({
            "loader": [{
                "storeGateKey": "inputDataStream",
                "argumentIndex": "0",
                "interfaceMode": str(ROOT.EFTrackingXrtParameters.InterfaceMode.INPUT),
            }],
            "unloader": [{
                "storeGateKey": "outputDataStream",
                "argumentIndex": "1",
                "interfaceMode": str(ROOT.EFTrackingXrtParameters.InterfaceMode.OUTPUT),
            }],
        }),
    ))

    return acc

if __name__ == "__main__":
    from argparse import ArgumentParser
    argumentParser = ArgumentParser()
    argumentParser.add_argument("--eosPath")
    argumentParser.add_argument("--bufferSize", type = int, default = 8192)
    argumentParser.add_argument("--inputTracksCsvPath")
    argumentParser.add_argument("--outputTracksCsvPath")

    arguments = argumentParser.parse_args()

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    from AthenaConfiguration.ComponentFactory import CompFactory 
    acc.addService(CompFactory.AthXRT.DeviceMgmtSvc(XclbinPathsList = [
        f"{arguments.eosPath}/project/a/atlas-eftracking/FPGA_compilation/FPGA_compilation_hw_emu/22_pathfinder_HLS/Pathfinder_hw.xclbin"
    ]))

    from EFTrackingFPGAUtility.EFTrackingDataStreamLoaderAlgorithmConfig import EFTrackingDataStreamLoaderAlgorithmCfg
    acc.merge(EFTrackingDataStreamLoaderAlgorithmCfg(
        flags,
        bufferSize = arguments.bufferSize,
        inputCsvPath = f"{arguments.eosPath}/project/a/atlas-eftracking/TestVectors/FPGATrackSim_TVs/Test_Vectors_v0-6-3/F600_Region34_SingleMuon/pattern_reco_output.bin",
        inputDataStream = "inputDataStream",
    ))

    acc.merge(PathfinderHlsCfg(
        flags,
        bufferSize = arguments.bufferSize,
    ))

    from EFTrackingFPGAUtility.EFTrackingDataStreamUnloaderAlgorithmConfig import EFTrackingDataStreamUnloaderAlgorithmCfg
    acc.merge(EFTrackingDataStreamUnloaderAlgorithmCfg(
        flags,
        outputCsvPath = "PathfinderHls.txt",
        outputDataStream = "outputDataStream",
    ))

    acc.run(2)

