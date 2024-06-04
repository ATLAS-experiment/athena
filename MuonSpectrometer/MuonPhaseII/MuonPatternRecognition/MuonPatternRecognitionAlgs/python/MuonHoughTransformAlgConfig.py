# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def MuonPhiHoughTransformAlgCfg(flags, name = "MuonPhiHoughTransformAlg", **kwargs):
    result = ComponentAccumulator()
    theAlg = CompFactory.MuonR4.MuonPhiHoughTransformAlg(name, **kwargs)
    result.addEventAlgo(theAlg, primary=True)
    return result

def MuonEtaHoughTransformAlgCfg(flags, name = "MuonEtaHoughTransformAlg", **kwargs):
    result = ComponentAccumulator()
    theAlg = CompFactory.MuonR4.MuonEtaHoughTransformAlg(name, **kwargs)
    result.addEventAlgo(theAlg, primary=True)
    return result

def MuonPatternRecognitionCfg(flags):
    result = ComponentAccumulator()
    from ActsAlignmentAlgs.AlignmentAlgsConfig import ActsGeometryContextAlgCfg
    result.merge(ActsGeometryContextAlgCfg(flags))
    result.merge(MuonEtaHoughTransformAlgCfg(flags))
    result.merge(MuonPhiHoughTransformAlgCfg(flags))
    return result


if __name__=="__main__":
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser, executeTest,setupHistSvcCfg
    parser = SetupArgParser()
    parser.set_defaults(nEvents = -1)
    parser.set_defaults(inputFile=["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonRecRTT/R4SimHits.pool.root"])
    parser.set_defaults(eventPrintoutLevel = 500)

    args = parser.parse_args()
    flags, cfg = setupGeoR4TestCfg(args)
    
    from xAODMuonSimHitCnv.MuonSimHitCnvCfg import MuonSimHitToMeasurementCfg
    cfg.merge(setupHistSvcCfg(flags,out_file=args.outRootFile,out_stream="MuonHoughTransform"))
    cfg.merge(MuonSimHitToMeasurementCfg(flags))
    from MuonSpacePointFormation.SpacePointFormationConfig import MuonSpacePointFormationCfg
    cfg.merge(MuonSpacePointFormationCfg(flags))
    cfg.merge(MuonPatternRecognitionCfg(flags))

    executeTest(cfg, args.nEvents)
