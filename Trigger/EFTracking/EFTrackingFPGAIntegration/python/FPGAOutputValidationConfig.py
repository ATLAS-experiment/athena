# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

def FPGAOutputValidationCfg(flags, **kwargs):
    kwargs.setdefault("pixelNames", ["ITkPixelClusters"])

    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    from AthenaConfiguration.ComponentFactory import CompFactory 
    acc.addEventAlgo(CompFactory.FPGAOutputValidationAlg(
        "FPGAOutputValidationAlg", 
        **kwargs
    ))

    acc.addService(CompFactory.THistSvc(
        name="THistSvc",
        Output=["FPGAOutputValidation DATAFILE='FPGAOutputValidation.root' OPT='RECREATE'"]
    ))

    return acc

if __name__=="__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    from argparse import ArgumentParser
    argumentParser = ArgumentParser()
    argumentParser.add_argument("--inputFiles", default = [], action = "append", required = True)
    argumentParser.add_argument("--outputFile", default = "FPGAOutputValidation.root")
    argumentParser.add_argument("--pixelNames", default = [], action = "append")
    argumentParser.add_argument("--stripNames", default = [], action = "append")
    argumentParser.add_argument("--threads", default = 1)

    arguments = argumentParser.parse_args()

    flags = initConfigFlags()
    flags.Concurrency.NumThreads = arguments.threads
    flags.Input.Files = arguments.inputFiles
    flags.Output.AODFileName = arguments.outputFile

    flags.lock()
    
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))

    acc.merge(FPGAOutputValidationCfg(flags, **{
        "pixelNames": arguments.pixelNames,
        "stripNames": arguments.stripNames,
    }))

    acc.run(-1)

