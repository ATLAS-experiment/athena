# Copyright (C) 2026 CERN for the benefit of the ATLAS collaboration

"""Muon conversion selection must not require absent detector inputs."""

import unittest
from contextlib import ExitStack
from unittest.mock import patch

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from VP1Algs.VP1AlgConfig import MuonRdoToDigitCfg


class MuonRdoToDigitConfigTest(unittest.TestCase):
    converters = ("MdtRdoToMdtDigitCfg", "MM_RdoToDigitCfg", "STGC_RdoToDigitCfg",
                  "TgcRdoToTgcDigitCfg")
    rdos = ("MdtCsmContainer#MDTCSM", "Muon::MM_RawDataContainer#MMRDO",
            "Muon::STGC_RawDataContainer#sTGCRDO", "TgcRdoContainer#TGCRDO")
    digits = ("MdtDigitContainer#MDT_DIGITS", "MmDigitContainer#MM_DIGITS",
              "sTgcDigitContainer#sTGC_DIGITS", "TgcDigitContainer#TGC_DIGITS")

    def configure(self, collections, overlay=False):
        flags = initConfigFlags()
        flags.Input.Files = []
        flags.Input.TypedCollections = list(collections)
        flags.Common.isOverlay = overlay
        flags.Overlay.BkgPrefix = "Bkg_"
        flags.lock()
        with ExitStack() as stack:
            factories = []
            for name in self.converters:
                accumulator = ComponentAccumulator()
                self.addCleanup(accumulator.wasMerged)
                factories.append(stack.enter_context(patch(
                    "MuonConfig.MuonByteStreamCnvTestConfig." + name,
                    return_value=accumulator)))
            log = stack.enter_context(patch("AthenaCommon.Logging.logging.getLogger"))
            cfg = MuonRdoToDigitCfg(flags)
            self.addCleanup(cfg.wasMerged)
        return factories, log.return_value

    def test_hits_without_rdos_logs_debug_and_schedules_no_conversion(self):
        factories, log = self.configure(["MDTSimHitCollection#MDT_Hits"])
        for factory in factories:
            factory.assert_not_called()
        self.assertEqual(log.debug.call_count, 4)
        log.warning.assert_not_called()

    def test_empty_input_schedules_no_conversion(self):
        factories, _ = self.configure([])
        for factory in factories:
            factory.assert_not_called()

    def test_each_detector_is_independent(self):
        for detector in range(len(self.converters)):
            with self.subTest(detector=detector):
                factories, log = self.configure([self.rdos[detector]])
                for index, factory in enumerate(factories):
                    self.assertEqual(factory.call_count, int(index == detector))
                self.assertEqual(log.debug.call_count, 3)
                log.warning.assert_not_called()

    def test_all_rdos_schedule_all_converters(self):
        factories, log = self.configure(self.rdos)
        for factory in factories:
            factory.assert_called_once()
        log.warning.assert_not_called()

    def test_existing_digits_do_not_get_a_second_writer(self):
        factories, log = self.configure(self.rdos + self.digits)
        for factory in factories:
            factory.assert_not_called()
        log.warning.assert_not_called()

    def test_digits_without_rdos_are_usable(self):
        factories, log = self.configure(self.digits)
        for factory in factories:
            factory.assert_not_called()
        log.warning.assert_not_called()

    def test_unrelated_key_or_type_does_not_enable_conversion(self):
        factories, _ = self.configure(["OtherType#MDTCSM", "MdtCsmContainer#OtherKey"])
        for factory in factories:
            factory.assert_not_called()

    def test_overlay_uses_prefixed_keys(self):
        collections = [entry.replace("#", "#Bkg_") for entry in self.rdos]
        factories, log = self.configure(collections, overlay=True)
        for factory in factories:
            factory.assert_called_once()
        log.warning.assert_not_called()


if __name__ == "__main__":
    unittest.main()
