
#Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def xRpcToRpcPrepDataCnvAlgCfg(flags,name = "xAODRpcToPrepDataCnvAlg", **kwargs):
    result = ComponentAccumulator()
    the_alg = CompFactory.MuonR4.xRpcMeasToRpcTrkPrdCnvAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def MuonPrepDataToxAODCnvAlg(flags,name="MuonPrepDataToxAODCnvAlg", **kwargs):
    result = ComponentAccumulator()
    the_alg = CompFactory.Muon.PRDxAODConvertorAlg(name=name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def SetupArgParser():
    from argparse import ArgumentParser
    from AthenaCommon.TestDefaults import defaultTestFiles

    parser = ArgumentParser()
    parser.add_argument("-t", "--threads", dest="threads", type=int, help="number of threads", default=1)
    parser.add_argument("-o", "--output", dest="output", default='PrepDataTest.pool.root', help="Text file containing each cabling channel", metavar="FILE")
    parser.add_argument("--inputFile", "-i", default=defaultTestFiles.ESD_RUN3_MC, 
                        help="Input file to run on ", nargs="+")
    return parser

def setupTestOutputCfg(flags,**kwargs):

    kwargs.setdefault("streamName","MPRDTestStream")
    kwargs.setdefault("AcceptAlgs",[])
  
    result = ComponentAccumulator()
    ### Setup an xAOD Stream to test the size of the Mdt container
    # =============================
    # Define contents of the format
    # =============================
    container_items = ["xAOD::MdtDriftCircleContainer#", "xAOD::MdtDriftCircleAuxContainer#",
                        "xAOD::RpcStripContainer#",  "xAOD::RpcStripAuxContainer#",
                        "xAOD::TgcStripContainer#",  "xAOD::TgcStripAuxContainer#",
                        "xAOD::MMClusterContainer#", "xAOD::MMClusterAuxContainer#",
                        "xAOD::sTgcStripContainer#", "xAOD::sTgcStripAuxContaine#",
                        "xAOD::sTgcWireContainer#",  "xAOD::sTgcWireAuxContainer#",
                        "xAOD::sTgcPadContainer#",   "xAOD::sTgcPadAuxContainer#",
                       ]
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    kwargs.setdefault("ItemList", container_items)
    result.merge(OutputStreamCfg(flags, **kwargs))
    return result

    
if __name__ == "__main__":
    from MuonCondTest.MdtCablingTester import setupServicesCfg
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    args = SetupArgParser().parse_args()

    flags = initConfigFlags()
    flags.Concurrency.NumThreads = args.threads
    flags.Concurrency.NumConcurrentEvents = args.threads  # Might change this later, but good enough for the moment.
    flags.Output.doWriteDAOD = True
    flags.Input.Files = args.inputFile
    flags.addFlag("Output.MPRDTestStreamFileName", args.output) 
    flags.addFlag("Output.doWriteMPRDTestStream", True)
    flags.lock()
    
    cfg = setupServicesCfg(flags)
    from MuonConfig.MuonDataPrepConfig import xAODUncalibMeasPrepCfg
    cfg.merge(xAODUncalibMeasPrepCfg(flags))
    cfg.merge(MuonPrepDataToxAODCnvAlg(flags))
    cfg.merge(setupTestOutputCfg(flags))

    cfg.printConfig(withDetails=True, summariseProps=True)
    flags.dump()
   
    sc = cfg.run(-1)
    if not sc.isSuccess():
        import sys
        sys.exit("Execution failed")

