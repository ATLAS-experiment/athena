#Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MuonBucketDumpCfg(flags, name="MuonBucketDumper", **kwargs):
    result = ComponentAccumulator()
    from MuonSpacePointFormation.SpacePointFormationConfig import MuonSpacePointFormationCfg
    result.merge(MuonSpacePointFormationCfg(flags))
    kwargs.setdefault("isMC", flags.Input.isMC)
    from RngComps.RngCompsConfig import AthRNGSvcCfg
    kwargs.setdefault("RndmSvc", result.getPrimaryAndMerge(AthRNGSvcCfg(flags)))
    spCont = []
    if flags.Detector.GeometryMDT or flags.Detector.GeometryRPC or flags.Detector.GeometryTGC:
        spCont+=["MuonSpacePoints"]
    if flags.Detector.GeometryMM or flags.Detector.GeometrysTGC:
        spCont+=["NswSpacePoints"]
    
    kwargs.setdefault("SpacePointKeys", spCont)

    
    the_alg = CompFactory.MuonR4.BucketDumperAlg(name=name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result
