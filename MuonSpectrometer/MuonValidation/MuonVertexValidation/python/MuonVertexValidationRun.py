#
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def SetupArgParser():
    from argparse import ArgumentParser

    parser = ArgumentParser()
    parser.add_argument( "-i", "--inputFile", required=True, help="Input file to run on ", nargs="+") # flexible number of arguments, gathered into a list
    parser.add_argument( "-o", "--outputFile", default="MSVtxVal_out.root", help="output root file")
    parser.add_argument("--maxEvents", default=-1, type=int, help="How many events shall be run maximally")
    parser.add_argument("--skipEvents", default=0, type=int, help="How many events shall be skipped")
    parser.add_argument("--threads", default=1, type=int, help="number of threads")

    return parser


def setupHistSvcCfg(flags, out_file="out.root", out_stream="MSVtxValidation"):
    result = ComponentAccumulator()
    if len(out_file) == 0: return result
    histSvc = CompFactory.THistSvc(Output=[f"{out_stream} DATAFILE='{out_file}', OPT='RECREATE'"])
    result.addService(histSvc, primary=True)
    return result


def MSVtxValidationCfg(flags, name="MSVertexValidationAlg", outStream="MSVtxValidation", outFile="out.root", **kwargs):
    # outStream defines the steam to place the tree and histograms 
    result = ComponentAccumulator()
    # setting algorithm properties here via kwargs.setdefault("<property name>", <property value>)
    alg = CompFactory.MSVtxValidationAlg(name, **kwargs)
    result.merge(setupHistSvcCfg(flags,out_file=outFile, out_stream=outStream))
    result.addEventAlgo(alg)

    return result


def execute(cfg):
    cfg.printConfig(withDetails=True, summariseProps=True)
    if not cfg.run().isSuccess(): exit(1)


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from MuonCondTest.MdtCablingTester import setupServicesCfg

    args = SetupArgParser().parse_args()
    flags = initConfigFlags()
    flags.Concurrency.NumThreads = args.threads
    flags.Exec.MaxEvents = args.maxEvents
    flags.Exec.SkipEvents = args.skipEvents
    flags.Concurrency.NumConcurrentEvents = args.threads
    flags.Input.Files = args.inputFile 
    flags.Scheduler.ShowDataDeps = True 
    flags.Scheduler.ShowDataFlow = True
    flags.lock()
    cfg = setupServicesCfg(flags)
    cfg.merge(MSVtxValidationCfg(flags, outFile=args.outputFile))
    execute(cfg)
