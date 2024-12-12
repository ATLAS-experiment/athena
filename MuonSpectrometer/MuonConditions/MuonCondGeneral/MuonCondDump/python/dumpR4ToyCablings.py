# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def RpcToyCablingJsonDumpAlgCfg(flags, name="RpcToyCablingJsonDumpAlg", **kwargs):
    result = ComponentAccumulator()
    the_alg = CompFactory.Muon.RpcToyCablingJsonDumpAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def MdtToyCablingJsonDumpAlgCfg(flags, name="MdtToyCablingJsonDumpAlg", **kwargs):
    result = ComponentAccumulator()
    the_alg = CompFactory.MdtToyCablingJsonDumpAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result
if __name__=="__main__":
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser, executeTest, geoModelFileDefault
    parser = SetupArgParser()
    parser.set_defaults(nEvents = 1)
    parser.set_defaults(noMM=True)
    parser.set_defaults(noSTGC=True)
    parser.set_defaults(noTgc=True)
    parser.add_argument("--setupRun4", default=False, action="store_true")

    args = parser.parse_args()
    args.geoModelFile = geoModelFileDefault(args.setupRun4)
    flags, cfg = setupGeoR4TestCfg(args)
    
    cfg.merge(RpcToyCablingJsonDumpAlgCfg(flags))
    cfg.merge(MdtToyCablingJsonDumpAlgCfg(flags))
    executeTest(cfg)
   