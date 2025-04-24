# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

def NSWDcsAlgTest(flags,alg_name="NSWDcsTestAlg", **kwargs):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()
    from AthenaConfiguration.ComponentFactory import CompFactory
    from MuonConfig.MuonCondAlgConfig import NswDcsDbAlgCfg
    result.merge(NswDcsDbAlgCfg(flags))
    the_alg = CompFactory.NswDcsTestAlg(alg_name, **kwargs)
    result.addEventAlgo(the_alg, primary=True)
    return result
    

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from .MdtCablingTester import SetupArgParser, setupServicesCfg
    
    parser = SetupArgParser()
    parser.set_defaults(inputFile=["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/data23/ESD/data23_cos.00448208.express_express.recon.ESD.x721/73events.data23_cos.00448208.express_express.recon.ESD.x721._lb0003._SFO-ALL._0001.1"])
    
    parser.add_argument("--LogName", default="LogFile", 
                        help="If the test is run multiple times to ensure reproducibility, then the dump of the test can be resteered")
    args = parser.parse_args()

    flags = initConfigFlags()
    flags.Concurrency.NumThreads = args.threads
    flags.Concurrency.NumConcurrentEvents = args.threads  # Might change this later, but good enough for the moment.
    flags.Output.ESDFileName = args.output
    flags.Input.Files = args.inputFile

    from AthenaConfiguration.TestDefaults import defaultConditionsTags
    flags.IOVDb.GlobalTag = defaultConditionsTags.RUN3_DATA23

    flags.lock()
   
    cfg = setupServicesCfg(flags)
    cfg.merge(NSWDcsAlgTest(flags, LogName = args.LogName))
    cfg.printConfig(withDetails=True, summariseProps=True)

    flags.dump()

    sc = cfg.run(1)
    if not sc.isSuccess():
        import sys
        sys.exit("Execution failed")

