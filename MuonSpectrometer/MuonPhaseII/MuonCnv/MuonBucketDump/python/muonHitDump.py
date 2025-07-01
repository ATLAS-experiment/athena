# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration



def main(args):
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, executeTest, setupHistSvcCfg
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.PerfMon.doFullMonMT = True

    flags, cfg = setupGeoR4TestCfg(args)

    cfg.merge(setupHistSvcCfg(flags,outFile=args.outRootFile,
                                    outStream="MuonHitDump"))

    from MuonConfig.MuonDataPrepConfig import xAODUncalibMeasPrepCfg
    cfg.merge(xAODUncalibMeasPrepCfg(flags))

    from MuonSpacePointFormation.SpacePointFormationConfig import MuonSpacePointFormationCfg 
    cfg.merge(MuonSpacePointFormationCfg(flags))

    from MuonTruthAlgsR4.MuonTruthAlgsConfig import MuonTruthAlgsCfg
    cfg.merge(MuonTruthAlgsCfg(flags))
    from MuonBucketDump.MuonBucketDumpConfig import MuonHitDumperCfg
    cfg.merge(MuonHitDumperCfg(flags))
    
    executeTest(cfg)

if __name__=="__main__":
    from MuonGeoModelTestR4.testGeoModel import SetupArgParser
    parser = SetupArgParser()
    parser.set_defaults(nEvents = -1)
    parser.set_defaults(outRootFile="MuonBucketDump_R3SimHits.root")
    parser.set_defaults(inputFile=[
                                   "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonGeomRTT/R3SimHits.pool.root"
                                    ])
    args = parser.parse_args()
    main(args)

    