#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

import unittest

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from PyJobTransforms.TransformUtils import processPreExec, processPostExec
from PyJobTransforms.trfJobOptions import RunArguments


class TestTransformUtils(unittest.TestCase):
    """Tests for TransformUtils"""
    
    def test_preExecPlain(self):
        runArgs = RunArguments()
        runArgs.preExec = ['flags.Input.JobNumber = 5']
        flags = initConfigFlags()
        processPreExec(runArgs, flags)
        self.assertEqual(flags.Input.JobNumber, 5)

    def test_preExecPlainCapitalised(self):
        runArgs = RunArguments()
        runArgs.preExec = ['ConfigFlags.Input.JobNumber = 5']
        flags = initConfigFlags()
        processPreExec(runArgs, flags)
        self.assertEqual(flags.Input.JobNumber, 5)

    def test_postExecPlain(self):
        runArgs = RunArguments()
        runArgs.postExec = ['cfg.getEventAlgo("HelloAlg").MyInt = flags.Input.JobNumber']
        flags = initConfigFlags()
        flags.lock()
        cfg = ComponentAccumulator()
        cfg.addEventAlgo(CompFactory.HelloAlg(MyInt=123))
        processPostExec(runArgs, flags, cfg)
        self.assertEqual(cfg.getEventAlgo('HelloAlg').MyInt, 1)
        cfg.wasMerged()

    def test_postExecPlainCapitalised(self):
        runArgs = RunArguments()
        runArgs.postExec = ['cfg.getEventAlgo("HelloAlg").MyInt = ConfigFlags.Input.JobNumber']
        flags = initConfigFlags()
        flags.lock()
        cfg = ComponentAccumulator()
        cfg.addEventAlgo(CompFactory.HelloAlg(MyInt=123))
        processPostExec(runArgs, flags, cfg)
        self.assertEqual(cfg.getEventAlgo('HelloAlg').MyInt, 1)
        cfg.wasMerged()

    def test_prePostExecCombined(self):
        runArgs = RunArguments()
        runArgs.preExec = ['ConfigFlags.Input.JobNumber = 5']
        runArgs.postExec = ['cfg.getEventAlgo("HelloAlg").MyInt = ConfigFlags.Input.JobNumber']
        flags = initConfigFlags()
        processPreExec(runArgs, flags)
        self.assertEqual(flags.Input.JobNumber, 5)
        flags.lock()
        cfg = ComponentAccumulator()
        cfg.addEventAlgo(CompFactory.HelloAlg(MyInt=123))
        processPostExec(runArgs, flags, cfg)
        self.assertEqual(cfg.getEventAlgo('HelloAlg').MyInt, 5)
        cfg.wasMerged()


if __name__ == '__main__':
    unittest.main()
