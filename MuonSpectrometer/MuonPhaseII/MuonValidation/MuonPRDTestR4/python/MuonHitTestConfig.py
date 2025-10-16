# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MuonHitTesterCfg(flags, name="MuonHitTester", outFile="SimHitTest.root", **kwargs):
    result = ComponentAccumulator()
    from MuonConfig.MuonConfigUtils import setupHistSvcCfg
    result.merge(setupHistSvcCfg(flags,outFile=outFile, outStream="MuonR4HitTest"))
    from ActsAlignmentAlgs.AlignmentAlgsConfig import ActsGeometryContextAlgCfg
    result.merge(ActsGeometryContextAlgCfg(flags))
    kwargs.setdefault("isMC", flags.Input.isMC)

    kwargs.setdefault("dumpPileUp", True)
    
    kwargs.setdefault("dumpPrds", False)
    kwargs.setdefault("dumpDigits", False)
    kwargs.setdefault("dumpSimHits", True)
    
    ### Overall simhit container dump protected by dumpSimHits property
    ### If property is set to true ensure that only the containers of the activated
    ### detectors are written
    kwargs.setdefault("dumpMdtSimHits", flags.Detector.GeometryMDT)
    kwargs.setdefault("dumpRpcSimHits", flags.Detector.GeometryRPC)
    kwargs.setdefault("dumpTgcSimHits", flags.Detector.GeometryTGC)
    kwargs.setdefault("dumpStgcSimHits",flags.Detector.GeometrysTGC)
    kwargs.setdefault("dumpMmSimHits", flags.Detector.GeometryMM)


    kwargs.setdefault("dumpMdtDigits", flags.Detector.GeometryMDT)
    kwargs.setdefault("dumpRpcDigits", flags.Detector.GeometryRPC)
    kwargs.setdefault("dumpTgcDigits", flags.Detector.GeometryTGC)
    kwargs.setdefault("dumpStgcDigits",flags.Detector.GeometrysTGC)
    kwargs.setdefault("dumpMmDigits", flags.Detector.GeometryMM)

    kwargs.setdefault("dumpMdtPrds", flags.Detector.GeometryMDT)
    kwargs.setdefault("dumpRpcPrds", flags.Detector.GeometryRPC)
    kwargs.setdefault("dumpTgcPrds", flags.Detector.GeometryTGC)
    kwargs.setdefault("dumpMmPrds", flags.Detector.GeometryMM)

    theAlg = CompFactory.MuonValR4.MuonHitTesterAlg(name, **kwargs)
    result.addEventAlgo(theAlg, primary = True)
    return result


def MuonDigiTestCfg(flags, name="MuonDigiTester", outFile="DigiTest.root", **kwargs):
    kwargs.setdefault("MdtSimHitKey", "MDT_SDO")
    kwargs.setdefault("RpcSimHitKey", "RPC_SDO")
    kwargs.setdefault("TgcSimHitKey", "TGC_SDO")
    kwargs.setdefault("MmSimHitKey", "MM_SDO")
    kwargs.setdefault("sTgcSimHitKey", "sTGC_SDO")
    kwargs.setdefault("dumpDigits", True)
    return MuonHitTesterCfg(flags, name = name, outFile = outFile, **kwargs)

def MuonPileUpTestCfg(flags, name="MuonDigiTester", outFile="DigiTest.root", **kwargs):
    kwargs.setdefault("MdtSimHitKey", "Bkg_MDT_SDO")
    kwargs.setdefault("RpcSimHitKey", "Bkg_RPC_SDO")
    kwargs.setdefault("TgcSimHitKey", "Bkg_TGC_SDO")
    kwargs.setdefault("MmSimHitKey", "Bkg_MM_SDO")
    kwargs.setdefault("sTgcSimHitKey", "Bkg_sTGC_SDO")
    kwargs.setdefault("EvtInfoKey", "Bkg_EventInfo")
    kwargs.setdefault("dumpDigits", True)
    return MuonHitTesterCfg(flags, name = name, outFile = outFile, **kwargs)


if __name__=="__main__":
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser, MuonPhaseIITestDefaults
    from MuonConfig.MuonConfigUtils import executeTest
    parser = SetupArgParser()
    parser.add_argument("--runTester", help="Choice on the tester to setup", default="SIM", choices=["SIM", "DIGI", "PILEUP"])
    parser.set_defaults(nEvents = -1)
    parser.set_defaults(inputFile= MuonPhaseIITestDefaults.HITS_PG_R3)
    parser.set_defaults(outRootFile="SimHitDumpNtuple.root")

    args = parser.parse_args()
    flags, cfg = setupGeoR4TestCfg(args)
    
    from MuonConfig.MuonDataPrepConfig import xAODUncalibMeasPrepCfg
    cfg.merge(xAODUncalibMeasPrepCfg(flags))
    
    if args.runTester != "SIM":
        if flags.Detector.GeometryMDT:
            from MuonConfig.MuonByteStreamCnvTestConfig import MdtRdoToMdtDigitCfg
            cfg.merge(MdtRdoToMdtDigitCfg(flags))
        if flags.Detector.GeometryRPC:
            from MuonConfig.MuonByteStreamCnvTestConfig import RpcRdoToRpcDigitCfg
            cfg.merge(RpcRdoToRpcDigitCfg(flags))
        if flags.Detector.GeometryTGC:
            from MuonConfig.MuonByteStreamCnvTestConfig import TgcRdoToTgcDigitCfg
            cfg.merge(TgcRdoToTgcDigitCfg(flags))
        if flags.Detector.GeometrysTGC:
            from MuonConfig.MuonByteStreamCnvTestConfig import STGC_RdoToDigitCfg
            cfg.merge(STGC_RdoToDigitCfg(flags))
        if flags.Detector.GeometryMM:
            from MuonConfig.MuonByteStreamCnvTestConfig import MM_RdoToDigitCfg
            cfg.merge(MM_RdoToDigitCfg(flags))

    if args.runTester == "SIM":
        cfg.merge(MuonHitTesterCfg(flags,outFile=args.outRootFile))
    elif args.runTester == "DIGI": 
        cfg.merge(MuonDigiTestCfg(flags,outFile=args.outRootFile))
    elif args.runTester == "PILEUP":
        cfg.merge(MuonPileUpTestCfg(flags,outFile=args.outRootFile))

    executeTest(cfg)

