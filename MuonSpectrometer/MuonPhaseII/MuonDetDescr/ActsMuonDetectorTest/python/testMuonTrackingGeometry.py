# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MuonTrackingGeometryTestCfg(flags, name = "ActsMuonTrackingGeometryTest", **kwargs):
  
    result = ComponentAccumulator()

    from xAODTruthCnv.xAODTruthCnvConfig import GEN_AOD2xAODCfg
    result.merge(GEN_AOD2xAODCfg(flags))

    from MuonTruthAlgsR4.MuonTruthAlgsConfig import MuonTruthAlgsCfg
    result.merge(MuonTruthAlgsCfg(flags, useSDO = False))

    result.getEventAlgo("MuonTruthSegmentMaker").useOnlyMuonHits = False
    result.getEventAlgo("TruthMuonMakerAlg").pdgIds=[13,998,999]

    from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
   
    kwargs.setdefault("TrackingGeometryTool", result.getPrimaryAndMerge(ActsTrackingGeometryToolCfg(flags)))

    the_alg = CompFactory.ActsTrk.ActsMuonTrackingGeometryTest(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)

    return result

if __name__ == "__main__":

    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser, executeTest, MuonPhaseIITestDefaults
    from MuonConfig.MuonConfigUtils import setupHistSvcCfg
    parser = SetupArgParser()
    parser.set_defaults(outRootFile="MuonNavigationTestR4_Gen3Geometry.root")
    parser.set_defaults(inputFile=MuonPhaseIITestDefaults.HITS_PG_R4)  
    parser.set_defaults(nEvents=10)
    parser.set_defaults(defaultGeoFile="RUN4")
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
    cfg.merge(AtlasFieldCacheCondAlgCfg(flags))
    cfg.merge(MuonTrackingGeometryTestCfg(flags))

    executeTest(cfg)