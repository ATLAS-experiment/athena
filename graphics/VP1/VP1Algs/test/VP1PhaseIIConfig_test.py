# Copyright (C) 2026 CERN for the benefit of the ATLAS collaboration

"""Check Phase-II geometry and conditions independently of tool configuration."""

import unittest
from unittest.mock import patch

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.TestDefaults import defaultConditionsTags, defaultGeometryTags
from VP1Algs.VP1AlgConfig import SetupVP1


class VP1PhaseIIConfigTest(unittest.TestCase):
    def test_phaseII_uses_run4_defaults(self):
        cfg = ComponentAccumulator()
        self.addCleanup(cfg.wasMerged)
        # Keep the real argument parsing and flag selection, without accessing
        # conditions databases, building geometry, or starting an event loop.
        with patch("sys.argv", ["vp1", "--phaseII", "--verboseAthena"]), \
             patch("AthenaConfiguration.MainServicesConfig.MainServicesCfg", return_value=cfg), \
             patch("VP1Algs.VP1AlgConfig.configureGeometry") as geometry, \
             patch("AthenaConfiguration.AthConfigFlags.AthConfigFlags.dump"), \
             patch.object(ComponentAccumulator, "run") as run:
            SetupVP1()
        geometry.assert_called_once()
        flags = geometry.call_args.args[0]
        self.assertEqual(flags.GeoModel.AtlasVersion, defaultGeometryTags.RUN4)
        self.assertEqual(flags.IOVDb.GlobalTag, defaultConditionsTags.RUN4_MC)
        run.assert_called_once()


if __name__ == "__main__":
    unittest.main()
