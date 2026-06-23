#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""Unit tests for the pure validation helpers of EventSelectionConfig.
These need no accumulator: a bare block instance exposes the methods."""

import unittest
from EventSelectionAlgorithms.EventSelectionConfig import EventSelectionConfig


class TestHelpers(unittest.TestCase):
    def setUp(self):
        self.b = EventSelectionConfig()

    def test_check_float_ok(self):
        for text, exp in [("0", 0.0), ("25000", 25000.0), ("1.5", 1.5)]:
            self.assertEqual(self.b.check_float(text), exp)

    def test_check_float_rejects_negative(self):
        self.assertRaises(ValueError, self.b.check_float, "-1")

    def test_check_float_allows_negative_when_not_required(self):
        self.assertEqual(self.b.check_float("-1", requirePositive=False), -1.0)

    def test_check_float_rejects_nonnumeric(self):
        self.assertRaises(ValueError, self.b.check_float, "abc")

    def test_check_int_ok(self):
        self.assertEqual(self.b.check_int("2"), 2)

    def test_check_int_rejects_float(self):
        self.assertRaises(ValueError, self.b.check_int, "1.5")

    def test_check_int_rejects_negative(self):
        self.assertRaises(ValueError, self.b.check_int, "-3")

    def test_check_sign_ok(self):
        for sym, enum in [("<", "LT"), (">", "GT"), ("==", "EQ"), (">=", "GE"), ("<=", "LE")]:
            self.assertEqual(self.b.check_sign(sym), enum)

    def test_check_sign_rejects_unknown(self):
        self.assertRaises(KeyError, self.b.check_sign, "=!")

    def test_check_btagging_ok(self):
        self.assertEqual(self.b.check_btagging("GN2v01:FixedCutBEff_85"),
                         ["GN2v01", "FixedCutBEff_85"])

    def test_check_btagging_rejects_malformed(self):
        self.assertRaises(ValueError, self.b.check_btagging, "GN2v01")

    def test_check_ghosts_single(self):
        cases = {"B": "GhostBHadronsFinalCount", "C": "GhostCHadronsFinalCount",
                 "T": "GhostTQuarksFinalCount", "W": "GhostWBosonsCount",
                 "Z": "GhostZBosonsCount", "H": "GhostHBosonsCount",
                 "TAU": "GhostTausFinalCount"}
        for token, exp in cases.items():
            self.assertEqual(self.b.check_ghosts(token), [exp])

    def test_check_ghosts_with_veto(self):
        self.assertEqual(self.b.check_ghosts("B!C"),
                         ["GhostBHadronsFinalCount", "GhostCHadronsFinalCount"])

    def test_check_ghosts_passthrough_unknown(self):
        self.assertEqual(self.b.check_ghosts("MyCustomGhost"), ["MyCustomGhost"])

    def test_check_decoration_empty(self):
        self.assertEqual(self.b.checkDecorationName(""), "")

    def test_check_decoration_appends_as_char(self):
        self.assertEqual(self.b.checkDecorationName("foo_%SYS%"), "foo_%SYS%,as_char")

    def test_check_decoration_preserves_existing(self):
        self.assertEqual(self.b.checkDecorationName("foo_%SYS%,as_char"), "foo_%SYS%,as_char")

    def test_check_decoration_and_chain(self):
        self.assertEqual(self.b.checkDecorationName("a_%SYS%&&b_%SYS%,as_char"),
                         "a_%SYS%,as_char&&b_%SYS%,as_char")


if __name__ == "__main__":
    unittest.main()
