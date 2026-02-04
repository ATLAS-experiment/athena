#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

def setupArgParser():
    from argparse import ArgumentParser
    from AthenaConfiguration.TestDefaults import defaultConditionsTags, defaultGeometryTags, defaultTestFiles
    parser = ArgumentParser()
    parser.add_argument("--inputFile", "-i", default= defaultTestFiles.EVNT, 
                        help="Input file to run on ", nargs="+")
    parser.add_argument("--geoModelFile", default = "", help="GeoModel SqLite file containing the muon geometry.")
    parser.add_argument("--outRootFile", default="NewGeoModelDump.root", help="Output ROOT file to dump the geomerty")
    parser.add_argument("--condTag", default=defaultConditionsTags.RUN3_MC, help="Conditions tag to use",
                                                                            choices=[defaultConditionsTags.RUN3_MC,
                                                                                     defaultConditionsTags.RUN3_DATA,
                                                                                     defaultConditionsTags.RUN4_MC ])
    parser.add_argument("--geoTag", default=defaultGeometryTags.RUN3, help="Geometry tag to use", choices=[defaultGeometryTags.RUN4,
                                                                                                           defaultGeometryTags.RUN3])
    return parser

if __name__ == "__main__":
    
    args = setupArgParser().parse_args()
    
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Concurrency.NumThreads = 1
    flags.Concurrency.NumConcurrentEvents = 1  # Might change this later, but good enough for the moment.
    flags.Input.Files = args.inputFile 
    flags.GeoModel.AtlasVersion = args.geoTag
    flags.IOVDb.GlobalTag = args.condTag
    flags.Scheduler.ShowDataDeps = True 
    flags.Scheduler.ShowDataFlow = True
    flags.Exec.MaxEvents = 1

    if len(args.geoModelFile) > 0:
        flags.GeoModel.SQLiteDBFullPath = args.geoModelFile
        flags.GeoModel.SQLiteDB = True
        from MuonGeoModelTestR4.testGeoModel import configureDefaultTagsCfg
        configureDefaultTagsCfg(flags)

    flags.lock()
    flags.dump(evaluate = True)


    
    from MuonConfig.MuonConfigUtils import executeTest, SetupMuonStandaloneCA
    cfg = SetupMuonStandaloneCA(flags)

    cfg.getService("MessageSvc").verboseLimit = 10000000
    cfg.getService("MessageSvc").debugLimit = 10000000

    from ActsConfig.ActsAnalysisConfig import ActsGeoDumpCfg
    cfg.merge(ActsGeoDumpCfg(flags, outFile = args.outRootFile))

    executeTest(cfg)



