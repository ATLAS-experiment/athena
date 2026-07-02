#Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
_log = logging.getLogger(__name__)


if __name__ == "__main__":
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser, MuonPhaseIITestDefaults
    from MuonConfig.MuonConfigUtils import executeTest, setupHistSvcCfg
    parser = SetupArgParser()
    parser.set_defaults(inputFile= MuonPhaseIITestDefaults.RDO_R3)
    parser.set_defaults(nEvents = 20)
    args = parser.parse_args()
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Detector.GeometryRPC = True  
    flags.Detector.GeometryTGC = False
    flags.Detector.GeometryMM = True
    flags.Detector.GeometrysTGC = True
    
    flags.Common.MsgSuppression = False
    flags.Output.RDOFileName = "test.RDO.pool.root"
    flags, acc = setupGeoR4TestCfg(args, flags)
    from AthenaCommon.Constants import DEBUG

    acc.merge(setupHistSvcCfg(flags,outFile = "L0MuonFullChain.root", 
                                    outStream = "EXPERT"))

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

    from MuonConfig.MuonByteStreamCnvTestConfig import STGC_RdoToDigitCfg, MM_RdoToDigitCfg
    
    if flags.Detector.GeometrysTGC:
        acc.merge(STGC_RdoToDigitCfg(flags, sTgcRdoContainer="sTGCRDO", sTgcDigitContainer="sTGC_DIGITS"))

    if flags.Detector.GeometryMM:
        acc.merge(MM_RdoToDigitCfg(flags, MmRdoContainer="MMRDO", MmDigitContainer="MM_DIGITS"))

    from L0MuonNSW.L0MuonNSWConfig import L0MuonNSWSimCfg
    acc.merge(L0MuonNSWSimCfg(flags, name="L0MuonNSWSim", OutputLevel=DEBUG))

    print("=== Registered services ===")
    for svc in acc.getServices():
        print(svc.name)

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    itemList = [
        "xAOD::RPCCandDataContainer#RPCCandData",
        "xAOD::RPCCandDataAuxContainer#RPCCandDataAux.",
        "xAOD::NSWCandDataContainer#NSWCandData",      
        "xAOD::NSWCandDataAuxContainer#NSWCandDataAux." 
    ]

    acc.merge(OutputStreamCfg(flags,
                              "RDO",
                              ItemList=itemList,
                              takeItemsFromInput=False))



    executeTest(acc)
