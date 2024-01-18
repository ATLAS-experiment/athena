# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 
def MdtCablMezzAlgCfg(flags, name = "MdtCablMezzAlg", **kwargs):
    from AthenaConfiguration.ComponentFactory import CompFactory
    from MuonCondTest.MdtCablingTester import setupServicesCfg
    result = setupServicesCfg(flags)
    from MuonConfig.MuonCablingConfig import MDTCablingConfigCfg
    result.merge(MDTCablingConfigCfg(flags))
    event_algo = CompFactory.MdtCablingJsonDumpAlg(name,**kwargs)
    result.addEventAlgo(event_algo, primary = True)
    return result

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from MuonCondTest.MdtCablingTester import SetupArgParser
    parser = SetupArgParser()
    parser.set_defaults(output="SummaryFile.txt")
    parser.set_defaults(mezzMap="MezzMapping.json")
    parser.set_defaults(cablingMap="MdtCabling.json")
   
    args = parser.parse_args()   
    flags = initConfigFlags()
    flags.Concurrency.NumThreads = args.threads
    flags.Concurrency.NumConcurrentEvents = args.threads  # Might change this later, but good enough for the moment.
    flags.Output.ESDFileName = args.output
    flags.Input.Files = args.inputFile
    flags.lock()   
    
    cfg = MdtCablMezzAlgCfg(flags,
                            SummaryFile=args.output,
                            OutMezzanineJSON=args.mezzMap,
                            OutCablingJSON=args.cablingMap)
    cfg.printConfig(withDetails=True, summariseProps=True)
    flags.dump()

    sc = cfg.run(1)
    if not sc.isSuccess():
        import sys
        sys.exit("Execution failed")


