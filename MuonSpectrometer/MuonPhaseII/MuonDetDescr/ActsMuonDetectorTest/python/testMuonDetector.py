# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Constants import VERBOSE

def MuonDetectorNavTestCfg(flags, name = "MuonDetectorNavTest", **kwargs):
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
    kwargs.setdefault("SimHitKeys", containerNames)

    the_alg = CompFactory.ActsTrk.MuonDetectorNavTest(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

if __name__=="__main__":
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser, executeTest,setupHistSvcCfg
    parser = SetupArgParser()
    parser.set_defaults(inputFile=["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/MuonGeomRTT/R3SimHits.pool.root"])
    parser.set_defaults(outRootFile="MuonNavigationTestR4.root")
    parser.set_defaults(nEvents=10)
    parser.add_argument("--dumpDetector", help="Save dump detector visualization", action='store_true', default=False )
    parser.add_argument("--dumpPassive", help="Save  detector visualization", action='store_true', default=False )
    parser.add_argument("--dumpDetectorVolumes", help="Save detector visualization", action='store_true', default=False )
    parser.add_argument("--noSensitives", help="Do not use sensitive detectors", action='store_true', default=False )
    parser.add_argument("--dumpMaterialSurfaces", help="Save material surfaces visualization", action='store_true', default=False )


    args = parser.parse_args()
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.PerfMon.doFullMonMT = True
    flags, cfg = setupGeoR4TestCfg(args,flags)

    cfg.merge(setupHistSvcCfg(flags, outFile=args.outRootFile, outStream="MuonNavigationTestR4"))
    from ActsGeometry.DetectorVolumeSvcCfg import DetectorVolumeSvcCfg
    cfg.merge(DetectorVolumeSvcCfg(flags))
    from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg
    cfg.merge(AtlasFieldCacheCondAlgCfg(flags))
    cfg.merge(MuonDetectorNavTestCfg(flags, OutputLevel=VERBOSE))
    cfg.getPublicTool("MuonDetectorBuilderTool").dumpDetector = args.dumpDetector
    cfg.getPublicTool("MuonDetectorBuilderTool").dumpPassive = args.dumpPassive
    cfg.getPublicTool("MuonDetectorBuilderTool").dumpDetectorVolumes = args.dumpDetectorVolumes
    cfg.getPublicTool("MuonDetectorBuilderTool").BuildSensitives = not args.noSensitives
    cfg.getPublicTool("MuonDetectorBuilderTool").dumpMaterialSurfaces = args.dumpMaterialSurfaces
    executeTest(cfg)
