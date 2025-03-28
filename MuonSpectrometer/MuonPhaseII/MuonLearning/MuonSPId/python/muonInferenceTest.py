# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration


#print ("Stonjek jetzt sag mal ob du den file findest")
if __name__ == "__main__":
    from MuonGeoModelTestR4.testGeoModel import SetupArgParser
    parser = SetupArgParser()
    parser.set_defaults(nEvents = -1)
    parser.set_defaults(outRootFile="InferenceHoughTest.root")
    parser.set_defaults(noMM=True)
    parser.set_defaults(noSTGC=True)
    parser.set_defaults(inputFile=[
                                    "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonGeomRTT/R3SimHits.pool.root"
                                    ])
    args = parser.parse_args()
    #print ( "Stonjek du sollst ein script ausfueheren")
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, executeTest, setupHistSvcCfg
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.PerfMon.doFullMonMT = True

    flags, cfg = setupGeoR4TestCfg(args,flags)

    from MuonConfig.MuonDataPrepConfig import xAODUncalibMeasPrepCfg
    cfg.merge(xAODUncalibMeasPrepCfg(flags))

    from MuonSpacePointFormation.SpacePointFormationConfig import MuonSpacePointFormationCfg 
    cfg.merge(MuonSpacePointFormationCfg(flags))
    from MuonInference.InferenceConfig import GraphBucketFilterToolCfg, GraphInferenceAlgCfg
    cfg.merge(GraphInferenceAlgCfg(flags,InferenceTools = [cfg.popToolsAndMerge(GraphBucketFilterToolCfg(flags))]))
    
    from MuonPatternRecognitionAlgs.MuonHoughTransformAlgConfig import MuonPatternRecognitionCfg, MuonSegmentFittingAlgCfg
    cfg.merge(MuonPatternRecognitionCfg(flags))
    cfg.getEventAlgo("MuonEtaHoughTransformAlg").SpacePointContainer = "FilteredMlSpacePoints"
    cfg.merge(MuonSegmentFittingAlgCfg(flags))
    
    cfg.merge(setupHistSvcCfg(flags,outFile=args.outRootFile,
                                    outStream="MuonEtaHoughTransformTest"))

    from MuonPatternRecognitionTest.PatternTestConfig import MuonHoughTransformTesterCfg, PatternVisualizationToolCfg
    
    cfg.merge(MuonHoughTransformTesterCfg(flags,
                                          VisualizationTool = cfg.popToolsAndMerge(PatternVisualizationToolCfg(flags, CanvasLimits =0))))
    
    executeTest(cfg)
    