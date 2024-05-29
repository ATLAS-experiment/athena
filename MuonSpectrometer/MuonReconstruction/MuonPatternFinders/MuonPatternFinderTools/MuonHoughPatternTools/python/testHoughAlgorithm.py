# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
         
def SetupArgParser():
    from argparse import ArgumentParser

    parser = ArgumentParser()
    parser.add_argument("-o", "--output", dest="output", default='', help="Text file containing each cabling channel", metavar="FILE")
    parser.add_argument("--inputFile", "-i", default=["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonRecRTT/UnitTestInput/Run3MC.ESD.pool.root"], 
                        help="Input file to run on ", nargs="+")
    return parser
    

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    args = SetupArgParser().parse_args()

    flags = initConfigFlags()
    flags.Concurrency.NumThreads = 1
    flags.Concurrency.NumConcurrentEvents = 1

    flags.Scheduler.CheckDependencies = True
    flags.Scheduler.ShowDataDeps = True
    flags.Scheduler.ShowDataFlow = True
    flags.Scheduler.ShowControlFlow = True
    flags.Scheduler.EnableVerboseViews = True
    flags.Scheduler.AutoLoadUnmetDependencies = True

    
    flags.Output.ESDFileName = args.output
    flags.Input.Files = args.inputFile    
    flags.lock()
    
    from MuonCondTest.MdtCablingTester import setupServicesCfg
    cfg = setupServicesCfg(flags)
    from MuonConfig.MuonSegmentFindingConfig import MuonLayerHoughAlgCfg, MuonLayerHoughToolCfg
    cfg.merge(MuonLayerHoughAlgCfg(flags,
                                    MuonLayerScanTool=cfg.popToolsAndMerge(
                                                    MuonLayerHoughToolCfg(flags,
                                                                          DebugHough=False))))

    from MuonHoughDataNtuple.MuonHoughDataNtuple import MakeMuonHoughDataNtuple
    cfg.merge(MakeMuonHoughDataNtuple(flags))
    cfg.printConfig(withDetails=True, summariseProps=True)
    flags.dump()
   
    sc = cfg.run(1)
    if not sc.isSuccess():
        import sys
        sys.exit("Execution failed")


