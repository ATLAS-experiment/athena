# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def MuonHoughTransformAlgCfg(flags, name = "MuonHoughTransformAlg", **kwargs):
    result = ComponentAccumulator()
    theAlg = CompFactory.MuonR4.MuonHoughTransformAlg(name, **kwargs)
    result.addEventAlgo(theAlg, primary=True)
    return result

if __name__=="__main__":
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser, executeTest,setupHistSvcCfg
    parser = SetupArgParser()
    parser.set_defaults(nEvents = -1)
    parser.set_defaults(inputFile=["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonRecRTT/R4SimHits.pool.root"])

    args = parser.parse_args()
    flags, cfg = setupGeoR4TestCfg(args)
    
    from xAODMuonSimHitCnv.MuonSimHitCnvCfg import xAODSimHitToMdtMeasCnvAlgCfg
    cfg.merge(setupHistSvcCfg(flags,out_file=args.outRootFile,out_stream="MuonHoughTransform"))
    cfg.merge(xAODSimHitToMdtMeasCnvAlgCfg(flags))
    from MuonSpacePointFormation.SpacePointFormationConfig import MuonSpacePointMakerAlgCfg
    cfg.merge(MuonSpacePointMakerAlgCfg(flags))
    cfg.merge(MuonHoughTransformAlgCfg(flags))

    # output spam reduction
    cfg.getService("AthenaHiveEventLoopMgr").EventPrintoutInterval=500


    executeTest(cfg, args.nEvents)
    
