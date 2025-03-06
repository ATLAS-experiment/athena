#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

import unittest

from Campaigns.Utils import Campaign

class TestCampaignEnum(unittest.TestCase):
    def test_strict_order(self):
        # test transitive ordering
        self.assertTrue(Campaign.Unknown < Campaign.MC16a)
        self.assertTrue(Campaign.MC16a > Campaign.Unknown)
        self.assertTrue(Campaign.MC16a < Campaign.MC16d)
        self.assertTrue(Campaign.MC16d > Campaign.MC16a)
        self.assertTrue(Campaign.MC16d < Campaign.MC16e)
        self.assertTrue(Campaign.MC16e > Campaign.MC16d)
        self.assertTrue(Campaign.MC16e < Campaign.MC20a)
        self.assertTrue(Campaign.MC20a > Campaign.MC16e)
        self.assertTrue(Campaign.MC20a < Campaign.MC20d)
        self.assertTrue(Campaign.MC20d > Campaign.MC20a)
        self.assertTrue(Campaign.MC20d < Campaign.MC20e)
        self.assertTrue(Campaign.MC20e > Campaign.MC20d)
        self.assertTrue(Campaign.MC20e < Campaign.MC21a)
        self.assertTrue(Campaign.MC21a > Campaign.MC20e)
        self.assertTrue(Campaign.MC21a < Campaign.MC23a)
        self.assertTrue(Campaign.MC23a > Campaign.MC21a)
        self.assertTrue(Campaign.MC23a < Campaign.MC23c)
        self.assertTrue(Campaign.MC23c > Campaign.MC23a)
        self.assertTrue(Campaign.MC23c < Campaign.MC23d)
        self.assertTrue(Campaign.MC23d > Campaign.MC23c)
        self.assertTrue(Campaign.MC23d < Campaign.MC23e)
        self.assertTrue(Campaign.MC23e > Campaign.MC23d)
        self.assertTrue(Campaign.MC23e < Campaign.MC23g)
        self.assertTrue(Campaign.MC23g > Campaign.MC23e)
        self.assertTrue(Campaign.MC23g < Campaign.PhaseII)
        self.assertTrue(Campaign.PhaseII > Campaign.MC23g)

        # test some extremes
        self.assertTrue(Campaign.Unknown < Campaign.PhaseII)
        self.assertTrue(Campaign.PhaseII > Campaign.Unknown)
        self.assertTrue(Campaign.MC20a < Campaign.MC23g)
        self.assertTrue(Campaign.MC23g > Campaign.MC20a)

    def test_order(self):
        # test transitive ordering
        self.assertTrue(Campaign.Unknown <= Campaign.MC16a)
        self.assertTrue(Campaign.MC16a >= Campaign.Unknown)
        self.assertTrue(Campaign.MC16a <= Campaign.MC16d)
        self.assertTrue(Campaign.MC16d >= Campaign.MC16a)
        self.assertTrue(Campaign.MC16d <= Campaign.MC16e)
        self.assertTrue(Campaign.MC16e >= Campaign.MC16d)
        self.assertTrue(Campaign.MC16e <= Campaign.MC20a)
        self.assertTrue(Campaign.MC20a >= Campaign.MC16e)
        self.assertTrue(Campaign.MC20a <= Campaign.MC20d)
        self.assertTrue(Campaign.MC20d >= Campaign.MC20a)
        self.assertTrue(Campaign.MC20d <= Campaign.MC20e)
        self.assertTrue(Campaign.MC20e >= Campaign.MC20d)
        self.assertTrue(Campaign.MC20e <= Campaign.MC21a)
        self.assertTrue(Campaign.MC21a >= Campaign.MC20e)
        self.assertTrue(Campaign.MC21a <= Campaign.MC23a)
        self.assertTrue(Campaign.MC23a >= Campaign.MC21a)
        self.assertTrue(Campaign.MC23a <= Campaign.MC23c)
        self.assertTrue(Campaign.MC23c >= Campaign.MC23a)
        self.assertTrue(Campaign.MC23c <= Campaign.MC23d)
        self.assertTrue(Campaign.MC23d >= Campaign.MC23c)
        self.assertTrue(Campaign.MC23d <= Campaign.MC23e)
        self.assertTrue(Campaign.MC23e >= Campaign.MC23d)
        self.assertTrue(Campaign.MC23e <= Campaign.MC23g)
        self.assertTrue(Campaign.MC23g >= Campaign.MC23e)
        self.assertTrue(Campaign.MC23g <= Campaign.PhaseII)
        self.assertTrue(Campaign.PhaseII >= Campaign.MC23g)

        # test some extremes
        self.assertTrue(Campaign.Unknown <= Campaign.PhaseII)
        self.assertTrue(Campaign.PhaseII >= Campaign.Unknown)
        self.assertTrue(Campaign.MC20a <= Campaign.MC23g)
        self.assertTrue(Campaign.MC23g >= Campaign.MC20a)

if __name__ == "__main__":
    unittest.main()
