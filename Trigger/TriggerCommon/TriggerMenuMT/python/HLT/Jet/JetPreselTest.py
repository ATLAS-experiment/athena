# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""Check that the jet preselection strings route to the right scenario.

The HitZ preselection stub (HZ) is also matched by the DIPZ pattern, so a
routing mistake sends a HitZ scenario into scenario_dipz, where it fails on a
regex assertion. Parsing both here catches that without a menu build.
"""

from TriggerMenuMT.HLT.Config.Utility.ChainDefInMenu import ChainProp
from TriggerMenuMT.HLT.Config.Utility.DictFromChainName import dictFromChainName
from TriggerMenuMT.HLT.Menu.Physics_pp_run3_v1 import MultiJetGroup
from TriggerMenuMT.HLT.Jet.JetPresel import (
    caloPreselJetHypoToolFromDict,
    roiPreselJetHypoToolFromDict,
)

import unittest

from AthenaConfiguration.AllConfigFlags import initConfigFlags
flags = initConfigFlags()
flags.Input.Files = []
flags.lock()


class PreselRouting(unittest.TestCase):
    def _tools(self, name):
        chain_dict = dictFromChainName(
            flags,
            ChainProp(name=name,
                      l1SeedThresholds=['FSNOSEED'],
                      groups=MultiJetGroup))
        return (caloPreselJetHypoToolFromDict(flags, chain_dict),
                roiPreselJetHypoToolFromDict(flags, chain_dict))

    def test_hitz(self):
        for presel in ('preselHZ120XX4c25',
                       'preselHZ120MAXMULT5cXX4c25',
                       'preselHZ120XX2c25XX2c25bgtwo95'):
            with self.subTest(presel=presel):
                calo, roi = self._tools(
                    f'HLT_j0_pf_ftf_{presel}_L13J35p0ETA23')
                self.assertIsNotNone(calo)
                self.assertIsNotNone(roi)

    def test_dipz_still_routes(self):
        calo, roi = self._tools(
            'HLT_j0_pf_ftf_preselZ120XX4c20_L13J35p0ETA23')
        self.assertIsNotNone(calo)
        self.assertIsNotNone(roi)


if __name__ == '__main__':
    unittest.main()
