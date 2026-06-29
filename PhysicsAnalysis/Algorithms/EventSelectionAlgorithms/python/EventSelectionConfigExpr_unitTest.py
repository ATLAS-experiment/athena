#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""Unit tests for the generic EXPR object-kinematic cuts.

The config-time validation (compatibility matrix + the two error classes) runs
before any algorithm is created, so the error tests pass without the C++
component. The happy-path tests require CP::ObjectKinematicSelectorAlg to be
built (they instantiate it through the real accumulator)."""

import unittest
from EventSelectionAlgorithms.EventSelectionConfigTestSupport import run, named, prop
from EventSelectionAlgorithms.EventSelectionConfig import (
    UnavailableFeatureError, InconsistentSettingsError,
)

ALL = dict(electrons="AnaElectrons", muons="AnaMuons", taus="AnaTaus", jets="AnaJets",
           largeRjets="AnaLRJets", photons="AnaPhotons", met="AnaMET", btagDecoration="btag")


class TestExprHappyPath(unittest.TestCase):
    def test_dR(self):
        alg = named(run("EXPR dR(jet[0], jet[1]) < 1.0", containers=ALL), "EXPR")[0]
        self.assertEqual(alg.getType(), "CP::ObjectKinematicSelectorAlg")
        self.assertEqual(prop(alg, "variable"), "dR")
        self.assertEqual(prop(alg, "sign"), "LT")
        self.assertEqual(prop(alg, "refValue"), 1.0)
        self.assertEqual(list(prop(alg, "operandKinds")), ["PARTICLE", "PARTICLE"])
        self.assertEqual(list(prop(alg, "indices")), [0, 1])

    def test_bjet_btag_applied(self):
        alg = named(run("EXPR pt(bjet[0]) > 30000", containers=ALL), "EXPR")[0]
        self.assertIn("btag", list(prop(alg, "selections"))[0])

    def test_mass_of_sum(self):
        alg = named(run("EXPR m(el[0]+mu[0]) > 80000", containers=ALL), "EXPR")[0]
        self.assertEqual(prop(alg, "variable"), "m")
        bare = [c.replace("_%SYS%", "") for c in prop(alg, "collections")]
        self.assertEqual(bare, ["AnaElectrons", "AnaMuons"])

    def test_dphi_met(self):
        alg = named(run("EXPR dPhi(met, jet[0]) > 2.0", containers=ALL), "EXPR")[0]
        self.assertEqual(list(prop(alg, "operandKinds")), ["MET", "PARTICLE"])
        self.assertEqual(prop(alg, "metTerm"), "Final")
        bare = [c.replace("_%SYS%", "") for c in prop(alg, "collections")]
        self.assertEqual(bare, ["AnaJets"])

    def test_whitespace_insensitive(self):
        a = named(run("EXPR dR(jet[0], jet[1]) < 1.0", containers=ALL), "EXPR")[0]
        b = named(run("EXPR dR(jet[0],jet[1])<1.0", containers=ALL), "EXPR")[0]
        self.assertEqual(prop(a, "variable"), prop(b, "variable"))
        self.assertEqual(list(prop(a, "indices")), list(prop(b, "indices")))


class TestExprInconsistent(unittest.TestCase):
    CASES = [
        "EXPR dR(jet[0]) < 1.0",
        "EXPR dR(jet[0], jet[1], jet[2]) < 1.0",
        "EXPR eta(met) < 1.0",
        "EXPR dR(met, jet[0]) < 1.0",
        "EXPR pt(met[0]) > 30000",
        "EXPR pt(met+jet[0]) > 30000",
        "EXPR dR(jet[0]+jet[1]) < 1.0",
        "EXPR pt(jet) > 30000",
    ]
    def test_inconsistent(self):
        for line in self.CASES:
            with self.subTest(line=line):
                self.assertRaises(InconsistentSettingsError, run, line, containers=ALL)


class TestExprUnavailable(unittest.TestCase):
    def test_unknown_variable(self):
        self.assertRaises(UnavailableFeatureError, run,
                          "EXPR mT2(jet[0], jet[1]) < 100000", containers=ALL)

    def test_unknown_collection(self):
        self.assertRaises(UnavailableFeatureError, run,
                          "EXPR pt(fjet[0]) > 30000", containers=ALL)

    def test_charge_is_deferred(self):
        # charge() is recognised but not yet implemented -> UnavailableFeatureError
        self.assertRaises(UnavailableFeatureError, run,
                          "EXPR charge(el[0]) == 1", containers=ALL)
        self.assertRaises(UnavailableFeatureError, run,
                          "EXPR charge(jet[0]) == 1", containers=ALL)


class TestExprMissingInput(unittest.TestCase):
    def test_missing_container(self):
        self.assertRaises(ValueError, run, "EXPR pt(jet[0]) > 30000")

    def test_bjet_missing_btag(self):
        self.assertRaises(ValueError, run, "EXPR pt(bjet[0]) > 30000",
                          containers={"jets": "AnaJets"})


if __name__ == "__main__":
    unittest.main()
