#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""Unit tests for OutputAnalysisConfig.splitBySelection: one tree per region,
each in its own stream and filled only for events passing that region.
"""

import unittest
from AnaAlgorithm.DualUseConfig import isAthena
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType
from AnalysisAlgorithmsConfig.ConfigSequence import ConfigSequence
from AsgAnalysisAlgorithms.OutputAnalysisConfig import OutputAnalysisConfig
from EventSelectionAlgorithms.EventSelectionConfig import EventSelectionConfig
from EventSelectionAlgorithms.EventSelectionConfigTestSupport import _new_accumulator, named, prop


def run_split(region_names, **options):
    algSeq, config = _new_accumulator(DataType.FullSim, [])
    config.setDefaultHistogramStream("HISTOGRAM")  # as in the runners, apart from the tree stream
    seq = ConfigSequence()
    for name in region_names:
        block = EventSelectionConfig()
        block.setOptionValue("selectionName", name)
        block.setOptionValue("selectionCuts", "RUN_NUMBER >= 0")
        seq.append(block)
    output = OutputAnalysisConfig()
    output.setOptionValue("vars", ["EventInfo.runNumber -> runNumber"])
    output.setOptionValue("splitBySelection", True)
    for opt, val in options.items():
        output.setOptionValue(opt, val)
    seq.append(output)
    seq.fullConfigure(config)
    if isAthena:
        algs = config.CA.getEventAlgos()
        config.CA.wasMerged()
        return algs, config.outputStreams()
    return list(algSeq), config.outputStreams()


class TestSplitOutput(unittest.TestCase):
    def test_one_tree_per_region(self):
        algs, streams = run_split(["SR", "CR", "SUB_hidden"])
        for region in ["SR", "CR"]:
            stream = f"ANALYSIS_{region}"
            for alg in named(algs, f"TreeMaker_{region}") + named(algs, f"NTupleMaker_{region}"):
                self.assertEqual(prop(alg, "RootStreamName"), stream)
            filler = named(algs, f"TreeFiller_{region}")
            self.assertEqual(len(filler), 1)
            self.assertEqual(prop(filler[0], "RootStreamName"), stream)
            self.assertEqual(prop(filler[0], "EventSelection"), f"pass_{region}_%SYS%,as_char")
            self.assertIn(stream, streams)                 # requested, so the runner declares it
        # no combined tree and no tree for SUB-regions
        self.assertEqual(len([a for a in algs if a.getType() == "CP::TreeMakerAlg"]), 2)
        self.assertNotIn("ANALYSIS", streams)              # so the runners write no tree.root

    def test_rntuple_one_ntuple_per_region(self):
        algs, streams = run_split(["SR", "CR"], outputFormat="RNTuple")
        self.assertEqual(len([a for a in algs if a.getType() == "CP::RNtupleTreeMakerAlg"]), 2)
        for region in ["SR", "CR"]:
            maker = named(algs, f"RNtupleMaker_{region}")
            self.assertEqual(len(maker), 1)
            self.assertEqual(prop(maker[0], "OutputStreamName"), f"ANALYSIS_{region}")
            self.assertEqual(prop(maker[0], "EventSelection"), f"pass_{region}_%SYS%,as_char")
            self.assertIn(f"ANALYSIS_{region}", streams)

    def test_single_region_is_unchanged(self):
        algs, streams = run_split(["SR", "SUB_hidden"])
        makers = [a for a in algs if a.getType() == "CP::TreeMakerAlg"]
        self.assertEqual(len(makers), 1)
        self.assertEqual(prop(makers[0], "RootStreamName"), "ANALYSIS")
        fillers = [a for a in algs if a.getType() == "CP::TreeFillerAlg"]
        self.assertEqual(len(fillers), 1)
        self.assertEqual(prop(fillers[0], "EventSelection", ""), "")
        self.assertIn("ANALYSIS", streams)
        self.assertNotIn("ANALYSIS_SR", streams)


if __name__ == "__main__":
    unittest.main()
