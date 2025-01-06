# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration 

# Todo: Move this helper algorithm to pyAthena
def EFTrackingDataStreamLoaderAlgorithmCfg(flags, **kwargs):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    kwargs.setdefault("bufferSize", 8192);

    from AthenaConfiguration.ComponentFactory import CompFactory 
    acc.addEventAlgo(CompFactory.EFTrackingDataStreamLoaderAlgorithm("EFTrackingDataStreamLoaderAlgorithm", **kwargs))

    return acc

def EFTrackingDataStreamUnloaderAlgorithmCfg(flags, **kwargs):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    from AthenaConfiguration.ComponentFactory import CompFactory 
    acc.addEventAlgo(CompFactory.EFTrackingDataStreamUnloaderAlgorithm("EFTrackingDataStreamUnloaderAlgorithm", **kwargs))

    return acc

def EFTrackingXrtAlgorithmCfg(flags, **kwargs):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    # Example settings. No sensible defaults as the firmware distribution 
    # (cvmfs, eos) is not yet stadardised.
    kwargs.setdefault("bufferSize", 8192);
    #kwargs.setdefault("xclbinPath", "SerialLoaderToSerialUnloader")
    #kwargs.setdefault("kernelDefinitions", {"serialLoader": [{"storeGateKey": "inputDataStream",
    #                                                         "argumentIndex": "0",
    #                                                         "interfaceMode": "INPUT"}],
    #                                        "serialUnloader" : [{"storeGateKey": "outputDataStream",
    #                                                            "argumentIndex": "1",
    #                                                            "interfaceMode": "OUTPUT"}]})

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
    argumentParser.add_argument("--bufferSize", default = 8192)

    from argparse import Action
    class JsonToDictAction(Action):
        def __call__(self, parser, namespace, values, option_string=None):
            from json import loads
            setattr(namespace, self.dest, loads(values))

    argumentParser.add_argument("--kernelDefinitions", action=JsonToDictAction)
    argumentParser.add_argument("--inputCsvPathToSgKeyMap", action=JsonToDictAction)
    argumentParser.add_argument("--outputCsvPathToSgKeyMap", action=JsonToDictAction)

    arguments = argumentParser.parse_args()

    for inputCsvPath, sgKey in arguments.inputCsvPathToSgKeyMap.items():
        acc.merge(EFTrackingDataStreamLoaderAlgorithmCfg(flags,
                                                         bufferSize = arguments.bufferSize,
                                                         inputCsvPath = inputCsvPath,
                                                         inputDataStream = sgKey))

    acc.merge(EFTrackingXrtAlgorithmCfg(flags, 
                                        bufferSize = arguments.bufferSize,
                                        xclbinPath = arguments.xclbinPath,
                                        kernelDefinitions = arguments.kernelDefinitions))

    for outputCsvPath, sgKey in arguments.outputCsvPathToSgKeyMap.items():
        acc.merge(EFTrackingDataStreamUnloaderAlgorithmCfg(flags,
                                                           outputCsvPath = outputCsvPath,
                                                           outputDataStream = sgKey))

    acc.run(1)

