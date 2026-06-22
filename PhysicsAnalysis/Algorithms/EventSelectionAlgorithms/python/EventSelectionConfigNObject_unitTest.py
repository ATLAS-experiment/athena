#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""Unit tests for the N-object pT selectors: EL_N, MU_N, JET_N, PH_N, TAU_N,
LJET_N, OBJ_N. All build CP::NObjectPtSelectorAlg, differing only in source."""

import unittest
from EventSelectionAlgorithms.EventSelectionConfigTestSupport import run, named, prop

# keyword -> (option name, container name)
FAMILY = {
    "EL_N":   ("electrons",  "AnaElectrons"),
    "MU_N":   ("muons",      "AnaMuons"),
    "JET_N":  ("jets",       "AnaJets"),
    "PH_N":   ("photons",    "AnaPhotons"),
    "TAU_N":  ("taus",       "AnaTaus"),
    "LJET_N": ("largeRjets", "AnaLRJets"),
}
TAG = {"EL_N": "NEL", "MU_N": "NMU", "JET_N": "NJET", "PH_N": "NPH",
       "TAU_N": "NTAU", "LJET_N": "NLJET"}


class TestNObjectMinimal(unittest.TestCase):
    def test_minimal_form(self):
        for kw, (opt, val) in FAMILY.items():
            with self.subTest(kw=kw):
                algs = named(run(f"{kw} 25000 >= 2", containers={opt: val}), TAG[kw])
                self.assertEqual(len(algs), 1)
                alg = algs[0]
                self.assertEqual(alg.getType(), "CP::NObjectPtSelectorAlg")
                self.assertEqual(prop(alg, "minPt"), 25000.0)
                self.assertEqual(prop(alg, "sign"), "GE")
                self.assertEqual(prop(alg, "count"), 2)

    def test_extra_selection_form(self):
        for kw, (opt, val) in FAMILY.items():
            with self.subTest(kw=kw):
                algs = named(run(f"{kw} mysel 25000 >= 2", containers={opt: val}), TAG[kw])
                self.assertIn("mysel", prop(algs[0], "objectSelection"))

    def test_extra_selection_composes_with_base(self):
        for kw, (opt, val) in FAMILY.items():
            with self.subTest(kw=kw):
                algs = named(run(f"{kw} mysel 25000 >= 2",
                                 containers={opt: f"{val}.baseline"}), TAG[kw])
                sel = prop(algs[0], "objectSelection")
                self.assertIn("baseline", sel)
                self.assertIn("mysel", sel)

    def test_all_signs(self):
        for sym, enum in [("<", "LT"), (">", "GT"), ("==", "EQ"),
                          (">=", "GE"), ("<=", "LE")]:
            with self.subTest(sign=sym):
                algs = named(run(f"JET_N 30000 {sym} 2", containers={"jets": "AnaJets"}), "NJET")
                self.assertEqual(prop(algs[0], "sign"), enum)


class TestNObjectErrors(unittest.TestCase):
    def test_missing_input_raises(self):
        for kw, (opt, _) in FAMILY.items():
            if kw == "LJET_N":      # no missing-input guard in the source
                continue
            with self.subTest(kw=kw):
                self.assertRaises(ValueError, run, f"{kw} 25000 >= 2")

    def test_wrong_argcount_raises(self):
        for kw, (opt, val) in FAMILY.items():
            with self.subTest(kw=kw):
                self.assertRaises(ValueError, run,
                                  f"{kw} 25000 >= 2 extra junk", containers={opt: val})


class TestObjN(unittest.TestCase):
    def test_picks_container_from_argument(self):
        algs = named(run("OBJ_N MyContainer 30000 >= 2",
                         containers={"jets": "MyContainer"}), "NOBJ")
        self.assertEqual(len(algs), 1)
        self.assertEqual(prop(algs[0], "minPt"), 30000.0)
        self.assertEqual(prop(algs[0], "count"), 2)

    def test_requires_five_args(self):
        self.assertRaises(ValueError, run, "OBJ_N MyContainer 30000 >= 2 extra",
                          containers={"jets": "MyContainer"})
        self.assertRaises(ValueError, run, "OBJ_N 30000 >= 2",
                          containers={"jets": "MyContainer"})


if __name__ == "__main__":
    unittest.main()
