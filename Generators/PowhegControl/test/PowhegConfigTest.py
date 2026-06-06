#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

import os
from pathlib import Path
import tempfile
import unittest

from PowhegControl.PowhegRunConfig import (
    PowhegGenerationOptions,
    PowhegRunConfig,
    PowhegWeight,
    PowhegWeightGroup,
)


class PowhegRunConfigTest(unittest.TestCase):

    def test_json_round_trip(self):
        plan = PowhegRunConfig(
            process="tt",
            beam_energy=13600.0,
            max_events=100,
            random_seed=42,
            n_cores=8,
            shower=True,
            output_lhe="PowhegOTF._1.events",
            settings={"hdamp": 258.75},
            weight_groups=(
                PowhegWeightGroup(
                    name="scale_variation",
                    parameters=("mu_R", "mu_F"),
                    combination_method="envelope",
                    weights=(
                        PowhegWeight("MUR2_MUF2", (2.0, 2.0)),
                    ),
                ),
            ),
            generation_options=PowhegGenerationOptions(
                remove_old_style_rwt_comments=True
            ),
        )

        restored = PowhegRunConfig.from_json(plan.to_json())
        self.assertEqual(restored, plan)

    def test_serialization_has_no_environment_side_effects(self):
        before = dict(os.environ)
        PowhegRunConfig(
            process="tt",
            beam_energy=13600.0,
            max_events=10,
            random_seed=1,
            n_cores=1,
            shower=False,
            output_lhe="events.lhe",
        ).to_json()
        self.assertEqual(dict(os.environ), before)

    def test_ca_construction_has_no_runtime_side_effects(self):
        from AthenaConfiguration.AllConfigFlags import initConfigFlags
        from AthenaCommon.SystemOfUnits import GeV
        from PowhegControl.PowhegConfig import (
            PowhegCfg,
            setupPowhegFlags,
        )

        flags = initConfigFlags()
        flags.Beam.Energy = 6800 * GeV
        flags.Exec.MaxEvents = 10
        flags.Random.SeedOffset = 7
        flags.Output.EVNTFileName = "test.EVNT.pool.root"
        flags.Output.TXTFileName = ""
        flags.Input.Files = []
        setupPowhegFlags(flags)
        flags.lock()

        environment = dict(os.environ)
        with tempfile.TemporaryDirectory() as workdir:
            files_before = set(Path(workdir).iterdir())
            ca = PowhegCfg(
                flags,
                process="tt",
                settings={"hdamp": 258.75},
                working_directory=workdir,
            )
            files_after = set(Path(workdir).iterdir())

        service = ca.getService("PowhegGenerationSvc")
        self.assertEqual(service.OutputLHE, "PowhegOTF._1.events")
        self.assertEqual(files_after, files_before)
        self.assertEqual(dict(os.environ), environment)


if __name__ == "__main__":
    unittest.main()
