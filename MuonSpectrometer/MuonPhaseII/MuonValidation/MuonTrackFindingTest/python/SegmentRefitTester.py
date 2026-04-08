# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
def SegmentRefitTestCfg(flags,name="SegmentRefitter", 
                        drawEvent=False, smearSegPars=False, **kwargs):
    result = ComponentAccumulator()
    from MuonPatternRecognitionAlgs.MuonPatternRecognitionConfig import ActsMuonSegmentRefitAlgCfg

    result.merge(ActsMuonSegmentRefitAlgCfg(flags, drawEvent=drawEvent, smearSegPars = smearSegPars))
    the_alg = CompFactory.MuonValR4.SegmentRefitTest(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def SegmentExtpTestCfg(falgs, name="SegmentExtrapolationTest", **kwargs):
    result = ComponentAccumulator()
    from ActsConfig.ActsGeometryConfig import ActsExtrapolationToolCfg
    kwargs.setdefault("ExtrapolationTool", 
            result.popToolsAndMerge(ActsExtrapolationToolCfg(flags,
                                                             FieldMode="StraightLine")))
    the_alg = CompFactory.MuonValR4.SegmentExtpTest(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

if __name__=="__main__":
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser, MuonPhaseIITestDefaults
    from MuonConfig.MuonConfigUtils import executeTest, setupHistSvcCfg
    parser = SetupArgParser()
    parser.add_argument("--noMonitorPlots", help="If set to true, there're no monitoring plots", 
                        default = False, action='store_true')
    parser.add_argument("--dumpObjFiles", help="If set to true, the spacepoints in the bucket are saved to disk",
                        default=False, action='store_true')
    parser.add_argument("--noPerfMon", help="If set to true, disable performance monitoring.",
                        default=False, action='store_true')
    parser.add_argument("--smearSegPars", help="If set to true the segment parameters are smeared",
                        default=False, action='store_true')
    parser.set_defaults(nEvents = -1)
  
    parser.set_defaults(outRootFile="MsTrkTester.root")
    parser.set_defaults(inputFile=MuonPhaseIITestDefaults.HITS_PG_R3)
   
    args = parser.parse_args()
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.PerfMon.doFullMonMT = not args.noPerfMon
    ####
    flags, cfg = setupGeoR4TestCfg(args,flags)
    cfg.getService("MessageSvc").setVerbose = ["ActsMuonSegmentRefitAlg", "SegmentExtrapolationTest"]
    # cfg.getService("MessageSvc").setDebug = ["SegmentExtrapolationTest"]
   
    # cfg.getService("MessageSvc").setVerbose = []
    cfg.getService("MessageSvc").setDebug = []
    
    cfg.merge(setupHistSvcCfg(flags,outFile=args.outRootFile,
                                    outStream="SegmentRefitTest"))


    from MuonConfig.MuonDataPrepConfig import xAODUncalibMeasPrepCfg
    cfg.merge(xAODUncalibMeasPrepCfg(flags))
    
    from MuonSpacePointFormation.SpacePointFormationConfig import MuonSpacePointFormationCfg 
    cfg.merge(MuonSpacePointFormationCfg(flags))

    from MuonPatternRecognitionAlgs.MuonPatternRecognitionConfig import MuonPatternRecognitionCfg
    cfg.merge(MuonPatternRecognitionCfg(flags))
    cfg.merge(SegmentRefitTestCfg(flags, drawEvent = args.dumpObjFiles,
                                         smearSegPars = args.smearSegPars))
    cfg.merge(SegmentExtpTestCfg(flags, drawEvent = args.dumpObjFiles))
   
    from MuonPatternRecognitionTest.PatternTestConfig import PatternVisualizationToolCfg

    cfg.getEventAlgo("MuonSegmentFittingAlg").VisualizationTool = cfg.popToolsAndMerge(PatternVisualizationToolCfg(flags, 
                                                                                        CanvasPreFix="SegmentPlotValid", outSubDir="SegmentValidPlots", 
                                                                                        displayTruthOnly = False, saveSinglePDFs = True, saveSummaryPDF= True))

    cfg.getEventAlgo("MuonR4xAODSegmentCnvAlg").convertBeamSpot = True
    executeTest(cfg)
