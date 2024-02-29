#Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def CsvMdtDriftCircleDumpCfg(flags, name="CsvDriftCircleDumper", **kwargs):
    result = ComponentAccumulator()
    the_alg = CompFactory.MdtDriftCircleCsvDumperAlg(name=name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def CsvMuonSimHitDumpCfg(flags, name="CsvMuonSimHitDumper", **kwargs):
    result = ComponentAccumulator()
    the_alg = CompFactory.MuonSimHitCsvDumperAlg(name = name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def CsvMuonStripDumpCfg(flags, name="CsvStripHitDumper", **kwargs):
    result = ComponentAccumulator()
    the_alg = CompFactory.MuonStripCsvDumperAlg(name = name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result
