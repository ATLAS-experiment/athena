# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def RpcToyCablingJsonDumpAlgCfg(flags, name="RpcToyCablingJsonDumpAlg", **kwargs):
    result = ComponentAccumulator()
    the_alg = CompFactory.RpcToyCablingJsonDumpAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

if __name__=="__main__":
    from MuonGeoModelTestR4.testGeoModel import setupGeoR4TestCfg, SetupArgParser, executeTest
    parser = SetupArgParser()
    parser.set_defaults(nEvents = 1)
    parser.set_defaults(noMM=True)
    parser.set_defaults(noSTGC=True)
    parser.set_defaults(noTgc=True)
    parser.set_defaults(noMdt=True)

    args = parser.parse_args()
    flags, cfg = setupGeoR4TestCfg(args)
    
    cfg.merge(RpcToyCablingJsonDumpAlgCfg(flags))
    executeTest(cfg, args.nEvents)
   