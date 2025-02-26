# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# jobOptions to activate the dump of the NSWPRDValAlg nTuple
# This file can be used with Digi_tf by specifying --postInclude MuonPRDTest.HitValAlgDigi.HitValAlgDigiCfg
# It dumps Truth, MuEntry and Hits, Digits, SDOs and RDOs for MM and sTGC
def HitValAlgDigiCfg(flags, name = "NSWPRDValAlg", **kwargs):
    kwargs.setdefault("doTruth", True)
    kwargs.setdefault("doMuEntry", True)

    kwargs.setdefault("doSimHits", True)
    kwargs.setdefault("doSDOs", True)
    kwargs.setdefault("doDigits", True)
    kwargs.setdefault("doRDOs", True)
    from MuonPRDTest.MuonPRDTestCfg import AddHitValAlgCfg
    return AddHitValAlgCfg(flags, name = name, outFile="NSWPRDValAlg.digi.ntuple.root", **kwargs)
