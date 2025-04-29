#Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def CsvSpacePointDumpCfg(flags, name="CsvDriftCircleDumper", **kwargs):
    result = ComponentAccumulator()
    from MuonSpacePointFormation.SpacePointFormationConfig import MuonSpacePointFormationCfg
    result.merge(MuonSpacePointFormationCfg(flags))
    spCont = []
    if flags.Detector.GeometryMDT or flags.Detector.GeometryRPC or flags.Detector.GeometryTGC:
        spCont+=["MuonSpacePoints"]
    if flags.Detector.GeometryMM or flags.Detector.GeometrysTGC:
        spCont+=["NswSpacePoints"]
    
    kwargs.setdefault("SpacePointKeys", spCont)
    the_alg = CompFactory.MuonR4.SpacePointCsvDumperAlg(name=name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result


def CsvMuonTruthSegmentDumpCfg(flags, name="CsvMuonTruthSegmentDumper", **kwargs):
    result = ComponentAccumulator()
    the_alg = CompFactory.MuonR4.TruthSegmentCsvDumperAlg(name = name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result
