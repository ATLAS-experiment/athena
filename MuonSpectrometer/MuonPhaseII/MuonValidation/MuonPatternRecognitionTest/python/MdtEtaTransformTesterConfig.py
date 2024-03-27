# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def MdtEtaTransformTesterCfg(flags, name = "MdtEtaTransformTester", **kwargs):
    result = ComponentAccumulator()
    theAlg = CompFactory.MuonValR4.MdtEtaTransformTester(name, **kwargs)
    result.addEventAlgo(theAlg, primary=True)
    return result

if __name__=="__main__":
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser, executeTest,setupHistSvcCfg
    parser = SetupArgParser()
    parser.set_defaults(nEvents = -1)
    parser.set_defaults(inputFile=["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonRecRTT/R4SimHits.pool.root"])

    args = parser.parse_args()
    flags, cfg = setupGeoR4TestCfg(args)
    
    from xAODMuonSimHitCnv.MuonSimHitCnvCfg import MuonSimHitToMeasurementCfg
    from MuonPatternRecognitionAlgs.MuonHoughTransformAlgConfig import MuonHoughTransformAlgCfg
    from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg
    # from PerfMonVTune.PerfMonVTuneConfig import VTuneProfilerServiceCfg
    cfg.merge(setupHistSvcCfg(flags,out_file=args.outRootFile,out_stream="MuonEtaHoughTransformTest"))
    cfg.merge(MuonSimHitToMeasurementCfg(flags))
    from MuonSpacePointFormation.SpacePointFormationConfig import MuonSpacePointMakerAlgCfg 
    cfg.merge(MuonSpacePointMakerAlgCfg(flags))
    cfg.merge(MuonHoughTransformAlgCfg(flags))
    cfg.merge(MdtEtaTransformTesterCfg(flags))
    cfg.merge(PerfMonMTSvcCfg(flags))
    # cfg.merge(VTuneProfilerServiceCfg(flags, ProfiledAlgs=["MuonHoughTransformAlg"]))

    # output spam reduction
    cfg.getService("AthenaHiveEventLoopMgr").EventPrintoutInterval=500


    executeTest(cfg, args.nEvents)
    