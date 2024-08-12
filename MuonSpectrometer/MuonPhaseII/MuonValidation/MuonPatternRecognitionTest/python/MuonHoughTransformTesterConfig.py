# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def MuonHoughTransformTesterCfg(flags, name = "MuonHoughTransformTester", **kwargs):
    result = ComponentAccumulator()
    containerNames = []
    ## If the tester runs on MC add the truth information
    if flags.Input.isMC:
        if flags.Detector.EnableMDT: containerNames+=["xMdtSimHits"]
        if flags.Detector.EnableMM: containerNames+=["xMmSimHits"]
        if flags.Detector.EnableRPC: containerNames+=["xRpcSimHits"]
        if flags.Detector.EnableTGC: containerNames+=["xTgcSimHits"]
        if flags.Detector.EnablesTGC: containerNames+=["xStgcSimHits"] 
    kwargs.setdefault("SimHitKeys", containerNames)
    theAlg = CompFactory.MuonValR4.MuonHoughTransformTester(name, **kwargs)    

    result.addEventAlgo(theAlg, primary=True)
    return result

if __name__=="__main__":
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser, executeTest,setupHistSvcCfg
    parser = SetupArgParser()
    parser.add_argument("--skipSegmentFit", 
                        help="Skip the segment fit step", action='store_true', default = False)
    parser.set_defaults(nEvents = -1)
    parser.set_defaults(noMM=True)
    parser.set_defaults(noSTGC=True)
    parser.set_defaults(outRootFile="HoughTransformTester.root")
    #parser.set_defaults(condTag="CONDBR2-BLKPA-2023-02")
    parser.set_defaults(inputFile=[
                                    #"/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/TCT_Run3/data22_13p6TeV.00431493.physics_Main.daq.RAW._lb0525._SFO-16._0001.data"
                                    "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonRecRTT/R4SimHits.pool.root"
                                    ])
    parser.set_defaults(eventPrintoutLevel = 500)
   
    args = parser.parse_args()
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.PerfMon.doFullMonMT = True
    flags, cfg = setupGeoR4TestCfg(args,flags)
    

    # from PerfMonVTune.PerfMonVTuneConfig import VTuneProfilerServiceCfg
    cfg.merge(setupHistSvcCfg(flags,outFile=args.outRootFile,
                                    outStream="MuonEtaHoughTransformTest"))

    if flags.Input.isMC:
        from MuonConfig.MuonSimHitCnvCfg import MuonSimHitToMeasurementCfg
        cfg.merge(MuonSimHitToMeasurementCfg(flags))
        from MuonTruthAlgsR4.MuonTruthAlgsConfig import TruthSegmentMakerCfg, TruthHitAssociationCfg
        cfg.merge(TruthSegmentMakerCfg(flags))
        cfg.merge(TruthHitAssociationCfg(flags))
    else:
        from MuonConfig.MuonBytestreamDecodeConfig import MuonByteStreamDecodersCfg
        cfg.merge(MuonByteStreamDecodersCfg(flags))
        from MuonConfig.MuonRdoDecodeConfig import MuonRDOtoPRDConvertorsCfg
        cfg.merge(MuonRDOtoPRDConvertorsCfg(flags))
    from MuonSpacePointFormation.SpacePointFormationConfig import MuonSpacePointFormationCfg 
    cfg.merge(MuonSpacePointFormationCfg(flags))

    from MuonPatternRecognitionAlgs.MuonHoughTransformAlgConfig import MuonPatternRecognitionCfg, MuonSegmentFittingAlgCfg
    cfg.merge(MuonPatternRecognitionCfg(flags))

    testerArgs = {}
    if args.skipSegmentFit: 
        testerArgs["SegmentKey"] = ""
    else: 
        cfg.merge(MuonSegmentFittingAlgCfg(flags))
    cfg.merge(MuonHoughTransformTesterCfg(flags, **testerArgs))
    
    
    executeTest(cfg)
    
