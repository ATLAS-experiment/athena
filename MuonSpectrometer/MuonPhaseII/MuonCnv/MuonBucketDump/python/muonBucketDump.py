# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration


def main(args):
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg
    from MuonConfig.MuonConfigUtils import executeTest, setupHistSvcCfg
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.PerfMon.doFullMonMT = True

    flags, cfg = setupGeoR4TestCfg(args)

    cfg.merge(setupHistSvcCfg(flags,outFile=args.outRootFile,
                                    outStream="MuonBucketDump"))

    from MuonConfig.MuonDataPrepConfig import xAODUncalibMeasPrepCfg
    cfg.merge(xAODUncalibMeasPrepCfg(flags))

    from MuonSpacePointFormation.SpacePointFormationConfig import MuonSpacePointFormationCfg 
    cfg.merge(MuonSpacePointFormationCfg(flags))

    from MuonPatternRecognitionAlgs.MuonPatternRecognitionConfig import MuonPatternRecognitionCfg
    cfg.merge(MuonPatternRecognitionCfg(flags))


    from MuonBucketDump.MuonBucketDumpConfig import MuonBucketDumpCfg
    from MuonPatternRecognitionTest.PatternTestConfig import PatternVisualizationToolCfg
    cfg.merge(MuonBucketDumpCfg(flags,
                                VisualizationTool = cfg.popToolsAndMerge(PatternVisualizationToolCfg(flags, CanvasLimits =0))))

    executeTest(cfg)

if __name__=="__main__":
    from MuonGeoModelTestR4.testGeoModel import SetupArgParser, MuonPhaseIITestDefaults
    parser = SetupArgParser()
    parser.set_defaults(nEvents = -1)
    parser.set_defaults(outRootFile="MuonBucketDump_R3SimHits.root")

    parser.set_defaults(inputFile=MuonPhaseIITestDefaults.HITS_PG_R3)
    args = parser.parse_args()
    main(args)

    