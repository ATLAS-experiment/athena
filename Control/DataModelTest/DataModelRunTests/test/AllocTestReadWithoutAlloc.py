#!/usr/bin/env athena.py
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.
#
# File: DataModelRunTests/test/AllocTestReadWithoutAlloc.py
# Author: snyder@bnl.gov
# Date: Nov 2023, from old config version of Nov 2022
# Purpose: Testing an xAOD object with a non-standard memory allocator.
#


from DataModelRunTests.DataModelTestConfig import \
    DataModelTestFlags, DataModelTestCfg, TestOutputCfg


def AllocTestReadWithoutAllocCfg (flags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    from AthenaConfiguration.ComponentFactory import CompFactory
    DMTest = CompFactory.DMTest

    acc.addEventAlgo (DMTest.AllocTestReadWithoutAlloc ('AllocTestReadWithoutAlloc'))

    return acc


flags = DataModelTestFlags (infile = 'xxx.root')
flags.fillFromArgs()
flags.lock()

import os
fname = os.path.splitext (flags.Input.Files[0])[0]

cfg = DataModelTestCfg (flags, 'AllocTestReadWithoutAlloc' + fname, loadWriteDicts = True)
cfg.merge (AllocTestReadWithoutAllocCfg (flags))

sc = cfg.run (flags.Exec.MaxEvents)
import sys
sys.exit (sc.isFailure())

