# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MuonSPIdDumpCfg(flags, name="MuonSPIdMaker", **kwargs):
    result = ComponentAccumulator()
    from MuonSpacePointFormation.SpacePointFormationConfig import MuonSpacePointFormationCfg
    result.merge(MuonSpacePointFormationCfg(flags))
    kwargs.setdefault("isMC", flags.Input.isMC)
    from MuonInference.InferenceConfig import GraphBucketFilterToolCfg
    kwargs.setdefault("GraphFilterTool", result.popToolsAndMerge(GraphBucketFilterToolCfg(flags)))
    the_alg = CompFactory.MuonR4.SPIdDumperAlg(name=name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result


def main(args):
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg
    from MuonConfig.MuonConfigUtils import executeTest, setupHistSvcCfg
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.PerfMon.doFullMonMT = True

    flags, cfg = setupGeoR4TestCfg(args,flags)

    cfg.merge(setupHistSvcCfg(flags,outFile=args.outRootFile, outStream="MuonSPId"))

    from MuonConfig.MuonDataPrepConfig import xAODUncalibMeasPrepCfg
    cfg.merge(xAODUncalibMeasPrepCfg(flags))

    from MuonSpacePointFormation.SpacePointFormationConfig import MuonSpacePointFormationCfg 
    cfg.merge(MuonSpacePointFormationCfg(flags))

    from MuonPatternRecognitionAlgs.MuonHoughTransformAlgConfig import MuonPatternRecognitionCfg, MuonSegmentFittingAlgCfg
    cfg.merge(MuonPatternRecognitionCfg(flags))

    cfg.merge(MuonSegmentFittingAlgCfg(flags))

    cfg.merge(MuonSPIdDumpCfg(flags))
    #cfg.getService("MessageSvc").setVerbose= [ "MuonSPIdMaker"]
    executeTest(cfg)


if __name__=="__main__":
    from MuonGeoModelTestR4.testGeoModel import SetupArgParser, MuonPhaseIITestDefaults
    parser = SetupArgParser()
    parser.set_defaults(nEvents = -1)
    parser.set_defaults(outRootFile="MuonSPId_R3SimHits.root")
    parser.set_defaults(inputFile=MuonPhaseIITestDefaults.HITS_PG_R3)
    args = parser.parse_args()
    main(args)

