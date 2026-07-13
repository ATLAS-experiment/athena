#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# self test of GeneratorAccumulator

import unittest

from AthenaCommon.CFElements import findSubSequence, findAlgorithm, parAND, parOR
from AthenaCommon.Constants import DEBUG
from AthenaCommon.Logging import log
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.MainServicesConfig import MainEvgenServicesCfg, addEvgenSequences
from GeneratorConfig.Sequences import EvgenSequence, EvgenSequenceFactory

TestAlgo = CompFactory.HelloAlg

ComponentAccumulator.debugMode="trackCA trackEventAlgo"


class TestGeneratorSequences(unittest.TestCase):
    def setUp(self):
        log.setLevel(DEBUG)

        self.flags = initConfigFlags()
        self.flags.Input.Files = []
        self.flags.Input.RunNumbers = [284500] # Set to either MC DSID or MC Run Number
        self.flags.Input.TimeStamps = [1] # dummy value
        self.flags.lock()

    def test_main(self):
        ca1 = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.Post))
        ca1.addEventAlgo(TestAlgo("PostAlgo1"))
        ca1.addEventAlgo(TestAlgo("PostAlgo2"), sequenceName=EvgenSequence.Post.value)

        self.assertIsNotNone(findAlgorithm(ca1.getSequence(EvgenSequence.Post.value), "PostAlgo2"), "Algorithm not placed in sub-sequence")

        from AthenaConfiguration.ComponentAccumulator import ConfigurationError
        self.assertRaises(ConfigurationError, lambda: ca1.addEventAlgo(TestAlgo("PostAlgo3", MyInt=6), sequenceName=EvgenSequence.Filter.value))

        ca = MainEvgenServicesCfg(self.flags, withSequences=True)
        ca.merge(ca1)

        self.assertIsNotNone(findAlgorithm(ca.getSequence(EvgenSequence.Post.value), "PostAlgo2"), "Algorithm not placed in sub-sequence")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(), "PostAlgo2", 6), "Algorithm not placed at the right depth")
        self.assertIsNotNone(findSubSequence(ca.getSequence(), EvgenSequence.Post.value), "The sequence is not added")

        ca.addEventAlgo(TestAlgo("PostAlgo3"), sequenceName=EvgenSequence.Post.value)
        self.assertIsNotNone(findAlgorithm(ca.getSequence(), "PostAlgo3", 6), "Algorithm not added at the end")

        ca.printConfig(prefix="test_main")

        ca.wasMerged()

    def test_override(self):
        """Testing merging one generator sequence into a different generator sequence"""
        ca1 = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.Post))
        ca1.addEventAlgo(TestAlgo("PostAlgo1"))

        ca = MainEvgenServicesCfg(self.flags, withSequences=True)
        ca.merge(ca1, sequenceName=EvgenSequence.PreFilter.value)

        self.assertIsNotNone(findAlgorithm(ca.getSequence(EvgenSequence.PreFilter.value), "PostAlgo1"), "Algorithm not placed in sub-sequence")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(), "PostAlgo1", 6), "Algorithm not placed at the right depth")

        ca.printConfig(prefix="test_override")

        ca.wasMerged()

    def test_override_plain(self):
        """Testing merging a generic ComponentAccumulator into the main one, with a sequence name override"""
        ca1 = ComponentAccumulator()
        ca1.addEventAlgo(TestAlgo("GenericAlgo1"))

        ca = MainEvgenServicesCfg(self.flags, withSequences=True)
        ca.merge(ca1, sequenceName=EvgenSequence.PreFilter.value)

        self.assertIsNotNone(findAlgorithm(ca.getSequence(EvgenSequence.PreFilter.value), "GenericAlgo1"), "Algorithm not placed in sub-sequence")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(), "GenericAlgo1", 6), "Algorithm not placed at the right depth")

        ca.printConfig(prefix="test_override_plain")

        ca.wasMerged()

    def test_complex_fragment(self):
        ca1 = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.Generator))
        ca1.addEventAlgo(TestAlgo("GeneratorAlgo1"))

        ca2 = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.Post))
        ca2.addEventAlgo(TestAlgo("PostAlgo1"))

        ca = ComponentAccumulator()
        addEvgenSequences(self.flags, ca)

        ca.merge(ca1)
        ca.merge(ca2)

        self.assertIsNotNone(findAlgorithm(ca.getSequence(EvgenSequence.Generator.value), "GeneratorAlgo1"), "Algorithm not placed in sub-sequence")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(), "GeneratorAlgo1", 3), "Algorithm not placed at the right depth")

        self.assertIsNotNone(findAlgorithm(ca.getSequence(EvgenSequence.Post.value), "PostAlgo1"), "Algorithm not placed in sub-sequence")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(), "PostAlgo1", 3), "Algorithm not placed at the right depth")

        ca.printConfig(prefix="test_complex_fragment")

        ca_main = MainEvgenServicesCfg(self.flags, withSequences=True)
        ca_main.merge(ca)

        self.assertIsNotNone(findAlgorithm(ca_main.getSequence(), "GeneratorAlgo1", 6), "Algorithm not placed at the right depth")
        self.assertIsNotNone(findAlgorithm(ca_main.getSequence(), "PostAlgo1", 6), "Algorithm not placed at the right depth")

        ca_main.printConfig(prefix="test_complex_fragment_main")

        ca_main.wasMerged()

        ca_plain = ComponentAccumulator()
        ca_plain.merge(ca1)
        ca_plain.merge(ca2)

        ca_plain.printConfig(prefix="test_complex_fragment_plain")

        ca_main_plain = MainEvgenServicesCfg(self.flags, withSequences=True)
        ca_main_plain.merge(ca_plain)

        self.assertIsNotNone(findAlgorithm(ca_main_plain.getSequence(), "GeneratorAlgo1", 6), "Algorithm not placed at the right depth")
        self.assertIsNotNone(findAlgorithm(ca_main_plain.getSequence(), "PostAlgo1", 6), "Algorithm not placed at the right depth")

        ca_main_plain.printConfig(prefix="test_complex_fragment_plain_merged")

        ca_main_plain.wasMerged()

    def test_filters(self):
        """Testing filters setup"""
        caPreFilter1 = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.PreFilter))
        caPreFilter1.addEventAlgo(TestAlgo("PreFilterAlgo1"))

        caFilter1 = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.Filter))
        caFilter1.merge(caPreFilter1)
        caFilter1.addEventAlgo(TestAlgo("FilterAlgo1"))

        caPreFilter2 = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.PreFilter))
        caPreFilter2.addEventAlgo(TestAlgo("PreFilterAlgo2"))

        caFilter2 = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.Filter))
        caFilter2.merge(caPreFilter2)
        caFilter2.addEventAlgo(TestAlgo("FilterAlgo2"))

        caPreFilter3 = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.PreFilter))
        caPreFilter3.addEventAlgo(TestAlgo("PreFilterAlgo3"))

        caFilter3 = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.Filter))
        caFilter3.merge(caPreFilter3)
        caFilter3.addEventAlgo(TestAlgo("FilterAlgo3"))

        caPreFilter4 = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.PreFilter))
        caPreFilter4.addEventAlgo(TestAlgo("PreFilterAlgo4"))

        caFilter4 = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.Filter))
        caFilter4.merge(caPreFilter4)
        caFilter4.addEventAlgo(TestAlgo("FilterAlgo4"))

        caPreFilter5 = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.PreFilter))
        caPreFilter5.addEventAlgo(TestAlgo("PreFilterAlgo5"))

        caFilter5 = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.Filter))
        caFilter5.merge(caPreFilter5)
        caFilter5.addEventAlgo(TestAlgo("FilterAlgo5"))

        # Simple example
        ca = MainEvgenServicesCfg(self.flags, withSequences=True)
        ca.merge(caFilter1)
        ca.merge(caFilter2)

        self.assertIsNotNone(findAlgorithm(ca.getSequence(EvgenSequence.PreFilter.value), "PreFilterAlgo1"), "Algorithm not placed in sub-sequence")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(EvgenSequence.PreFilter.value), "PreFilterAlgo2"), "Algorithm not placed in sub-sequence")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(), "PreFilterAlgo1", 6), "Algorithm not placed at the right depth")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(), "PreFilterAlgo2", 6), "Algorithm not placed at the right depth")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(EvgenSequence.Filter.value), "FilterAlgo1"), "Algorithm not placed in sub-sequence")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(EvgenSequence.Filter.value), "FilterAlgo2"), "Algorithm not placed in sub-sequence")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(), "FilterAlgo1", 6), "Algorithm not placed at the right depth")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(), "FilterAlgo2", 6), "Algorithm not placed at the right depth")

        ca.printConfig(prefix="test_filter_simple")
        ca.wasMerged()

        # Complex example 1
        ca = MainEvgenServicesCfg(self.flags, withSequences=True)

        caOR = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.Filter))
        caOR.addSequence(parOR("FilterOR"))
        caOR.merge(caFilter1, sequenceName="FilterOR")
        caOR.merge(caFilter2, sequenceName="FilterOR")

        ca.merge(caOR)

        self.assertIsNotNone(findAlgorithm(ca.getSequence(EvgenSequence.PreFilter.value), "PreFilterAlgo1"), "Algorithm not placed in sub-sequence")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(EvgenSequence.PreFilter.value), "PreFilterAlgo2"), "Algorithm not placed in sub-sequence")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(), "PreFilterAlgo1", 6), "Algorithm not placed at the right depth")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(), "PreFilterAlgo2", 6), "Algorithm not placed at the right depth")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(EvgenSequence.Filter.value), "FilterAlgo1"), "Algorithm not placed in sub-sequence")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(EvgenSequence.Filter.value), "FilterAlgo2"), "Algorithm not placed in sub-sequence")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(), "FilterAlgo1", 7), "Algorithm not placed at the right depth")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(), "FilterAlgo2", 7), "Algorithm not placed at the right depth")

        ca.printConfig(prefix="test_filter_complex1")
        ca.wasMerged()

        # Complex example 2: ((1 || 2) && 3) || 4
        ca = MainEvgenServicesCfg(self.flags, withSequences=True)

        caOR = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.Filter))
        caOR.addSequence(parOR("FilterOR_123vs4"))
        caOR.addSequence(parAND("FilterAND_12and3"), parentName="FilterOR_123vs4")
        caOR.addSequence(parOR("FilterOR_12"), parentName="FilterAND_12and3")
        caOR.merge(caFilter1, sequenceName="FilterOR_12")
        caOR.merge(caFilter2, sequenceName="FilterOR_12")
        caOR.merge(caFilter3, sequenceName="FilterAND_12and3")
        caOR.merge(caFilter4, sequenceName="FilterOR_123vs4")

        ca.merge(caOR)

        self.assertIsNotNone(findAlgorithm(ca.getSequence(EvgenSequence.PreFilter.value), "PreFilterAlgo1"), "Algorithm not placed in sub-sequence")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(EvgenSequence.PreFilter.value), "PreFilterAlgo2"), "Algorithm not placed in sub-sequence")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(EvgenSequence.PreFilter.value), "PreFilterAlgo3"), "Algorithm not placed in sub-sequence")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(EvgenSequence.PreFilter.value), "PreFilterAlgo4"), "Algorithm not placed in sub-sequence")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(), "PreFilterAlgo1", 6), "Algorithm not placed at the right depth")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(), "PreFilterAlgo2", 6), "Algorithm not placed at the right depth")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(), "PreFilterAlgo3", 6), "Algorithm not placed at the right depth")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(), "PreFilterAlgo4", 6), "Algorithm not placed at the right depth")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(EvgenSequence.Filter.value), "FilterAlgo1"), "Algorithm not placed in sub-sequence")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(EvgenSequence.Filter.value), "FilterAlgo2"), "Algorithm not placed in sub-sequence")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(EvgenSequence.Filter.value), "FilterAlgo3"), "Algorithm not placed in sub-sequence")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(EvgenSequence.Filter.value), "FilterAlgo4"), "Algorithm not placed in sub-sequence")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(), "FilterAlgo1", 9), "Algorithm not placed at the right depth")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(), "FilterAlgo2", 9), "Algorithm not placed at the right depth")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(), "FilterAlgo3", 8), "Algorithm not placed at the right depth")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(), "FilterAlgo4", 7), "Algorithm not placed at the right depth")

        ca.printConfig(prefix="test_filter_complex2")
        ca.wasMerged()

        # Complex example 3: ((1 && 2) || (3 && 4)) && 5
        ca = MainEvgenServicesCfg(self.flags, withSequences=True)

        caOR = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.Filter))
        caOR.addSequence(parOR("FilterOR_12vs34"))
        caOR.addSequence(parAND("FilterAND_12"), parentName="FilterOR_12vs34")
        caOR.addSequence(parAND("FilterAND_34"), parentName="FilterOR_12vs34")
        caOR.merge(caFilter1, sequenceName="FilterAND_12")
        caOR.merge(caFilter2, sequenceName="FilterAND_12")
        caOR.merge(caFilter3, sequenceName="FilterAND_34")
        caOR.merge(caFilter4, sequenceName="FilterAND_34")
        caOR.merge(caFilter5)

        ca.merge(caOR)

        self.assertIsNotNone(findAlgorithm(ca.getSequence(EvgenSequence.PreFilter.value), "PreFilterAlgo1"), "Algorithm not placed in sub-sequence")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(EvgenSequence.PreFilter.value), "PreFilterAlgo2"), "Algorithm not placed in sub-sequence")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(EvgenSequence.PreFilter.value), "PreFilterAlgo3"), "Algorithm not placed in sub-sequence")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(EvgenSequence.PreFilter.value), "PreFilterAlgo4"), "Algorithm not placed in sub-sequence")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(EvgenSequence.PreFilter.value), "PreFilterAlgo5"), "Algorithm not placed in sub-sequence")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(), "PreFilterAlgo1", 6), "Algorithm not placed at the right depth")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(), "PreFilterAlgo2", 6), "Algorithm not placed at the right depth")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(), "PreFilterAlgo3", 6), "Algorithm not placed at the right depth")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(), "PreFilterAlgo4", 6), "Algorithm not placed at the right depth")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(), "PreFilterAlgo5", 6), "Algorithm not placed at the right depth")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(EvgenSequence.Filter.value), "FilterAlgo1"), "Algorithm not placed in sub-sequence")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(EvgenSequence.Filter.value), "FilterAlgo2"), "Algorithm not placed in sub-sequence")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(EvgenSequence.Filter.value), "FilterAlgo3"), "Algorithm not placed in sub-sequence")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(EvgenSequence.Filter.value), "FilterAlgo4"), "Algorithm not placed in sub-sequence")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(EvgenSequence.Filter.value), "FilterAlgo5"), "Algorithm not placed in sub-sequence")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(), "FilterAlgo1", 8), "Algorithm not placed at the right depth")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(), "FilterAlgo2", 8), "Algorithm not placed at the right depth")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(), "FilterAlgo3", 8), "Algorithm not placed at the right depth")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(), "FilterAlgo4", 8), "Algorithm not placed at the right depth")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(), "FilterAlgo5", 6), "Algorithm not placed at the right depth")

        ca.printConfig(prefix="test_filter_complex3")
        ca.wasMerged()



if __name__ == "__main__":
    unittest.main()
