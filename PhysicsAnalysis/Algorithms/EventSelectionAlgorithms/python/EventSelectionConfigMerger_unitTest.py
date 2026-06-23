#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""Unit tests for EventSelectionMergerConfig: the OR-merge across regions.

run_merger builds real regions (RUN_NUMBER + SAVE) named SR, CR and SUB_hidden,
then appends the merger. Each region's SAVE also emits a CP::SaveFilterAlg, so
the merger's own filter is identified by its 'EventSelectionMerger' name prefix.
"""

import unittest
from EventSelectionAlgorithms.EventSelectionConfigTestSupport import run_merger, named, prop


class TestMerger(unittest.TestCase):
    def test_ors_all_regions_excluding_subregions(self):
        algs = run_merger(["SR", "CR", "SUB_hidden"])
        merger = named(algs, "EventSelectionMerger")
        self.assertEqual(len(merger), 1)
        self.assertEqual(merger[0].getType(), "CP::SaveFilterAlg")
        sel = prop(merger[0], "selection")
        self.assertIn("pass_SR_%SYS%", sel)
        self.assertIn("pass_CR_%SYS%", sel)
        self.assertNotIn("pass_SUB_hidden", sel)      # SUB-regions are excluded
        self.assertIn("||", sel)                       # regions are OR'd
        self.assertEqual(prop(merger[0], "selectionName"), "pass_anySelection_%SYS%")


if __name__ == "__main__":
    unittest.main()
