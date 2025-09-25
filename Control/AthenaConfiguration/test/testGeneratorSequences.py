#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# self test of GeneratorAccumulator

import unittest

from AthenaCommon.CFElements import findSubSequence, findAlgorithm
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
        ca1 = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.Post))
        ca1.addEventAlgo(TestAlgo("PostAlgo1"))

        ca = MainEvgenServicesCfg(self.flags, withSequences=True)
        ca.merge(ca1, sequenceName=EvgenSequence.PreFilter.value)

        self.assertIsNotNone(findAlgorithm(ca.getSequence(EvgenSequence.PreFilter.value), "PostAlgo1"), "Algorithm not placed in sub-sequence")
        self.assertIsNotNone(findAlgorithm(ca.getSequence(), "PostAlgo1", 6), "Algorithm not placed at the right depth")

        ca.printConfig(prefix="test_override")

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


if __name__ == "__main__":
    unittest.main()
