# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# jobOptions to activate the dump of the NSWPRDValAlg nTuple
# This file can be used with Digi_tf by specifying --postInclude MuonPRDTest.HitValAlgDigi.HitValAlgDigiCfg
# It dumps Truth, MuEntry and Hits, Digits, SDOs and RDOs for MM and sTGC

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def HitValAlgDigiCfg(flags, name = "NSWPRDValAlg", **kwargs):
    result = ComponentAccumulator()

    from MuonGeoModelTestR4.testGeoModel import setupHistSvcCfg
    result.merge(setupHistSvcCfg(flags, outFile="NSWPRDValAlg.digi.ntuple.root", outStream="MUONHITVALIDSTREAM"))

    kwargs.setdefault("doTruth", True)
    kwargs.setdefault("doMuEntry", True)

    kwargs.setdefault("doMMHit", flags.Detector.EnableMM)
    kwargs.setdefault("doMMDigit", flags.Detector.EnableMM)
    kwargs.setdefault("doMMRDO", flags.Detector.EnableMM)
    kwargs.setdefault("doMMSDO", flags.Detector.EnableMM)
    kwargs.setdefault("doMMPRD", False)
    kwargs.setdefault("doMMFastDigit", False)

    kwargs.setdefault("doSTGCHit", flags.Detector.EnablesTGC)
    kwargs.setdefault("doSTGCDigit", flags.Detector.EnablesTGC) 
    kwargs.setdefault("doSTGCRDO", flags.Detector.EnablesTGC)
    kwargs.setdefault("doSTGCSDO", flags.Detector.EnablesTGC)
    kwargs.setdefault("doSTGCPRD", False)
    kwargs.setdefault("doSTGCFastDigit", False)

    kwargs.setdefault("doRPCHit", True)
    kwargs.setdefault("doRPCSDO", True)
    kwargs.setdefault("doRPCDigit", True)

    kwargs.setdefault("doMDTHit", True)
    kwargs.setdefault("doMDTSDO", True)
    kwargs.setdefault("doMDTDigit", True)

    kwargs.setdefault("doTGCHit", True)
    kwargs.setdefault("doTGCSDO", True)
    kwargs.setdefault("doTGCDigit", True)
    kwargs.setdefault("doTGCRDO", True)

    kwargs.setdefault("doCSCHit", flags.Detector.EnableCSC)
    kwargs.setdefault("doCSCSDO", flags.Detector.EnableCSC)
    kwargs.setdefault("doCSCDigit", flags.Detector.EnableCSC)
    kwargs.setdefault("doCSCRDO", flags.Detector.EnableCSC)
    if not  flags.Detector.EnableCSC:
        kwargs.setdefault("CscRDODecoder","")
    kwargs.setdefault("doCSCPRD", False)

    valAlg = CompFactory.MuonVal.HitValAlg(name, **kwargs)
    result.addEventAlgo(valAlg)

    return result
