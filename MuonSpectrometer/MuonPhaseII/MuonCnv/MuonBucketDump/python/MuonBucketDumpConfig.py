#Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MuonBucketDumpCfg(flags, name="MuonBucketDumper", **kwargs):
    result = ComponentAccumulator()
    from MuonSpacePointFormation.SpacePointFormationConfig import MuonSpacePointFormationCfg
    result.merge(MuonSpacePointFormationCfg(flags))
    kwargs.setdefault("isMC", flags.Input.isMC)
    from RngComps.RngCompsConfig import AthRNGSvcCfg
    acc = ComponentAccumulator()
    kwargs.setdefault("RndmSvc", acc.getPrimaryAndMerge(AthRNGSvcCfg(flags)))
    the_alg = CompFactory.MuonR4.BucketDumperAlg(name=name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result
