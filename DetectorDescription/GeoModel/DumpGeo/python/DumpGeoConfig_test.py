# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

"""Unit tests for the DumpGeo ComponentAccumulator configuration."""

import unittest
from unittest.mock import patch

from AthenaConfiguration.AthConfigFlags import AthConfigFlags

from DumpGeo.DumpGeoConfig import DumpGeoCfg
from DumpGeo.DumpGeoConfigFlags import createDumpGeoConfigFlags


class DumpGeoConfigTest(unittest.TestCase):
    """Characterization tests for DumpGeoCfg."""

    @staticmethod
    def _flags(*, output_file="", filters=None, force_overwrite=False,
               show_treetops=False):
        flags = AthConfigFlags()
        flags.addFlag("GeoModel.AtlasVersion", "ATLAS-UNIT-TEST-00-00-00")
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
        flags = self._flags()

        self.assertEqual(flags.GeoModel.DumpGeo.OutputFileName, "")
        self.assertEqual(flags.GeoModel.DumpGeo.FilterDetManagers, [])
        self.assertFalse(flags.GeoModel.DumpGeo.ForceOverwrite)
        self.assertFalse(flags.GeoModel.DumpGeo.ShowTreetopContent)

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

    def test_force_overwrite_removes_existing_file(self):
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
        remove.assert_called_once_with("existing.db")

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
