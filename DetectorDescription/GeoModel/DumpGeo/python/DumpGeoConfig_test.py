# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

"""Unit tests for the DumpGeo ComponentAccumulator configuration."""

import unittest
from types import SimpleNamespace
from unittest.mock import patch

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.ComponentAccumulator import ConfigurationError

from DumpGeo.DumpGeoConfig import (
    DumpGeoCfg,
    configureDumpGeoInputFlags,
    dumpGeoHasInputFiles,
    dumpGeoOutputFileName,
    logZDCFailureReminder,
    resolveDumpGeoGeometryTag,
    validateDumpGeoOutputFile,
)
from DumpGeo.DumpGeoConfigFlags import createDumpGeoConfigFlags


class DumpGeoConfigTest(unittest.TestCase):
    """Characterization tests for DumpGeoCfg."""

    @staticmethod
    def _flags(*, output_file="", filters=None, force_overwrite=False,
               show_treetops=False,
               atlas_version="ATLAS-UNIT-TEST-00-00-00"):
        flags = AthConfigFlags()
        flags.addFlag("GeoModel.AtlasVersion", atlas_version)
        createDumpGeoConfigFlags(flags)
        flags.GeoModel.DumpGeo.OutputFileName = output_file
        flags.GeoModel.DumpGeo.FilterDetManagers = filters or []
        flags.GeoModel.DumpGeo.ForceOverwrite = force_overwrite
        flags.GeoModel.DumpGeo.ShowTreetopContent = show_treetops
        flags.lock()
        return flags

    def _algorithm(self, flags, **kwargs):
        with patch("DumpGeo.DumpGeoConfig.os.path.exists", return_value=False):
            accumulator = DumpGeoCfg(flags, **kwargs)
        self.addCleanup(accumulator.wasMerged)
        return accumulator.getPrimary()

    def test_default_flags(self):
        flags = AthConfigFlags()
        createDumpGeoConfigFlags(flags)
        flags.lock()

        self.assertEqual(flags.GeoModel.DumpGeo.OutputFileName, "")
        self.assertEqual(flags.GeoModel.DumpGeo.FilterDetManagers, [])
        self.assertFalse(flags.GeoModel.DumpGeo.ForceOverwrite)
        self.assertFalse(flags.GeoModel.DumpGeo.ShowTreetopContent)

    def test_configuration_does_not_dump_flags_at_info_level(self):
        flags = self._flags()

        with (
            patch.object(AthConfigFlags, "dump") as dump,
            patch(
                "DumpGeo.DumpGeoConfig._logger.isEnabledFor",
                return_value=False,
            ),
        ):
            accumulator = DumpGeoCfg(flags)

        self.addCleanup(accumulator.wasMerged)
        dump.assert_not_called()

    def test_configuration_dumps_flags_at_debug_level(self):
        flags = self._flags()

        with (
            patch.object(AthConfigFlags, "dump") as dump,
            patch(
                "DumpGeo.DumpGeoConfig._logger.isEnabledFor",
                return_value=True,
            ),
        ):
            accumulator = DumpGeoCfg(flags)

        self.addCleanup(accumulator.wasMerged)
        dump.assert_called_once_with("GeoModel.DumpGeo")

    def test_automatic_output_filename(self):
        algorithm = self._algorithm(self._flags())

        self.assertEqual(
            algorithm.OutSQLiteFileName,
            "geometry-ATLAS-UNIT-TEST-00-00-00.db",
        )

    def test_automatic_filename_and_filter(self):
        algorithm = self._algorithm(
            self._flags(filters=["Pixel", "Tile"])
        )

        self.assertEqual(
            algorithm.OutSQLiteFileName,
            "geometry-ATLAS-UNIT-TEST-00-00-00-Pixel-Tile.db",
        )
        self.assertEqual(algorithm.UserFilterDetManager, ["Pixel", "Tile"])

    def test_custom_output_filename(self):
        algorithm = self._algorithm(
            self._flags(output_file="custom-geometry.db")
        )

        self.assertEqual(algorithm.OutSQLiteFileName, "custom-geometry.db")

    def test_show_treetop_content(self):
        algorithm = self._algorithm(self._flags(show_treetops=True))

        self.assertTrue(algorithm.ShowTreetopContent)

    def test_explicit_properties_override_flags(self):
        algorithm = self._algorithm(
            self._flags(show_treetops=True),
            ShowTreetopContent=False,
            OutSQLiteFileName="from-kwargs.db",
        )

        self.assertFalse(algorithm.ShowTreetopContent)
        self.assertEqual(algorithm.OutSQLiteFileName, "from-kwargs.db")

    def test_explicit_output_filename_is_validated(self):
        flags = self._flags(output_file="from-flags.db")

        with patch(
            "DumpGeo.DumpGeoConfig.os.path.exists",
            side_effect=lambda path: path == "from-kwargs.db",
        ):
            with self.assertRaisesRegex(ConfigurationError, "from-kwargs.db"):
                DumpGeoCfg(flags, OutSQLiteFileName="from-kwargs.db")

    def test_force_overwrite_configures_explicit_output_filename(self):
        flags = self._flags(
            output_file="from-flags.db",
            force_overwrite=True,
        )

        with (
            patch(
                "DumpGeo.DumpGeoConfig.os.path.exists",
                side_effect=lambda path: path == "from-kwargs.db",
            ),
            patch("DumpGeo.DumpGeoConfig.os.remove") as remove,
        ):
            accumulator = DumpGeoCfg(
                flags,
                OutSQLiteFileName="from-kwargs.db",
            )

        self.addCleanup(accumulator.wasMerged)
        algorithm = accumulator.getPrimary()
        self.assertEqual(algorithm.OutSQLiteFileName, "from-kwargs.db")
        self.assertTrue(algorithm.ForceOverwrite)
        remove.assert_not_called()

    def test_force_overwrite_does_not_delete_during_configuration(self):
        flags = self._flags(
            output_file="existing.db",
            force_overwrite=True,
        )

        with (
            patch("DumpGeo.DumpGeoConfig.os.path.exists", return_value=True),
            patch("DumpGeo.DumpGeoConfig.os.remove") as remove,
        ):
            accumulator = DumpGeoCfg(flags)

        self.addCleanup(accumulator.wasMerged)
        self.assertTrue(accumulator.getPrimary().ForceOverwrite)
        remove.assert_not_called()

    def test_explicit_force_overwrite_property_is_authoritative(self):
        flags = self._flags(
            output_file="existing.db",
            force_overwrite=False,
        )

        with (
            patch("DumpGeo.DumpGeoConfig.os.path.exists", return_value=True),
            patch("DumpGeo.DumpGeoConfig.os.remove") as remove,
        ):
            accumulator = DumpGeoCfg(flags, ForceOverwrite=True)

        self.addCleanup(accumulator.wasMerged)
        self.assertTrue(accumulator.getPrimary().ForceOverwrite)
        remove.assert_not_called()

    def test_existing_file_without_force_raises(self):
        flags = self._flags(output_file="existing.db")

        with (
            patch("DumpGeo.DumpGeoConfig.os.path.exists", return_value=True),
            patch("DumpGeo.DumpGeoConfig.os.remove") as remove,
        ):
            with self.assertRaisesRegex(ConfigurationError, "existing.db"):
                DumpGeoCfg(flags)

        remove.assert_not_called()

    def test_force_overwrite_preflight_does_not_remove_existing_file(self):
        with (
            patch("DumpGeo.DumpGeoConfig.os.path.exists", return_value=True),
            patch("DumpGeo.DumpGeoConfig.os.remove") as remove,
        ):
            validateDumpGeoOutputFile("existing.db", force_overwrite=True)

        remove.assert_not_called()

    def test_output_filename_helper(self):
        flags = self._flags(filters=["Pixel", "Tile"])

        self.assertEqual(
            dumpGeoOutputFileName(flags),
            "geometry-ATLAS-UNIT-TEST-00-00-00-Pixel-Tile.db",
        )

    def test_detdescr_overrides_generic_geometry_flag(self):
        self.assertEqual(
            resolveDumpGeoGeometryTag(
                "ALIAS-TAG",
                "GENERIC-TAG",
                "FALLBACK-TAG",
            ),
            "ALIAS-TAG",
        )

    def test_geometry_tag_is_preserved_without_detdescr(self):
        self.assertEqual(
            resolveDumpGeoGeometryTag(
                None,
                "GEOMETRY-TAG",
                "FALLBACK-TAG",
            ),
            "GEOMETRY-TAG",
        )

    def test_geometry_fallback_is_used_when_no_tag_is_available(self):
        self.assertEqual(
            resolveDumpGeoGeometryTag(None, None, "FALLBACK-TAG"),
            "FALLBACK-TAG",
        )

    def test_input_file_detection(self):
        """Only actual filenames enable input metadata configuration."""
        self.assertFalse(dumpGeoHasInputFiles([]))
        self.assertFalse(
            dumpGeoHasInputFiles(["_ATHENA_GENERIC_INPUTFILE_NAME_"])
        )
        self.assertTrue(dumpGeoHasInputFiles(["events.pool.root"]))

    def test_inputless_flags_are_initialized_without_an_event_file(self):
        flags = SimpleNamespace(
            Input=SimpleNamespace(
                Files=["_ATHENA_GENERIC_INPUTFILE_NAME_"]
            ),
            IOVDb=SimpleNamespace(),
        )

        self.assertTrue(configureDumpGeoInputFlags(flags))
        self.assertEqual(flags.Input.Files, [])
        self.assertEqual(flags.Input.RunNumbers, [330000])
        self.assertEqual(flags.Input.TimeStamps, [1])
        self.assertEqual(flags.Input.TypedCollections, [])
        self.assertTrue(flags.Input.isMC)

    def test_real_input_files_are_preserved(self):
        flags = SimpleNamespace(
            Input=SimpleNamespace(Files=["events.pool.root"]),
            IOVDb=SimpleNamespace(),
        )

        self.assertFalse(configureDumpGeoInputFlags(flags))
        self.assertEqual(flags.Input.Files, ["events.pool.root"])

    def test_resolved_geometry_tag_is_used_in_output_filename(self):
        geometry_tag = resolveDumpGeoGeometryTag(
            None,
            "METADATA-TAG",
            "FALLBACK-TAG",
        )
        flags = self._flags(atlas_version=geometry_tag)

        self.assertEqual(
            dumpGeoOutputFileName(flags),
            "geometry-METADATA-TAG.db",
        )

    def test_zdc_failure_reminder_is_logged(self):
        with patch("DumpGeo.DumpGeoConfig._logger.error") as logger_error:
            logZDCFailureReminder(zdc_enabled=True, run_succeeded=False)

        logger_error.assert_called_once()
        self.assertIn("ZDC geometry was enabled", logger_error.call_args[0][0])

    def test_zdc_failure_reminder_is_suppressed_when_not_applicable(self):
        with patch("DumpGeo.DumpGeoConfig._logger.error") as logger_error:
            logZDCFailureReminder(zdc_enabled=False, run_succeeded=False)
            logZDCFailureReminder(zdc_enabled=True, run_succeeded=True)

        logger_error.assert_not_called()

    def test_custom_filename_and_filter(self):
        """A custom filename must not disable DetectorManager filtering."""
        algorithm = self._algorithm(
            self._flags(
                output_file="filtered.db",
                filters=["Pixel", "Tile"],
            )
        )

        self.assertEqual(algorithm.UserFilterDetManager, ["Pixel", "Tile"])

    def test_algorithm_name(self):
        """DumpGeoCfg passes its name argument to the algorithm."""
        algorithm = self._algorithm(self._flags(), name="RequestedDumpGeo")

        self.assertEqual(algorithm.name, "RequestedDumpGeo")


if __name__ == "__main__":
    unittest.main()
