# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# jobOptions to activate the dump of the NSWPRDValAlg nTuple
# This file can be used with Reco_tf by specifying --postInclude MuonPRDTest.HitValAlgReco.HitValAlgRecoCfg
# It dumps Truth, MuEntry and Hits, Digits, SDOs and RDOs for MM and sTGC
def HitValAlgRecoCfg(flags, name="RecoValidAlg", outFile="NSWPRDValAlg.reco.ntuple.root", **kwargs):
    kwargs.setdefault("doTruth", flags.Input.isMC)
    kwargs.setdefault("doMuEntry", flags.Input.isMC)
    kwargs.setdefault("doSDOs", flags.Input.isMC)
    kwargs.setdefault("doPRDs", True)

    from MuonPRDTest.MuonPRDTestCfg import AddHitValAlgCfg
    return AddHitValAlgCfg(flags, name=name, outFile=outFile, **kwargs)
