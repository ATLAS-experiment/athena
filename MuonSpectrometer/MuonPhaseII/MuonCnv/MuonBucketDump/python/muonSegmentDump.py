# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration



def main(args):
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg
    from MuonConfig.MuonConfigUtils import executeTest, setupHistSvcCfg
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.PerfMon.doFullMonMT = False

    flags, cfg = setupGeoR4TestCfg(args)

    cfg.merge(setupHistSvcCfg(flags,outFile=args.outRootFile,
                                    outStream="MuonSegmentDump"))

    from MuonConfig.MuonDataPrepConfig import xAODUncalibMeasPrepCfg
    cfg.merge(xAODUncalibMeasPrepCfg(flags))

    from MuonSpacePointFormation.SpacePointFormationConfig import MuonSpacePointFormationCfg 
    cfg.merge(MuonSpacePointFormationCfg(flags))

    from MuonPatternRecognitionAlgs.MuonPatternRecognitionConfig import MuonPatternRecognitionCfg
    cfg.merge(MuonPatternRecognitionCfg(flags))

    # Truth information if MC
    if flags.Input.isMC:
        from MuonTruthAlgsR4.MuonTruthAlgsConfig import MuonTruthAlgsCfg
        cfg.merge(MuonTruthAlgsCfg(flags))

    from MuonBucketDump.MuonBucketDumpConfig import MuonSegmentDumpCfg
    cfg.merge(MuonSegmentDumpCfg(flags))

    executeTest(cfg)

if __name__=="__main__":
    from MuonGeoModelTestR4.testGeoModel import SetupArgParser, MuonPhaseIITestDefaults
    parser = SetupArgParser()
    parser.set_defaults(nEvents = -1)
    parser.set_defaults(outRootFile="MuonSegmentDump_R3SimHits.root")

    parser.set_defaults(inputFile=MuonPhaseIITestDefaults.HITS_PG_R3)
    args = parser.parse_args()
    main(args)

    