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
    OfflineMuonBranchToolCfg,
    OfflineMuonReferenceToolCfg,
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

    def assert_reference(self, branch, property_name, expected):
        self.assertEqual(
            getattr(branch, property_name).getFullJobOptName(),
            expected.getFullJobOptName(),
        )

    def test_common_producer_defaults_do_not_enable_references(self):
        self.acc.merge(MuonTriggerValidationCfg(self.flags))
        algorithm = self.acc.getEventAlgo("MuonTriggerValidationAlg")
        self.assertEqual(len(algorithm.BranchTools), 0)
        self.assertFalse(self.acc.getPublicTools())
        self.assertTrue(algorithm.IsMC)
        self.assertTrue(algorithm.WriteEventInfo)
        self.assertEqual(
            list(self.acc.getService("THistSvc").Output),
            ["MUONTRIGGERVALID DATAFILE='MuonTriggerValidation.root' "
             "OPT='RECREATE'"],
        )

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

    def test_default_offline_reference_on_data(self):
        flags = initConfigFlags()
        flags.Input.Files = []
        flags.Input.isMC = False
        flags.lock()
        offline = self.acc.popToolsAndMerge(OfflineMuonBranchToolCfg(flags))
        self.acc.merge(MuonTriggerValidationCfg(
            flags, BranchTools=[offline], configureHistSvc=False
        ))
        reference = self.acc.getPublicTool("OfflineMuonReferenceTool")
        self.assertEqual(str(reference.MuonKey), "StoreGateSvc+Muons")
        branches = self.branches()
        self.assertEqual(set(branches), {"OfflineMuonBranchTool"})
        self.assert_reference(
            branches["OfflineMuonBranchTool"],
            "OfflineMuonReferenceTool", reference,
        )
        self.assertEqual({tool.name for tool in self.acc.getPublicTools()},
                         {reference.name})
        self.assertFalse(self.acc.getEventAlgo("MuonTriggerValidationAlg").IsMC)

    def test_combined_references_reuse_injected_providers(self):
        truth_reference = self.acc.getPrimaryAndMerge(TruthMuonReferenceToolCfg(
            self.flags, name="SharedMuonTruth"
        ))
        offline_reference = self.acc.getPrimaryAndMerge(
            OfflineMuonReferenceToolCfg(
                self.flags, name="SharedOfflineMuons", MuonKey="ValidationMuons"
            )
        )
        truth = self.acc.popToolsAndMerge(TruthMuonBranchToolCfg(
            self.flags, TruthMuonReferenceTool=truth_reference
        ))
        offline = self.acc.popToolsAndMerge(OfflineMuonBranchToolCfg(
            self.flags, OfflineMuonReferenceTool=offline_reference
        ))
        self.acc.merge(MuonTriggerValidationCfg(
            self.flags, BranchTools=[truth, offline], configureHistSvc=False
        ))
        branches = self.branches()
        self.assertEqual(set(branches),
                         {"TruthMuonBranchTool", "OfflineMuonBranchTool"})
        self.assert_reference(branches["TruthMuonBranchTool"],
                              "TruthMuonReferenceTool", truth_reference)
        self.assert_reference(branches["OfflineMuonBranchTool"],
                              "OfflineMuonReferenceTool", offline_reference)
        self.assertEqual(str(offline_reference.MuonKey), "ValidationMuons")
        self.assertEqual({tool.name for tool in self.acc.getPublicTools()},
                         {truth_reference.name, offline_reference.name})


if __name__ == "__main__":
    unittest.main()
