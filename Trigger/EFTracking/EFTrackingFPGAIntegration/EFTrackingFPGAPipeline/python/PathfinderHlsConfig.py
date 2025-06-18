# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration 

import ROOT

def PathfinderHlsCfg(flags, **kwargs):
    kwargs.setdefault("bufferSize", 8192)

    from json import dumps
    kwargs.setdefault(
        "kernelDefinitionsJsonString",
        dumps({
            "loader:{loader_1}": [{
                "storeGateKey": "inputTrackDataStream",
                "argumentIndex": "0",
                "interfaceMode": str(ROOT.EFTrackingXrtParameters.InterfaceMode.INPUT),
            }],
            "loader:{loader_2}": [{
                "storeGateKey": "inputHitDataStream",
                "argumentIndex": "0",
                "interfaceMode": str(ROOT.EFTrackingXrtParameters.InterfaceMode.INPUT),
            }],
            "unloader": [{
                "storeGateKey": "outputDataStream",
                "argumentIndex": "1",
                "interfaceMode": str(ROOT.EFTrackingXrtParameters.InterfaceMode.OUTPUT),
            }],
        }),
    )

    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    from EFTrackingFPGAPipeline.EFTrackingXrtAlgorithmConfig import EFTrackingXrtAlgorithmCfg
    acc.merge(EFTrackingXrtAlgorithmCfg(flags, **kwargs))

    return acc

if __name__ == "__main__":
    from argparse import ArgumentParser
    argumentParser = ArgumentParser()
    argumentParser.add_argument("--eosPath", default = "/eos/")
    argumentParser.add_argument("--bufferSize", type = int, default = 8192)
    argumentParser.add_argument("--verbose", action = "store_true")

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

    acc.addService(CompFactory.AthXRT.DeviceMgmtSvc(
        XclbinPathsList = [f"{arguments.eosPath}/project/a/atlas-eftracking/FPGA_compilation/FPGA_compilation_hw/22_pathfinder_HLS/Pathfinder_hw.xclbin"],
    ))

    from EFTrackingFPGAUtility.EFTrackingDataStreamLoaderAlgorithmConfig import EFTrackingDataStreamLoaderAlgorithmCfg
    acc.merge(EFTrackingDataStreamLoaderAlgorithmCfg(
        flags,
        name = "trackDataStreamLoader",
        bufferSize = arguments.bufferSize,
        inputCsvPath = f"{arguments.eosPath}/project/a/atlas-eftracking/TestVectors/FPGATrackSim_TVs/Test_Vectors_v0-6-3/F600_Region34_SingleMuon/pattern_reco_output.txt",
        inputDataStream = "inputTrackDataStream",
    ))

    acc.merge(EFTrackingDataStreamLoaderAlgorithmCfg(
        flags,
        name = "hitDataStreamLoader",
        bufferSize = arguments.bufferSize,
        # We are still waiting on a proper hits test vector for the pathfinder 
        # but this will do in the meantime (just needs to run without crashing).
        inputCsvPath = f"{arguments.eosPath}/project/a/atlas-eftracking/TestVectors/FPGATrackSim_TVs/Test_Vectors_v0-6-3/F600_Region34_SingleMuon/pattern_reco_output.txt",
        inputDataStream = "inputHitDataStream",
    ))

    # Once the pathfinder neural net has been trained on (r, phi, z) we can 
    # remove this step (along with the actual algorithm).
    from EFTrackingFPGAUtility.MasqueradeCoordinatesConfig import MasqueradeCoordinatesCfg
    acc.merge(MasqueradeCoordinatesCfg(
        flags,
        name = "TrackMasqueradeCoordinates",
        cylindricalDataStream = "inputTrackDataStream",
        cartesianDataStream = "inputMasqueradedTrackDataStream",
    ))

    acc.merge(MasqueradeCoordinatesCfg(
        flags,
        name = "HitMasqueradeCoordinates",
        cylindricalDataStream = "inputHitDataStream",
        cartesianDataStream = "inputMasqueradedHitDataStream",
    ))

    from json import dumps
    acc.merge(PathfinderHlsCfg(
        flags,
        bufferSize = arguments.bufferSize,
        kernelDefinitionsJsonString = dumps({
            "loader:{loader_1}": [{
                "storeGateKey": "inputMasqueradedTrackDataStream",
                "argumentIndex": "0",
                "interfaceMode": str(ROOT.EFTrackingXrtParameters.InterfaceMode.INPUT),
            }],
            "loader:{loader_2}": [{
                "storeGateKey": "inputMasqueradedHitDataStream",
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

    from EFTrackingFPGAUtility.EFTrackingDataStreamUnloaderAlgorithmConfig import EFTrackingDataStreamUnloaderAlgorithmCfg
    acc.merge(EFTrackingDataStreamUnloaderAlgorithmCfg(
        flags,
        outputCsvPath = "PathfinderHls.txt",
        outputDataStream = "outputDataStream",
    ))

    acc.run(2)

