# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MuonHitTesterCfg(flags, name="MuonHitTester", outFile="SimHitTest.root", **kwargs):
    result = ComponentAccumulator()
    from MuonGeoModelTestR4.testGeoModel import setupHistSvcCfg
    cfg.merge(setupHistSvcCfg(flags,out_file=outFile, out_stream="MuonR4HitTest"))
    kwargs.setdefault("isMC", flags.Input.isMC)


    kwargs.setdefault("dumpMdtSimHits", flags.Detector.GeometryMDT)
    kwargs.setdefault("dumpRpcSimHits", flags.Detector.GeometryRPC)
    kwargs.setdefault("dumpTgcSimHits", flags.Detector.GeometryTGC)
    kwargs.setdefault("dumpStgcSimHits",flags.Detector.GeometrysTGC)
    kwargs.setdefault("dumpMmSimHits", flags.Detector.GeometryMM)

    theAlg = CompFactory.MuonValR4.MuonHitTesterAlg(name, **kwargs)
    result.addEventAlgo(theAlg, primary = True)
    return result

if __name__=="__main__":
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser, executeTest
    parser = SetupArgParser()
    parser.set_defaults(nEvents = -1)
    parser.set_defaults(inputFile=["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonRecRTT/R4SimHits.pool.root"])
    parser.set_defaults(eventPrintoutLevel = 500)

    args = parser.parse_args()
    flags, cfg = setupGeoR4TestCfg(args)
    cfg.merge(MuonHitTesterCfg(flags,outFile=args.outRootFile))
    executeTest(cfg, args.nEvents)
