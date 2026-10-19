# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""Check that the jet preselection strings route to the right scenario.

The HitZ preselection stub (HZ) is also matched by the DIPZ pattern, so a
routing mistake sends a HitZ scenario into scenario_dipz, where it fails on a
regex assertion. Parsing both here catches that without a menu build.

Only single-leg chains are checked here. dictFromChainName leaves every
chainPartIndex at 0, which the FastReduction configurer rejects as
"nonsequential chainParts" for any multi-leg chain -- the indices are assigned
later, during menu generation. Multi-leg preselections are therefore covered by
the HLT_<menu> tests, not here.
"""

from TriggerMenuMT.HLT.Config.Utility.ChainDefInMenu import ChainProp
from TriggerMenuMT.HLT.Config.Utility.DictFromChainName import dictFromChainName
from TriggerMenuMT.HLT.Menu.Physics_pp_run4_v1 import MultiJetGroup
from TriggerMenuMT.HLT.Jet.JetPresel import (
    caloPreselJetHypoToolFromDict,
    roiPreselJetHypoToolFromDict,
    hitZPreselJetHypoToolFromDict,
)

import unittest

from AthenaConfiguration.AllConfigFlags import initConfigFlags
flags = initConfigFlags()
flags.Input.Files = []
flags.lock()

HITZ_CHAINS = [
    'EF_j0_pf_ftf_preselHZ87MAXMULT30iXX4i20_L13jJ40',
    'EF_j0_pf_ftf_preselHZ99MAXMULT30iXX4i20_L13jJ40',
    'EF_j0_pf_ftf_preselHZ118MAXMULT30iXX4i20_L13jJ40',
    'EF_j0_pf_ftf_preselHZ145MAXMULT20iXX6i20_L14jJ40',
    'EF_j0_pf_ftf_preselHZ167MAXMULT20iXX6i20_L14jJ40',
    'EF_j0_pf_ftf_preselHZ196MAXMULT20iXX6i20_L14jJ40',
]

DIPZ_CHAINS = [
    'HLT_j0_pf_ftf_preselZ120XX4c20_L13J35p0ETA23',
    'HLT_j0_pf_ftf_preselZ120MAXMULT20cXX4c20_L13J35p0ETA23',
]


class PreselRouting(unittest.TestCase):
    def _check(self, name, taggingToolGen):
        chain_dict = dictFromChainName(
            flags,
            ChainProp(name=name,
                      l1SeedThresholds=['FSNOSEED'],
                      groups=MultiJetGroup))
        self.assertIsNotNone(caloPreselJetHypoToolFromDict(flags, chain_dict))
        self.assertIsNotNone(taggingToolGen(flags, chain_dict))

    def test_hitz(self):
        for name in HITZ_CHAINS:
            with self.subTest(chain=name):
                self._check(name, hitZPreselJetHypoToolFromDict)

    def test_dipz_still_routes(self):
        for name in DIPZ_CHAINS:
            with self.subTest(chain=name):
                self._check(name, roiPreselJetHypoToolFromDict)


if __name__ == '__main__':
    unittest.main()
