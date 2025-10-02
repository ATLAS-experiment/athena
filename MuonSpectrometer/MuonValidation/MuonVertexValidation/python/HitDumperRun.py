#
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#


def splitOnComma(inputs):
    files = []
    for item in inputs: files.extend(item.split(','))

    return files


def GetArgsFromParser():
    from argparse import ArgumentParser

    parser = ArgumentParser()
    parser.add_argument( "-i", "--inputFile", required=True, help="Input files to run on. Files can be comma or space separated", nargs="+")
    parser.add_argument( "-o", "--outputFile", default="MSVtxVal_out.NTUP.root", help="output root file")
    parser.add_argument("--maxEvents", default=-1, type=int, help="How many events shall be run maximally")
    parser.add_argument("--skipEvents", default=0, type=int, help="How many events shall be skipped")
    parser.add_argument("--threads", default=1, type=int, help="number of threads")
    
    args = parser.parse_args()
    args.inputFile = splitOnComma(args.inputFile) # to support comma separated input files

    return args


def execute(cfg):
    cfg.printConfig(withDetails=True, summariseProps=True)
    if not cfg.run().isSuccess(): exit(1)


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from MuonCondTest.MdtCablingTester import setupServicesCfg

    args = GetArgsFromParser()
    flags = initConfigFlags()
    flags.Concurrency.NumThreads = args.threads
    flags.Exec.MaxEvents = args.maxEvents
    flags.Exec.SkipEvents = args.skipEvents
    flags.Concurrency.NumConcurrentEvents = args.threads
    flags.Input.Files = args.inputFile 
    flags.Scheduler.ShowDataDeps = True 
    flags.Scheduler.ShowDataFlow = True
    flags.Detector.EnableMM = False
    flags.Detector.EnablesTGC = False
    flags.Detector.EnableRPC = False
    flags.Detector.EnableMDT = False
    flags.Detector.EnableTGC = False
    flags.Detector.EnableCSC = False
    flags.lock()

    cfg = setupServicesCfg(flags)
    from MuonPRDTest.HitValAlgReco import HitValAlgRecoCfg
    cfg.merge(HitValAlgRecoCfg(flags, outFile=args.outputFile, 
                               doTruth=False, doMuEntry=False, 
                               doSDOs=True, doPRDs=True,
                               doMDTSDO=True, doMDTPRD=True,
                               doRPCSDO=True, doRPCPRD=True))

    flags.dump(evaluate = True)
    execute(cfg)
