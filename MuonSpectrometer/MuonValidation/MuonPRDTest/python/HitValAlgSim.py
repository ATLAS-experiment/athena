# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# jobOptions to activate the dump of the MuonHitValAlg nTuple
# This file can be used with Sim_tf by specifying --postInclude MuonPRDTest.HitValAlgSim.HitValAlgSimCfg
# It dumps Truth, MuEntry and Hits, Digits, SDOs and RDOs for MM and sTGC
def HitValAlgSimCfg(flags, name = "MuonSimHitValidAlg",outFile="MuonHitValAlg.sim.ntuple.root", **kwargs):
    kwargs.setdefault("doTruth", True)
    kwargs.setdefault("doMuEntry", True)
    kwargs.setdefault("doSimHits", True)
    from MuonPRDTest.MuonPRDTestCfg import AddHitValAlgCfg
    return AddHitValAlgCfg(flags, name = name, outFile=outFile, **kwargs)
 