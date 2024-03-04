# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
def MuonChamberToolTestCfg(flags, name="MuonChamberToolTest", **kwargs):
    result = ComponentAccumulator()
    from ActsGeometry.DetectorVolumeSvcCfg import DetectorVolumeSvcCfg
    kwargs.setdefault("DetectorVolumeSvc", result.getPrimaryAndMerge(DetectorVolumeSvcCfg(flags)))
    the_alg = CompFactory.MuonGMR4.MuonChamberToolTest(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)    
    return result

if __name__=="__main__":
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser, executeTest
    parser = SetupArgParser()

    args = parser.parse_args()
    flags, cfg = setupGeoR4TestCfg(args)    
    ###
    cfg.merge(MuonChamberToolTestCfg(flags))
    executeTest(cfg, num_events = args.nEvents)


