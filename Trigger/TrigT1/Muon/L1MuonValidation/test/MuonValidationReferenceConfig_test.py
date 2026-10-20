# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""Exercise reference composition with real Athena configurables, without I/O.

This test requires the built Athena component dictionaries. It checks the
configuration graph, not detector extrapolation or event processing.
"""

import math
import unittest

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

from L1MuonValidation.L1MuonValidationConfig import (
    MuonTriggerValidationCfg,
    TruthMuonBranchToolCfg,
    TruthMuonReferenceToolCfg,
)


class MuonValidationReferenceConfigTest(unittest.TestCase):
    def setUp(self):
        self.flags = initConfigFlags()
        self.flags.Input.Files = []
        self.flags.Input.isMC = True
        self.flags.lock()
        self.acc = ComponentAccumulator()

    def tearDown(self):
        self.acc.wasMerged()

    def branches(self):
        algorithm = self.acc.getEventAlgo("MuonTriggerValidationAlg")
        return {tool.name: tool for tool in algorithm.BranchTools}

    def test_common_producer_accepts_common_content_without_tgc(self):
        reference = self.acc.getPrimaryAndMerge(
            TruthMuonReferenceToolCfg(self.flags)
        )
        self.assertEqual(reference.MinTruthPt, 0.0)
        self.assertEqual(reference.MinTruthAbsEta, 0.0)
        self.assertTrue(math.isfinite(reference.MaxTruthAbsEta))
        self.assertGreater(reference.MaxTruthAbsEta, 100.0)
        truth = self.acc.popToolsAndMerge(
            TruthMuonBranchToolCfg(
                self.flags, TruthMuonReferenceTool=reference
            )
        )
        self.acc.merge(MuonTriggerValidationCfg(
            self.flags, BranchTools=[truth], configureHistSvc=False
        ))
        self.assertEqual(set(self.branches()), {"TruthMuonBranchTool"})
        self.assertEqual(len(self.acc.getPublicTools()), 1)


if __name__ == "__main__":
    unittest.main()
