# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

import unittest
from AthenaConfiguration.AthConfigFlags import AthConfigFlags

def createDiTauConfigFlags():
    ditau_cfg = AthConfigFlags()
    
    ditau_cfg.addFlag("DiTau.Enabled", True)
    ditau_cfg.addFlag("DiTau.doDiTauRec", True)
    ditau_cfg.addFlag("DiTau.DiTauContainer", ["DiTauJets","DiTauJetsLowPt"]) 
    ditau_cfg.addFlag("DiTau.JetSeedPt", [300000,50000])
    ditau_cfg.addFlag("DiTau.SeedJetCollection", ["AntiKt10LCTopoJets","AntiKt10EMPFlowJets"])
    ditau_cfg.addFlag("DiTau.MaxEta", 2.5)
    ditau_cfg.addFlag("DiTau.Rjet", 1.0)
    ditau_cfg.addFlag("DiTau.Rsubjet", 0.2)
    ditau_cfg.addFlag("DiTau.Rcore", 0.1)
    ditau_cfg.addFlag("DiTau.doTJVA", True)
    ditau_cfg.addFlag("DiTau.doRunDiTauDiscriminant", False)
     
    return ditau_cfg

class DiTestTauRecConfigFlags(unittest.TestCase):
    def runTest(self):
        createDiTauConfigFlags()


if __name__ == "__main__":
    unittest.main()


