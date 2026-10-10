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
    flags.Detector.GeometryTGC = True
    flags.Detector.GeometryMM = True
    flags.Detector.GeometrysTGC = True
    
    flags.Common.MsgSuppression = False
    flags.Output.RDOFileName = "test.RDO.pool.root"
    flags, acc = setupGeoR4TestCfg(args, flags)
    from AthenaCommon.Constants import DEBUG

    run_rpc_chain = flags.Detector.GeometryRPC
    run_mdt_chain = run_rpc_chain and flags.Detector.GeometryMDT
    if not run_rpc_chain:
        _log.warning(
            "Skipping the truth-based RPC and dependent MDT chains")
    elif not run_mdt_chain:
        _log.warning("Skipping the MDT chain because MDT geometry is disabled")

    acc.merge(setupHistSvcCfg(flags,outFile = "L0MuonFullChain.root", 
                                    outStream = "EXPERT"))

    if run_rpc_chain:
        from DerivationFrameworkMCTruth.MCTruthCommonConfig import HepMCtoXAODTruthCfg
        acc.merge(HepMCtoXAODTruthCfg(flags))
        from MuonConfig.MuonByteStreamCnvTestConfig import RpcRdoToRpcDigitCfg
        acc.merge(RpcRdoToRpcDigitCfg(flags))
        ## Create the xAOD::TruthParticles for RPC simulation

        from L1MuonS1RPC.L1MuonS1RPCConfig import L1MuonRPCSimCfg
        acc.merge(L1MuonRPCSimCfg(flags,
            name="L1MuonRPCSim",
            OutputLevel=DEBUG))

    from L1MuonS1TGC.L0MuonS1TGCConfig import L0MuonTGCSimCfg
    acc.merge(L0MuonTGCSimCfg(flags,
        configureHistSvc=False,
        OutputLevel=DEBUG))

    from L1MuonEndcap.L0MuonEndcapConfig import L0MuonEndcapAlgCfg
    acc.merge(L0MuonEndcapAlgCfg(flags))

    if run_mdt_chain:
        from MuonConfig.MuonRdoDecodeConfig import MdtRDODecodeCfg
        acc.merge(MdtRDODecodeCfg(flags, name = "MdtRdoToMdtPrepData",
                                         RDOContainer = "MDTCSM" ))

        from RegionSelector.RegSelToolConfig import regSelTool_MDT_Cfg
        from L1MuonMDT.L0MuonMDTConfig import L0MuonMDTSimCfg
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

    from L1MuonNSW.L1MuonNSWConfig import L1MuonNSWSimCfg
    acc.merge(L1MuonNSWSimCfg(flags, name="L1MuonNSWSim", OutputLevel=DEBUG))

    print("=== Registered services ===")
    for svc in acc.getServices():
        print(svc.name)

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    itemList = []
    if run_rpc_chain:
        itemList += [
            "xAOD::L1RPCCandDataContainer#L1RPCCandData",
            "xAOD::L1RPCCandDataAuxContainer#L1RPCCandDataAux.",
        ]
    itemList += [
        "xAOD::TGCCandDataContainer#L0MuonTGCCandData",
        "xAOD::TGCCandDataAuxContainer#L0MuonTGCCandDataAux.",
        "xAOD::SectorLogicCandDataContainer#L0MuonTGCSectorLogicCandData",
        "xAOD::SectorLogicCandDataAuxContainer#L0MuonTGCSectorLogicCandDataAux.",
        "xAOD::L1NSWCandDataContainer#L1NSWCandidates",
        "xAOD::L1NSWCandDataAuxContainer#L1NSWCandidatesAux."
    ]

    acc.merge(OutputStreamCfg(flags,
                              "RDO",
                              ItemList=itemList,
                              takeItemsFromInput=False))



    executeTest(acc)
