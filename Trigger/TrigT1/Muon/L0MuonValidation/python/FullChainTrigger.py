#Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory

from AthenaCommon.Logging import logging
_log = logging.getLogger(__name__)


if __name__ == "__main__":
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser, executeTest
    parser = SetupArgParser()
    parser.set_defaults(inputFile= ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonGeomRTT/myRDO.R3.pool.root"])
    parser.set_defaults(nEvents = 20)
    args = parser.parse_args()
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Detector.GeometryRPC = True  
    flags.Detector.GeometryTGC = False
    flags.Detector.GeometryMM = False
    flags.Detector.GeometrysTGC = False
    flags.Detector.EnableMM = False
    flags.Detector.EnablesTGC = False
    flags.Detector.EnableTGC = False
    flags.Common.MsgSuppression = False
    flags, acc = setupGeoR4TestCfg(args, flags)
    from AthenaCommon.Constants import DEBUG

    histSvc = CompFactory.THistSvc(Output=[
        "EXPERT DATAFILE='L0MuonFullChain.root' OPT='RECREATE'"
    ])
    acc.addService(histSvc)

    from DerivationFrameworkMCTruth.MCTruthCommonConfig import HepMCtoXAODTruthCfg
    acc.merge(HepMCtoXAODTruthCfg(flags))
    from MuonConfig.MuonByteStreamCnvTestConfig import RpcRdoToRpcDigitCfg
    acc.merge(RpcRdoToRpcDigitCfg(flags))
    ## Create the xAOD::TruthParticles for RPC simulation
   

    from L0MuonS1RPC.L0MuonS1RPCConfig import L0MuonRPCSimCfg
    acc.merge(L0MuonRPCSimCfg(flags,
        name="L0MuonRPCSim",
        OutputLevel=DEBUG))

    from MuonConfig.MuonRdoDecodeConfig import MdtRDODecodeCfg
    acc.merge(MdtRDODecodeCfg(flags, name = "MdtRdoToMdtPrepData", 
                                     RDOContainer = "MDTCSM" ))  
   
    from RegionSelector.RegSelToolConfig import regSelTool_MDT_Cfg
    from L0MuonMDT.L0MuonMDTConfig import L0MuonMDTSimCfg
    acc.merge(L0MuonMDTSimCfg(flags,
                             name = "L0MuonMDTSim",
                             OutputLevel = DEBUG,
                             RegSel_MDT = acc.popToolsAndMerge(regSelTool_MDT_Cfg(flags))
                            ))

    from AthenaConfiguration.ComponentFactory import CompFactory

    try:
        th = acc.getService("THistSvc")
    except KeyError:
        th = CompFactory.THistSvc("THistSvc")
        acc.addService(th)
    
    th.Output = ["EXPERT DATAFILE='L0MuonFullChain.root' OPT='RECREATE'"]
    
    print("THistSvc.Output =", th.Output)


    print("=== Registered services ===")
    for svc in acc.getServices():
        print(svc.name)


    executeTest(acc)
