# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# jobOptions to activate the dump of the NSWPRDValAlg nTuple
# This file can be used with Sim_tf by specifying --postInclude MuonPRDTest.HitValAlgSim.HitValAlgSimCfg
# It dumps Truth, MuEntry and Hits, Digits, SDOs and RDOs for MM and sTGC

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def HitValAlgSimCfg(flags, name = "MuonSimHitValidAlg", **kwargs):
    result = ComponentAccumulator()

    from MuonGeoModelTestR4.testGeoModel import setupHistSvcCfg
    result.merge(setupHistSvcCfg(flags, outFile="NSWPRDValAlg.sim.ntuple.root", outStream="MUONHITVALIDSTREAM"))

    kwargs.setdefault("doTruth", True)
    kwargs.setdefault("doMuEntry", True)

    kwargs.setdefault("doMDTHit", True)
    kwargs.setdefault("doRPCHit", True)
    kwargs.setdefault("doTGCHit", True)

    kwargs.setdefault("doMMHit", flags.Detector.EnableMM)
    kwargs.setdefault("doSTGCHit", flags.Detector.EnablesTGC)
    kwargs.setdefault("doCSCHit", flags.Detector.EnableCSC)

    if not  flags.Detector.EnableCSC:
        kwargs.setdefault("CscRDODecoder","") # Remove the tool to prevent initializing CSC calibration tool
    
    the_alg = CompFactory.MuonVal.HitValAlg(name, **kwargs)
    result.addEventAlgo(the_alg)

    return result
