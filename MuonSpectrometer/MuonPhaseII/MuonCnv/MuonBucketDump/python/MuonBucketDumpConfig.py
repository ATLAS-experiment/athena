#Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def MuonHitDumperCfg(flags, name="MuonHitDumper", **kwargs):
    result = ComponentAccumulator()
    spCont = []
    if flags.Detector.GeometryMDT or flags.Detector.GeometryRPC or flags.Detector.GeometryTGC:
        spCont+=["MuonSpacePoints"]
    if flags.Detector.GeometryMM or flags.Detector.GeometrysTGC:
        spCont+=["NswSpacePoints"]
    kwargs.setdefault("SpacePointKeys", spCont)
    result.addEventAlgo(CompFactory.MuonR4.MlHitDumperAlg(name, **kwargs))
    return result

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

def MuonSegmentDumpCfg(flags, name="MuonSegmentDumper", **kwargs):
    result = ComponentAccumulator()
    from MuonSpacePointFormation.SpacePointFormationConfig import MuonSpacePointFormationCfg
    result.merge(MuonSpacePointFormationCfg(flags))
    
    spCont = []
    if flags.Detector.GeometryMDT or flags.Detector.GeometryRPC or flags.Detector.GeometryTGC:
        spCont+=["MuonSpacePoints"]
    if flags.Detector.GeometryMM or flags.Detector.GeometrysTGC:
        spCont+=["NswSpacePoints"]
    
    segCont = ""
    if flags.Detector.GeometryMDT or flags.Detector.GeometryRPC or flags.Detector.GeometryTGC:
        segCont = "MuonSegmentsFromR4"
    
    kwargs.setdefault("SpacePointKeys", spCont)
    kwargs.setdefault("SegmentKeys", segCont)
    
    the_alg = CompFactory.MuonR4.SegmentDumperAlg(name=name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result
