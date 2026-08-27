#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

import unittest

# Always import this to initialize the semantics registry.
from AthenaConfiguration import AtlasSemantics
from GaudiConfig2._configurables import Configurable, Property
from GeneratorConfig.GeneratorSettingsSemantics import (
    GeneratorSettingsKeep,
    GeneratorSettingsLayer,
    GeneratorSettingsPrecedence,
    GeneratorSettingsSemantics,
)


class GeneratorSettingsTestAlg(Configurable):
    __cpp_type__ = "GeneratorSettingsTestAlg"
    __component_type__ = "Algorithm"
    Commands = Property(
        "std::vector<std::string>",
        [],
        semantics="GeneratorSettings<std::string>",
    )


class TestGeneratorSettingsSemantics(unittest.TestCase):
    """Test generator settings property semantics"""

    semantics = GeneratorSettingsSemantics("GeneratorSettings<std::string>")

    def _layer(self, source, values, precedence, separators="=",
               keep=GeneratorSettingsKeep.LAST):
        return GeneratorSettingsLayer(
            source=source,
            values=tuple(values),
            precedence=precedence,
            separators=separators,
            keep=keep,
            report_context="TestGeneratorSettings.Commands",
        )

    def test_semantics(self):
        alg = GeneratorSettingsTestAlg()
        self.assertIsInstance(
            alg._descriptors["Commands"].semantics,
            GeneratorSettingsSemantics,
        )

    def test_keep_last_overrides_same_setting_key(self):
        result = self.semantics.merge(
            self._layer(
                "base",
                ["Main:timesAllowErrors = 500", "ParticleDecays:limitTau0 = on"],
                GeneratorSettingsPrecedence.BASE,
            ),
            self._layer(
                "user",
                ["Main:timesAllowErrors = 60000"],
                GeneratorSettingsPrecedence.USER,
            ),
        )

        self.assertEqual(
            result.data,
            ["ParticleDecays:limitTau0 = on", "Main:timesAllowErrors = 60000"],
        )

    def test_process_settings_override_tune_and_are_overridden_by_user(self):
        result = self.semantics.merge(
            self._layer(
                "tune",
                ["HardQCD:all = off"],
                GeneratorSettingsPrecedence.TUNE,
            ),
            self._layer(
                "process",
                ["HardQCD:all = on"],
                GeneratorSettingsPrecedence.PROCESS,
            ),
        )
        result = self.semantics.merge(
            result,
            self._layer(
                "user",
                ["HardQCD:all = off"],
                GeneratorSettingsPrecedence.USER,
            ),
        )

        self.assertEqual(result.data, ["HardQCD:all = off"])

    def test_identical_assignment_with_different_spacing_is_deduplicated(self):
        result = self.semantics.merge(
            self._layer(
                "tune",
                ["PDF:pSet = LHAPDF6:MSTW2008lo68cl"],
                GeneratorSettingsPrecedence.TUNE,
            ),
            self._layer(
                "user",
                ["PDF:pSet=LHAPDF6:MSTW2008lo68cl"],
                GeneratorSettingsPrecedence.USER,
            ),
        )

        self.assertEqual(result.data, ["PDF:pSet=LHAPDF6:MSTW2008lo68cl"])

    def test_keep_first_keeps_lower_precedence_value_when_requested(self):
        result = self.semantics.merge(
            self._layer(
                "base",
                ["Main:timesAllowErrors = 500"],
                GeneratorSettingsPrecedence.BASE,
                keep=GeneratorSettingsKeep.FIRST,
            ),
            self._layer(
                "user",
                ["Main:timesAllowErrors = 60000"],
                GeneratorSettingsPrecedence.USER,
                keep=GeneratorSettingsKeep.FIRST,
            ),
        )

        self.assertEqual(result.data, ["Main:timesAllowErrors = 500"])

    def test_custom_separator_is_used_for_deduplication(self):
        result = self.semantics.merge(
            self._layer(
                "base",
                ["beamEnergy: 6500"],
                GeneratorSettingsPrecedence.BASE,
                separators=":",
            ),
            self._layer(
                "user",
                ["beamEnergy: 6800"],
                GeneratorSettingsPrecedence.USER,
                separators=":",
            ),
        )

        self.assertEqual(result.data, ["beamEnergy: 6800"])

    def test_different_parsing_settings_are_rejected(self):
        result = self.semantics.merge(
            self._layer(
                "base",
                ["beamEnergy = 6500"],
                GeneratorSettingsPrecedence.BASE,
                separators="=",
            ),
            self._layer(
                "user",
                ["beamEnergy: 6800"],
                GeneratorSettingsPrecedence.USER,
                separators=":",
            ),
        )

        with self.assertRaisesRegex(ValueError, "different parsing settings"):
            result.data

    def test_unparsed_commands_are_deduplicated_by_full_text(self):
        result = self.semantics.merge(
            self._layer(
                "base",
                ["doSomething   withoutSeparator"],
                GeneratorSettingsPrecedence.BASE,
            ),
            self._layer(
                "user",
                ["doSomething withoutSeparator"],
                GeneratorSettingsPrecedence.USER,
            ),
        )

        self.assertEqual(result.data, ["doSomething withoutSeparator"])

    def test_duplicate_precedence_is_rejected(self):
        result = self.semantics.merge(
            self._layer(
                "base_a",
                ["Main:timesAllowErrors = 500"],
                GeneratorSettingsPrecedence.BASE,
            ),
            self._layer(
                "base_b",
                ["ParticleDecays:limitTau0 = on"],
                GeneratorSettingsPrecedence.BASE,
            ),
        )

        with self.assertRaisesRegex(ValueError, "duplicate precedence BASE"):
            result.data

    def test_string_conversion_returns_final_command_list(self):
        result = self.semantics.merge(
            self._layer(
                "base",
                ["Main:timesAllowErrors = 500", "ParticleDecays:limitTau0 = on"],
                GeneratorSettingsPrecedence.BASE,
            ),
            self._layer(
                "user",
                ["Main:timesAllowErrors = 60000"],
                GeneratorSettingsPrecedence.USER,
            ),
        )

        with self.assertLogs("GeneratorSettingsSemantics", level="WARNING") as logs:
            result_string = str(result)

        self.assertEqual(
            result_string,
            "['ParticleDecays:limitTau0 = on', 'Main:timesAllowErrors = 60000']",
        )
        self.assertTrue(any(
            "Potential issue with generator settings" in message
            for message in logs.output
        ))
        self.assertTrue(any(
            "Main:timesAllowErrors" in message
            for message in logs.output
        ))
        self.assertTrue(any(
            "conflicting setting 'Main:timesAllowErrors' across sources "
            "[base, user] -> [500, 60000 (kept)]" in message
            for message in logs.output
        ))

    def test_configurable_merge_is_idempotent(self):
        base = GeneratorSettingsTestAlg(
            "Pythia8_i",
            Commands=self._layer(
                "base",
                ["Main:timesAllowErrors = 500"],
                GeneratorSettingsPrecedence.BASE,
            ),
        )
        user = GeneratorSettingsTestAlg(
            "Pythia8_i",
            Commands=self._layer(
                "user",
                ["Main:timesAllowErrors = 60000"],
                GeneratorSettingsPrecedence.USER,
            ),
        )

        base.merge(user)
        user.merge(base)

        self.assertEqual(base.Commands.data, ["Main:timesAllowErrors = 60000"])
        self.assertEqual(user.Commands.data, ["Main:timesAllowErrors = 60000"])
        self.assertEqual(len(base.Commands.layers), 2)
        self.assertEqual(len(user.Commands.layers), 2)

    def test_report_keeps_duplicate_and_conflict_information(self):
        result = self.semantics.merge(
            self._layer(
                "base",
                ["PDF:pSet = LHAPDF6:MSTW2008lo68cl", "Main:timesAllowErrors = 500"],
                GeneratorSettingsPrecedence.BASE,
            ),
            self._layer(
                "user",
                ["PDF:pSet=LHAPDF6:MSTW2008lo68cl", "Main:timesAllowErrors = 60000"],
                GeneratorSettingsPrecedence.USER,
            ),
        )
        _, report = result._resolve()

        self.assertEqual(
            report["duplicates_across_sources"],
            [{"source": "base", "duplicate_of_source": "user", "count": 1}],
        )
        self.assertEqual(
            [entry["key"] for entry in report["conflicts"]],
            ["Main:timesAllowErrors"],
        )

    def test_logs_include_duplicate_setting_details(self):
        result = self.semantics.merge(
            self._layer(
                "tune",
                ["PDF:pSet = LHAPDF6:MSTW2008lo68cl"],
                GeneratorSettingsPrecedence.TUNE,
            ),
            self._layer(
                "user",
                ["PDF:pSet=LHAPDF6:MSTW2008lo68cl"],
                GeneratorSettingsPrecedence.USER,
            ),
        )

        with self.assertLogs("GeneratorSettingsSemantics", level="WARNING") as logs:
            _ = str(result)

        self.assertTrue(any(
            "duplicate setting from sources [tune, user]" in message
            for message in logs.output
        ))
        self.assertTrue(any(
            "PDF:pSet = LHAPDF6:MSTW2008lo68cl" in message
            for message in logs.output
        ))


if __name__ == "__main__":
    unittest.main()
