# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MuonTrackingGeometryTestCfg(flags, name = "ActsMuonTrackingGeometryTest", **kwargs):
  
    result = ComponentAccumulator()
    containerNames = []
    if flags.Detector.EnableMDT:
        containerNames+=["xMdtSimHits"]
    if flags.Detector.EnableMM:
        containerNames+=["xMmSimHits"]
    if flags.Detector.EnableRPC:
        containerNames+=["xRpcSimHits"]
    if flags.Detector.EnableTGC:
        containerNames+=["xTgcSimHits"]
    if flags.Detector.EnablesTGC:
        containerNames+=["xStgcSimHits"]
    
    from MuonTruthAlgsR4.MuonTruthAlgsConfig import TruthSegmentMakerCfg, TruthSegmentToTruthPartAssocCfg, SdoMultiTruthMakerCfg
    from MuonConfig.MuonTruthAlgsConfig import TruthMuonMakerAlgCfg, MuonTruthHitCountsAlgCfg
    from xAODTruthCnv.xAODTruthCnvConfig import GEN_AOD2xAODCfg
    result.merge(GEN_AOD2xAODCfg(flags))

    result.merge(TruthMuonMakerAlgCfg(flags, pdgIds=[13,998,999]))
    result.merge(MuonTruthHitCountsAlgCfg(flags))
    result.merge(TruthSegmentToTruthPartAssocCfg(flags))
    result.merge(SdoMultiTruthMakerCfg(flags))

    result.merge(TruthSegmentMakerCfg(flags, SimHitKeys=containerNames, useOnlyMuonHits = False))

    from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
   
    kwargs.setdefault("TrackingGeometryTool", result.getPrimaryAndMerge(ActsTrackingGeometryToolCfg(flags)))

    the_alg = CompFactory.ActsTrk.ActsMuonTrackingGeometryTest(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)

    return result

if __name__ == "__main__":

    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser, executeTest
    from MuonConfig.MuonConfigUtils import setupHistSvcCfg
    parser = SetupArgParser()
    parser.set_defaults(outRootFile="MuonNavigationTestR4_Gen3Geometry.root")
    parser.set_defaults(inputFile=["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonGeomRTT/R4SimHits.pool.root"])  
    parser.set_defaults(nEvents=10)
    parser.set_defaults(geoModelFile="/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonGeomRTT/GeoDB/ATLAS-P2-RUN4-01-00-00_MSOnly.db")
    parser.add_argument("--gen3", action="store_true", help="Use Gen3 geometry + construction")
    parser.add_argument("--objout", action="store_true", help="Obj output of the tracking geometry")
    args = parser.parse_args()

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()


    flags.PerfMon.doFullMonMT = True
    flags.Acts.TrackingGeometry.UseBlueprint = args.gen3
    flags.Acts.TrackingGeometry.ObjDebugOutput = args.objout

    flags, cfg = setupGeoR4TestCfg(args,flags)
    cfg.merge(setupHistSvcCfg(flags, outFile=args.outRootFile, outStream="MuonNavigationTestGen3R4"))
    from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg
    from AthenaCommon.Constants import INFO
    cfg.merge(AtlasFieldCacheCondAlgCfg(flags))
    cfg.merge(MuonTrackingGeometryTestCfg(flags, OutputLevel=INFO))

    executeTest(cfg)