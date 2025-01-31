# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration 

import ROOT

def EFTrackingXrtAlgorithmCfg(flags, **kwargs):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    kwargs.setdefault("bufferSize", 8192)
    kwargs.setdefault(
       "xclbinPath", 
       "/eos/project-a/atlas-eftracking/FPGA_compilation/FPGA_compilation_hw/12_road2track_HLS/Road2Track_hw.xclbin"
    )

    from json import dumps
    kwargs.setdefault("kernelDefinitionsJsonString", dumps({
      "loader": [{"storeGateKey": "inputDataStream",
                  "argumentIndex": 0,
                  "interfaceMode": ROOT.EFTrackingXrtParameters.InterfaceMode.INPUT}],
      "unloader": [{"storeGateKey": "outputDataStream",
                    "argumentIndex": 1,
                    "interfaceMode": ROOT.EFTrackingXrtParameters.InterfaceMode.OUTPUT}]}))

    from AthenaConfiguration.ComponentFactory import CompFactory 
    EFTrackingXrtAlgorithm = CompFactory.EFTrackingXrtAlgorithm("EFTrackingXrtAlgorithm", **kwargs)

    acc.addEventAlgo(EFTrackingXrtAlgorithm)

    return acc

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    from argparse import ArgumentParser
    argumentParser = ArgumentParser()
    argumentParser.add_argument("--xclbinPath")
    argumentParser.add_argument("--bufferSize", type = int, default = 8192)

    from argparse import Action
    class JsonToDictAction(Action):
        def __call__(self, parser, namespace, values, option_string=None):
            # This is a hack but this is only needed when mapping commandline 
            # arguments into an enum.
            #
            # The underlying dictionary can be hardcoded in dedicated Cfg
            # functions, see defaults in EFTrackingXrtAlgorithmCfg 
            #
            # Get the members of InterfaceModes that are not present in an 
            # EmptyEnum (i.e. the actual members of InterfaceModes).
            interfaceModes = [(member, int(getattr(ROOT.EFTrackingXrtParameters.InterfaceMode, member)))
                              for member in dir(ROOT.EFTrackingXrtParameters.InterfaceMode) 
                              if member not in dir(ROOT.EFTrackingXrtParameters.EmptyEnum)]

            for interfaceModeString, interfaceModeValue in interfaceModes:
                values = values.replace(interfaceModeString, str(interfaceModeValue)) 

            from json import loads
            setattr(namespace, self.dest, loads(values))

    argumentParser.add_argument("--kernelDefinitions", action=JsonToDictAction)
    argumentParser.add_argument("--inputCsvPathToSgKeyMap", action=JsonToDictAction)
    argumentParser.add_argument("--outputCsvPathToSgKeyMap", action=JsonToDictAction)

    arguments = argumentParser.parse_args()

    from AthenaConfiguration.ComponentFactory import CompFactory 
    acc.addService(CompFactory.AthXRT.DeviceMgmtSvc(XclbinPathsList = [arguments.xclbinPath]))

    for inputCsvPath, sgKey in arguments.inputCsvPathToSgKeyMap.items():
        from EFTrackingFPGAIntegration.EFTrackingDataStreamLoaderAlgorithmConfig import EFTrackingDataStreamLoaderAlgorithmCfg
        acc.merge(EFTrackingDataStreamLoaderAlgorithmCfg(flags,
                                                         bufferSize = arguments.bufferSize,
                                                         inputCsvPath = inputCsvPath,
                                                         inputDataStream = sgKey))

    from json import dumps
    acc.merge(EFTrackingXrtAlgorithmCfg(flags, 
                                        bufferSize = arguments.bufferSize,
                                        xclbinPath = arguments.xclbinPath,
                                        kernelDefinitionsJsonString = dumps(arguments.kernelDefinitions)))

    for outputCsvPath, sgKey in arguments.outputCsvPathToSgKeyMap.items():
        from EFTrackingFPGAIntegration.EFTrackingDataStreamUnloaderAlgorithmConfig import EFTrackingDataStreamUnloaderAlgorithmCfg
        acc.merge(EFTrackingDataStreamUnloaderAlgorithmCfg(flags,
                                                           outputCsvPath = outputCsvPath,
                                                           outputDataStream = sgKey))

    acc.run(1)

