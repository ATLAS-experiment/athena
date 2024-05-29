# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration


from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MuonScaleFactorTestAlgCfg(flags, name="MuonScaleFactorTestAlg", **kwargs):
    result = ComponentAccumulator()
    from MuonSelectorTools.MuonSelectorToolsConfig import MuonSelectionToolCfg
    kwargs.setdefault("MuonSelectionTool", result.popToolsAndMerge(MuonSelectionToolCfg(flags)))
    from AsgAnalysisAlgorithms.PileupReweightingAlgConfig import PileupReweightingToolCfg
    kwargs.setdefault("PileupReweightingTool", result.popToolsAndMerge(PileupReweightingToolCfg(flags)))
    the_alg = CompFactory.CP.MuonScaleFactorTestAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def MuonEfficiencyCorrectionsCfg(flags, name="MuonEffiCorrections", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("ApplyKinematicSystematic", False)
    eff_tool = CompFactory.CP.MuonEfficiencyScaleFactors(name, **kwargs)
    result.setPrivateTools(eff_tool)
    return result

def setupHistSvcCfg(flags, out_file="EffTester.root"):
    result = ComponentAccumulator()
    if len(out_file) == 0: return result
    histSvc = CompFactory.THistSvc(Output=["MUONEFFTESTER DATAFILE='{out_file}', OPT='RECREATE'".format(out_file = out_file)])
    result.addService(histSvc, primary=True)
    return result

def setupArgParser():
    from argparse import ArgumentParser

    parser = ArgumentParser()
    parser.add_argument("--threads", type=int, help="number of threads", default=1)
    parser.add_argument("--inputFile", "-i", default=[
                        "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/mc20_13TeV.361107.PowhegPythia8EvtGen_AZNLOCTEQ6L1_Zmumu.recon.AOD.e3601_s3681_r13167/AOD.27312826._000061.pool.root.1"
                        ], 
                        help="Input file to run on ", nargs="+")
    parser.add_argument("--outRootFile", default="EffiDump.root", help="Output ROOT file to dump the geomerty")
    parser.add_argument("--maxEvents", type=int, help="Maximum events to run on", default = -1)
    return parser



if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags 

    args = setupArgParser().parse_args()

    flags = initConfigFlags()
    flags.Concurrency.NumThreads = args.threads
    flags.Concurrency.NumConcurrentEvents = args.threads  # Might change this later, but good enough for the moment.
    flags.Input.Files = args.inputFile
    flags.Scheduler.ShowDataDeps = True 
    flags.Scheduler.ShowDataFlow = True
    flags.lock()
    flags.dump(evaluate=True)


    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)
    ### Setup the file reading
    from AthenaConfiguration.Enums import Format
    if flags.Input.Format == Format.POOL:
        from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
        cfg.merge(PoolReadCfg(flags))
    
    cfg.merge(setupHistSvcCfg(flags, out_file = args.outRootFile))
    sf_tools= [cfg.popToolsAndMerge(MuonEfficiencyCorrectionsCfg(flags,
                                                                WorkingPoint="Loose"))]
    cfg.merge(MuonScaleFactorTestAlgCfg(flags, EfficiencyTools = sf_tools))

    cfg.printConfig(withDetails=True, summariseProps=True)
   
    sc = cfg.run(args.maxEvents)
    if not sc.isSuccess():
        import sys
        sys.exit("Execution failed")






    

