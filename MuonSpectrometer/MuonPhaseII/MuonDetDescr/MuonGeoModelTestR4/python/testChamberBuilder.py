# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
def MuonChamberToolTestCfg(flags, name="MuonChamberToolTest", **kwargs):
    result = ComponentAccumulator()
    from ActsConfig.ActsGeometryConfig import ActsTrackingGeometrySvcCfg
    kwargs.setdefault("TrackingGeometrySvc", result.getPrimaryAndMerge(ActsTrackingGeometrySvcCfg(flags)))
    the_alg = CompFactory.MuonGMR4.MuonChamberToolTest(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)    
    return result

if __name__=="__main__":
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser, executeTest
    parser = SetupArgParser()

    args = parser.parse_args()

     
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Acts.TrackingGeometry.UseBlueprint=True
    flags, cfg = setupGeoR4TestCfg(args,flags)    
    ###
    cfg.merge(MuonChamberToolTestCfg(flags))
    cfg.getService("MessageSvc").verboseLimit = 100000
    executeTest(cfg)


