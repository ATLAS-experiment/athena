# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

import unittest
from AthenaConfiguration.AthConfigFlags import AthConfigFlags

def createDiTauConfigFlags():
    ditau_cfg = AthConfigFlags()
    ditau_cfg.addFlag("DiTau.DiTauContainer", ["HLT_DiTauJets"]) 
    ditau_cfg.addFlag("DiTau.JetSeedPt", 200000)
    ditau_cfg.addFlag("DiTau.MaxEta", 2.5)
    ditau_cfg.addFlag("DiTau.Rjet", 1.0)
    ditau_cfg.addFlag("DiTau.Rsubjet", 0.2)
    ditau_cfg.addFlag("DiTau.Rcore", 0.1)
    ditau_cfg.addFlag("DiTau.CalibFolder", 'TrigTauRec/00-11-02/')
    ditau_cfg.addFlag('DiTau.DiTauIDModel', 'DiTauOmni_v1p0/boosted_ditau_omni_model.onnx')
    return ditau_cfg


class DiTestTauRecConfigFlags(unittest.TestCase):
    def runTest(self):
        createDiTauConfigFlags()


if __name__ == "__main__":
    unittest.main()


