#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""Cross-cut behaviour: preselection chaining, EVENTFLAG, GLOBALTRIGMATCH,
IMPORT, RUN_NUMBER dataType dependence, SAVE, and a debugMode smoke test."""

import unittest
from EventSelectionAlgorithms.EventSelectionConfigTestSupport import run, selectors, named, prop, DataType


class TestPreselectionChaining(unittest.TestCase):
    def test_chains_sequentially(self):
        algs = selectors(run("EL_N 25000 >= 1\nJET_N 30000 >= 2\nMET > 30000",
                             containers={"electrons": "AnaElectrons", "jets": "AnaJets", "met": "AnaMET"}))
        el = named(algs, "NEL")[0]
        jet = named(algs, "NJET")[0]
        met = named(algs, "MET")[0]
        self.assertEqual(prop(el, "eventPreselection", ""), "")
        self.assertIn("NEL_1", prop(jet, "eventPreselection"))
        self.assertIn("NJET_2", prop(met, "eventPreselection"))

    def test_seeded_by_preselection_option(self):
        algs = named(run("JET_N 30000 >= 2", containers={"jets": "AnaJets"},
                         preselection="pass_common_%SYS%"), "NJET")
        self.assertIn("pass_common", prop(algs[0], "eventPreselection"))


class TestFlags(unittest.TestCase):
    def test_eventflag_feeds_next_cut(self):
        algs = named(run("EVENTFLAG mytrigger_%SYS%\nJET_N 30000 >= 2",
                         containers={"jets": "AnaJets"}), "NJET")
        self.assertIn("mytrigger", prop(algs[0], "eventPreselection"))

    def test_globaltrigmatch_default(self):
        algs = named(run("GLOBALTRIGMATCH\nJET_N 30000 >= 2",
                         containers={"jets": "AnaJets"}), "NJET")
        self.assertIn("globalTriggerMatch", prop(algs[0], "eventPreselection"))

    def test_globaltrigmatch_postfix(self):
        algs = named(run("GLOBALTRIGMATCH _LooseID\nJET_N 30000 >= 2",
                         containers={"jets": "AnaJets"}), "NJET")
        self.assertIn("globalTriggerMatch_LooseID", prop(algs[0], "eventPreselection"))

    def test_import_feeds_next_cut(self):
        algs = named(run("IMPORT presel\nJET_N 30000 >= 2",
                         containers={"jets": "AnaJets"}), "NJET")
        self.assertIn("pass_presel", prop(algs[0], "eventPreselection"))


class TestRunNumber(unittest.TestCase):
    def test_random_run_number_depends_on_datatype(self):
        for dt, expected in [(DataType.Data, False), (DataType.FullSim, True)]:
            with self.subTest(dataType=dt):
                algs = named(run("RUN_NUMBER >= 300000", dataType=dt), "RUN_NUMBER")
                self.assertEqual(prop(algs[0], "useRandomRunNumber"), expected)

    def test_properties(self):
        algs = named(run("RUN_NUMBER >= 300000"), "RUN_NUMBER")
        self.assertEqual(prop(algs[0], "sign"), "GE")
        self.assertEqual(prop(algs[0], "runNumber"), 300000)


class TestSave(unittest.TestCase):
    def test_save_creates_filter(self):
        algs = run("EL_N 25000 >= 1\nSAVE", containers={"electrons": "AnaElectrons"})
        save = [a for a in algs if a.getType() == "CP::SaveFilterAlg"]
        self.assertEqual(len(save), 1)
        self.assertEqual(prop(save[0], "selectionName"), "pass_SR_%SYS%,as_char")
        self.assertEqual(prop(save[0], "decorationName"), "ntuplepass_SR_%SYS%")
        self.assertTrue(prop(save[0], "noFilter"))

    def test_save_bad_argcount(self):
        self.assertRaises(ValueError, run, "EL_N 25000 >= 1\nSAVE now",
                          containers={"electrons": "AnaElectrons"})


class TestDebugMode(unittest.TestCase):
    def test_debugmode_runs_and_keeps_selectors(self):
        # debugMode registers a per-cut output branch; here we assert it does not
        # perturb the selector algorithms or raise during configuration.
        algs = selectors(run("JET_N 30000 >= 2", containers={"jets": "AnaJets"}, debugMode=True))
        self.assertEqual(len(named(algs, "NJET")), 1)


if __name__ == "__main__":
    unittest.main()
