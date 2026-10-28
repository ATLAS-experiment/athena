# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from types import SimpleNamespace
import unittest

from AthenaConfiguration.Enums import ProductionStep
from EventSelectorAthenaPool.AthenaPoolAddressProviderSvcConfig import AthenaPoolAddressProviderSvcCfg
from EventSelectorAthenaPool.EventSelectorAthenaPoolConfig import EventSelectorAthenaPoolCfg


class EventSelectorConfigTest(unittest.TestCase):
    def setUp(self):
        self.flags = SimpleNamespace(
            Input=SimpleNamespace(
                Files=["primary.pool.root"], SecondaryFiles=[],
                OverrideRunNumber=False, RunAndLumiOverrideList=[],
                ConditionsRunNumber=0, RunNumbers=[123],
                LumiBlockNumbers=[4], TimeStamps=[567]),
            Exec=SimpleNamespace(SkipEvents=7),
            MP=SimpleNamespace(UseSharedReader=False, UseSharedWriter=False),
            Overlay=SimpleNamespace(SkipSecondaryEvents=-1, DataOverlay=False),
            Common=SimpleNamespace(
                isOverlay=False, ProductionStep=ProductionStep.Reconstruction),
            Sim=SimpleNamespace(
                DoFullChain=False, ISF=SimpleNamespace(ReSimulation=False)),
            IOVDb=SimpleNamespace(RunToTimestampDict={123: 567}))
        self.accumulators = []

    def tearDown(self):
        for acc in self.accumulators:
            acc.wasMerged()

    def selectors(self):
        acc = EventSelectorAthenaPoolCfg(self.flags)
        self.accumulators.append(acc)
        return acc

    def providers(self, **kwargs):
        acc = AthenaPoolAddressProviderSvcCfg(self.flags, **kwargs)
        self.accumulators.append(acc)
        return acc

    def test_primary_selector(self):
        for shared_reader, shared_writer in [(False, False), (False, True), (True, False)]:
            with self.subTest(shared_reader=shared_reader, shared_writer=shared_writer):
                self.flags.MP.UseSharedReader = shared_reader
                self.flags.MP.UseSharedWriter = shared_writer
                acc = self.selectors()
                selector = acc.getService("EventSelector")
                selector_type = ("EventSelectorAthenaPoolSharedIO" if shared_reader
                                 else "EventSelectorAthenaPool")
                self.assertEqual(selector.getFullJobOptName(), selector_type + "/EventSelector")
                self.assertEqual(selector.InputCollections, self.flags.Input.Files)
                self.assertEqual(selector.SkipEvents, 7)
                conversion = ("AthenaPoolSharedIOCnvSvc" if shared_reader or shared_writer
                              else "AthenaPoolCnvSvc")
                self.assertEqual(selector.ConversionService.name, conversion)
                self.assertEqual(len(acc.getServices()), 1)

    def test_secondary_selectors(self):
        self.flags.Input.SecondaryFiles = ["secondary.pool.root"]
        for skip in [-1, 0, 11]:
            for data_overlay in [False, True]:
                for shared_reader, shared_writer in [(False, False), (False, True), (True, False)]:
                    with self.subTest(skip=skip, data_overlay=data_overlay,
                                      shared_reader=shared_reader, shared_writer=shared_writer):
                        self.flags.Overlay.SkipSecondaryEvents = skip
                        self.flags.Common.isOverlay = data_overlay
                        self.flags.Overlay.DataOverlay = data_overlay
                        self.flags.MP.UseSharedReader = shared_reader
                        self.flags.MP.UseSharedWriter = shared_writer
                        acc = self.selectors()
                        primary = acc.getService("EventSelector")
                        secondary = acc.getService("SecondaryEventSelector")
                        self.assertEqual(primary.getFullJobOptName(),
                                         "DoubleEventSelectorAthenaPool/EventSelector")
                        self.assertEqual(primary.InputCollections, self.flags.Input.Files)
                        self.assertEqual(primary.SkipEvents, skip if skip >= 0 else 7)
                        self.assertFalse(primary.ProcessMetadata)
                        self.assertEqual(primary.OverrideRunNumber, data_overlay)
                        if data_overlay:
                            self.assertEqual(primary.RunNumber, 123)
                            self.assertEqual(primary.FirstLB, 4)
                            self.assertEqual(primary.InitialTimeStamp, 567)
                        self.assertEqual(secondary.getFullJobOptName(),
                                         "EventSelectorAthenaPool/SecondaryEventSelector")
                        self.assertEqual(secondary.InputCollections, self.flags.Input.SecondaryFiles)
                        self.assertEqual(secondary.SkipEvents, 7)
                        self.assertTrue(secondary.IsSecondary)
                        self.assertEqual(len(acc.getServices()), 2)

    def test_resimulation_override(self):
        self.flags.Common.ProductionStep = ProductionStep.Simulation
        self.flags.Sim.ISF.ReSimulation = True
        selector = self.selectors().getService("EventSelector")
        self.assertTrue(selector.OverrideRunNumber)
        self.assertEqual(selector.RunNumber, 123)
        self.assertEqual(selector.FirstLB, 4)
        self.assertEqual(selector.InitialTimeStamp, 567)

    def test_secondary_input_does_not_apply_primary_run_override(self):
        self.flags.Input.SecondaryFiles = ["secondary.pool.root"]
        self.flags.Input.OverrideRunNumber = True
        self.flags.Common.ProductionStep = ProductionStep.Simulation
        self.flags.Sim.ISF.ReSimulation = True
        acc = self.selectors()
        self.assertFalse(acc.getService("EventSelector").OverrideRunNumber)
        self.assertFalse(acc.getService("SecondaryEventSelector").OverrideRunNumber)
        self.assertEqual(len(acc.getServices()), 2)

    def test_primary_address_provider(self):
        acc = self.providers()
        provider = acc.getService("AthenaPoolAddressProviderSvc")
        self.assertEqual(list(acc.getService("ProxyProviderSvc").ProviderNames),
                         [provider.getFullJobOptName()])
        acc.getService("MetaDataStore")
        self.assertEqual(len(acc.getServices()), 3)

    def test_primary_address_provider_kwargs(self):
        acc = self.providers(name="CustomProvider", DataHeaderKey="CustomHeader")
        provider = acc.getService("CustomProvider")
        self.assertEqual(provider.DataHeaderKey, "CustomHeader")
        self.assertEqual(list(acc.getService("ProxyProviderSvc").ProviderNames),
                         [provider.getFullJobOptName()])

    def test_secondary_address_providers(self):
        self.flags.Input.SecondaryFiles = ["secondary.pool.root"]
        acc = self.providers()
        primary = acc.getService("AthenaPoolAddressProviderSvcPrimary")
        secondary = acc.getService("AthenaPoolAddressProviderSvcSecondary")
        self.assertEqual(primary.DataHeaderKey, "EventSelector")
        self.assertEqual(primary.AttributeListKey, "Input")
        self.assertEqual(secondary.DataHeaderKey, "SecondaryEventSelector")
        self.assertEqual(list(acc.getService("ProxyProviderSvc").ProviderNames),
                         [primary.getFullJobOptName(), secondary.getFullJobOptName()])
        acc.getService("MetaDataStore")
        self.assertEqual(len(acc.getServices()), 4)

    def test_secondary_address_provider_kwargs(self):
        self.flags.Input.SecondaryFiles = ["secondary.pool.root"]
        acc = self.providers(OutputLevel=3)
        self.assertEqual(acc.getService("AthenaPoolAddressProviderSvcPrimary").OutputLevel, 3)
        self.assertEqual(acc.getService("AthenaPoolAddressProviderSvcSecondary").OutputLevel, 3)

    def test_merging_helpers(self):
        for secondary in [False, True]:
            with self.subTest(secondary=secondary):
                self.flags.Input.SecondaryFiles = ["secondary.pool.root"] if secondary else []
                acc = self.providers()
                acc.merge(self.selectors())
                acc.getService("EventSelector")
                self.assertEqual(len(acc.getServices()), 6 if secondary else 4)
                expected_providers = 2 if secondary else 1
                self.assertEqual(len(acc.getService("ProxyProviderSvc").ProviderNames),
                                 expected_providers)


if __name__ == "__main__":
    unittest.main()
