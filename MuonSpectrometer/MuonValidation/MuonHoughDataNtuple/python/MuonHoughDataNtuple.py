# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MakeMuonHoughDataNtuple(flags, name="MuonHoughDataNtuple", **kwargs):
    result = ComponentAccumulator()
    from MuonGeoModelTestR4.testGeoModel import setupHistSvcCfg
    result.merge(setupHistSvcCfg(flags, out_file="MuonHoughDataNtuple.root", out_stream="MuonHoughDataNtuple"))
    theAlg = CompFactory.MuonHoughDataNtuple(name, **kwargs)
    result.addEventAlgo(theAlg, primary=True)
    return result
